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

#include "daisy_seed.h"

// MUX pin definitions
extern const daisy::Pin MUX_S0;
extern const daisy::Pin MUX_S1;
extern const daisy::Pin MUX_S2;
extern const daisy::Pin MUX_S3;

extern const daisy::Pin MUX1_ADC; // Pots
extern const daisy::Pin MUX2_ADC; // CV

// Function to initialize the multiplexer pins
void InitMultiplexerPins();

// Function to read multiplexer channels
void ReadMultiplexers(daisy::DaisySeed &hw);
