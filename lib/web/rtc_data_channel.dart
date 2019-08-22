import 'dart:async';
// ignore: uri_does_not_exist
import 'dart:html' as HTML;
// ignore: uri_does_not_exist
import 'dart:js' as JS;
import 'dart:typed_data';

enum MessageType {
  text, binary
}

final _typeStringToMessageType = <String, MessageType>{
  'text': MessageType.text,
  'binary': MessageType.binary
};

final _messageTypeToTypeString = <MessageType, String>{
  MessageType.text: 'text',
  MessageType.binary: 'binary'
};

class RTCDataChannelInit {
  bool ordered = true;
  int maxRetransmitTime = -1;
  int maxRetransmits = -1;
  String protocol = 'sctp'; //sctp | quic
  String binaryType = 'text'; // "binary" || text
  bool negotiated = false;
  int id = 0;
  Map<String, dynamic> toMap() {
    return {
      'ordered': ordered,
      'maxRetransmitTime': maxRetransmitTime,
      'maxRetransmits': maxRetransmits,
      'protocol': protocol,
      'negotiated': negotiated,
      'id': id
    };
  }
}

/// A class that represents a datachannel message.
/// Can either contain binary data as a [Uint8List] or
/// text data as a [String].
class RTCDataChannelMessage {
  dynamic _data;
  bool _isBinary;

  /// Construct a text message with a [String].
  RTCDataChannelMessage(String text) {
    this._data = text;
    this._isBinary = false;
  }

  /// Construct a binary message with a [Uint8List].
  RTCDataChannelMessage.fromBinary(Uint8List binary) {
    this._data = binary;
    this._isBinary = true;
  }

  /// Tells whether this message contains binary.
  /// If this is false, it's a text message.
  bool get isBinary => _isBinary;

  MessageType get type => isBinary ? MessageType.binary : MessageType.text;

  /// Text contents of this message as [String].
  /// Use only on text messages.
  /// See: [isBinary].
  String get text => _data;

  /// Binary contents of this message as [Uint8List].
  /// Use only on binary messages.
  /// See: [isBinary].
  Uint8List get binary => _data;
}

enum RTCDataChannelState {
  RTCDataChannelConnecting,
  RTCDataChannelOpen,
  RTCDataChannelClosing,
  RTCDataChannelClosed,
}

typedef void RTCDataChannelStateCallback(RTCDataChannelState state);
typedef void RTCDataChannelOnMessageCallback(RTCDataChannelMessage data);

class RTCDataChannel {
  final HTML.RtcDataChannel _jsDc;
  RTCDataChannelStateCallback onDataChannelState;
  RTCDataChannelOnMessageCallback onMessage;
  RTCDataChannelState _state = RTCDataChannelState.RTCDataChannelConnecting;

  /// Get current state.
  RTCDataChannelState get state => _state;

  final _stateChangeController = StreamController<RTCDataChannelState>.broadcast(sync: true);
  final _messageController = StreamController<RTCDataChannelMessage>.broadcast(sync: true);

  /// Stream of state change events. Emits the new state on change.
  /// Closes when the [RTCDataChannel] is closed.
  Stream<RTCDataChannelState> stateChangeStream;

  /// Stream of incoming messages. Emits the message.
  /// Closes when the [RTCDataChannel] is closed.
  Stream<RTCDataChannelMessage> messageStream;

  RTCDataChannel(this._jsDc) {
    stateChangeStream = _stateChangeController.stream;
    messageStream = _messageController.stream;
    _jsDc.onClose.listen((_) {
      _state = RTCDataChannelState.RTCDataChannelClosed;
      _stateChangeController.add(_state);
      if (onDataChannelState != null) {
        onDataChannelState(_state);
      }
    });
    _jsDc.onOpen.listen((_) {
      _state = RTCDataChannelState.RTCDataChannelOpen;
      _stateChangeController.add(_state);
      if (onDataChannelState != null) {
        onDataChannelState(_state);
      }
    });
    _jsDc.onMessage.listen((event) async {
      RTCDataChannelMessage msg = await _parse(event.data);
      _messageController.add(msg);
      if (onMessage != null) {
        onMessage(msg);
      }
    });
  }

  Future<RTCDataChannelMessage> _parse(dynamic data) async {
    if (data is String)
      return RTCDataChannelMessage(data);
    dynamic arrayBuffer;
    if (data is HTML.Blob) {
      // This should never happen actually
      final promise = JS.JsObject.fromBrowserObject(data).callMethod('arrayBuffer');
      arrayBuffer = await HTML.promiseToFuture(promise);
    } else {
      arrayBuffer = data;
    }
    print("Object got from DataChannel ${arrayBuffer} with type ${arrayBuffer.runtimeType}");
    //TODO: convert ArrayBuffer to Uint8Array
    //https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/ArrayBuffer
    throw UnimplementedError();
  }

  Future<void> send(RTCDataChannelMessage message) {
    if (!message.isBinary) {
      _jsDc.send(message.text);
    } else {
      // This may just work
      _jsDc.send(message.binary);
      // If not, convert to ArrayBuffer/Blob
    }
    return Future.value();
  }

  Future<void> close() {
    _jsDc.close();
    return Future.value();
  }

}
