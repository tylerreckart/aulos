#include "daisysp.h"
#include "daisy_patch.h"

using namespace daisysp;
using namespace daisy;

DaisyPatch hw;

// Oscillators
Oscillator main_oscillator, osc_sub1, osc_sub2, osc_sub3, osc_sub4;

// Filters
Svf formant_filter;

// Scales (16 predefined musical scales)
const char *scale_names[] = {
    "Chromatic", "Major", "Minor", "Pentatonic", "Blues", 
    "Whole Tone", "Dorian", "Phrygian", "Lydian", "Mixolydian",
    "Locrian", "Harmonic Minor", "Melodic Minor", "Arabic", 
    "Gypsy", "Japanese"
};

const int scales[16][7] = {
    {0, 1, 2, 3, 4, 5, 6},    // Chromatic
    {0, 2, 4, 5, 7, 9, 11},   // Major
    {0, 2, 3, 5, 7, 8, 10},   // Minor
    {0, 2, 4, 7, 9, -1, -1},  // Pentatonic
    {0, 3, 5, 6, 7, 10, -1},  // Blues
    {0, 2, 4, 6, 8, 10, -1},  // Whole Tone
    {0, 2, 3, 5, 7, 9, 10},   // Dorian
    {0, 1, 3, 5, 7, 8, 10},   // Phrygian
    {0, 2, 4, 6, 7, 9, 11},   // Lydian
    {0, 2, 4, 5, 7, 9, 10},   // Mixolydian
    {0, 1, 3, 5, 6, 8, 10},   // Locrian
    {0, 2, 3, 5, 7, 8, 11},   // Harmonic Minor
    {0, 2, 3, 5, 7, 9, 11},   // Melodic Minor
    {0, 1, 4, 5, 7, 8, 11},   // Arabic
    {0, 1, 4, 5, 7, 8, 10},   // Gypsy
    {0, 1, 5, 7, 8, 10, -1},  // Japanese
};

int selected_scale = 0; // Default to Chromatic

// MIDI and pitch variables
int   root_note = 60;  // C3
float pitch_main, pitch_sub[4];
float volume;

// Helper function to quantize a note to a scale
int QuantizeToScale(int note, int scale_index)
{
    int octave         = note / 12;
    int note_in_octave = note % 12;
    const int *scale   = scales[scale_index];

    // Find nearest note in scale
    for(int i = 0; i < 7 && scale[i] != -1; i++)
    {
        if(note_in_octave <= scale[i])
        {
            return octave * 12 + scale[i];
        }
    }
    // Wrap to next octave
    return (octave + 1) * 12 + scale[0];
}

// Display update function
void UpdateDisplay()
{
    char line1[32], line2[32];

    // Format display text
    snprintf(line1, 32, "Root: C%d (%d Hz)",
             (root_note - 60) / 12,
             (int)mtof(root_note));
    snprintf(line2, 32, "Scale: %s", scale_names[selected_scale]);

    // Clear display
    hw.display.Fill(false);

    // Write text
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(line1, Font_6x8, true);

    hw.display.SetCursor(0, 10);
    hw.display.WriteString(line2, Font_6x8, true);

    // Update display
    hw.display.Update();
}

