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
#include "daisysp.h"
#include "osc.h"
#include "env.h"
#include <cmath>

daisysp::Oscillator osc1_sine[TOTAL_OSCS];
daisysp::Oscillator osc1_saw[TOTAL_OSCS];

daisysp::Oscillator osc2_sine[TOTAL_OSCS];
daisysp::Oscillator osc2_square[TOTAL_OSCS];

float osc1_root_freq = 440.0f;    // root frequency
float osc1_morph     = 0.0f;      // 0 = sine, 1 = saw
float osc1_volume    = 0.8f;      // 0 - 1

float osc2_root_freq = 440.0f;    // root freq
float osc2_morph     = 0.0f;      // 0 = sine, 1 = saw
float osc2_volume    = 0.8f;      // 0 - 1

void InitOscillator(daisysp::Oscillator &osc, float samplerate, uint8_t wave)
{
    osc.Init(samplerate);
    osc.SetAmp(0.5f);
    osc.SetWaveform(wave);
    float random_phase = static_cast<float>(rand()) / RAND_MAX;
    osc.PhaseAdd(random_phase * 2.0f * M_PI);
}

void InitOscillatorArrays(float sr)
{
    using namespace daisysp;
    for(int i = 0; i < TOTAL_OSCS; i++)
    {
        InitOscillator(osc1_sine[i], sr, Oscillator::WAVE_SIN);
        InitOscillator(osc1_saw[i],  sr, Oscillator::WAVE_SAW);
        InitOscillator(osc2_sine[i], sr, Oscillator::WAVE_SIN);
        InitOscillator(osc2_square[i],  sr, Oscillator::WAVE_SQUARE);
    }
}

void UpdateOscillatorFrequencies()
{
    for(int i = 0; i < TOTAL_OSCS; i++)
    {
        // osc1
        float osc1_freq = (i == 0) ? osc1_root_freq : (osc1_root_freq / (i + 1));
        osc1_sine[i].SetFreq(osc1_freq);
        osc1_saw[i].SetFreq(osc1_freq);
        // osc2
        float osc2_freq = (i == 0) ? osc1_root_freq : (osc1_root_freq / (i + 1));
        osc2_sine[i].SetFreq(osc2_freq);
        osc2_square[i].SetFreq(osc2_freq);
    }
}
