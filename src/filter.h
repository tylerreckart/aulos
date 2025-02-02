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
#pragma once

#include <vector>

class BiquadFilter
{
  public:
    BiquadFilter();
    void Reset();
    void SetBandPass(float sr, float centerFreq, float bandwidth);
    float Process(float x);

  private:
    float b0_, b1_, b2_;
    float a1_, a2_;
    float z1_, z2_;
    void Normalize();
};

// -------------------------------------------------
// FormantFilter
// -------------------------------------------------
class FormantFilter
{
  public:
    FormantFilter(size_t numFormants = 3);
    void Init(float sr);
    void SetFreq(float f);
    void SetBandwidth(float bw);
    void SetAmp(float a);
    void SetResonance(float r);
    float Process(float x);

  private:
    float samplerate_;
    float amp_;
    float resonance_;
    std::vector<BiquadFilter> filters_;
    std::vector<float> freq_;
    std::vector<float> bw_;
    void UpdateFilters();
};

extern FormantFilter formant_filter;

extern float formant_freq;
extern float formant_bw;
extern float formant_amp;
extern float formant_resonance;
