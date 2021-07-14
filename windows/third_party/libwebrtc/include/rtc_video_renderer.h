#ifndef LIB_WEBRTC_RTC_VIDEO_RENDERER_HXX
#define LIB_WEBRTC_RTC_VIDEO_RENDERER_HXX

#include "rtc_types.h"

namespace libwebrtc {

template <typename VideoFrameT>
class RTCVideoRenderer {
 public:
  virtual ~RTCVideoRenderer() {}

  virtual void OnFrame(VideoFrameT frame) = 0;
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_VIDEO_RENDERER_HXX
