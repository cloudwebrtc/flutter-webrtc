#ifndef LIB_WEBRTC_RTC_VIDEO_TRACK_HXX
#define LIB_WEBRTC_RTC_VIDEO_TRACK_HXX

#include "rtc_types.h"

#include "rtc_media_track.h"
#include "rtc_video_frame.h"
#include "rtc_video_renderer.h"

namespace libwebrtc {

class RTCVideoTrack : public RTCMediaTrack {
 public:
  virtual void AddRenderer(
      RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer) = 0;

  virtual void RemoveRenderer(
      RTCVideoRenderer<scoped_refptr<RTCVideoFrame>>* renderer) = 0;

 protected:
  ~RTCVideoTrack() {}
};
}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_VIDEO_TRACK_HXX
