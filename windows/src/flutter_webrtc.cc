#include "flutter_webrtc.h"

#include "flutter_webrtc/flutter_web_r_t_c_plugin.h"

namespace flutter_webrtc_plugin {

FlutterWebRTC::FlutterWebRTC(FlutterWebRTCPlugin* plugin)
    : FlutterWebRTCBase::FlutterWebRTCBase(plugin->messenger(),
                                           plugin->textures()),
      FlutterVideoRendererManager::FlutterVideoRendererManager(this),
      FlutterMediaStream::FlutterMediaStream(this),
      FlutterPeerConnection::FlutterPeerConnection(this),
      FlutterDataChannel::FlutterDataChannel(this) {}

FlutterWebRTC::~FlutterWebRTC() {}

void FlutterWebRTC::HandleMethodCall(
    const flutter::MethodCall<EncodableValue>& method_call,
    std::unique_ptr<flutter::MethodResult<EncodableValue>> result) {
  if (method_call.method_name().compare("createPeerConnection") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const EncodableMap configuration = findMap(params, "configuration");
    const EncodableMap constraints = findMap(params, "constraints");
    CreateRTCPeerConnection(configuration, constraints, std::move(result));
  } else if (method_call.method_name().compare("getUserMedia") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const EncodableMap constraints = findMap(params, "constraints");
    GetUserMedia(constraints, std::move(result));
  } else if (method_call.method_name().compare("getDisplayMedia") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const EncodableMap constraints = findMap(params, "constraints");
    result->NotImplemented();
  } else if (method_call.method_name().compare("getSources") == 0) {
    GetSources(std::move(result));
  } else if (method_call.method_name().compare("mediaStreamGetTracks") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string streamId = findString(params, "streamId");
    MediaStreamGetTracks(streamId, std::move(result));
  } else if (method_call.method_name().compare("createOffer") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "constraints");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createOfferFailed",
                    "createOffer() peerConnection is null");
      return;
    }
    CreateOffer(constraints, pc, std::move(result));
  } else if (method_call.method_name().compare("createAnswer") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "constraints");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createAnswerFailed",
                    "createAnswer() peerConnection is null");
      return;
    }
    CreateAnswer(constraints, pc, std::move(result));
  } else if (method_call.method_name().compare("addStream") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string streamId = findString(params, "streamId");
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (!stream) {
      result->Error("addStreamFailed", "addStream() stream not found!");
      return;
    }
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("addStreamFailed", "addStream() peerConnection is null");
      return;
    }
    pc->AddStream(stream);
    result->Success(nullptr);
  } else if (method_call.method_name().compare("removeStream") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string streamId = findString(params, "streamId");
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (!stream) {
      result->Error("removeStreamFailed", "removeStream() stream not found!");
      return;
    }
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("removeStreamFailed",
                    "removeStream() peerConnection is null");
      return;
    }
    pc->RemoveStream(stream);
    result->Success(nullptr);
  } else if (method_call.method_name().compare("setLocalDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "description");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("setLocalDescriptionFailed",
                    "setLocalDescription() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCSessionDescription> description =
        RTCSessionDescription::Create(findString(constraints, "type").c_str(),
                                    findString(constraints, "sdp").c_str(),
                                    &error);

    SetLocalDescription(description.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("setRemoteDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "description");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("setRemoteDescriptionFailed",
                    "setRemoteDescription() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCSessionDescription> description =
        RTCSessionDescription::Create(findString(constraints, "type").c_str(),
                                    findString(constraints, "sdp").c_str(),
                                    &error);

    SetRemoteDescription(description.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("addCandidate") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "candidate");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("addCandidateFailed",
                    "addCandidate() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCIceCandidate> rtc_candidate =
        RTCIceCandidate::Create(findString(constraints, "candidate").c_str(),
                              findString(constraints, "sdpMid").c_str(),
                              findInt(constraints, "sdpMLineIndex"), &error);

    AddIceCandidate(rtc_candidate.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("getStats") == 0) {
  } else if (method_call.method_name().compare("createDataChannel") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createDataChannelFailed",
                    "createDataChannel() peerConnection is null");
      return;
    }

    const std::string label = findString(params, "label");
    const EncodableMap dataChannelDict = findMap(params, "dataChannelDict");

    CreateDataChannel(label, dataChannelDict, pc, std::move(result));
  } else if (method_call.method_name().compare("dataChannelSend") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("dataChannelSendFailed",
                    "dataChannelSend() peerConnection is null");
      return;
    }

    int dataChannelId = findInt(params, "dataChannelId");
    const std::string type = findString(params, "type");
    const EncodableValue data = findEncodableValue(params, "data");
    RTCDataChannel* data_channel = DataChannelFormId(dataChannelId);
    if (data_channel == nullptr) {
      result->Error("dataChannelSendFailed",
                    "dataChannelSend() data_channel is null");
      return;
    }
    DataChannelSend(data_channel, type, data, std::move(result));
  } else if (method_call.method_name().compare("dataChannelClose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("dataChannelCloseFailed",
                    "dataChannelClose() peerConnection is null");
      return;
    }

    int dataChannelId = findInt(params, "dataChannelId");
    RTCDataChannel* data_channel = DataChannelFormId(dataChannelId);
    if (data_channel == nullptr) {
      result->Error("dataChannelCloseFailed",
                    "dataChannelClose() data_channel is null");
      return;
    }
    DataChannelClose(data_channel, std::move(result));
  } else if (method_call.method_name().compare("streamDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string stream_id = findString(params, "streamId");
    MediaStreamDispose(stream_id, std::move(result));
  } else if (method_call.method_name().compare("mediaStreamTrackSetEnable") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string track_id = findString(params, "trackId");
    MediaStreamTrackSetEnable(track_id, std::move(result));
  } else if (method_call.method_name().compare("trackDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string track_id = findString(params, "trackId");
    MediaStreamTrackDispose(track_id, std::move(result));
  } else if (method_call.method_name().compare("peerConnectionClose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("peerConnectionCloseFailed",
                    "peerConnectionClose() peerConnection is null");
      return;
    }
    RTCPeerConnectionClose(pc, peerConnectionId, std::move(result));
  } else if (method_call.method_name().compare("createVideoRenderer") == 0) {
    CreateVideoRendererTexture(std::move(result));
  } else if (method_call.method_name().compare("videoRendererDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    int64_t texture_id = findLongInt(params, "textureId");
    VideoRendererDispose(texture_id, std::move(result));
  } else if (method_call.method_name().compare("videoRendererSetSrcObject") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string stream_id = findString(params, "streamId");
    int64_t texture_id = findLongInt(params, "textureId");

    SetMediaStream(texture_id, stream_id);
    result->Success(nullptr);
  } else if (method_call.method_name().compare(
                 "mediaStreamTrackSwitchCamera") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string track_id = findString(params, "trackId");
    MediaStreamTrackSwitchCamera(track_id, std::move(result));
  } else if (method_call.method_name().compare("setVolume") == 0) {
  } else if (method_call.method_name().compare("getLocalDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "description");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("GetLocalDescription",
                    "GetLocalDescription() peerConnection is null");
      return;
    }

    GetLocalDescription(pc, std::move(result));
  } else if (method_call.method_name().compare("getRemoteDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap constraints = findMap(params, "description");
    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("GetRemoteDescription",
                    "GetRemoteDescription() peerConnection is null");
      return;
    }

    GetRemoteDescription(pc, std::move(result));
  } else if (method_call.method_name().compare("mediaStreamAddTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());

    const std::string streamId = findString(params, "streamId");
    const std::string trackId = findString(params, "trackId");

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (stream == nullptr) {
      result->Error("MediaStreamAddTrack",
                    "MediaStreamAddTrack() stream is null");
      return;
    }

    scoped_refptr<RTCMediaTrack> track = MediaTracksForId(trackId);
    if (track == nullptr) {
      result->Error("MediaStreamAddTrack",
                    "MediaStreamAddTrack() track is null");
      return;
    }

    MediaStreamAddTrack(stream, track, std::move(result));
    std::string kind = to_std_string(track->kind());
    for (int i = 0; i < renders_.size(); i++) {
      FlutterVideoRenderer* renderer = renders_.at(i).get();
      if (renderer->CheckMediaStream(streamId) && 0 == kind.compare("video")) {
          renderer->SetVideoTrack(static_cast<RTCVideoTrack*>(track.get()));
      }
    };
  } else if (method_call.method_name().compare("mediaStreamRemoveTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());

    const std::string streamId = findString(params, "streamId");
    const std::string trackId = findString(params, "trackId");

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (stream == nullptr) {
      result->Error("MediaStreamRemoveTrack",
                    "MediaStreamRemoveTrack() stream is null");
      return;
    }

    scoped_refptr<RTCMediaTrack> track = MediaTracksForId(trackId);
    if (track == nullptr) {
      result->Error("MediaStreamRemoveTrack",
                    "MediaStreamRemoveTrack() track is null");
      return;
    }

    MediaStreamRemoveTrack(stream, track, std::move(result));

    for (int i = 0; i < renders_.size(); i++) {
      FlutterVideoRenderer* renderer = renders_.at(i).get();
      if (renderer->CheckVideoTrack(streamId)) {
        renderer->SetVideoTrack(nullptr);
      }
    };
  } else if (method_call.method_name().compare("addTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());

    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const std::string trackId = findString(params, "trackId");
    const EncodableList streamIds = findList(params, "streamIds");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("AddTrack", "AddTrack() peerConnection is null");
      return;
    }

    scoped_refptr<RTCMediaTrack> track = MediaTracksForId(trackId);
    if (track == nullptr) {
      result->Error("AddTrack", "AddTrack() track is null");
      return;
    };
    std::list<std::string> listId;
    for (EncodableValue value : streamIds) {
      listId.push_back(GetValue<std::string>(value));
    }

    AddTrack(pc, track, listId, std::move(result));

  } else if (method_call.method_name().compare("removeTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());

    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const std::string senderId = findString(params, "senderId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("removeTrack", "removeTrack() peerConnection is null");
      return;
    }

    RemoveTrack(pc, senderId, std::move(result));

  } else if (method_call.method_name().compare("addTransceiver") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");
    const EncodableMap transceiverInit = findMap(params, "transceiverInit");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("addTransceiver",
                    "addTransceiver() peerConnection is null");
      return;
    }

    const std::string trackId = findString(params, "trackId");
    RTCMediaTrack* track = MediaTrackForId(trackId);
    if (nullptr == track) {
      result->Error("AddTransceiver",
                    "AddTransceiver() peerCtrackonnection is null");
      return;
    }

    AddTransceiver(pc, track, transceiverInit, std::move(result));
  } else if (method_call.method_name().compare("getTransceivers") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("getTransceivers",
                    "getTransceivers() peerConnection is null");
      return;
    }

    GetTransceivers(pc, std::move(result));
  } else if (method_call.method_name().compare("getReceivers") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("getReceivers", "getReceivers() peerConnection is null");
      return;
    }

    GetReceivers(pc, std::move(result));

  } else if (method_call.method_name().compare("getSenders") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("getSenders", "getSenders() peerConnection is null");
      return;
    }

    GetSenders(pc, std::move(result));
  } else if (method_call.method_name().compare("rtpSenderDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpSenderDispose",
                    "rtpSenderDispose() peerConnection is null");
      return;
    }

    const std::string rtpSenderId = findString(params, "rtpSenderId");
    if (0 < rtpSenderId.size()) {
      if (pc == nullptr) {
        result->Error("rtpSenderDispose",
                      "rtpSenderDispose() rtpSenderId is null or empty");
        return;
      }
    }
    RtpSenderDispose(pc, rtpSenderId, std::move(result));

  } else if (method_call.method_name().compare("rtpSenderSetTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpSenderSetTrack",
                    "rtpSenderSetTrack() peerConnection is null");
      return;
    }

    const std::string trackId = findString(params, "trackId");
    RTCMediaTrack* track = MediaTrackForId(trackId);
    if (nullptr == track) {
      result->Error("rtpSenderSetTrack", "rtpSenderSetTrack() track is null");
      return;
    }

    const std::string rtpSenderId = findString(params, "rtpSenderId");
    if (0 < rtpSenderId.size()) {
      if (pc == nullptr) {
        result->Error("rtpSenderSetTrack",
                      "rtpSenderSetTrack() rtpSenderId is null or empty");
        return;
      }
    }
    RtpSenderSetTrack(pc, track, rtpSenderId, std::move(result));
  } else if (method_call.method_name().compare("rtpSenderReplaceTrack") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpSenderReplaceTrack",
                    "rtpSenderReplaceTrack() peerConnection is null");
      return;
    }

    const std::string trackId = findString(params, "trackId");
    RTCMediaTrack* track = MediaTrackForId(trackId);
    if (nullptr == track) {
      result->Error("rtpSenderReplaceTrack",
                    "rtpSenderReplaceTrack() track is null");
      return;
    }

    const std::string rtpSenderId = findString(params, "rtpSenderId");
    if (0 < rtpSenderId.size()) {
      if (pc == nullptr) {
        result->Error("rtpSenderReplaceTrack",
                      "rtpSenderReplaceTrack() rtpSenderId is null or empty");
        return;
      }
    }
    RtpSenderReplaceTrack(pc, track, rtpSenderId, std::move(result));
  } else if (method_call.method_name().compare("rtpSenderSetParameters") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpSenderSetParameters",
                    "rtpSenderSetParameters() peerConnection is null");
      return;
    }

    const std::string rtpSenderId = findString(params, "rtpSenderId");
    if (0 < rtpSenderId.size()) {
      if (pc == nullptr) {
        result->Error("rtpSenderSetParameters",
                      "rtpSenderSetParameters() rtpSenderId is null or empty");
        return;
      }
    }

    const EncodableMap parameters = findMap(params, "parameters");
    if (0 < parameters.size()) {
      result->Error("rtpSenderSetParameters",
                    "rtpSenderSetParameters() parameters is null or empty");
      return;
    }

    RtpSenderSetParameters(pc, rtpSenderId, parameters, std::move(result));
  } else if (method_call.method_name().compare("rtpTransceiverStop") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpTransceiverStop",
                    "rtpTransceiverStop() peerConnection is null");
      return;
    }

    const std::string rtpTransceiverId = findString(params, "rtpTransceiverId");
    if (0 < rtpTransceiverId.size()) {
      if (pc == nullptr) {
        result->Error("rtpTransceiverStop",
                      "rtpTransceiverStop() rtpTransceiverId is null or empty");
        return;
      }
    }

    RtpTransceiverStop(pc, rtpTransceiverId, std::move(result));
  } else if (method_call.method_name().compare(
                 "rtpTransceiverGetCurrentDirection") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error(
          "rtpTransceiverGetCurrentDirection",
          "rtpTransceiverGetCurrentDirection() peerConnection is null");
      return;
    }

    const std::string rtpTransceiverId = findString(params, "rtpTransceiverId");
    if (0 < rtpTransceiverId.size()) {
      if (pc == nullptr) {
        result->Error("rtpTransceiverGetCurrentDirection",
                      "rtpTransceiverGetCurrentDirection() rtpTransceiverId is "
                      "null or empty");
        return;
      }
    }

    RtpTransceiverGetCurrentDirection(pc, rtpTransceiverId, std::move(result));
  } else if (method_call.method_name().compare("rtpTransceiverSetDirection") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("rtpTransceiverSetDirection",
                    "rtpTransceiverSetDirection() peerConnection is null");
      return;
    }

    const std::string rtpTransceiverId = findString(params, "rtpTransceiverId");
    if (0 < rtpTransceiverId.size()) {
      if (pc == nullptr) {
        result->Error("rtpTransceiverGetCurrentDirection",
                      "rtpTransceiverGetCurrentDirection() rtpTransceiverId is "
                      "null or empty");
        return;
      }
    }

    const std::string direction = findString(params, "direction");
    if (0 < rtpTransceiverId.size()) {
      if (pc == nullptr) {
        result->Error(
            "rtpTransceiverGetCurrentDirection",
            "rtpTransceiverGetCurrentDirection() direction is null or empty");
        return;
      }
    }

    RtpTransceiverSetDirection(pc, rtpTransceiverId, direction,
                               std::move(result));
  } else if (method_call.method_name().compare("setConfiguration") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());
    const std::string peerConnectionId = findString(params, "peerConnectionId");

    RTCPeerConnection* pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("setConfiguration",
                    "setConfiguration() peerConnection is null");
      return;
    }

  const EncodableMap configuration = findMap(params, "configuration");
    if (0 < configuration.size()) {
      if (pc == nullptr) {
        result->Error("setConfiguration",
                      "setConfiguration() configuration is null or empty");
        return;
      }
    }
    SetConfiguration(pc, configuration,  std::move(result));
  } else if (method_call.method_name().compare("captureFrame") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const EncodableMap params =
        GetValue<EncodableMap>(*method_call.arguments());

    const std::string path = findString(params, "path");
    if (0 < path.size()) {
      result->Error("captureFrame",
                    "captureFrame() path is null or empty");
      return;
    }

    const std::string trackId = findString(params, "trackId");
    RTCMediaTrack* track = MediaTrackForId(trackId);
    if (nullptr == track) {
      result->Error("captureFrame",
                    "captureFrame() track is null");
      return;
    }
    std::string kind = to_std_string(track->kind());
    if (0 != kind.compare("video")) {
      result->Error("captureFrame",
                    "captureFrame() track not is video track");
      return;
    }
    CaptureFrame((RTCVideoTrack*)track, path, std::move(result));
  
  } else {
    result->NotImplemented();
  }
}

}  // namespace flutter_webrtc_plugin
