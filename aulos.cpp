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
#include "daisy_patch.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

// --------------------- Oscillators (Main + Subharmonics) ---------------------
Oscillator osc1, osc1_sub1, osc1_sub2, osc1_sub3, osc1_sub4;
Oscillator osc2, osc2_sub1, osc2_sub2, osc2_sub3, osc2_sub4;

// --------------------- Scale Data ---------------------
static constexpr int NUM_SCALES = 10;
const char* scale_names[NUM_SCALES] = {
    "chromatic", "major",    "minor",    "pentatonic", "blues",
    "phrygian",  "lydian",   "mixolydian", "arabic",   "japanese",
};

const int scales[NUM_SCALES][7] = {
    {0, 1, 2, 3, 4, 5, 6},   // chromatic
    {0, 2, 4, 5, 7, 9, 11},  // major
    {0, 2, 3, 5, 7, 8, 10},  // minor
    {0, 2, 4, 7, 9, -1, -1}, // pentatonic
    {0, 3, 5, 6, 7, 10, -1}, // blues
    {0, 1, 3, 5, 7, 8, 10},  // phrygian
    {0, 2, 4, 6, 7, 9, 11},  // lydian
    {0, 2, 3, 5, 7, 9, 11},  // mixolydian
    {0, 1, 4, 5, 7, 8, 11},  // arabic
    {0, 1, 5, 7, 8, 10, -1}, // japanese
};

// Scale selection for each oscillator
int selected_scale = 0;

// --------------------- Wave Data ---------------------
static constexpr int NUM_WAVES = 5;
const char* wave_names[NUM_WAVES] = {
    "sine", "tri", "saw", "ramp", "square"};

int waveform1 = 0; // Waveform for oscillator 1
int waveform2 = 0; // Waveform for oscillator 2

// --------------------- Pitch Control ---------------------
int osc1_root_note = 60; // MIDI note for oscillator 1 (C3)
int osc2_root_note = 24; // MIDI note for oscillator 2 (C1)

float pitch_main  = 0.f; 
float pitch_sub[4];

float pitch_bass  = 0.f; 
float pitch_bsub[4];

float osc1_volume = 0.8f;
float osc2_volume = 0.8f;

// --------------------- Quantization Control ---------------------
bool quantize_enabled = true;

// --------------------- Encoder Mode ---------------------
enum class EncoderMode
{
    WAVEFORM_1, // Change waveform of oscillator 1
    WAVEFORM_2, // Change waveform of oscillator 2
    SCALE,      // Change scale
    QUANTIZE,   // Toggle quantization
};

static EncoderMode encoder_mode = EncoderMode::WAVEFORM_1;

// -------------------------------------------------
// Quantize a MIDI note to the selected scale
// -------------------------------------------------
int QuantizeToScale(int note, int scale_index)
{
    int octave         = note / 12;
    int note_in_octave = note % 12;
    const int* scale   = scales[scale_index];

    for (int i = 0; i < 7 && scale[i] != -1; i++)
    {
        if (note_in_octave <= scale[i])
            return octave * 12 + scale[i];
    }
    // If we didn't find a scale note in this octave, wrap up to the next
    return (octave + 1) * 12 + scale[0];
}

// -------------------------------------------------
// Update the OLED display
// -------------------------------------------------
void UpdateOled()
{
    patch.display.Fill(false);

    // Display current Encoder Mode
    patch.display.SetCursor(0, 0);
    patch.display.WriteString("mode: ", Font_6x8, true);
    switch (encoder_mode)
    {
    case EncoderMode::WAVEFORM_1:
        patch.display.WriteString("osc1 waveform", Font_6x8, true);
        break;
    case EncoderMode::WAVEFORM_2:
        patch.display.WriteString("osc2 waveform", Font_6x8, true);
        break;
    case EncoderMode::QUANTIZE:
        patch.display.WriteString("quant", Font_6x8, true);
        break;
    case EncoderMode::SCALE:
        patch.display.WriteString("scale", Font_6x8, true);
        break;
    }

    // Show OSC1 info
    patch.display.SetCursor(0, 10);
    patch.display.WriteString("osc1: ", Font_6x8, true);
    patch.display.WriteString(wave_names[waveform1], Font_6x8, true);

    // Show OSC2 info
    patch.display.SetCursor(0, 20);
    patch.display.WriteString("osc2: ", Font_6x8, true);
    patch.display.WriteString(wave_names[waveform2], Font_6x8, true);

    // Quantize status
    patch.display.SetCursor(0, 30);
    patch.display.WriteString("quant: ", Font_6x8, true);
    patch.display.WriteString(quantize_enabled ? "on " : "off", Font_6x8, true);

    // If quant is enabled, show selected scale
    if (quantize_enabled)
    {
        patch.display.SetCursor(0, 40);
        patch.display.WriteString("scale: ", Font_6x8, true);
        patch.display.WriteString(scale_names[selected_scale], Font_6x8, true);
    }

    patch.display.Update();
}

