#include "flutter_media_stream.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define DEFAULT_FPS 30

namespace flutter_webrtc_plugin {

void FlutterMediaStream::GetUserMedia(
    const EncodableMap& constraints,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCMediaStream> stream =
      base_->factory_->CreateStream(uuid.c_str());

  EncodableMap params;
  params[EncodableValue("streamId")] = uuid;

  auto it = constraints.find(EncodableValue("audio"));
  if (it != constraints.end()) {
    EncodableValue audio = it->second;
    if (TypeIs<bool>(audio)) {
      if (true == GetValue<bool>(audio)) {
        GetUserAudio(constraints, stream, params);
      }
    } else if (TypeIs<EncodableMap>(audio)) {
      GetUserAudio(constraints, stream, params);
    }
  } else {
	  params[EncodableValue("audioTracks")] = EncodableValue(EncodableList());
  }

  it = constraints.find(EncodableValue("video"));
  if (it != constraints.end()) {
    EncodableValue video = it->second;
     if (TypeIs<bool>(video)) {
       if (true == GetValue<bool>(video)) {
         GetUserVideo(constraints, stream, params);
       }
     } else if (TypeIs<EncodableMap>(video)) {
       GetUserVideo(constraints, stream, params);
    }
  } else {
	  params[EncodableValue("videoTracks")] = EncodableValue(EncodableList());
  }

  base_->local_streams_[uuid] = stream;
  result->Success(EncodableValue(params));
}

void addDefaultAudioConstraints(
    scoped_refptr<RTCMediaConstraints> audioConstraints) {
  audioConstraints->AddOptionalConstraint("googNoiseSuppression", "true");
  audioConstraints->AddOptionalConstraint("googEchoCancellation", "true");
  audioConstraints->AddOptionalConstraint("echoCancellation", "true");
  audioConstraints->AddOptionalConstraint("googEchoCancellation2", "true");
  audioConstraints->AddOptionalConstraint("googDAEchoCancellation", "true");
}

void FlutterMediaStream::GetUserAudio(const EncodableMap& constraints,
                                      scoped_refptr<RTCMediaStream> stream,
                                      EncodableMap& params) {
  bool enable_audio = false;
  scoped_refptr<RTCMediaConstraints> audioConstraints;
  auto it = constraints.find(EncodableValue("audio"));
  if (it != constraints.end()) {
    EncodableValue audio = it->second;
    if (TypeIs<bool>(audio)) {
      audioConstraints = RTCMediaConstraints::Create();
      addDefaultAudioConstraints(audioConstraints);
      enable_audio = GetValue<bool>(audio);
    }
    if (TypeIs<EncodableMap>(audio)) {
      audioConstraints =
          base_->ParseMediaConstraints(GetValue<EncodableMap>(audio));
      enable_audio = true;
    }
  }

  // TODO: Select audio device by sourceId,

  if (enable_audio) {
    scoped_refptr<RTCAudioSource> source =
        base_->factory_->CreateAudioSource("audio_input");
    std::string uuid = base_->GenerateUUID();
    scoped_refptr<RTCAudioTrack> track =
        base_->factory_->CreateAudioTrack(source, uuid.c_str());

    std::string track_id = to_std_string(track->id());

    EncodableMap track_info;
    track_info[EncodableValue("id")] = to_std_string(track->id());
    track_info[EncodableValue("label")] = to_std_string(track->id());
    track_info[EncodableValue("kind")] = to_std_string(track->kind());
    track_info[EncodableValue("enabled")] = track->enabled();

    EncodableList audioTracks;
    audioTracks.push_back(EncodableValue(track_info));
    params[EncodableValue("audioTracks")] = EncodableValue(audioTracks);
    stream->AddTrack(track);

    base_->local_tracks_[to_std_string(track->id())] = track;
  }
}

std::string getFacingMode(const EncodableMap& mediaConstraints) {
  return mediaConstraints.find(EncodableValue("facingMode")) !=
                 mediaConstraints.end()
             ? GetValue<std::string>(mediaConstraints.find(EncodableValue("facingMode"))
                   ->second)
             : "";
}

std::string getSourceIdConstraint(const EncodableMap& mediaConstraints) {
  auto it = mediaConstraints.find(EncodableValue("optional"));
  if (it != mediaConstraints.end() &&
      TypeIs<EncodableList>(it->second)) {
    EncodableList optional = GetValue<EncodableList>(it->second);
    for (size_t i = 0, size = optional.size(); i < size; i++) {
      if (TypeIs<EncodableMap>(optional[i])) {
        EncodableMap option = GetValue<EncodableMap>(optional[i]);
        auto it2 = option.find(EncodableValue("sourceId"));
        if (it2 != option.end() && TypeIs<std::string>(it2->second)) {
          return GetValue<std::string>(it2->second);
        }
      }
    }
  }
  return "";
}

void FlutterMediaStream::GetUserVideo(const EncodableMap& constraints,
                                      scoped_refptr<RTCMediaStream> stream,
                                      EncodableMap& params) {
  EncodableMap video_constraints;
  EncodableMap video_mandatory;
  auto it = constraints.find(EncodableValue("video"));
  if (it != constraints.end() && TypeIs<EncodableMap>(it->second)) {
    EncodableMap video_map =   GetValue<EncodableMap>(it->second);
    if (video_map.find(EncodableValue("mandatory")) != video_map.end()) {
      video_mandatory =
          GetValue<EncodableMap>(video_map.find(EncodableValue("mandatory"))->second);
    }
  }

  std::string facing_mode = getFacingMode(video_constraints);
  //bool isFacing = facing_mode == "" || facing_mode != "environment";
  std::string sourceId = getSourceIdConstraint(video_constraints);
  /*
  int width = video_mandatory["minWidth"].isNumeric()
                  ? video_mandatory["minWidth"].asInt()
                  : DEFAULT_WIDTH;
  int height = video_mandatory["minHeight"].isNumeric()
                   ? video_mandatory["minHeight"].asInt()
                   : DEFAULT_HEIGHT;
  int fps = video_mandatory["minFrameRate"].isNumeric()
                ? video_mandatory["minFrameRate"].asInt()
                : DEFAULT_FPS;
 */
  scoped_refptr<RTCVideoCapturer> video_capturer;
  char strNameUTF8[256];
  char strGuidUTF8[256];
  int nb_video_devices = base_->video_device_->NumberOfDevices();

  for (int i = 0; i < nb_video_devices; i++) {
    base_->video_device_->GetDeviceName(i, strNameUTF8, 256, strGuidUTF8, 256);
    if (sourceId != "" && sourceId == strGuidUTF8) {
      video_capturer = base_->video_device_->Create(strNameUTF8, i);
      break;
    }
  }

  if (nb_video_devices == 0) return;

  if (!video_capturer.get()) {
    base_->video_device_->GetDeviceName(0, strNameUTF8, 128, strGuidUTF8, 128);
    video_capturer = base_->video_device_->Create(strNameUTF8, 0);
  }
  const char* video_source_label = "video_input";
  scoped_refptr<RTCVideoSource> source = base_->factory_->CreateVideoSource(
      video_capturer, video_source_label,
      base_->ParseMediaConstraints(video_constraints));

  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCVideoTrack> track =
      base_->factory_->CreateVideoTrack(source, uuid.c_str());

  EncodableList videoTracks;
  EncodableMap info;
  info[EncodableValue("id")] = to_std_string(track->id());
  info[EncodableValue("label")] = to_std_string(track->id());
  info[EncodableValue("kind")] = to_std_string(track->kind());
  info[EncodableValue("enabled")] = track->enabled();
  videoTracks.push_back(EncodableValue(info));
  params[EncodableValue("videoTracks")] = EncodableValue(videoTracks);

  stream->AddTrack(track);

  base_->local_tracks_[to_std_string(track->id())] = track;
}

void FlutterMediaStream::GetSources(
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  EncodableList sources;

  int nb_audio_devices = base_->audio_device_->RecordingDevices();
  char strNameUTF8[128];
  char strGuidUTF8[128];

  for (uint16_t i = 0; i < nb_audio_devices; i++) {
    base_->audio_device_->RecordingDeviceName(i, strNameUTF8, strGuidUTF8);
    EncodableMap audio;
    audio[EncodableValue("label")] = std::string(strNameUTF8);
    audio[EncodableValue("deviceId")] = std::string(strGuidUTF8);
    audio[EncodableValue("facing")] = "";
    audio[EncodableValue("kind")] = "audioinput";
    sources.push_back(EncodableValue(audio));
  }

  nb_audio_devices = base_->audio_device_->PlayoutDevices();
  for (uint16_t i = 0; i < nb_audio_devices; i++) {
    base_->audio_device_->PlayoutDeviceName(i, strNameUTF8, strGuidUTF8);
    EncodableMap audio;
    audio[EncodableValue("label")] = std::string(strGuidUTF8);
    audio[EncodableValue("deviceId")] = std::string(strNameUTF8);
    audio[EncodableValue("facing")] = "";
    audio[EncodableValue("kind")] = "audiooutput";
    sources.push_back(EncodableValue(audio));
  }

  int nb_video_devices = base_->video_device_->NumberOfDevices();
  for (int i = 0; i < nb_video_devices; i++) {
    base_->video_device_->GetDeviceName(i, strNameUTF8, 128, strGuidUTF8, 128);
    EncodableMap video;
    video[EncodableValue("label")] = std::string(strGuidUTF8);
    video[EncodableValue("deviceId")] = std::string(strNameUTF8);
    video[EncodableValue("facing")] = i == 1 ? "front" : "back";
    video[EncodableValue("kind")] = "videoinput";
    sources.push_back(EncodableValue(video));
  }
  EncodableMap params;
  params[EncodableValue("sources")] = sources;
  result->Success(EncodableValue(params));
}

void FlutterMediaStream::MediaStreamGetTracks(
    const std::string& stream_id,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  scoped_refptr<RTCMediaStream> stream = base_->MediaStreamForId(stream_id);

  if (stream) {
    EncodableMap params;
    EncodableList audioTracks;

    auto audio_tracks = stream->audio_tracks();
    for (auto track : audio_tracks) {
      base_->local_tracks_[to_std_string(track->id())] = track;
      EncodableMap info;
      info[EncodableValue("id")] = to_std_string(track->id());
      info[EncodableValue("label")] = to_std_string(track->id());
      info[EncodableValue("kind")] = to_std_string(track->kind());
      info[EncodableValue("enabled")] = track->enabled();
      info[EncodableValue("remote")] = true;
      info[EncodableValue("readyState")] = "live";
      audioTracks.push_back(EncodableValue(info));
    }
    params[EncodableValue("audioTracks")] = audioTracks;

    EncodableList videoTracks;
    auto video_tracks = stream->video_tracks();
    for (auto track : video_tracks) {
      base_->local_tracks_[to_std_string(track->id())] = track;
      EncodableMap info;
      info[EncodableValue("id")] = to_std_string(track->id());
      info[EncodableValue("label")] = to_std_string(track->id());
      info[EncodableValue("kind")] = to_std_string(track->kind());
      info[EncodableValue("enabled")] = track->enabled();
      info[EncodableValue("remote")] = true;
      info[EncodableValue("readyState")] = "live";
      videoTracks.push_back(EncodableValue("info"));
    }

    params[EncodableValue("videoTracks")] = videoTracks;

    result->Success(EncodableValue("params"));
  } else {
    result->Error("MediaStreamGetTracksFailed",
                  "MediaStreamGetTracks() media stream is null !");
  }
}

void FlutterMediaStream::MediaStreamDispose(
    const std::string& stream_id,
    std::unique_ptr<MethodResult<EncodableValue>> result) {
  scoped_refptr<RTCMediaStream> stream = base_->MediaStreamForId(stream_id);
  vector<scoped_refptr<RTCAudioTrack>> audio_tracks = stream->audio_tracks();
  
  for (auto track : audio_tracks) {
    stream->RemoveTrack(track);
    base_->local_tracks_.erase(to_std_string(track->id()));
  }

  vector<scoped_refptr<RTCVideoTrack>> video_tracks = stream->video_tracks();
  for (auto track : video_tracks) {
    stream->RemoveTrack(track);
    base_->local_tracks_.erase(to_std_string(track->id()));
  }
  base_->RemoveStreamForId(stream_id);
  result->Success();
}

void FlutterMediaStream::MediaStreamTrackSetEnable(
    const std::string& track_id,
    std::unique_ptr<MethodResult<EncodableValue>> result) {}

void FlutterMediaStream::MediaStreamTrackSwitchCamera(
    const std::string& track_id,
    std::unique_ptr<MethodResult<EncodableValue>> result) {}

void FlutterMediaStream::MediaStreamTrackDispose(
    const std::string& track_id,
    std::unique_ptr<MethodResult<EncodableValue>> result) {}
}  // namespace flutter_webrtc_plugin
