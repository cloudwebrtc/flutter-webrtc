#ifndef LIB_WEBRTC_HXX
#define LIB_WEBRTC_HXX

#include "rtc_peerconnection_factory.h"
#include "rtc_types.h"

namespace libwebrtc {

class LibWebRTC {
 public:
  LIB_WEBRTC_API static bool Initialize();

  LIB_WEBRTC_API static scoped_refptr<RTCPeerConnectionFactory>
  CreateRTCPeerConnectionFactory();

  LIB_WEBRTC_API static void Terminate();
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_HXX
