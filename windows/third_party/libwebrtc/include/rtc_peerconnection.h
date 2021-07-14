#ifndef LIB_WEBRTC_RTC_PEERCONNECTION_HXX
#define LIB_WEBRTC_RTC_PEERCONNECTION_HXX

#include "rtc_audio_track.h"
#include "rtc_data_channel.h"
#include "rtc_ice_candidate.h"
#include "rtc_media_stream.h"
#include "rtc_mediaconstraints.h"
#include "rtc_rtp_sender.h"
#include "rtc_rtp_transceiver.h"
#include "rtc_session_description.h"
#include "rtc_video_source.h"
#include "rtc_video_track.h"

namespace libwebrtc {

enum SessionDescriptionErrorType {
  kPeerConnectionInitFailed = 0,
  kCreatePeerConnectionFailed,
  kSDPParseFailed,
};

enum RTCSignalingState {
  RTCSignalingStateStable,
  RTCSignalingStateHaveLocalOffer,
  RTCSignalingStateHaveRemoteOffer,
  RTCSignalingStateHaveLocalPrAnswer,
  RTCSignalingStateHaveRemotePrAnswer,
  RTCSignalingStateClosed
};

enum RTCIceGatheringState {
  RTCIceGatheringStateNew,
  RTCIceGatheringStateGathering,
  RTCIceGatheringStateComplete
};

enum RTCIceConnectionState {
  RTCIceConnectionStateNew,
  RTCIceConnectionStateChecking,
  RTCIceConnectionStateCompleted,
  RTCIceConnectionStateConnected,
  RTCIceConnectionStateFailed,
  RTCIceConnectionStateDisconnected,
  RTCIceConnectionStateClosed,
  RTCIceConnectionStateMax,
};

class MediaTrackStatistics {
 public:
  MediaTrackStatistics() {}
  int64_t bytes_received = 0;
  int64_t bytes_sent = 0;
  int packets_lost = 0;
  int packets_received = 0;
  int packets_sent = 0;
  int frame_rate_sent = 0;
  int frame_rate_received = 0;
  uint32_t rtt = 0;

  int64_t ssrc = 0;
  string msid;
  string kind;
  string direction;

 public:
  MediaTrackStatistics(const MediaTrackStatistics* stats) { copy(*stats); }

  MediaTrackStatistics& operator=(const MediaTrackStatistics& rhs) {
    if (&rhs == this)
      return *this;
    return copy(rhs);
  }

  MediaTrackStatistics& copy(const MediaTrackStatistics& rhs) {
    direction = rhs.direction;
    kind = rhs.kind;
    packets_sent = rhs.packets_sent;
    packets_received = rhs.packets_received;
    packets_lost = rhs.packets_lost;
    bytes_sent = rhs.bytes_sent;
    bytes_received = rhs.bytes_received;
    frame_rate_sent = rhs.frame_rate_sent;
    frame_rate_received = rhs.frame_rate_received;
    ssrc = rhs.ssrc;
    rtt = rhs.rtt;
    msid = rhs.msid;
    return *this;
  }
};

class TrackStatsObserver : public RefCountInterface {
 public:
  virtual void OnComplete(const MediaTrackStatistics& reports) = 0;

 protected:
  ~TrackStatsObserver() {}
};

typedef fixed_size_function<void(const string sdp, const string type)>
    OnSdpCreateSuccess;

typedef fixed_size_function<void(const char* erro)> OnSdpCreateFailure;

typedef fixed_size_function<void()> OnSetSdpSuccess;

typedef fixed_size_function<void(const char* error)> OnSetSdpFailure;

typedef fixed_size_function<void(const char* sdp, const char* type)>
    OnGetSdpSuccess;

typedef fixed_size_function<void(const char* error)> OnGetSdpFailure;

class RTCPeerConnectionObserver {
 public:
  virtual void OnSignalingState(RTCSignalingState state) = 0;

  virtual void OnIceGatheringState(RTCIceGatheringState state) = 0;

  virtual void OnIceConnectionState(RTCIceConnectionState state) = 0;

