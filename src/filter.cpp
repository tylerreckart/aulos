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
#include "filter.h"
#include <cmath>

static constexpr float TWO_PI = 6.28318530717959f;

// --------------------- BiquadFilter ---------------------
BiquadFilter::BiquadFilter()
{
    Reset();
}

void BiquadFilter::Reset()
{
    b0_ = b1_ = b2_ = 0.f;
    a1_ = a2_ = 0.f;
    z1_ = z2_ = 0.f;
}

void BiquadFilter::SetBandPass(float sr, float centerFreq, float bandwidth)
{
    if(centerFreq < 1.f)
        centerFreq = 1.f;
    if(centerFreq > sr * 0.49f)
        centerFreq = sr * 0.49f;
    if(bandwidth <= 0.f)
        bandwidth = 0.01f;

    float omega = TWO_PI * centerFreq / sr;
    float alpha = sinf(omega)
        * sinhf(M_LN2 / 2.f * bandwidth * omega / sinf(omega));
    float cosw = cosf(omega);

    b0_ = alpha;
    b1_ = 0.f;
    b2_ = -alpha;
    a1_ = -2.f * cosw;
    a2_ = 1.f - alpha;
    Normalize();
}

float BiquadFilter::Process(float x)
{
    float y = b0_ * x + z1_;
    z1_ = b1_ * x - a1_ * y + z2_;
    z2_ = b2_ * x - a2_ * y;
    return y;
}

void BiquadFilter::Normalize()
{
    float norm = 1.f + a1_ + a2_;
    if(norm > 0.f)
    {
        b0_ /= norm;
        b1_ /= norm;
        b2_ /= norm;
        a1_ /= norm;
        a2_ /= norm;
    }
}

// --------------------- FormantFilter ---------------------
FormantFilter formant_filter;
float formant_freq      = 500.f;
float formant_bw        = 100.f;
float formant_amp       = 1.f;
float formant_resonance = 1.f;

FormantFilter::FormantFilter(size_t numFormants)
{
    if(numFormants < 1)
        numFormants = 3;
    filters_.resize(numFormants);
    freq_.resize(numFormants, 500.f);
    bw_.resize(numFormants, 100.f);
    samplerate_ = 48000.f;
    amp_        = 1.f;
    resonance_  = 1.f;
    UpdateFilters();
}

void FormantFilter::Init(float sr)
{
    samplerate_ = sr;
    UpdateFilters();
}

void FormantFilter::SetFreq(float f)
{
    for(size_t i = 0; i < filters_.size(); i++)
        freq_[i] = f;
    UpdateFilters();
}

void FormantFilter::SetBandwidth(float bw)
{
    for(size_t i = 0; i < filters_.size(); i++)
        bw_[i] = bw;
    UpdateFilters();
}

void FormantFilter::SetAmp(float a)
{
    amp_ = a;
}

void FormantFilter::SetResonance(float r)
{
    resonance_ = r;
    UpdateFilters();
}

float FormantFilter::Process(float x)
{
    float y = x;
    for(size_t i = 0; i < filters_.size(); i++)
        y = filters_[i].Process(y);
    return y * amp_;
}

void FormantFilter::UpdateFilters()
{
    for(size_t i = 0; i < filters_.size(); i++)
    {
        float adjustedBW = bw_[i] / resonance_;
        filters_[i].SetBandPass(samplerate_, freq_[i], adjustedBW);
    }
}
