#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

// --------------------- Oscillators / Filter ---------------------
Oscillator main_oscillator, osc_sub1, osc_sub2, osc_sub3, osc_sub4;
Svf formant_filter;

// --------------------- Scale Data ---------------------
static constexpr int NUM_SCALES = 16;
const char *scale_names[NUM_SCALES] = {
    "chromatic", "major", "minor", "pentatonic", "blues",
    "whole", "dorian", "phrygian", "lydian", "mixolydian",
    "locrian", "harm. min", "mel. min", "arabic", "gypsy",
    "japanese"};

const int scales[NUM_SCALES][7] = {
    {0, 1, 2, 3, 4, 5, 6},
    {0, 2, 4, 5, 7, 9, 11},
    {0, 2, 3, 5, 7, 8, 10},
    {0, 2, 4, 7, 9, -1, -1},
    {0, 3, 5, 6, 7, 10, -1},
    {0, 2, 4, 6, 8, 10, -1},
    {0, 2, 3, 5, 7, 9, 10},
    {0, 1, 3, 5, 7, 8, 10},
    {0, 2, 4, 6, 7, 9, 11},
    {0, 2, 4, 5, 7, 9, 10},
    {0, 1, 3, 5, 6, 8, 10},
    {0, 2, 3, 5, 7, 8, 11},
    {0, 2, 3, 5, 7, 9, 11},
    {0, 1, 4, 5, 7, 8, 11},
    {0, 1, 4, 5, 7, 8, 10},
    {0, 1, 5, 7, 8, 10, -1},
};

int selected_scale = 0;

// --------------------- Wave Data ---------------------
static constexpr int NUM_WAVES = 5;
const char *wave_names[NUM_WAVES] = {
    "sine", "triangle", "saw", "ramp", "square"};

// --------------------- Pitch / Filter / Etc. ---------------------
int root_note = 60; // C3
float pitch_main = 0.f;
float pitch_sub[4];

float volume = 0.8f;
float filter_freq = 1000.f;
int waveform = 0;

// --------------------- Encoder Mode ---------------------
enum class EncoderMode
{
    WAVEFORM,
    SCALE
};
static EncoderMode encoder_mode = EncoderMode::WAVEFORM;

// --------------------- Single-Channel Scope Buffer ---------------------
static constexpr size_t SCOPE_BUF_SIZE = 128;
static float scope_buffer[SCOPE_BUF_SIZE];
static size_t scope_idx = 0;

// -------------------------------------------------
// Quantize a MIDI note to the selected scale
// -------------------------------------------------
int QuantizeToScale(int note, int scale_index)
{
    int octave = note / 12;
    int note_in_octave = note % 12;
    const int *scale = scales[scale_index];

    for (int i = 0; i < 7 && scale[i] != -1; i++)
    {
        if (note_in_octave <= scale[i])
            return octave * 12 + scale[i];
    }
    // Wrap to next octave
    return (octave + 1) * 12 + scale[0];
}

// -------------------------------------------------
// Draw a single-channel scope (time-domain) in rows 20..63
// -------------------------------------------------
void DrawScope()
{
    int center_y = 42;
    float amplitude = 20.f; // scale [-1..+1] => 20 px

    for (size_t x = 0; x < (SCOPE_BUF_SIZE - 1); x++)
    {
        float s1 = scope_buffer[x];
        float s2 = scope_buffer[x + 1];

        // Convert from [-1..+1] => screen coords
        int y1 = static_cast<int>(center_y - s1 * amplitude);
        int y2 = static_cast<int>(center_y - s2 * amplitude);

        // clamp vertically to [20..63]
        if (y1 < 20)
            y1 = 20;
        if (y1 > 63)
            y1 = 63;
        if (y2 < 20)
            y2 = 20;
        if (y2 > 63)
            y2 = 63;

        // Draw line from (x, y1) to (x+1, y2)
        patch.display.DrawLine(x, y1, x + 1, y2, true);
    }
}

// -------------------------------------------------
// Update the display: two lines + single-channel scope
// -------------------------------------------------
void UpdateOled()
{
    patch.display.Fill(false);

    // -- Line 1: Show WAVEFORM, with '*' if that's the current mode
    patch.display.SetCursor(0, 0);
    if (encoder_mode == EncoderMode::WAVEFORM)
        patch.display.WriteString("*waveform: ", Font_6x8, true);
    else
        patch.display.WriteString(" waveform: ", Font_6x8, true);
    patch.display.WriteString(wave_names[waveform], Font_6x8, true);

    // -- Line 2: Show SCALE, with '*' if that's the current mode
    patch.display.SetCursor(0, 10);
    if (encoder_mode == EncoderMode::SCALE)
        patch.display.WriteString("*scale: ", Font_6x8, true);
    else
        patch.display.WriteString(" scale: ", Font_6x8, true);
    patch.display.WriteString(scale_names[selected_scale], Font_6x8, true);

    // Draw the single-channel scope below row=20
    DrawScope();

    patch.display.Update();
}

