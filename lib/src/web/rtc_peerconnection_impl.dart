import 'dart:async';
import 'dart:html' as html;
import 'dart:js' as js;
import 'dart:js_util' as jsutil;

import 'package:flutter_webrtc/src/interface/rtc_track_event.dart';
import 'package:flutter_webrtc/src/web/rtc_rtp_transceiver_impl.dart';

import '../interface/enums.dart';
import '../interface/media_stream.dart';
import '../interface/media_stream_track.dart';
import '../interface/rtc_data_channel.dart';
import '../interface/rtc_dtmf_sender.dart';
import '../interface/rtc_ice_candidate.dart';
import '../interface/rtc_peerconnection.dart';
import '../interface/rtc_rtp_receiver.dart';
import '../interface/rtc_rtp_sender.dart';
import '../interface/rtc_rtp_transceiver.dart';
import '../interface/rtc_session_description.dart';
import '../interface/rtc_stats_report.dart';
import 'media_stream_impl.dart';
import 'media_stream_track_impl.dart';
import 'rtc_data_channel_impl.dart';
import 'rtc_dtmf_sender_impl.dart';
import 'rtc_rtp_receiver_impl.dart';
import 'rtc_rtp_sender_impl.dart';

/*
 *  PeerConnection
 */
class RTCPeerConnectionWeb extends RTCPeerConnection {
  RTCPeerConnectionWeb(this._peerConnectionId, this._jsPc) {
    _jsPc.onAddStream.listen((mediaStreamEvent) {
      final jsStream = mediaStreamEvent.stream;
      if (jsStream == null) {
        throw Exception('Unable to get the stream from the event');
      }
      if (jsStream.id == null) {
        throw Exception('The stream must have a valid identifier');
      }

      final _remoteStream = _remoteStreams.putIfAbsent(
          jsStream.id!, () => MediaStreamWeb(jsStream, _peerConnectionId));

      onAddStream?.call(_remoteStream);

      jsStream.onAddTrack.listen((mediaStreamTrackEvent) {
        final jsTrack =
            (mediaStreamTrackEvent as html.MediaStreamTrackEvent).track;
        if (jsTrack == null) {
          throw Exception('The Media Stream track is null');
        }
        final track = MediaStreamTrackWeb(jsTrack);
        _remoteStream.addTrack(track, addToNative: false).then((_) {
          onAddTrack?.call(_remoteStream, track);
        });
      });

      jsStream.onRemoveTrack.listen((mediaStreamTrackEvent) {
        final jsTrack =
            (mediaStreamTrackEvent as html.MediaStreamTrackEvent).track;
        if (jsTrack == null) {
          throw Exception('The Media Stream track is null');
        }
        final track = MediaStreamTrackWeb(jsTrack);
        _remoteStream.removeTrack(track, removeFromNative: false).then((_) {
          onRemoveTrack?.call(_remoteStream, track);
        });
      });
    });

    _jsPc.onDataChannel.listen((dataChannelEvent) {
      if (dataChannelEvent.channel != null) {
        onDataChannel?.call(RTCDataChannelWeb(dataChannelEvent.channel!));
      }
    });

    _jsPc.onIceCandidate.listen((iceEvent) {
      if (iceEvent.candidate != null) {
        onIceCandidate?.call(_iceFromJs(iceEvent.candidate!));
      }
    });

    _jsPc.onIceConnectionStateChange.listen((_) {
      _iceConnectionState =
          iceConnectionStateForString(_jsPc.iceConnectionState);
      onIceConnectionState?.call(_iceConnectionState!);
    });

    jsutil.setProperty(_jsPc, 'onicegatheringstatechange', js.allowInterop((_) {
      _iceGatheringState = iceGatheringStateforString(_jsPc.iceGatheringState);
      onIceGatheringState?.call(_iceGatheringState!);
    }));

    _jsPc.onRemoveStream.listen((mediaStreamEvent) {
      if (mediaStreamEvent.stream?.id != null) {
        final _remoteStream =
        _remoteStreams.remove(mediaStreamEvent.stream!.id);
        if (_remoteStream != null) {
          onRemoveStream?.call(_remoteStream);
        }
      }
    });

    _jsPc.onSignalingStateChange.listen((_) {
      _signalingState = signalingStateForString(_jsPc.signalingState);
      onSignalingState?.call(_signalingState!);
    });

    _jsPc.onIceConnectionStateChange.listen((_) {
      _connectionState = peerConnectionStateForString(_jsPc.iceConnectionState);
      onConnectionState?.call(_connectionState!);
    });

    _jsPc.onNegotiationNeeded.listen((_) {
      onRenegotiationNeeded?.call();
    });

    _jsPc.onTrack.listen((trackEvent) {
      if (trackEvent.track != null && trackEvent.receiver != null) {
        onTrack?.call(
          RTCTrackEvent(
            track: MediaStreamTrackWeb(trackEvent.track!),
            receiver: RTCRtpReceiverWeb(trackEvent.receiver!),
            transceiver: RTCRtpTransceiverWeb.fromJsObject(
                jsutil.getProperty(trackEvent, 'transceiver')),
            streams: (trackEvent.streams != null)
                ? trackEvent.streams!
                .map((e) => MediaStreamWeb(e, _peerConnectionId))
                .toList()
                : [],
          ),
        );
      }
    });
  }

