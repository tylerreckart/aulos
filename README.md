# Aulos
<img src="https://upload.wikimedia.org/wikipedia/commons/4/41/Banquet_Euaion_Louvre_G467_n2_cropped.jpg" alt="Image depicting an Aulos (double reedpipes or double clarinets), from Attic red-figure cup, Banquet Euaion Louvre" width="320px"/>

This repository contains the firmware and source code for a modern recreation of the Trautonium, an early electronic musical instrument known for its expressive control and subharmonic character. The project is named _Aulos_ after the ancient Greek double-piped wind instrument, celebrated for its rich, expressive tones and cultural significance in early music. This name reflects the project's aim to recreate an instrument with similar expressive capabilities.

## Overview

The firmware, written in C++, is designed for the DaisyDSP microcontroller and Arduino platforms. The .bin file is a precompiled firmware binary ready for flashing onto a supported device.

## Core Functions

### Signal Generation

The aulos.cpp source implements core audio signal generation using:

- **Subharmonic Oscillators-** Configurable via parameterized subharmonic ratios.

- **Multiple Waveforms-** Multiple selectable waveforms for tonal variation.

- **Formant Resonant Filter**

### Input Handling

Control Voltage (CV): Accepts CV signals to manipulate the following parameters:

- **Fundamental Frequency __(quantized to one of 16 selected scales)__**

- **Filter Frequency**

- **Filter Resonance**

- **Output Volume**

### Output Processing

- **Audio Output-** Generates line-level audio through onboard DAC.

- **Polyphony-** Supports monophonic or limited polyphonic modes.

### Installation

Flash the firmware binary to a compatible device.

Alternatively, use an Arduino IDE to compile and upload aulos.cpp.

## Contributing

Contributions to enhance functionality or improve compatibility are welcome. Submit issues or pull requests to collaborate.

## License

This project is licensed under the MIT License. See the LICENSE file for details.