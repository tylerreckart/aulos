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
#include "mux.h"
#include "control.h"

using namespace daisy;

// GPIO handles for S0-S3
dsy_gpio mux_s0, mux_s1, mux_s2, mux_s3;

const Pin MUX_S0 = seed::D2;
const Pin MUX_S1 = seed::D3;
const Pin MUX_S2 = seed::D4;
const Pin MUX_S3 = seed::D5;

const Pin MUX1_ADC = seed::A0; // Pots
const Pin MUX2_ADC = seed::A1; // CV

void InitMultiplexerPins()
{
    mux_s0.pin  = MUX_S0;
    mux_s1.pin  = MUX_S1;
    mux_s2.pin  = MUX_S2;
    mux_s3.pin  = MUX_S3;

    mux_s0.mode = DSY_GPIO_MODE_OUTPUT_PP;
    mux_s1.mode = DSY_GPIO_MODE_OUTPUT_PP;
    mux_s2.mode = DSY_GPIO_MODE_OUTPUT_PP;
    mux_s3.mode = DSY_GPIO_MODE_OUTPUT_PP;

    dsy_gpio_init(&mux_s0);
    dsy_gpio_init(&mux_s1);
    dsy_gpio_init(&mux_s2);
    dsy_gpio_init(&mux_s3);
}

void ReadMultiplexers(DaisySeed &hw)
{
    for(int i = 0; i < 16; i++)
    {
        // Set MUX selection pins
        dsy_gpio_write(&mux_s0, (i & 0x01));
        dsy_gpio_write(&mux_s1, (i & 0x02) >> 1);
        dsy_gpio_write(&mux_s2, (i & 0x04) >> 2);
        dsy_gpio_write(&mux_s3, (i & 0x08) >> 3);

        // Read pot ADC if in range
        if(i < NUM_POTS)
        {
            float new_value = hw.adc.GetFloat(0);
            pot_values[i] = pot_values[i] * (1.f - SMOOTHING_FACTOR)
                          + new_value * SMOOTHING_FACTOR;
        }

        // Read CV ADC if in range
        if(i < NUM_CV)
        {
            float new_value = hw.adc.GetFloat(1);
            cv_values[i] = cv_values[i] * (1.f - SMOOTHING_FACTOR)
                         + new_value * SMOOTHING_FACTOR;
        }
    }
}
