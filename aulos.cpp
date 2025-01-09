#include "daisysp.h"
#include "daisy_patch.h"
#include <string>

using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

// --------------------- Oscillators ---------------------
Oscillator main_oscillator, osc_sub1, osc_sub2, osc_sub3, osc_sub4;
Oscillator bass_oscillator, bosc_sub1, bosc_sub2, bosc_sub3, bosc_sub4;

// --------------------- Scale Data ---------------------
static constexpr int NUM_SCALES = 10;
const char *scale_names[NUM_SCALES] = {
    "chromatic",
    "major",
    "minor",
    "pentatonic",
    "blues",
    "phrygian",
    "lydian",
    "mixolydian",
    "arabic",
    "japanese",
};

const int scales[NUM_SCALES][7] = {
    {0, 1, 2, 3, 4, 5, 6},
    {0, 2, 4, 5, 7, 9, 11},
    {0, 2, 3, 5, 7, 8, 10},
    {0, 2, 4, 7, 9, -1, -1},
    {0, 3, 5, 6, 7, 10, -1},
    {0, 1, 3, 5, 7, 8, 10},
    {0, 2, 4, 6, 7, 9, 11},
    {0, 2, 3, 5, 7, 9, 11},
    {0, 1, 4, 5, 7, 8, 11},
    {0, 1, 5, 7, 8, 10, -1},
};

int selected_scale = 0;

// --------------------- Wave Data ---------------------
static constexpr int NUM_WAVES = 5;
const char *wave_names[NUM_WAVES] = {
    "sine", "triangle", "saw", "ramp", "square"};

int waveform = 0;

// --------------------- Pitch Control ---------------------
int root_note = 60; // C3
float pitch_main = 0.f;
float pitch_sub[4];

int bass_root_note = 24;
float pitch_bass = 0.f;
float pitch_bsub[4];

float main_volume = 0.8f;
float bass_volume = 0.8f;

// --------------------- Encoder Mode ---------------------
enum class EncoderMode
{
    WAVEFORM,
    SCALE
};

