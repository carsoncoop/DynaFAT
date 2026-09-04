# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

Requirements:
- C++
- CMake
- JUCE framework

Goals:
- Implement EnvelopeFollower class
- Implement multiband compression class 
  → Begin with a simple, one band, downward compressor without artifacts
  → Create upward compression
  → Split into 3 bands
- Organize Distortion into its own class
- Reduce user controls to be concise while allowing for customizability
- Optimize CPU performance in buffer processing
- Minimize code required for parameter instantiation & loading

Future Features:
- Adjustable attack/release curve of envelope follower, allowing for a sharp J-curve

Known bugs:
- UI's compressor toggle automatically switches on after window opened

Reminder for Developer:
- PC VST Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop VST Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"

Current To-Do List:
- Implement Distortion Class & ensure it works the same as before when called in processBlock
- Remove enumerated parameters in the PluginProcessor for OOP implementation

Effect Class Layout:
- Member variables store state of parameters through setter functions
- Member functions process single samples of audio, only passing in the sample as a parameter
   → Access member variables for parameter values