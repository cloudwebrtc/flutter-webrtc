import 'package:webrtc/WebRTC.dart' show WebRTC;
import 'package:webrtc/RTCDataChannel.dart';
import 'package:webrtc/RTCSessionDescrption.dart';
import 'package:webrtc/RTCIceCandidate.dart';
import 'package:webrtc/MediaStream.dart';
import 'package:webrtc/MediaStreamTrack.dart';
import 'package:webrtc/RTCStatsReport.dart';
import 'package:flutter/services.dart';
import 'dart:async';

enum RTCSignalingState {
  RTCSignalingStateStable,
  RTCSignalingStateHaveLocalOffer,
  RTCSignalingStateHaveRemoteOffer,
  RTCSignalingStateHaveLocalPranswer,
  RTCSignalingStateHaveRemotePranswer,
  RTCSignalingStateClosed
}

enum RTCIceGatheringState {
  RTCIceGatheringStateNew,
  RTCIceGatheringStateGathering,
  RTCIceGatheringStateComplete
}

enum RTCIceConnectionState {
  RTCIceConnectionStateNew,
  RTCIceConnectionStateChecking,
  RTCIceConnectionStateCompleted,
  RTCIceConnectionStateFailed,
  RTCIceConnectionStateDisconnected,
  RTCIceConnectionStateClosed
}

class Constraints {
  String key;
  String value;
  Constraints(String key, String value) {
    this.key = key;
    this.value = value;
  }
}

class MediaConstraints {
  List<Constraints> mandatorys;
  List<Constraints> constraints;
}

enum IceTransportsType { kNone, kRelay, kNoHost, kAll }

class IceServer {
  String uri;
  List<String> urls;
  String username;
  String password;
  String hostname;
}

class RTCConfiguration {
  List<IceServer> servers;
  IceTransportsType type;
  String toString() {
    return '';
  }
}

class RTCOfferAnswerOptions {
  bool offerToReceiveVideo;
  bool offerToReceiveAudio;
}

/**
 * 回调类型定义.
 */
typedef void SignalingStateCallback(RTCSignalingState state);
typedef void IceGatheringStateCallback(RTCIceGatheringState state);
typedef void IceConnectionStateCallback(RTCIceConnectionState state);
typedef void IceCandidateCallback(RTCIceCandidate candidate);
typedef void AddStreamCallback(MediaStream stream);
typedef void RemoveStreamCallback(MediaStream stream);
typedef void AddTrackCallback(MediaStream stream, MediaStreamTrack track);
typedef void RemoveTrackCallback(MediaStream stream, MediaStreamTrack track);

/*
 *  PeerConnection
 */
class RTCPeerConnection {
  // private:
  String _peerConnectionId;
  MethodChannel _channel = WebRTC.methodChannel();
  StreamSubscription<dynamic> _eventSubscription;
  List<MediaStream> localStreams;
  List<MediaStream> remoteStreams;
  RTCDataChannel dataChannel;

  // public: delegate
  SignalingStateCallback onSignalingState;
  IceGatheringStateCallback onIceGatheringState;
  IceConnectionStateCallback onIceConnectionState;
  IceCandidateCallback onIceCandidate;
  AddStreamCallback onAddStream;
  RemoveStreamCallback onRemoveStream;
  AddTrackCallback onAddTrack;
  RemoveTrackCallback onRemoveTrack;
  dynamic onRenegotiationNeeded;

  final Map<String, dynamic> DEFAULT_SDP_CONSTRAINTS = {
    "mandatory": {
      "OfferToReceiveAudio": true,
      "OfferToReceiveVideo": true,
    },
    "optional": [],
  };

  RTCPeerConnection(Map<String, dynamic> configuration) {
    initialize(configuration);
  }

  /*
     * PeerConnection 事件监听器
     */
  void eventListener(dynamic event) {
    final Map<dynamic, dynamic> map = event;
    switch (map['event']) {
      case 'signalingState':
        break;
      case 'iceGatheringState':
        break;
      case 'iceConnectionState':
        break;
      case 'onCandidate':
        break;
      case 'onAddStream':
        break;
      case 'onRemoveStream':
        break;
      case 'onAddTrack':
        break;
      case 'onRemoveTrack':
        break;
      case 'didOpenDataChannel':
        break;
      case 'dataChannelStateChanged':
        break;
      case 'dataChannelReceiveMessage':
        break;
      case 'onRenegotiationNeeded':
        break;
    }
  }

