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

using namespace daisy;
using namespace daisysp;

// Global hardware object
DaisySeed hw;

// Forward declaration of our AudioCallback:
static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size);

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

    // Init formant filter
    formant_filter.Init(sr);
    formant_filter.SetFreq(formant_freq);
    formant_filter.SetBandwidth(formant_bw);
    formant_filter.SetAmp(formant_amp);
    formant_filter.SetResonance(formant_resonance);

    // Init oscillators
    InitOscillatorArrays(sr);

    // Start audio
    hw.StartAudio(AudioCallback);

    // Infinite loop (all real-time work happens in the AudioCallback)
    while(1) {}

    return 0; // not reached
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

    // Convert root MIDI note to frequency
    float main_freq = MidiToFreq(osc1_root_note);

    // Set frequencies for main + subharmonics
    for(int i = 0; i < TOTAL_OSCS; i++)
    {
        float freq = (i == 0) ? main_freq : (main_freq / (i + 1.0f));
        osc1_sine[i].SetFreq(freq);
        osc1_saw[i].SetFreq(freq);
    }

    static const float sub_weights[TOTAL_OSCS] = {1.0f, 0.4f, 0.3f, 0.2f, 0.1f};

    for(size_t n = 0; n < size; n++)
    {
        // Sum up sine + saw (with subharmonics), crossfade by osc1_morph
        float mono_out = 0.f;
        for(int i = 0; i < TOTAL_OSCS; i++)
        {
            float s_sin = osc1_sine[i].Process();
            float s_saw = osc1_saw[i].Process();
            // crossfade
            float s_mix = (1.f - osc1_morph) * s_sin + (osc1_morph) * s_saw;
            // apply sub weighting
            s_mix *= sub_weights[i];
            mono_out += s_mix;
        }

        // overall volume
        mono_out *= osc1_volume;

        // formant filtering
        float filtered = formant_filter.Process(mono_out);

        // optional clamp
        if(filtered > 1.f)
            filtered = 1.f;
        else if(filtered < -1.f)
            filtered = -1.f;

        // stereo out
        out[0][n] = filtered;
        out[1][n] = filtered;
    }
}
