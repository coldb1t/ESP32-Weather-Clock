#pragma once

class BuzzerModule
{
public:
  explicit constexpr BuzzerModule(const uint8_t pin, const uint8_t channel = 0u)
      : pin(pin), channel(channel) {}

  inline void setup(const uint32_t freq_hz = 2000u, const uint8_t resolution_bits = 8u)
  {
    ledcSetup(channel, freq_hz, resolution_bits);
    ledcAttachPin(pin, channel);
    ledcWrite(channel, 0u);
    current_duty = 0u;
  }

  inline void play_tone(const uint32_t duty = 128u)
  {
    if (current_duty == duty)
    {
      return;
    }

    ledcWrite(channel, duty);
    current_duty = duty;
  }

  inline void stop_tone()
  {
    play_tone(0u);
  }

private:
  uint8_t pin = 0u;
  uint8_t channel = 0u;
  uint32_t current_duty = 0u;
};