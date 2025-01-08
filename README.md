This repository contains the firmware and source code for a modern recreation of the Trautonium, an early electronic musical instrument known for its expressive ribbon controller and subharmonic character.

## Overview

The firmware, written in C++, is designed for the DaisyDSP microcontroller and Arduino platforms. The .bin file is a precompiled firmware binary ready for flashing onto a supported device.

## Core Functions

### Signal Generation

The trautonium.cpp source implements core audio signal generation using:

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

Alternatively, use an Arduino IDE to compile and upload trautonium.cpp.

## Contributing

Contributions to enhance functionality or improve compatibility are welcome. Submit issues or pull requests to collaborate.

## License

This project is licensed under the MIT License. See the LICENSE file for details.