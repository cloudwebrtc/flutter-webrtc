#ifndef LIB_WEBRTC_DTLS_TRANSPORT_H_
#define LIB_WEBRTC_DTLS_TRANSPORT_H_

#include "base/refcount.h"
#include "rtc_types.h"

namespace libwebrtc {

class RTCDtlsTransportInformation : public RefCountInterface {
 public:
  enum class RTCDtlsTransportState {
    kNew,         // Has not started negotiating yet.
    kConnecting,  // In the process of negotiating a secure connection.
    kConnected,   // Completed negotiation and verified fingerprints.
    kClosed,      // Intentionally closed.
    kFailed,      // Failure due to an error or failing to verify a remote
    // fingerprint.
    kNumValues
  };
  virtual RTCDtlsTransportInformation& operator=(
      scoped_refptr<RTCDtlsTransportInformation> c) = 0;

  virtual RTCDtlsTransportState state() const = 0;
  virtual int ssl_cipher_suite() const = 0;
  virtual int srtp_cipher_suite() const = 0;
};

class RTCDtlsTransportObserver {
 public:
  virtual void OnStateChange(RTCDtlsTransportInformation info) = 0;

  virtual void OnError(const int type, const char* message) = 0;

 protected:
  virtual ~RTCDtlsTransportObserver() = default;
};

class RTCDtlsTransport : public RefCountInterface {
  LIB_WEBRTC_API static scoped_refptr<RTCDtlsTransport> Create();

 public:
  virtual scoped_refptr<RTCDtlsTransportInformation> GetInformation() = 0;

  virtual void RegisterObserver(RTCDtlsTransportObserver* observer) = 0;

  virtual void UnregisterObserver() = 0;
};

}  // namespace libwebrtc

#endif  // API_DTLS_TRANSPORT_INTERFACE_H_
