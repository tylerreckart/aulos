// ----------------------------------------------------------------------------
// Copyright 2025 Tyler Reckart
//
// Author: Tyler Reckart (tyler.reckart@gmail.com)
// ----------------------------------------------------------------------------
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
// ----------------------------------------------------------------------------
#include "daisy_seed.h"
#include "daisysp.h"

#include "mux.h"
#include "filter.h"
#include "osc.h"
#include "control.h"
#include "env.h"

using namespace daisy;
using namespace daisysp;

// Global hardware object
DaisySeed hw;

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size);

FormantFilter osc1_formant_filter;
float osc1_formant_freq      = 500.0f;  // Default center frequency (Hz)
float osc1_formant_bw        = 100.0f;  // Default bandwidth (Hz)
float osc1_formant_amp       = 1.0f;    // Default amplitude (gain factor)
float osc1_formant_resonance = 0.5f;    // Resonance factor (normalized)

FormantFilter osc2_formant_filter;
float osc2_formant_freq      = 700.0f;  // Default center frequency (Hz)
float osc2_formant_bw        = 120.0f;  // Default bandwidth (Hz)
float osc2_formant_amp       = 1.0f;    // Default amplitude (gain factor)
float osc2_formant_resonance = 0.5f;    // Resonance factor (normalized)


int main(void)
{
    // Initialize Daisy Seed
    hw.Init();
    float sr = hw.AudioSampleRate();

    // Init multiplexer pins
    InitMultiplexerPins();

    // Configure two ADC channels as single inputs
    AdcChannelConfig adc_cfg[2];
    adc_cfg[0].InitSingle(MUX1_ADC);
    adc_cfg[1].InitSingle(MUX2_ADC);
    hw.adc.Init(adc_cfg, 2);
    hw.adc.Start();

    // Init formant filters
    osc1_formant_filter.Init(sr);
    osc1_formant_filter.SetFreq(osc1_formant_freq);
    osc1_formant_filter.SetBandwidth(osc1_formant_bw);
    osc1_formant_filter.SetAmp(osc1_formant_amp);
    osc1_formant_filter.SetResonance(osc1_formant_resonance);

    osc2_formant_filter.Init(sr);
    osc2_formant_filter.SetFreq(osc2_formant_freq);
    osc2_formant_filter.SetBandwidth(osc2_formant_bw);
    osc2_formant_filter.SetAmp(osc2_formant_amp);
    osc2_formant_filter.SetResonance(osc2_formant_resonance);

    // Init oscillators
    InitOscillatorArrays(sr);

    // Init ADSR envelopes
    osc1_env.Init(sr);
    osc1_env.SetTime(ADSR_SEG_ATTACK, 0.01f); // Quick attack
    osc1_env.SetTime(ADSR_SEG_DECAY,  0.1f);
    osc1_env.SetTime(ADSR_SEG_RELEASE, 0.5f);
    osc1_env.SetSustainLevel(0.7f);

    osc2_env.Init(sr);
    osc2_env.SetTime(ADSR_SEG_ATTACK, 0.01f);
    osc2_env.SetTime(ADSR_SEG_DECAY,  0.1f);
    osc2_env.SetTime(ADSR_SEG_RELEASE, 0.5f);
    osc2_env.SetSustainLevel(0.7f);

    // Start audio
    hw.StartAudio(AudioCallback);

    // Infinite loop (all real-time work happens in the AudioCallback)
    while(1) {}
}

// -------------------------------------------------
// AudioCallback
// -------------------------------------------------
static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    // Update hardware pots/CVs
    UpdateControls(hw);

    // Set oscillator frequency directly in Hz
    for(int i = 0; i < TOTAL_OSCS; i++)
    {
        float freq = (i == 0) ? osc1_root_freq : (osc1_root_freq / (i + 1.0f));
        osc1_sine[i].SetFreq(freq);
        osc1_saw[i].SetFreq(freq);
    }

    static const float sub_weights[TOTAL_OSCS] = {1.0f, 0.4f, 0.3f, 0.2f, 0.1f};

    for(size_t n = 0; n < size; n++)
    {
        // Apply envelope-FIXME by implementing trigger/open gate logic
        float osc1_env_amp = osc1_env.Process(true);
        float osc2_env_amp = osc2_env.Process(true);

        // Sum up oscs with subharmonics, crossfade by morph factors
        float osc1_out = 0.f;
        float osc2_out = 0.f;
        for(int i = 0; i < TOTAL_OSCS; i++)
        {
            float s1_sin = osc1_sine[i].Process();
            float s1_saw = osc1_saw[i].Process();
            float s2_sin = osc2_sine[i].Process();
            float s2_sqr = osc2_square[i].Process();
            // crossfade
            float s1_mix = (1.f - osc1_morph) * s1_sin + (osc1_morph) * s1_saw;
            float s2_mix = (1.f - osc2_morph) * s2_sin + (osc2_morph) * s2_sqr;
            // apply sub weighting
            s1_mix *= sub_weights[i];
            s2_mix *= sub_weights[i];
            osc1_out += s1_mix;
            osc2_out += s2_mix;
        }

        // Apply envelope and volume
        osc1_out *= osc1_env_amp * osc1_volume;
        osc2_out *= osc2_env_amp * osc2_volume;

        // Stereo out- FIXME
        out[0][n] = osc1_out;
        out[1][n] = osc2_out;
    }
}
