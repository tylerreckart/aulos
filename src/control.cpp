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
#include "env.h"

#include <cmath>

float pot_values[NUM_POTS] = {0};
float cv_values[NUM_CV]    = {0};

const float SMOOTHING_FACTOR = 0.1f;

float osc1_envelope_shape = 0.5f; // Envelope shape (0 to 1)
float osc2_envelope_shape = 0.5f; // Envelope shape (0 to 1)

void UpdateControls(daisy::DaisySeed &hw)
{
    // Read from multiplexers into pot_values[], cv_values[]
    ReadMultiplexers(hw);

    // osc1
    float k0 = pot_values[0]; // Root frequency
    float k1 = pot_values[1]; // Morph
    float k2 = pot_values[2]; // Formant frequency
    float k3 = pot_values[3]; // Formant bandwidth
    float k4 = pot_values[4]; // Formant resonance
    float k5 = pot_values[5]; // Envelope
    // osc2
    float k6 = pot_values[0]; // Root frequency
    float k7 = pot_values[1]; // Morph
    float k8 = pot_values[2]; // Formant frequency
    float k9 = pot_values[3]; // Formant bandwidth
    float k10 = pot_values[4]; // Formant resonance
    float k11 = pot_values[5]; // Envelope

    // Root frequency in Hz (20Hz - 5000Hz)
    {
        float minF = 20.f;
        float maxF = 5000.f;
        // osc1
        osc1_root_freq = minF + (maxF - minF) * k0;
        // osc2
        osc2_root_freq = minF + (maxF - minF) * k6;
    }

    // Morph (0 to 1)
    {
        // osc1
        osc1_morph = k1;
        osc1_morph = fmaxf(0.f, fminf(osc1_morph, 1.f));
        // osc2
        osc2_morph = k7;
        osc2_morph = fmaxf(0.f, fminf(osc2_morph, 1.f));
    }

    // Formant Frequency (100Hz - 5000Hz)
    {
        float minF = 100.f;
        float maxF = 5000.f;
        // osc1
        osc1_formant_freq = minF + (maxF - minF) * k2;
        osc1_formant_filter.SetFreq(osc1_formant_freq);
        // osc2
        osc2_formant_freq = minF + (maxF - minF) * k8;
        osc2_formant_filter.SetFreq(osc2_formant_freq);
    }

    // Formant Bandwidth (50Hz - 1000Hz)
    {
        float minBW = 50.f;
        float maxBW = 1000.f;
        // osc1
        osc1_formant_bw = minBW + (maxBW - minBW) * k3;
        osc1_formant_filter.SetBandwidth(osc1_formant_bw);
        // osc2
        osc2_formant_bw = minBW + (maxBW - minBW) * k9;
        osc2_formant_filter.SetBandwidth(osc2_formant_bw);
    }

    // Resonance Factor (1.0 - 10.0)
    {
        float minR = 1.0f;
        float maxR = 10.0f;
        // osc1
        osc1_formant_resonance = minR + (maxR - minR) * k4;
        osc1_formant_filter.SetResonance(osc1_formant_resonance);
        // osc2
        osc2_formant_resonance = minR + (maxR - minR) * k10;
        osc2_formant_filter.SetResonance(osc2_formant_resonance);
    }

    // Envelope Shape (0 to 1)
    {
        // osc1
        osc1_envelope_shape = k5;
        osc1_envelope_shape = fmaxf(0.f, fminf(osc1_envelope_shape, 1.f));
        // osc2
        osc2_envelope_shape = k11;
        osc2_envelope_shape = fmaxf(0.f, fminf(osc2_envelope_shape, 1.f));
    }
}