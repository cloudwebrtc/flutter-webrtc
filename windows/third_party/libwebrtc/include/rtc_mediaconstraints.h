#ifndef LIB_WEBRTC_RTC_MEDIA_CONSTRAINTS_HXX
#define LIB_WEBRTC_RTC_MEDIA_CONSTRAINTS_HXX

#include "rtc_types.h"

namespace libwebrtc {

class RTCMediaConstraints : public RefCountInterface {
 public:
  // These keys are google specific.
  LIB_WEBRTC_API static const char*
      kGoogEchoCancellation;  // googEchoCancellation

  LIB_WEBRTC_API static const char*
      kExtendedFilterEchoCancellation;  // googEchoCancellation2
  LIB_WEBRTC_API static const char*
      kDAEchoCancellation;                             // googDAEchoCancellation
  LIB_WEBRTC_API static const char* kAutoGainControl;  // googAutoGainControl
  LIB_WEBRTC_API static const char*
      kExperimentalAutoGainControl;                     // googAutoGainControl2
  LIB_WEBRTC_API static const char* kNoiseSuppression;  // googNoiseSuppression
  LIB_WEBRTC_API static const char*
      kExperimentalNoiseSuppression;                  // googNoiseSuppression2
  LIB_WEBRTC_API static const char* kHighpassFilter;  // googHighpassFilter
  LIB_WEBRTC_API static const char*
      kTypingNoiseDetection;  // googTypingNoiseDetection
  LIB_WEBRTC_API static const char* kAudioMirroring;  // googAudioMirroring
  LIB_WEBRTC_API static const char*
      kAudioNetworkAdaptorConfig;  // goodAudioNetworkAdaptorConfig

  // Constraint keys for CreateOffer / CreateAnswer
  // Specified by the W3C PeerConnection spec
  LIB_WEBRTC_API static const char*
      kOfferToReceiveVideo;  // OfferToReceiveVideo
  LIB_WEBRTC_API static const char*
      kOfferToReceiveAudio;  // OfferToReceiveAudio
  LIB_WEBRTC_API static const char*
      kVoiceActivityDetection;                    // VoiceActivityDetection
  LIB_WEBRTC_API static const char* kIceRestart;  // IceRestart
  // These keys are google specific.
  LIB_WEBRTC_API static const char* kUseRtpMux;  // googUseRtpMUX

  // Constraints values.
  LIB_WEBRTC_API static const char* kValueTrue;   // true
  LIB_WEBRTC_API static const char* kValueFalse;  // false

  // PeerConnection constraint keys.
  // Temporary pseudo-constraints used to enable DTLS-SRTP
  LIB_WEBRTC_API static const char* kEnableDtlsSrtp;  // Enable DTLS-SRTP
  // Temporary pseudo-constraints used to enable DataChannels
  LIB_WEBRTC_API static const char*
      kEnableRtpDataChannels;  // Enable RTP DataChannels
  // Google-specific constraint keys.
  // Temporary pseudo-constraint for enabling DSCP through JS.
  LIB_WEBRTC_API static const char* kEnableDscp;  // googDscp
  // Constraint to enable IPv6 through JS.
  LIB_WEBRTC_API static const char* kEnableIPv6;  // googIPv6
  // Temporary constraint to enable suspend below min bitrate feature.
  LIB_WEBRTC_API static const char* kEnableVideoSuspendBelowMinBitrate;
  // googSuspendBelowMinBitrate
  // Constraint to enable combined audio+video bandwidth estimation.
  LIB_WEBRTC_API static const char*
      kCombinedAudioVideoBwe;  // googCombinedAudioVideoBwe
  LIB_WEBRTC_API static const char*
      kScreencastMinBitrate;  // googScreencastMinBitrate
  LIB_WEBRTC_API static const char*
      kCpuOveruseDetection;  // googCpuOveruseDetection

  // Specifies number of simulcast layers for all video tracks
  // with a Plan B offer/answer
  // (see RTCOfferAnswerOptions::num_simulcast_layers).
  LIB_WEBRTC_API static const char* kNumSimulcastLayers;

 public:
  LIB_WEBRTC_API static scoped_refptr<RTCMediaConstraints> Create();

  virtual void AddMandatoryConstraint(const string key, const string value) = 0;

  virtual void AddOptionalConstraint(const string key, const string value) = 0;

 protected:
  virtual ~RTCMediaConstraints() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_MEDIA_CONSTRAINTS_HXX