// -------------------------------------------------
// Update controls (knobs + encoder button)
// -------------------------------------------------
void UpdateControls()
{
    patch.ProcessDigitalControls();
    patch.ProcessAnalogControls();

    float knob1 = patch.GetKnobValue(DaisyPatch::CTRL_1);
    float knob2 = patch.GetKnobValue(DaisyPatch::CTRL_2);
    float knob3 = patch.GetKnobValue(DaisyPatch::CTRL_3);
    float knob4 = patch.GetKnobValue(DaisyPatch::CTRL_4);

    // root_note for pitch
    root_note = static_cast<int>(knob1 * 127.f);

    // Filter freq and resonance
    filter_freq = 200.f + (knob2 * 300.f);
    formant_filter.SetFreq(filter_freq);
    float res_val = knob3 * 0.95f; // limit to <1.0
    formant_filter.SetRes(res_val);

    // Volume
    volume = knob4;

    // Encoder button toggles mode
    static bool old_pressed = false;
    bool curr_pressed = patch.encoder.Pressed();
    if (curr_pressed && !old_pressed)
    {
        if (encoder_mode == EncoderMode::WAVEFORM)
            encoder_mode = EncoderMode::SCALE;
        else
            encoder_mode = EncoderMode::WAVEFORM;
    }
    old_pressed = curr_pressed;

    // Encoder turn
    int increment = patch.encoder.Increment();
    if (increment != 0)
    {
        if (encoder_mode == EncoderMode::WAVEFORM)
        {
            waveform += increment;
            waveform = (waveform + NUM_WAVES) % NUM_WAVES;
        }
        else
        {
            selected_scale += increment;
            selected_scale = (selected_scale + NUM_SCALES) % NUM_SCALES;
        }
    }
}

// -------------------------------------------------
// Audio callback
// -------------------------------------------------
static void AudioCallback(AudioHandle::InputBuffer in,
                          AudioHandle::OutputBuffer out,
                          size_t size)
{
    // Update controls once per block
    UpdateControls();

    // Quantize root note to scale
    int q_note = QuantizeToScale(root_note, selected_scale);
    pitch_main = mtof(q_note);

    // Subharmonics
    pitch_sub[0] = pitch_main / 2.f;
    pitch_sub[1] = pitch_main / 3.f;
    pitch_sub[2] = pitch_main / 4.f;
    pitch_sub[3] = pitch_main / 5.f;

    // Set oscillator freq/wave
    main_oscillator.SetFreq(pitch_main);
    osc_sub1.SetFreq(pitch_sub[0]);
    osc_sub2.SetFreq(pitch_sub[1]);
    osc_sub3.SetFreq(pitch_sub[2]);
    osc_sub4.SetFreq(pitch_sub[3]);

    main_oscillator.SetWaveform((uint8_t)waveform);
    osc_sub1.SetWaveform((uint8_t)waveform);
    osc_sub2.SetWaveform((uint8_t)waveform);
    osc_sub3.SetWaveform((uint8_t)waveform);
    osc_sub4.SetWaveform((uint8_t)waveform);

    for (size_t i = 0; i < size; i++)
    {
        float main_out = main_oscillator.Process();
        float sub1_out = osc_sub1.Process();
        float sub2_out = osc_sub2.Process();
        float sub3_out = osc_sub3.Process();
        float sub4_out = osc_sub4.Process();

        // Left channel: main_osc
        float left = main_out * volume;

        // Right channel: subharmonics
        float right = (0.4f * sub1_out + 0.3f * sub2_out + 0.2f * sub3_out + 0.1f * sub4_out) * volume;

        // Filter
        float mix_sum = left + right;
        formant_filter.Process(mix_sum);
        float filtered_lp = formant_filter.Low();

        // final L/R
        left = filtered_lp * 0.5f;
        right = filtered_lp * 0.5f;

        // Write final audio
        out[0][i] = left;
        out[1][i] = right;
        out[2][i] = 0.f;
        out[3][i] = 0.f;

        // --- Store LEFT channel in scope_buffer for time-domain display ---
        scope_buffer[scope_idx] = left; // in [-1..+1] range
        scope_idx = (scope_idx + 1) % SCOPE_BUF_SIZE;
    }
}

// -------------------------------------------------
// Main
// -------------------------------------------------
int main(void)
{
    patch.Init();
    float samplerate = patch.AudioSampleRate();

    // Init oscillators
    waveform = 0;

    main_oscillator.Init(samplerate);
    osc_sub1.Init(samplerate);
    osc_sub2.Init(samplerate);
    osc_sub3.Init(samplerate);
    osc_sub4.Init(samplerate);

    main_oscillator.SetAmp(0.8f);
    osc_sub1.SetAmp(0.8f);
    osc_sub2.SetAmp(0.8f);
    osc_sub3.SetAmp(0.8f);
    osc_sub4.SetAmp(0.8f);

    // Init filter
    formant_filter.Init(samplerate);
    formant_filter.SetFreq(filter_freq);
    formant_filter.SetRes(0.5f);

    // Start ADC and Audio
    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    while (1)
    {
        UpdateOled();
    }
}
