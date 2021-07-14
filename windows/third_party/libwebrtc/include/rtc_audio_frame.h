#ifndef AUDIO_FRAME_HXX
#define AUDIO_FRAME_HXX

#include "media_manager_types.h"

namespace b2bua {

class AudioFrame {
 public:
  MEDIA_MANAGER_API static AudioFrame* Create();
  MEDIA_MANAGER_API static AudioFrame* Create(int id,
                                              uint32_t timestamp,
                                              const int16_t* data,
                                              size_t samples_per_channel,
                                              int sample_rate_hz,
                                              size_t num_channels = 1);

  virtual void Release() = 0;

 public:
  virtual void UpdateFrame(int id,
                           uint32_t timestamp,
                           const int16_t* data,
                           size_t samples_per_channel,
                           int sample_rate_hz,
                           size_t num_channels = 1) = 0;

  virtual void CopyFrom(const AudioFrame& src) = 0;

  virtual void Add(const AudioFrame& frame_to_add) = 0;

  virtual void Mute() = 0;

  virtual const int16_t* data() = 0;

  virtual size_t samples_per_channel() = 0;

  virtual int sample_rate_hz() = 0;

  virtual size_t num_channels() = 0;

  virtual uint32_t timestamp() = 0;

  virtual int id() = 0;
};

};  // namespace b2bua

#endif
