
#ifndef LIB_WEBRTC_DTMF_SENDER__H_
#define LIB_WEBRTC_DTMF_SENDER__H_

#include "base/refcount.h"
#include "rtc_types.h"

namespace libwebrtc {

class RTCDtmfSenderObserver {
 public:
  virtual void OnToneChange(const string tone, const string tone_buffer) = 0;

  virtual void OnToneChange(const string tone) = 0;

 protected:
  virtual ~RTCDtmfSenderObserver() = default;
};

class RTCDtmfSender : public RefCountInterface {
 public:
  static const int kDtmfDefaultCommaDelayMs = 2000;

  virtual void RegisterObserver(RTCDtmfSenderObserver* observer) = 0;

  virtual void UnregisterObserver() = 0;

  virtual bool InsertDtmf(const string tones,
                          int duration,
                          int inter_tone_gap) = 0;

  virtual bool InsertDtmf(const string tones,
                          int duration,
                          int inter_tone_gap,
                          int comma_delay) = 0;

  virtual bool CanInsertDtmf() = 0;

  virtual const string tones() const = 0;

  virtual int duration() const = 0;

  virtual int inter_tone_gap() const = 0;

  virtual int comma_delay() const = 0;
};

}  // namespace libwebrtc

#endif  // API_DTMF_SENDER__H_
