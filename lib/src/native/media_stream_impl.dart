import 'dart:async';

import '../interface/media_stream.dart';
import '../interface/media_stream_track.dart';
import 'media_stream_track_impl.dart';
import 'utils.dart';

class MediaStreamNative extends MediaStream {
  MediaStreamNative(String streamId, String ownerTag)
      : super(streamId, ownerTag);

  factory MediaStreamNative.fromMap(Map<dynamic, dynamic> map) {
    return MediaStreamNative(map['streamId'], map['ownerTag'])
      ..setMediaTracks(map['audioTracks'], map['videoTracks']);
  }

  final _channel = WebRTC.methodChannel();

  final _audioTracks = <MediaStreamTrack>[];
  final _videoTracks = <MediaStreamTrack>[];

  void setMediaTracks(List<dynamic>? audioTracks, List<dynamic>? videoTracks) {
    _audioTracks.clear();
    audioTracks?.forEach((track) {
      _audioTracks.add(MediaStreamTrackNative(
          track['id'], track['label'], track['kind'], track['enabled']));
    });

    _videoTracks.clear();
    videoTracks?.forEach((track) {
      _videoTracks.add(MediaStreamTrackNative(
          track['id'], track['label'], track['kind'], track['enabled']));
    });
  }

  @override
  List<MediaStreamTrack> getTracks() {
    return <MediaStreamTrack>[..._audioTracks, ..._videoTracks];
  }

  @override
  Future<void> getMediaTracks() async {
    final response = await WebRTC.invokeMethod(
      'mediaStreamGetTracks',
      <String, dynamic>{'streamId': id},
    );

    setMediaTracks(response['audioTracks'], response['videoTracks']);
  }

  @override
  Future<void> addTrack(MediaStreamTrack track,
      {bool addToNative = true}) async {
    if (track.kind == 'audio') {
      _audioTracks.add(track);
    } else {
      _videoTracks.add(track);
    }

    if (addToNative) {
      await _channel.invokeMethod('mediaStreamAddTrack',
          <String, dynamic>{'streamId': id, 'trackId': track.id});
    }
  }

  @override
  Future<void> removeTrack(MediaStreamTrack track,
      {bool removeFromNative = true}) async {
    if (track.kind == 'audio') {
      _audioTracks.removeWhere((it) => it.id == track.id);
    } else {
      _videoTracks.removeWhere((it) => it.id == track.id);
    }

    if (removeFromNative) {
      await _channel.invokeMethod('mediaStreamRemoveTrack',
          <String, dynamic>{'streamId': id, 'trackId': track.id});
    }
  }

  @override
  List<MediaStreamTrack> getAudioTracks() {
    return _audioTracks;
  }

  @override
  List<MediaStreamTrack> getVideoTracks() {
    return _videoTracks;
  }

  @override
  Future<Null> dispose() async {
    await _channel.invokeMethod(
      'streamDispose',
      <String, dynamic>{'streamId': id},
    );
  }

  @override
  // TODO(cloudwebrtc): Implement
  bool get active => throw UnimplementedError();

  @override
  MediaStream clone() {
    // TODO(cloudwebrtc): Implement
    throw UnimplementedError();
  }
}
