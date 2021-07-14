#ifndef LIB_WEBRTC_RTC_RTP_TRANSCEIVER_HXX
#define LIB_WEBRTC_RTC_RTP_TRANSCEIVER_HXX

#include "base/refcount.h"
#include "rtc_rtp_parameters.h"
#include "rtc_rtp_receiver.h"
#include "rtc_rtp_sender.h"
#include "rtc_types.h"

namespace libwebrtc {

class RTCRtpTransceiverInit : public RefCountInterface {
 public:
  LIB_WEBRTC_API static scoped_refptr<RTCRtpTransceiverInit> Create(
      RTCRtpTransceiverDirection direction,
      const vector<string> stream_ids,
      const vector<scoped_refptr<RTCRtpEncodingParameters>> encodings);

  virtual RTCRtpTransceiverDirection direction() = 0;
  virtual void set_direction(RTCRtpTransceiverDirection value) = 0;

  virtual const vector<string> stream_ids() = 0;
  virtual void set_stream_ids(const vector<string> ids) = 0;

  virtual const vector<scoped_refptr<RTCRtpEncodingParameters>>
  send_encodings() = 0;
  virtual void set_send_encodings(
      const vector<scoped_refptr<RTCRtpEncodingParameters>> send_encodings) = 0;
};

class RTCRtpTransceiver : public RefCountInterface {
 public:
  virtual RTCMediaType media_type() const = 0;

  virtual const string mid() const = 0;

  virtual scoped_refptr<RTCRtpSender> sender() const = 0;

  virtual scoped_refptr<RTCRtpReceiver> receiver() const = 0;

  virtual bool Stopped() const = 0;

  virtual bool Stopping() const = 0;

  virtual RTCRtpTransceiverDirection direction() const = 0;

  virtual const string SetDirectionWithError(
      RTCRtpTransceiverDirection new_direction) = 0;

  virtual RTCRtpTransceiverDirection current_direction() const = 0;

  virtual RTCRtpTransceiverDirection fired_direction() const = 0;

  virtual const string StopStandard() = 0;

  virtual void StopInternal() = 0;

  // virtual string set_codec_preferences(vector<RTCRtpCodecCapability> codecs)
  // = 0;

  // virtual vector<RTCRtpCodecCapability> codec_preferences() const = 0;

  // virtual vector<RTCRtpHeaderExtensionCapability> HeaderExtensionsToOffer()
  // const = 0;

  // virtual std::vector<RTCRtpHeaderExtensionCapability>
  // HeaderExtensionsNegotiated() const = 0;

  // virtual webrtc::RTCError SetOfferedRtpHeaderExtensions(vector<const
  // RTCRtpHeaderExtensionCapability> header_extensions_to_offer);
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_TYPES_HXX
