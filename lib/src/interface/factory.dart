import 'media_recorder.dart';
import 'media_stream.dart';
import 'navigator.dart';
import 'rtc_peerconnection.dart';
import 'rtc_video_renderer.dart';

abstract class RTCFactory {

  Future setFactoryConfiguration(Map<String, dynamic> configuration);

  Future<RTCPeerConnection> createPeerConnection(
      Map<String, dynamic> configuration,
      [Map<String, dynamic> constraints]);

  Future<MediaStream> createLocalMediaStream(String label);

  Navigator get navigator;

  MediaRecorder mediaRecorder();

  VideoRenderer videoRenderer();
}
