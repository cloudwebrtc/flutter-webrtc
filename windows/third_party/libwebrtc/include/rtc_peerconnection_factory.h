#ifndef LIB_WEBRTC_RTC_PEERCONNECTION_FACTORY_HXX
#define LIB_WEBRTC_RTC_PEERCONNECTION_FACTORY_HXX

#include "rtc_types.h"

#include "rtc_audio_source.h"
#include "rtc_audio_track.h"
#include "rtc_media_stream.h"
#include "rtc_mediaconstraints.h"
#include "rtc_video_device.h"
#include "rtc_video_source.h"

namespace libwebrtc {

class RTCPeerConnection;
class RTCAudioDevice;
class RTCVideoDevice;

class RTCPeerConnectionFactory : public RefCountInterface {
 public:
  virtual bool Initialize() = 0;

  virtual bool Terminate() = 0;

  virtual scoped_refptr<RTCPeerConnection> Create(
      const RTCConfiguration& configuration,
      scoped_refptr<RTCMediaConstraints> constraints) = 0;

  virtual void Delete(scoped_refptr<RTCPeerConnection> peerconnection) = 0;

  virtual scoped_refptr<RTCAudioDevice> GetAudioDevice() = 0;

  virtual scoped_refptr<RTCVideoDevice> GetVideoDevice() = 0;

  virtual scoped_refptr<RTCAudioSource> CreateAudioSource(
      const string audio_source_label) = 0;

  virtual scoped_refptr<RTCVideoSource> CreateVideoSource(
      scoped_refptr<RTCVideoCapturer> capturer,
      const string video_source_label,
      scoped_refptr<RTCMediaConstraints> constraints) = 0;

  virtual scoped_refptr<RTCAudioTrack> CreateAudioTrack(
      scoped_refptr<RTCAudioSource> source,
      const string track_id) = 0;

  virtual scoped_refptr<RTCVideoTrack> CreateVideoTrack(
      scoped_refptr<RTCVideoSource> source,
      const string track_id) = 0;

  virtual scoped_refptr<RTCMediaStream> CreateStream(
      const string stream_id) = 0;
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_PEERCONNECTION_FACTORY_HXX
