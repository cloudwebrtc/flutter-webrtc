#ifndef LIB_WEBRTC_RTC_AUDIO_DEVICE_HXX
#define LIB_WEBRTC_RTC_AUDIO_DEVICE_HXX

#include "rtc_types.h"

namespace libwebrtc {

class RTCAudioDevice : public RefCountInterface {
 public:
  static const int kAdmMaxDeviceNameSize = 128;
  static const int kAdmMaxFileNameSize = 512;
  static const int kAdmMaxGuidSize = 128;

 public:
  // Device enumeration
  virtual int16_t PlayoutDevices() = 0;

  virtual int16_t RecordingDevices() = 0;

  virtual int32_t PlayoutDeviceName(uint16_t index,
                                    char name[kAdmMaxDeviceNameSize],
                                    char guid[kAdmMaxGuidSize]) = 0;

  virtual int32_t RecordingDeviceName(uint16_t index,
                                      char name[kAdmMaxDeviceNameSize],
                                      char guid[kAdmMaxGuidSize]) = 0;

  // Device selection
  virtual int32_t SetPlayoutDevice(uint16_t index) = 0;

  virtual int32_t SetRecordingDevice(uint16_t index) = 0;

 protected:
  virtual ~RTCAudioDevice() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_AUDIO_DEVICE_HXX
