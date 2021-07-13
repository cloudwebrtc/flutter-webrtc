import 'dart:async';

import '../interface/factory.dart';
import '../interface/media_recorder.dart';
import '../interface/media_stream.dart';
import '../interface/navigator.dart';
import '../interface/rtc_peerconnection.dart';
import '../interface/rtc_video_renderer.dart';
import 'media_recorder_impl.dart';
import 'media_stream_impl.dart';
import 'navigator_impl.dart';
import 'rtc_peerconnection_impl.dart';
import 'rtc_video_renderer_impl.dart';
import 'utils.dart';

class RTCFactoryNative extends RTCFactory {
  RTCFactoryNative._internal();

  static final RTCFactory instance = RTCFactoryNative._internal();

  @override
  Future setFactoryConfiguration(Map<String, dynamic> configuration) async{
    var _channel = WebRTC.methodChannel();
    Map<String,dynamic> map = Map();
    map["configuration"] = configuration;
    await _channel.invokeMethod("setFactoryConfiguration",map);
  }

  @override
  Future<MediaStream> createLocalMediaStream(String label) async {
    var _channel = WebRTC.methodChannel();

    final response = await _channel
        .invokeMethod<Map<dynamic, dynamic>>('createLocalMediaStream');

    if (response == null) {
      throw Exception('createLocalMediaStream return null, something wrong');
    }

    return MediaStreamNative(response['streamId'], label);
  }

  @override
  Future<RTCPeerConnection> createPeerConnection(
      Map<String, dynamic> configuration,
      [Map<String, dynamic> constraints = const {}]) async {
    var defaultConstraints = <String, dynamic>{
      'mandatory': {},
      'optional': [
        {'DtlsSrtpKeyAgreement': true},
      ],
    };

    final response = await WebRTC.invokeMethod(
      'createPeerConnection',
      <String, dynamic>{
        'configuration': configuration,
        'constraints': constraints.isEmpty ? defaultConstraints : constraints
      },
    );

    String peerConnectionId = response['peerConnectionId'];
    return RTCPeerConnectionNative(peerConnectionId, configuration);
  }

  @override
  MediaRecorder mediaRecorder() {
    return MediaRecorderNative();
  }

  @override
  VideoRenderer videoRenderer() {
    return RTCVideoRendererNative();
  }

  @override
  Navigator get navigator => NavigatorNative();
}
