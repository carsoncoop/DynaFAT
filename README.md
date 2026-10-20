# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

🎥 [Work In Progress Demo Video](https://www.youtube.com/watch?v=gI5iUk-rYCE)

Requirements:
- C++
- CMake
- JUCE framework

Future Features:
- Adjustable curve of envelope follower, allowing for a sharp J-curve (sharp vs. shallow fall off)
- Allow user to save and load presets
- "High quality" button that uses oversampling

Known bugs:
- Alg options set to SoftClip whenever menu is closed.
- Compressing even in dead zone.

Current To-Do List:
- Split compressor bands with Linkwitz-Riley filters once per buffer, and process each sample individually
- Implement "passes" for compressor
- Implement downsample in distortion.process() function

Effect Class Layout:
- Member variables store state of parameters through setter functions
- Member functions process single samples of audio, only passing in the sample as a parameter
  → Access member variables for parameter values

Reminder for Developer:
- PC VST Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop VST Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"