#ifndef LIB_WEBRTC_RTC_DATA_CHANNEL_HXX
#define LIB_WEBRTC_RTC_DATA_CHANNEL_HXX

#include "rtc_types.h"

namespace libwebrtc {

enum RTCDataChannelState {
  RTCDataChannelConnecting,
  RTCDataChannelOpen,
  RTCDataChannelClosing,
  RTCDataChannelClosed,
};

struct RTCDataChannelInit {
  bool ordered = true;
  bool reliable = true;
  int maxRetransmitTime = -1;
  int maxRetransmits = -1;
  string protocol = {"sctp"};  // sctp | quic
  bool negotiated = false;
  int id = 0;
};

class RTCDataChannelObserver {
 public:
  virtual void OnStateChange(RTCDataChannelState state) = 0;

  virtual void OnMessage(const char* buffer, int length, bool binary) = 0;

 protected:
  virtual ~RTCDataChannelObserver() = default;
};

class RTCDataChannel : public RefCountInterface {
 public:
  virtual void Send(const string data, bool binary = false) = 0;

  virtual void Close() = 0;

  virtual void RegisterObserver(RTCDataChannelObserver* observer) = 0;

  virtual void UnregisterObserver() = 0;

  virtual const string label() const = 0;

  virtual int id() const = 0;

  virtual RTCDataChannelState state() = 0;

 protected:
  virtual ~RTCDataChannel() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_DATA_CHANNEL_HXX