  void errorListener(Object obj) {
    final PlatformException e = obj;
  }

  void initialize(Map<String, dynamic> configuration) async {
    _channel = WebRTC.methodChannel();

    Map<String, dynamic> constraints = {
      "mandatory": {},
      "optional": [
        {"DtlsSrtpKeyAgreement": true},
      ],
    };

    final Map<dynamic, dynamic> response = await _channel.invokeMethod(
      'createPeerConnection',
      <String, dynamic>{
        'configuration': configuration,
        'constraints': constraints
      },
    );

    _peerConnectionId = response["peerConnectionId"];
    _eventSubscription = _eventChannelFor(_peerConnectionId)
        .receiveBroadcastStream()
        .listen(eventListener, onError: errorListener);
  }

  Future<Null> dispose() async {
    await _eventSubscription?.cancel();
    await _channel.invokeMethod(
      'dispose',
      <String, dynamic>{'peerConnectionId': _peerConnectionId},
    );
  }

  EventChannel _eventChannelFor(String peerConnectionId) {
    return new EventChannel(
        'cloudwebrtc.com/WebRTC/peerConnectoinEvent$peerConnectionId');
  }

  dynamic createOffer(Map<String, dynamic> constraints) async {
    try {
      final Map<dynamic, dynamic> response =
          await _channel.invokeMethod('createOffer', <String, dynamic>{
        'peerConnectionId': this._peerConnectionId,
        'constraints': constraints.length == 0? DEFAULT_SDP_CONSTRAINTS : constraints,
      });

      String sdp = response['sdp'];
      String type = response['type'];
      return new RTCSessionDescrption(sdp, type);
    } on PlatformException catch (e) {
      throw 'Unable to createOffer: ${e.message}';
    }
  }

  dynamic createAnswer(Map<String, dynamic> constraints) async {
    try {
      final Map<dynamic, dynamic> response =
          await _channel.invokeMethod('createAnswer', <String, dynamic>{
        'peerConnectionId': this._peerConnectionId,
        'constraints': constraints.length == 0? DEFAULT_SDP_CONSTRAINTS : constraints,
      });
      if (response['error']) {
        throw response['error'];
      }
      String sdp = response['sdp'];
      String type = response['type'];
      return new RTCSessionDescrption(sdp, type);
    } on PlatformException catch (e) {
      throw 'Unable to createAnswer: ${e.message}';
    }
  }

  void addStream(MediaStream stream) {
    _channel.invokeMethod('addStream', <String, dynamic>{
      'peerConnectionId': this._peerConnectionId,
      'streamId': stream.id,
    });
  }

  void removeStream(MediaStream stream) {
    _channel.invokeMethod('removeStream', <String, dynamic>{
      'peerConnectionId': this._peerConnectionId,
      'streamId': stream.id,
    });
  }

  void setLocalDescription(RTCSessionDescrption description) {
    try {
      _channel.invokeMethod('setLocalDescription', <String, dynamic>{
        'peerConnectionId': this._peerConnectionId,
        'description': description.toMap(),
      });
    } on PlatformException catch (e) {
      throw 'Unable to setLocalDescription: ${e.message}';
    }
  }

  void setRemoteDescription(RTCSessionDescrption description) {
    try {
      _channel.invokeMethod('setRemoteDescription', <String, dynamic>{
        'peerConnectionId': this._peerConnectionId,
        'description': description.toMap(),
      });
    } on PlatformException catch (e) {
      throw 'Unable to setRemoteDescription: ${e.message}';
    }
  }

 Future<StatsReport> getStats(MediaStreamTrack track) async {
   try {
      final Map<dynamic, dynamic> response =
          await _channel.invokeMethod('getStats', <String, dynamic>{
        'peerConnectionId': this._peerConnectionId,
        'track': track.id
      });
      Map<String,dynamic> stats = response["stats"];
      return new StatsReport(stats);
    } on PlatformException catch (e) {
      throw 'Unable to getStats: ${e.message}';
    }
 }

  List<MediaStream> getLocalStreams() {
    return localStreams;
  }

  List<MediaStream> getRemoteStreams() {
    return remoteStreams;
  }

  RTCDataChannel createDataChannel() {
    return dataChannel;
  }
}