static EncoderMode encoder_mode = EncoderMode::WAVEFORM;

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
// Update the display
// -------------------------------------------------
void UpdateOled()
{
    patch.display.Fill(false);
    patch.display.SetCursor(0, 0);
    patch.display.WriteString("~~~~~~~~~~ αὐλός ~~~~~~~~~~", Font_6x8, true);

    patch.display.SetCursor(0, 10);
    if (encoder_mode == EncoderMode::WAVEFORM)
        patch.display.WriteString("* waveform: ", Font_6x8, true);
    else
        patch.display.WriteString("  waveform: ", Font_6x8, true);
    patch.display.WriteString(wave_names[waveform], Font_6x8, true);

    patch.display.SetCursor(0, 20);
    if (encoder_mode == EncoderMode::SCALE)
        patch.display.WriteString("* scale: ", Font_6x8, true);
    else
        patch.display.WriteString("  scale: ", Font_6x8, true);
    patch.display.WriteString(scale_names[selected_scale], Font_6x8, true);

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

    root_note = static_cast<int>(knob1 * 127.f);
    bass_root_note = static_cast<int>(knob3 * 127.f);

    main_volume = knob2;
    bass_volume = knob4;

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
// Calculate subharmonics for a given base frequency
// -------------------------------------------------
void CalculateSubharmonics(float base_freq, float *subharmonics, int count)
{
    for (int i = 0; i < count; ++i)
    {
        subharmonics[i] = base_freq / (i + 2);
    }
}

// --------------------------------------------------------------
// Process frequency and waveform changes for the main oscillator
// --------------------------------------------------------------
void UpdateOsc(float pitch, int waveform)
{
    pitch_sub[0] = pitch / 2.f;
    pitch_sub[1] = pitch / 3.f;
    pitch_sub[2] = pitch / 4.f;
    pitch_sub[3] = pitch / 5.f;

    main_oscillator.SetFreq(pitch);
    osc_sub1.SetFreq(pitch_sub[0]);
    osc_sub2.SetFreq(pitch_sub[1]);
    osc_sub3.SetFreq(pitch_sub[2]);
    osc_sub4.SetFreq(pitch_sub[3]);

    main_oscillator.SetWaveform((uint8_t)waveform);
    osc_sub1.SetWaveform((uint8_t)waveform);
    osc_sub2.SetWaveform((uint8_t)waveform);
    osc_sub3.SetWaveform((uint8_t)waveform);
    osc_sub4.SetWaveform((uint8_t)waveform);
}

// --------------------------------------------------------------
// Process frequency and waveform changes for the bass oscillator
// --------------------------------------------------------------
void UpdateBassOsc(float pitch, int waveform)
{
    pitch_bsub[0] = pitch / 2.f;
    pitch_bsub[1] = pitch / 3.f;
    pitch_bsub[2] = pitch / 4.f;
    pitch_bsub[3] = pitch / 5.f;

    bass_oscillator.SetFreq(pitch);
    bosc_sub1.SetFreq(pitch_bsub[0]);
    bosc_sub2.SetFreq(pitch_bsub[1]);
    bosc_sub3.SetFreq(pitch_bsub[2]);
    bosc_sub4.SetFreq(pitch_bsub[3]);

    bass_oscillator.SetWaveform((uint8_t)waveform);
    bosc_sub1.SetWaveform((uint8_t)waveform);
    bosc_sub2.SetWaveform((uint8_t)waveform);
    bosc_sub3.SetWaveform((uint8_t)waveform);
    bosc_sub4.SetWaveform((uint8_t)waveform);
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

    int main_q_note = QuantizeToScale(root_note, selected_scale);
    pitch_main = mtof(main_q_note);
    UpdateOsc(pitch_main, waveform);

    int bass_q_note = QuantizeToScale(bass_root_note, selected_scale);
    pitch_bass = mtof(bass_q_note);
    UpdateBassOsc(pitch_bass, waveform);

    for (size_t i = 0; i < size; i++)
    {
        float channel1_main_out = main_oscillator.Process();
        float channel1_sub1_out = osc_sub1.Process();
        float channel1_sub2_out = osc_sub2.Process();
        float channel1_sub3_out = osc_sub3.Process();
        float channel1_sub4_out = osc_sub4.Process();

        // Left channel: main_osc
        float channel1_left = channel1_main_out * main_volume;
        // Right channel: subharmonics
        float channel1_right = (0.4f * channel1_sub1_out + 0.3f * channel1_sub2_out + 0.2f * channel1_sub3_out + 0.1f * channel1_sub4_out) * main_volume;

        float channel2_main_out = bass_oscillator.Process();
        float channel2_sub1_out = bosc_sub1.Process();
        float channel2_sub2_out = bosc_sub2.Process();
        float channel2_sub3_out = bosc_sub3.Process();
        float channel2_sub4_out = bosc_sub4.Process();

        // Left channel: bass subharmonics
        float channel2_left = (0.4f * channel2_sub1_out + 0.3f * channel2_sub2_out + 0.2f * channel2_sub3_out + 0.1f * channel2_sub4_out) * bass_volume;
        // Right channel: bass_osc
        float channel2_right = channel2_main_out * bass_volume;

        float max_val = std::max({channel1_left, channel1_right, channel2_left, channel2_right});
        if (max_val > 1.0f)
        {
            channel1_left /= max_val;
            channel1_right /= max_val;
            channel2_left /= max_val;
            channel2_right /= max_val;
        }

        // Write final audio
        out[0][i] = channel1_left;
        out[1][i] = channel1_right;
        out[2][i] = channel2_left;
        out[3][i] = channel2_right;
    }
}

// -------------------------------------------------
// Initialize an oscillator
// -------------------------------------------------
void InitOscillator(Oscillator &osc, float amp, float samplerate)
{
    osc.Init(samplerate);
    osc.SetAmp(amp);
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

    InitOscillator(main_oscillator, 0.8f, samplerate);
    InitOscillator(osc_sub1, 0.8f, samplerate);
    InitOscillator(osc_sub2, 0.8f, samplerate);
    InitOscillator(osc_sub3, 0.8f, samplerate);
    InitOscillator(osc_sub4, 0.8f, samplerate);

    InitOscillator(bass_oscillator, 0.8f, samplerate);
    InitOscillator(bosc_sub1, 0.8f, samplerate);
    InitOscillator(bosc_sub2, 0.8f, samplerate);
    InitOscillator(bosc_sub3, 0.8f, samplerate);
    InitOscillator(bosc_sub4, 0.8f, samplerate);

    // Start ADC and Audio
    patch.StartAdc();
    patch.StartAudio(AudioCallback);

    while (1)
    {
        UpdateOled();
    }
}
