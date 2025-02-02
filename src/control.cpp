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
#include "control.h"
#include "mux.h"
#include "filter.h"
#include "osc.h"

#include <cmath>

float pot_values[NUM_POTS] = {0};
float cv_values[NUM_CV]    = {0};

const float SMOOTHING_FACTOR = 0.1f;

float MidiToFreq(int midinote)
{
    return 440.f * powf(2.f, (midinote - 69) / 12.f);
}

void UpdateControls(daisy::DaisySeed &hw)
{
    // 1) Read from multiplexers into pot_values[], cv_values[]
    ReadMultiplexers(hw);

    // 2) Map pots to parameters
    // Pot 0 -> MIDI note
    // Pot 1 -> morph
    // Pot 2 -> formant freq
    // Pot 3 -> formant BW
    // Pot 4 -> formant resonance
    float k0 = pot_values[0];
    float k1 = pot_values[1];
    float k2 = pot_values[2];
    float k3 = pot_values[3];
    float k4 = pot_values[4];

    // k0 -> MIDI note [0..127]
    osc1_root_note = static_cast<int>(k0 * 127.f);
    if(osc1_root_note < 0)   osc1_root_note = 0;
    if(osc1_root_note > 127) osc1_root_note = 127;

    // k1 -> morph [0..1]
    osc1_morph = k1;
    if(osc1_morph < 0.f) osc1_morph = 0.f;
    if(osc1_morph > 1.f) osc1_morph = 1.f;

    // k2 -> formant frequency (100..5000)
    {
        float minF = 100.f;
        float maxF = 5000.f;
        formant_freq = minF + (maxF - minF) * k2;
        formant_filter.SetFreq(formant_freq);
    }

    // k3 -> formant bandwidth (50..1000)
    {
        float minBW = 50.f;
        float maxBW = 1000.f;
        formant_bw = minBW + (maxBW - minBW) * k3;
        formant_filter.SetBandwidth(formant_bw);
    }

    // k4 -> resonance factor [1..10]
    {
        float minR = 1.0f;
        float maxR = 10.0f;
        formant_resonance = minR + (maxR - minR) * k4;
        formant_filter.SetResonance(formant_resonance);
    }
}