  final String _peerConnectionId;
  late final html.RtcPeerConnection _jsPc;
  final _localStreams = <String, MediaStream>{};
  final _remoteStreams = <String, MediaStream>{};
  final _configuration = <String, dynamic>{};

  RTCSignalingState? _signalingState;
  RTCIceGatheringState? _iceGatheringState;
  RTCIceConnectionState? _iceConnectionState;
  RTCPeerConnectionState? _connectionState;

  @override
  RTCSignalingState? get signalingState => _signalingState;

  @override
  RTCIceGatheringState? get iceGatheringState => _iceGatheringState;

  @override
  RTCIceConnectionState? get iceConnectionState => _iceConnectionState;

  @override
  RTCPeerConnectionState? get connectionState => _connectionState;

  @override
  Future<void> dispose() {
    _jsPc.close();
    return Future.value();
  }

  @override
  Map<String, dynamic> get getConfiguration => _configuration;

  @override
  Future<void> setConfiguration(Map<String, dynamic> configuration) {
    _configuration.addAll(configuration);

    _jsPc.setConfiguration(configuration);
    return Future.value();
  }

  @override
  Future<RTCSessionDescription> createOffer([Map<String, dynamic>? constraints]) async {
    final offer = await _jsPc.createOffer(constraints);
    return _sessionFromJs(offer);
  }

  @override
  Future<RTCSessionDescription> createAnswer([Map<String, dynamic>? constraints]) async {
    final answer = await _jsPc.createAnswer(constraints);
    return _sessionFromJs(answer);
  }

  @override
  Future<void> addStream(MediaStream stream) {
    var _native = stream as MediaStreamWeb;
    _localStreams.putIfAbsent(
        stream.id, () => MediaStreamWeb(_native.jsStream, _peerConnectionId));
    _jsPc.addStream(_native.jsStream);
    return Future.value();
  }

  @override
  Future<void> removeStream(MediaStream stream) async {
    var _native = stream as MediaStreamWeb;
    _localStreams.remove(stream.id);
    _jsPc.removeStream(_native.jsStream);
    return Future.value();
  }

  @override
  Future<void> setLocalDescription(RTCSessionDescription description) async {
    await _jsPc.setLocalDescription(description.toMap());
  }

  @override
  Future<void> setRemoteDescription(RTCSessionDescription description) async {
    await _jsPc.setRemoteDescription(description.toMap());
  }

  @override
  Future<RTCSessionDescription?> getLocalDescription() async {
    if (null == _jsPc.localDescription) {
      return null;
    }
    return _sessionFromJs(_jsPc.localDescription);
  }

  @override
  Future<RTCSessionDescription?> getRemoteDescription() async {
    if (null == _jsPc.remoteDescription) {
      return null;
    }
    return _sessionFromJs(_jsPc.remoteDescription);
  }

  @override
  Future<void> addCandidate(RTCIceCandidate candidate)  {
    try {
      var ret = jsutil.callMethod(_jsPc, 'addIceCandidate', [_iceToJs(candidate)]);
      return jsutil.promiseToFuture(ret);
    } catch (e) {
      return Future.error(e);
    }
  }

  @override
  Future<List<StatsReport>> getStats([MediaStreamTrack? track]) async {
    var stats;
    if (track != null) {
      var jsTrack = (track as MediaStreamTrackWeb).jsTrack;
      stats = await jsutil.promiseToFuture<dynamic>(
          jsutil.callMethod(_jsPc, 'getStats', [jsTrack]));
    } else {
      stats = await _jsPc.getStats();
    }

    var report = <StatsReport>[];
    stats.forEach((key, value) {
      report.add(
          StatsReport(value['id'], value['type'], value['timestamp'], value));
    });
    return report;
  }