  virtual void OnIceCandidate(scoped_refptr<RTCIceCandidate> candidate) = 0;

  virtual void OnAddStream(scoped_refptr<RTCMediaStream> stream) = 0;

  virtual void OnRemoveStream(scoped_refptr<RTCMediaStream> stream) = 0;

  virtual void OnDataChannel(scoped_refptr<RTCDataChannel> data_channel) = 0;

  virtual void OnRenegotiationNeeded() = 0;

  virtual void OnTrack(scoped_refptr<RTCRtpTransceiver> transceiver) = 0;

  virtual void OnAddTrack(vector<scoped_refptr<RTCMediaStream>> streams,
                          scoped_refptr<RTCRtpReceiver> receiver) = 0;

  virtual void OnRemoveTrack(scoped_refptr<RTCRtpReceiver> receiver) = 0;

 protected:
  virtual ~RTCPeerConnectionObserver() {}
};

class RTCPeerConnection : public RefCountInterface {
 public:
  virtual int AddStream(scoped_refptr<RTCMediaStream> stream) = 0;

  virtual int RemoveStream(scoped_refptr<RTCMediaStream> stream) = 0;

  virtual scoped_refptr<RTCDataChannel> CreateDataChannel(
      const string label,
      const RTCDataChannelInit* dataChannelDict) = 0;

  virtual void CreateOffer(OnSdpCreateSuccess success,
                           OnSdpCreateFailure failure,
                           scoped_refptr<RTCMediaConstraints> constraints) = 0;

  virtual void CreateAnswer(OnSdpCreateSuccess success,
                            OnSdpCreateFailure failure,
                            scoped_refptr<RTCMediaConstraints> constraints) = 0;

  virtual void Close() = 0;

  virtual void SetLocalDescription(const string sdp,
                                   const string type,
                                   OnSetSdpSuccess success,
                                   OnSetSdpFailure failure) = 0;

  virtual void SetRemoteDescription(const string sdp,
                                    const string type,
                                    OnSetSdpSuccess success,
                                    OnSetSdpFailure failure) = 0;

  virtual void GetLocalDescription(OnGetSdpSuccess success,
                                   OnGetSdpFailure failure) = 0;

  virtual void GetRemoteDescription(OnGetSdpSuccess success,
                                    OnGetSdpFailure failure) = 0;

  virtual void AddCandidate(const string mid,
                            int mid_mline_index,
                            const string candiate) = 0;

  virtual void RegisterRTCPeerConnectionObserver(
      RTCPeerConnectionObserver* observer) = 0;

  virtual void DeRegisterRTCPeerConnectionObserver() = 0;

  virtual vector<scoped_refptr<RTCMediaStream>> local_streams() = 0;

  virtual vector<scoped_refptr<RTCMediaStream>> remote_streams() = 0;

  virtual bool GetStats(const RTCAudioTrack* track,
                        scoped_refptr<TrackStatsObserver> observer) = 0;

  virtual bool GetStats(const RTCVideoTrack* track,
                        scoped_refptr<TrackStatsObserver> observer) = 0;

  virtual scoped_refptr<RTCRtpTransceiver> AddTransceiver(
      scoped_refptr<RTCMediaTrack> track,
      scoped_refptr<RTCRtpTransceiverInit> init) = 0;

  virtual scoped_refptr<RTCRtpTransceiver> AddTransceiver(
      scoped_refptr<RTCMediaTrack> track) = 0;

  virtual scoped_refptr<RTCRtpSender> AddTrack(
      scoped_refptr<RTCMediaTrack> track,
      const vector<string> streamIds) = 0;

  virtual bool RemoveTrack(scoped_refptr<RTCRtpSender> render) = 0;

  virtual vector<scoped_refptr<RTCRtpSender>> senders() = 0;

  virtual vector<scoped_refptr<RTCRtpTransceiver>> transceivers() = 0;

  virtual vector<scoped_refptr<RTCRtpReceiver>> receivers() = 0;

 protected:
  virtual ~RTCPeerConnection() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_PEERCONNECTION_HXX