// -------------------------------------------------
// Update controls (knobs + encoder button)
// -------------------------------------------------
void UpdateControls()
{
    patch.ProcessDigitalControls();
    patch.ProcessAnalogControls();

    // Read knobs
    float knob1 = patch.GetKnobValue(DaisyPatch::CTRL_1);
    float knob2 = patch.GetKnobValue(DaisyPatch::CTRL_2);
    float knob3 = patch.GetKnobValue(DaisyPatch::CTRL_3);
    float knob4 = patch.GetKnobValue(DaisyPatch::CTRL_4);

    // Map knob1 -> Osc1 MIDI note  (0..127)
    osc1_root_note = static_cast<int>(knob1 * 127.f);

    // Map knob3 -> Osc2 MIDI note  (0..127)
    osc2_root_note = static_cast<int>(knob3 * 127.f);

    // Map knob2 -> Osc1 volume
    osc1_volume = knob2;
    // Map knob4 -> Osc2 volume
    osc2_volume = knob4;

    // Check encoder button press: cycle through 4 modes
    static bool old_pressed = false;
    bool       curr_pressed = patch.encoder.Pressed();
    if (curr_pressed && !old_pressed)
    {
        // Advance to next mode
        auto next_mode = static_cast<int>(encoder_mode) + 1;
        encoder_mode   = static_cast<EncoderMode>(next_mode % 4);
    }
    old_pressed = curr_pressed;

    // Check for encoder rotation
    int increment = patch.encoder.Increment();
    if (increment != 0)
    {
        switch (encoder_mode)
        {
        case EncoderMode::WAVEFORM_1:
            waveform1 += increment;
            waveform1 = (waveform1 + NUM_WAVES) % NUM_WAVES;
            break;

        case EncoderMode::WAVEFORM_2:
            waveform2 += increment;
            waveform2 = (waveform2 + NUM_WAVES) % NUM_WAVES;
            break;

        case EncoderMode::QUANTIZE:
            // Toggle on any rotation
            quantize_enabled = !quantize_enabled;
            break;

        case EncoderMode::SCALE:
            // Adjust scale index (whether quant is on or off, we allow cycling)
            selected_scale += increment;
            if (selected_scale < 0)
                selected_scale += NUM_SCALES;
            selected_scale = selected_scale % NUM_SCALES;
            break;
        }
    }
}

// -------------------------------------------------
// Initialize an oscillator with random phase
// -------------------------------------------------
void InitOscillator(Oscillator &osc, float amp, float samplerate)
{
    osc.Init(samplerate);
    osc.SetAmp(amp);
    float random_phase = static_cast<float>(rand()) / RAND_MAX; // random [0,1]
    osc.PhaseAdd(random_phase * 2.0f * M_PI);
}

// -------------------------------------------------
// UpdateOsc1: Set frequency/waveform for osc1 + subharmonics
// -------------------------------------------------
void UpdateOsc1(float pitch, int wave)
{
    // Compute subharmonic frequencies in a small loop
    for (int i = 0; i < 4; i++)
        pitch_sub[i] = pitch / float(i + 2);

    osc1.SetFreq(pitch);
    osc1_sub1.SetFreq(pitch_sub[0]);
    osc1_sub2.SetFreq(pitch_sub[1]);
    osc1_sub3.SetFreq(pitch_sub[2]);
    osc1_sub4.SetFreq(pitch_sub[3]);

    osc1.SetWaveform((uint8_t)wave);
    osc1_sub1.SetWaveform((uint8_t)wave);
    osc1_sub2.SetWaveform((uint8_t)wave);
    osc1_sub3.SetWaveform((uint8_t)wave);
    osc1_sub4.SetWaveform((uint8_t)wave);
}

