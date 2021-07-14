#ifndef LIB_WEBRTC_RTC_TYPES_HXX
#define LIB_WEBRTC_RTC_TYPES_HXX

#ifdef LIB_WEBRTC_API_EXPORTS
#define LIB_WEBRTC_API __declspec(dllexport)
#elif defined(LIB_WEBRTC_API_DLL)
#define LIB_WEBRTC_API __declspec(dllimport)
#elif !defined(WIN32)
#define LIB_WEBRTC_API __attribute__((visibility("default")))
#else
#define LIB_WEBRTC_API
#endif

#include "base/fixed_size_function.h"
#include "base/string.h"
#include "base/inlined_vector.h"
#include "base/refcount.h"
#include "base/scoped_ref_ptr.h"

#include <vector>
#include <map>

namespace libwebrtc {

enum { kMaxIceServerSize = 8 };

template <typename T>
using vector = bsp::inlined_vector<T, 4, true>;

template <typename Key, typename T>
using map = std::map<Key, T>;

enum MediaSecurityType { kSRTP_None = 0, kSDES_SRTP, kDTLS_SRTP };

enum class RTCMediaType { ANY, AUDIO, VIDEO, DATA };

struct IceServer {
  string uri;
  string username;
  string password;
};

enum IceTransportsType { kNone, kRelay, kNoHost, kAll };

enum TcpCandidatePolicy {
  kTcpCandidatePolicyEnabled,
  kTcpCandidatePolicyDisabled
};

enum CandidateNetworkPolicy {
  kCandidateNetworkPolicyAll,
  kCandidateNetworkPolicyLowCost
};

enum RtcpMuxPolicy {
  kRtcpMuxPolicyNegotiate,
  kRtcpMuxPolicyRequire,
};

enum BundlePolicy {
  kBundlePolicyBalanced,
  kBundlePolicyMaxBundle,
  kBundlePolicyMaxCompat
};

enum class SdpSemantics { kPlanB, kUnifiedPlan };

struct RTCConfiguration {
  IceServer ice_servers[kMaxIceServerSize];
  IceTransportsType type = kAll;
  BundlePolicy bundle_policy = kBundlePolicyBalanced;
  RtcpMuxPolicy rtcp_mux_policy = kRtcpMuxPolicyRequire;
  CandidateNetworkPolicy candidate_network_policy = kCandidateNetworkPolicyAll;
  TcpCandidatePolicy tcp_candidate_policy = kTcpCandidatePolicyEnabled;

  int ice_candidate_pool_size = 0;

  MediaSecurityType srtp_type = kDTLS_SRTP;
  SdpSemantics sdp_semantics = SdpSemantics::kPlanB;
  bool offer_to_receive_audio = true;
  bool offer_to_receive_video = true;
  // private
  bool use_rtp_mux = true;
  uint32_t local_audio_bandwidth = 128;
  uint32_t local_video_bandwidth = 512;
};

struct SdpParseError {
 public:
  // The sdp line that causes the error.
  string line;
  // Explains the error.
  string description;
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_TYPES_HXX
