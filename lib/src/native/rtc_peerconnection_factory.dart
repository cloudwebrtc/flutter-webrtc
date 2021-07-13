import 'dart:async';

import '../interface/media_recorder.dart';
import '../interface/media_stream.dart';
import '../interface/navigator.dart';
import '../interface/rtc_peerconnection.dart';
import '../interface/rtc_video_renderer.dart';
import 'factory_impl.dart';

Future setFactoryConfiguration(Map<String, dynamic> configuration) async {
  await RTCFactoryNative.instance.setFactoryConfiguration(configuration);
}

Future<RTCPeerConnection> createPeerConnection(
    Map<String, dynamic> configuration,
    [Map<String, dynamic> constraints = const {}]) async {
  return RTCFactoryNative.instance
      .createPeerConnection(configuration, constraints);
}

Future<MediaStream> createLocalMediaStream(String label) async {
  return RTCFactoryNative.instance.createLocalMediaStream(label);
}

MediaRecorder mediaRecorder() {
  return RTCFactoryNative.instance.mediaRecorder();
}

VideoRenderer videoRenderer() {
  return RTCFactoryNative.instance.videoRenderer();
}

Navigator get navigator => RTCFactoryNative.instance.navigator;
