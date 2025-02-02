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
#include "osc.h"
#include <cstdlib>  // rand()
#include <cmath>    // M_PI

daisysp::Oscillator osc1_sine[TOTAL_OSCS];
daisysp::Oscillator osc1_saw[TOTAL_OSCS];

int   osc1_root_note = 60;
float osc1_morph     = 0.0f;
float osc1_volume    = 0.8f;

void InitOscillator(daisysp::Oscillator &osc, float samplerate, uint8_t wave)
{
    osc.Init(samplerate);
    osc.SetAmp(0.5f);
    osc.SetWaveform(wave); // e.g., Oscillator::WAVE_SIN
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
    }
}