  @override
  List<MediaStream> getLocalStreams() =>
      _jsPc.getLocalStreams().map((e) => _localStreams[e.id]!).toList();

  @override
  List<MediaStream> getRemoteStreams() =>
      _jsPc
          .getRemoteStreams()
          .map((jsStream) => _remoteStreams[jsStream.id]!)
          .toList();

  @override
  Future<RTCDataChannel> createDataChannel(String label, RTCDataChannelInit dataChannelDict) {
    final map = dataChannelDict.toMap();
    if (dataChannelDict.binaryType == 'binary') {
      map['binaryType'] = 'arraybuffer'; // Avoid Blob in data channel
    }

    final jsDc = _jsPc.createDataChannel(label, map);
    return Future.value(RTCDataChannelWeb(jsDc));
  }

  @override
  Future<void> close() async {
    _jsPc.close();
    return Future.value();
  }

  @override
  RTCDTMFSender createDtmfSender(MediaStreamTrack track) {
    var _native = track as MediaStreamTrackWeb;
    var jsDtmfSender = _jsPc.createDtmfSender(_native.jsTrack);
    return RTCDTMFSenderWeb(jsDtmfSender);
  }

  //
  // utility section
  //

  RTCIceCandidate _iceFromJs(html.RtcIceCandidate candidate) =>
      RTCIceCandidate(
        candidate.candidate,
        candidate.sdpMid,
        candidate.sdpMLineIndex,
      );

  html.RtcIceCandidate _iceToJs(RTCIceCandidate c) =>
      html.RtcIceCandidate(c.toMap());

  RTCSessionDescription _sessionFromJs(html.RtcSessionDescription? sd) =>
      RTCSessionDescription(sd?.sdp, sd?.type);

  @override
  Future<RTCRtpSender> addTrack(MediaStreamTrack track,
      [MediaStream? stream]) async {
    var jStream = (stream as MediaStreamWeb).jsStream;
    var jsTrack = (track as MediaStreamTrackWeb).jsTrack;
    var sender = _jsPc.addTrack(jsTrack, jStream);
    return RTCRtpSenderWeb.fromJsSender(sender);
  }

  @override
  Future<bool> removeTrack(RTCRtpSender sender) async {
    var nativeSender = sender as RTCRtpSenderWeb;
    // var nativeTrack = nativeSender.track as MediaStreamTrackWeb;
    jsutil.callMethod(_jsPc, 'removeTrack', [nativeSender.jsRtpSender]);
    return Future<bool>.value(true);
  }

  @override
  Future<List<RTCRtpSender>> getSenders() async {
    var senders = jsutil.callMethod(_jsPc, 'getSenders', []);
    var list = <RTCRtpSender>[];
    senders.forEach((e) {
      list.add(RTCRtpSenderWeb.fromJsSender(e));
    });
    return list;
  }

  @override
  Future<List<RTCRtpReceiver>> getReceivers() async {
    var receivers = jsutil.callMethod(_jsPc, 'getReceivers', []);

    var list = <RTCRtpReceiver>[];
    receivers.forEach((e) {
      list.add(RTCRtpReceiverWeb(e));
    });

    return list;
  }

  @override
  Future<List<RTCRtpTransceiver>> getTransceivers() async {
    var transceivers = jsutil.callMethod(_jsPc, 'getTransceivers', []);

    var list = <RTCRtpTransceiver>[];
    transceivers.forEach((e) {
      list.add(RTCRtpTransceiverWeb.fromJsObject(e));
    });

    return list;
  }

  //'audio|video', { 'direction': 'recvonly|sendonly|sendrecv' }
  @override
  Future<RTCRtpTransceiver> addTransceiver({MediaStreamTrack? track,
    RTCRtpMediaType? kind,
    RTCRtpTransceiverInit? init}) async {
    var kindLabel = kind != null ? typeRTCRtpMediaTypetoString[kind] : null;
    var kindOrTrack = kindLabel ?? (track as MediaStreamTrackWeb).jsTrack;
    final jsOptions = jsutil
        .jsify(init != null ? RTCRtpTransceiverInitWeb.initToMap(init) : {});
    var transceiver =
    jsutil.callMethod(_jsPc, 'addTransceiver', [kindOrTrack, jsOptions]);
    return RTCRtpTransceiverWeb.fromJsObject(transceiver,
        peerConnectionId: _peerConnectionId);
  }
}