// -------------------------------------------------
// UpdateOsc2: Set frequency/waveform for osc2 + subharmonics
// -------------------------------------------------
void UpdateOsc2(float pitch, int wave)
{
    // Compute subharmonic frequencies
    for (int i = 0; i < 4; i++)
        pitch_bsub[i] = pitch / float(i + 2);

    osc2.SetFreq(pitch);
    osc2_sub1.SetFreq(pitch_bsub[0]);
    osc2_sub2.SetFreq(pitch_bsub[1]);
    osc2_sub3.SetFreq(pitch_bsub[2]);
    osc2_sub4.SetFreq(pitch_bsub[3]);

    osc2.SetWaveform((uint8_t)wave);
    osc2_sub1.SetWaveform((uint8_t)wave);
    osc2_sub2.SetWaveform((uint8_t)wave);
    osc2_sub3.SetWaveform((uint8_t)wave);
    osc2_sub4.SetWaveform((uint8_t)wave);
}

// -------------------------------------------------
// Audio callback
// -------------------------------------------------
static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out,
                          size_t size)
{
    // Update controls once per audio block
    UpdateControls();

    // Apply quantization if enabled for osc1
    int main_note = quantize_enabled
                        ? QuantizeToScale(osc1_root_note, selected_scale)
                        : osc1_root_note;
    pitch_main = mtof(main_note);
    UpdateOsc1(pitch_main, waveform1);

    // Apply quantization if enabled for osc2
    int bass_note = quantize_enabled
                        ? QuantizeToScale(osc2_root_note, selected_scale)
                        : osc2_root_note;
    pitch_bass = mtof(bass_note);
    UpdateOsc2(pitch_bass, waveform2);

    // Render audio
    for (size_t i = 0; i < size; i++)
    {
        // Main + subharmonics
        float out_main = osc1.Process();
        float out_sub1 = osc1_sub1.Process();
        float out_sub2 = osc1_sub2.Process();
        float out_sub3 = osc1_sub3.Process();
        float out_sub4 = osc1_sub4.Process();

        // Bass + subharmonics
        float out_bass  = osc2.Process();
        float out_bsub1 = osc2_sub1.Process();
        float out_bsub2 = osc2_sub2.Process();
        float out_bsub3 = osc2_sub3.Process();
        float out_bsub4 = osc2_sub4.Process();

        // Mix volumes
        float osc1_left  = out_main * osc1_volume;
        float osc1_right = (0.4f * out_sub1 + 0.3f * out_sub2
                            + 0.2f * out_sub3 + 0.1f * out_sub4)
                            * osc1_volume;

        float osc2_left  = (0.4f * out_bsub1 + 0.3f * out_bsub2
                            + 0.2f * out_bsub3 + 0.1f * out_bsub4)
                            * osc2_volume;
        float osc2_right = out_bass * osc2_volume;

        // Prevent clipping by normalizing if needed
        float max_val = std::max({fabsf(osc1_left), fabsf(osc1_right),
                                  fabsf(osc2_left), fabsf(osc2_right)});
        if (max_val > 1.0f)
        {
            osc1_left  /= max_val;
            osc1_right /= max_val;
            osc2_left  /= max_val;
            osc2_right /= max_val;
        }

        // Write final audio to 4 channels
        out[0][i] = osc1_left;   // Channel 1 Left
        out[1][i] = osc1_right;  // Channel 1 Right
        out[2][i] = osc2_left;   // Channel 2 Left

        // Bug fix: use osc2_right here instead of osc1_right
        out[3][i] = osc2_right;  // Channel 2 Right
    }
}

// -------------------------------------------------
// Main
// -------------------------------------------------
int main(void)
{
    // Initialize hardware (Daisy Patch)
    patch.Init();
    float samplerate = patch.AudioSampleRate();

    // Initialize all oscillators with random phases
    InitOscillator(osc1,      0.8f, samplerate);
    InitOscillator(osc1_sub1, 0.8f, samplerate);
    InitOscillator(osc1_sub2, 0.8f, samplerate);
    InitOscillator(osc1_sub3, 0.8f, samplerate);
    InitOscillator(osc1_sub4, 0.8f, samplerate);

    InitOscillator(osc2,      0.8f, samplerate);
    InitOscillator(osc2_sub1, 0.8f, samplerate);
    InitOscillator(osc2_sub2, 0.8f, samplerate);
    InitOscillator(osc2_sub3, 0.8f, samplerate);
    InitOscillator(osc2_sub4, 0.8f, samplerate);

    // Start ADC (for knobs) and audio
    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    // Main loop: update the OLED
    while (1)
    {
        UpdateOled();
    }
}
