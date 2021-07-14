#ifndef LIB_WEBRTC_RTC_MEDIA_TRACK_HXX
#define LIB_WEBRTC_RTC_MEDIA_TRACK_HXX

#include "rtc_types.h"

namespace libwebrtc {

/*Media Track interface*/
class RTCMediaTrack : public RefCountInterface {
 public:
  enum RTCTrackState {
    kLive,
    kEnded,
  };
  virtual RTCTrackState state() const = 0;

  /*track type: audio/video*/
  virtual const string kind() const = 0;

  /*track id*/
  virtual const string id() const = 0;

  virtual bool enabled() const = 0;

  /*mute track*/
  virtual bool set_enabled(bool enable) = 0;

 protected:
  ~RTCMediaTrack() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_MEDIA_TRACK_HXX
