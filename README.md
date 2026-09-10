# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

Requirements:
- C++
- CMake
- JUCE framework

Goals:
- Implement multiband compression class 
  - Begin with a simple, one band, downward compressor without artifacts 
  - Create upward compression 
  - Split into 3 bands
- Reduce user controls to be concise while allowing for customizability
- Optimize CPU performance in buffer processing
- Minimize code required for parameter instantiation & loading
- Minimize code for UI updates
- Uncomment filter implementation and turn it into a class like the other effects

Future Features:
- Adjustable curve of envelope follower, allowing for a sharp J-curve (sharp vs. shallow fall off)
- Allow user to save and load presets

Future Knobs:
- Envelope Follower:
  - Mix/"Dynamix"
- Compressor
  - Input gain
- Master
  - Master gain

Known bugs:
- UI's buttons reset visually when closed, but not in the dsp
- Compressor button broken

Current To-Do List:
- Optimize parameter instantiation in classes to continue working on compressor. More bands will mean MANY more parameters.
- Upward Compression
- Make a filter class

Effect Class Layout:
- Member variables store state of parameters through setter functions
- Member functions process single samples of audio, only passing in the sample as a parameter
   → Access member variables for parameter values

Reminder for Developer:
- PC VST Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop VST Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"