#pragma once

#include <Arduino.h>
#include <AudioTools.h>
#include <AudioTools/AudioCodecs/CodecADPCM.h>
#include <driver/dac.h>
#include <esp_task_wdt.h>
#include <AfskDemodulator.h>
#include <math.h>
#include "globals.h"
#include "protocol.h"
#include "debug.h"
#include "dsp/audioResampler.h"
#include "dsp/softSquelchEffect.h"

class SerialOutput : public AudioOutput {
public:
  size_t write(const uint8_t *data, size_t len) override {
    if (len > 0) {
      if (len > PROTO_MTU) {
        len = PROTO_MTU;
      }
      if (mode == MODE_RX) {
        sendAudio((uint8_t*)data, len);
      }
      return len;
    }
    return len;
  } 
};

#define DECAY_TIME 0.25  // seconds

class DCOffsetRemover : public AudioEffect {
public:
  DCOffsetRemover(float decay_time = 0.25f, float sample_rate = AUDIO_SAMPLE_RATE): prev_y(0.0f) {
    alpha = 1.0f - expf(-1.0f / (sample_rate * (decay_time / logf(2.0f))));
  }
  DCOffsetRemover(const DCOffsetRemover &) = default;
  effect_t process(effect_t input) {
    return active() ? remove_dc(input) : input;
  }
  DCOffsetRemover *clone() override {
    return new DCOffsetRemover(*this);
  }
private:
  float prev_y;
  float alpha;
  int16_t remove_dc(int16_t x) {
    prev_y = alpha * x + (1.0f - alpha) * prev_y;
    return x - (int16_t)prev_y;
  }
};

static void onAfskPacketDecoded(const uint8_t *frame, size_t len) {
  if (frame && len > 0) {
    pulseAprsRxLED();
    sendAx25Packet(frame, len);
  }
}

AfskDemodulator afskDemod(AUDIO_SAMPLE_RATE, 2, onAfskPacketDecoded);

class AfskTapEffect : public AudioEffect {
public:
  AfskTapEffect *clone() override {
    return new AfskTapEffect(*this);
  }

  effect_t process(effect_t input) {
    if (active()) {
      samples[sampleCount++] = (int16_t)input;
      if (sampleCount >= AFSK_TAP_BUFFER_SAMPLES) {
        afskDemod.processSamples(samples, sampleCount);
        sampleCount = 0;
      }
    }
    return input;
  }

  void flush() {
    if (sampleCount > 0) {
      afskDemod.processSamples(samples, sampleCount);
      sampleCount = 0;
    }
    afskDemod.flush();
  }

private:
  static const size_t AFSK_TAP_BUFFER_SAMPLES = 256;
  int16_t samples[AFSK_TAP_BUFFER_SAMPLES];
  size_t sampleCount = 0;
};

bool rxStreamConfigured = false;
AnalogAudioStream in;
AudioInfo rxInfo(AUDIO_SAMPLE_RATE, 1, 16);
AudioInfo rxAudioInfo(AUDIO_WIRE_SAMPLE_RATE, 1, 16);
SerialOutput rxAudioOutput;
ADPCMEncoder rxAdpcmEncoder(AV_CODEC_ID_ADPCM_IMA_WAV, AUDIO_FRAME_BYTES);
EncodedAudioStream rxOut(&rxAudioOutput, &rxAdpcmEncoder);
AudioDownsampleConverter rxDownsample;
AudioEffectStream effects(in);  
ConverterStream<int16_t> rxDownsampledEffects(effects, rxDownsample);
StreamCopy rxCopier(rxOut, rxDownsampledEffects, AUDIO_FRAME_SAMPLES_48K * sizeof(int16_t));
Boost mute(0.0);
Boost gain(16.0);
DCOffsetRemover dcOffsetRemover(DECAY_TIME, AUDIO_SAMPLE_RATE);
AfskTapEffect afskTapEffect;
SoftSquelchEffect softSquelchEffect(AUDIO_SAMPLE_RATE, ZCR_DECAY_TIME, SQ_CLOSE_DELAY);

inline void injectADCBias() {
  dac_output_enable(DAC_CHANNEL_2);  // GPIO26 (DAC1)
  dac_output_voltage(DAC_CHANNEL_2, (255.0 / 3.3) * hw.adcBias);
} 

inline void setUpADCAttenuator() {
  adc1_config_channel_atten(I2S_ADC_CHANNEL, hw.adcAttenuation);
}

void initI2SRx() {
  if (rxStreamConfigured) {
    return;
  }
  injectADCBias();
  setUpADCAttenuator();
  //AudioToolsLogger.begin(debugPrinter, AudioToolsLogLevel::Debug);
  auto config = in.defaultConfig(RX_MODE);
  config.copyFrom(rxInfo);
  config.is_auto_center_read = false; // We use dcOffsetRemover instead
  config.use_apll = true;
  // AudioTools 1.0.3 / AnalogConfigESP32V1 no longer exposes auto_clear.
  // DC offset handling is already disabled above with is_auto_center_read=false.
  config.adc_pin = hw.pins.pinAudioIn;
  config.sample_rate = AUDIO_SAMPLE_RATE * 1.00;
  in.begin(config);
  // effects
  effects.clear();
  afskTapEffect.setActive(true);
  effects.addEffect(dcOffsetRemover);
  effects.addEffect(gain);
  effects.addEffect(afskTapEffect);
  effects.addEffect(softSquelchEffect);
  effects.addEffect(mute);
  effects.begin(rxInfo);
  // open output
  rxDownsample.begin();
  rxOut.begin(rxAudioInfo);
  rxCopier.setMinCopySize(sizeof(int16_t));
  rxCopier.setCheckAvailable(false);
  rxStreamConfigured = true;
}

void endI2SRx() {
  if (rxStreamConfigured) {
    afskTapEffect.flush();
    rxOut.end();
    effects.end();
    in.end();
  }
  rxStreamConfigured = false;
}
  
void rxAudioLoop() {
  softSquelchEffect.setHardwareSquelched(digitalRead(hw.pins.pinSq) == HIGH);
  if (mode == MODE_RX || mode == MODE_STOPPED) {
    mute.setActive(squelched);
    rxCopier.copy();
    esp_task_wdt_reset();
  }
}