void AudioCallback(const float* const* in, float** out, size_t size)
{
    hw.ProcessAnalogControls();
    hw.ProcessDigitalControls();

    //-------------------------------------------------------------------------
    // Reading knob values (returns 0.0 to 1.0)
    // DaisyPatch has 4 knobs: CTRL_1, CTRL_2, CTRL_3, CTRL_4
    //-------------------------------------------------------------------------
    float pitch_cv        = hw.GetKnobValue(DaisyPatch::CTRL_1); // Fund. pitch
    float filter_freq_cv  = hw.GetKnobValue(DaisyPatch::CTRL_2); // Filter freq
    float filter_res_cv   = hw.GetKnobValue(DaisyPatch::CTRL_3); // Filter res
    float volume_cv       = hw.GetKnobValue(DaisyPatch::CTRL_4); // Volume

    //-------------------------------------------------------------------------
    // Map pitch knob to MIDI notes (0.0..1.0 mapped to +/- 24 semitones)
    //-------------------------------------------------------------------------
    int   pitch_offset    = static_cast<int>(pitch_cv * 24.0f) - 12; // ±1 octave
    root_note             = 60 + pitch_offset;

    // Quantize root note to the selected scale
    root_note = QuantizeToScale(root_note, selected_scale);

    // Convert MIDI note to frequency
    pitch_main = mtof(root_note);

    // Calculate subharmonic frequencies
    pitch_sub[0] = pitch_main / 2.0f; // First subharmonic
    pitch_sub[1] = pitch_main / 3.0f; // Second subharmonic
    pitch_sub[2] = pitch_main / 4.0f; // Third subharmonic
    pitch_sub[3] = pitch_main / 5.0f; // Fourth subharmonic

    // Set oscillator frequencies
    main_oscillator.SetFreq(pitch_main);
    osc_sub1.SetFreq(pitch_sub[0]);
    osc_sub2.SetFreq(pitch_sub[1]);
    osc_sub3.SetFreq(pitch_sub[2]);
    osc_sub4.SetFreq(pitch_sub[3]);

    //-------------------------------------------------------------------------
    // Map filter parameters from CV
    //-------------------------------------------------------------------------
    float filter_freq = fmap(filter_freq_cv, 200.0f, 5000.0f);
    float filter_res  = fmap(filter_res_cv, 0.1f, 1.0f);

    formant_filter.SetFreq(filter_freq);
    formant_filter.SetRes(filter_res);

    //-------------------------------------------------------------------------
    // Set volume
    //-------------------------------------------------------------------------
    volume = volume_cv;

    // Update display
    UpdateDisplay();

    //-------------------------------------------------------------------------
    // AUDIO
    //-------------------------------------------------------------------------
    for(size_t i = 0; i < size; i++)
    {
        // Generate signals
        float main_out = main_oscillator.Process();
        float sub1_out = osc_sub1.Process();
        float sub2_out = osc_sub2.Process();
        float sub3_out = osc_sub3.Process();
        float sub4_out = osc_sub4.Process();

        // Mix oscillators
        float mixed_signal = main_out
                   + 0.4f * sub1_out
                   + 0.3f * sub2_out
                   + 0.2f * sub3_out
                   + 0.1f * sub4_out;

        // Process through Svf (does not return a float)
        formant_filter.Process(mixed_signal);

        // Grab the lowpass output
        float filtered_signal_lp = formant_filter.Low();

        // Set outputs (stereo)
        out[0][i] = filtered_signal_lp * volume;
        out[1][i] = filtered_signal_lp * volume;
        out[2][i] = 0.0f;
        out[3][i] = 0.0f;
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    // Initialize oscillators
    main_oscillator.Init(samplerate);
    main_oscillator.SetWaveform(Oscillator::WAVE_SIN);

    osc_sub1.Init(samplerate);
    osc_sub1.SetWaveform(Oscillator::WAVE_SIN);

    osc_sub2.Init(samplerate);
    osc_sub2.SetWaveform(Oscillator::WAVE_SQUARE);

    osc_sub3.Init(samplerate);
    osc_sub3.SetWaveform(Oscillator::WAVE_TRI);

    osc_sub4.Init(samplerate);
    osc_sub4.SetWaveform(Oscillator::WAVE_SAW);

    // Initialize filter
    formant_filter.Init(samplerate);
    // Optionally specify filter mode if you want only lowpass:
    // formant_filter.SetType(Svf::Type::LOWPASS);

    // Start audio callback
    hw.StartAudio(AudioCallback);

    while(1)
    {
        // Infinite loop
    }
}
