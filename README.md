# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

Requirements:
- C++
- JUCE framework

Goals:
- Implement EnvelopeFollower class
- Implement multiband compression class
  -> Begin with a simple, one band, downward compressor without artifacts
  -> Create upward compression
  -> Split into 3 bands
- Organize Distortion into it's own class
- Reduce user controls to be concise while allowing for customizability
- Optimize CPU performance in buffer processing
- Minimize code required for parameter instantiation & loading

Future Features:
- Adjustable attack/release curve of envelope follower, allowing for a sharp J-curve

Known bugs:
- UI's compressor toggle automatically switches on after window opened

Reminder for Developer:
- PC Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"