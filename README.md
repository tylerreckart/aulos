# Aulos
<img src="https://upload.wikimedia.org/wikipedia/commons/4/41/Banquet_Euaion_Louvre_G467_n2_cropped.jpg" alt="Image depicting an Aulos (double reedpipes or double clarinets), from Attic red-figure cup, Banquet Euaion Louvre" width="320px"/>

## Demo

Please click the thumbnail below to be directed to the video demonstration of the current firmware: 

[![aulos video demonstration](http://i3.ytimg.com/vi/ZTuEySEleiw/hqdefault.jpg)](https://www.youtube.com/watch?v=ZTuEySEleiw?si=39wRLsX7yXqIgUet)  

## Overview

This repository contains the firmware and source code for a modern recreation of the Trautonium, an early electronic musical instrument known for its expressive control and subharmonic character. The project is named _Aulos_ after the ancient Greek double-piped wind instrument, celebrated for its rich, expressive tones and cultural significance in early music. This name reflects the project's aim to recreate an instrument with similar expressive capabilities. The firmware, written in C++, is designed for the DaisyDSP microcontroller and Arduino platforms. The .bin file is a precompiled firmware binary ready for flashing onto a supported device.

## Features

- **Two independent oscillators** (Osc1 and Osc2), each with **four subharmonic voices**.  
- **Multiple waveforms** (Sine, Triangle, Saw, Ramp, Square) selectable independently for each oscillator.  
- **Scale-based quantization** with selectable scales (Major, Minor, Pentatonic, etc.).  
- **Configurable root note** for each oscillator via dedicated knobs.  
- **Volume control** for each oscillator and its subharmonics.  

## Dependencies

- **[libDaisy](https://github.com/electro-smith/libDaisy)** – Daisy hardware abstraction library.  
- **[DaisySP](https://github.com/electro-smith/DaisySP)** – DSP library for the Daisy platform.  
- **[arm-none-eabi-gcc](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm)** (or equivalent) for compiling.  

Depending on your workflow, you can either use the **Daisy toolchain** (CMake-based) or a **PlatformIO** environment configured for Daisy development.

## Building and Flashing

1. **Clone the repository** (or download the source):
```bash
   git clone https://github.com/yourusername/aulos.git
   cd aulos
```
2. Ensure the Daisy toolchain is installed and properly set up:
    
    a. Install DaisyDuino/Make/CMake environment.
    
    b. Build the firmware:
    
    c. If using the Make-based environment:

```bash
make
```

    d. Or, if using CMake:

```bash
mkdir build
cd build
cmake ..
make
```

3. Flash the binary to the Daisy Patch:

    a. Connect your Daisy Patch via USB in DFU mode.
    b. Run:

```bash
make program-dfu
```

    c. or use the relevant CMake/PlatformIO target.
    d. After flashing completes, reboot the Daisy Patch. Aulos will automatically run.

## Usage

Once installed, the Aulos firmware boots immediately into audio generation mode. The subharmonic oscillators are layered over two main oscillators. Control is offered via the four knobs and the rotary encoder.

## Controls
- **Knob 1**: Root note (0–127 MIDI) for Oscillator 1.
- **Knob 2**: Volume for Oscillator 1.
- **Knob 3**: Root note (0–127 MIDI) for Oscillator 2.
- **Knob 4**: Volume for Oscillator 2.

Rotary Encoder Button: Cycles through encoder modes:

- **WAVEFORM_1**: Change waveform for Oscillator 1.
- **WAVEFORM_2**: Change waveform for Oscillator 2.
- **SCALE**: Change the active scale.
- **QUANTIZE**: Toggle quantization on/off.

Rotary Encoder Rotation:
- In **WAVEFORM_1** or **WAVEFORM_2** mode, rotating changes the waveform index.
- In **SCALE** mode, rotating cycles through available scales.
- In **QUANTIZE** mode, any rotation toggles quantization.

## License
This project is licensed under the MIT License. You are free to use, modify, and distribute this software in accordance with the terms of the MIT License. See the LICENSE file for more details.