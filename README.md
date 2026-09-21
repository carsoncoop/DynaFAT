# DynaFAT
DynaFAT is a real time audio processing VST made for electronic music production. DynaFAT preserves dynamics while adding richness to bass synths through aggressive distortion, multiband compression, and envelope following.

Requirements:
- C++
- CMake
- JUCE framework

Goals:
- Implement multiband compression class
- Reduce user controls to be concise while allowing for customizability
- Optimize CPU performance in buffer processing
- Uncomment filter implementation and turn it into a class like the other effects

Future Features:
- Adjustable curve of envelope follower, allowing for a sharp J-curve (sharp vs. shallow fall off)
- Allow user to save and load presets
- "Makeup Gain" button next to master volume that will set it automatically

Future Knobs:
- Compressor
  - Input gain


Known bugs:
- Alg options set to SoftClip whenever menu is closed.
- Compressing even in dead zone.

Current To-Do List:
- Split bands and compress separately like OTT
- Once all 3 bands are being compressed. Figure out what OTT is doing uniquely for signature sound

Effect Class Layout:
- Member variables store state of parameters through setter functions
- Member functions process single samples of audio, only passing in the sample as a parameter
   → Access member variables for parameter values

Reminder for Developer:
- PC VST Path: "F:/ProgramData/Ableton/Live 9 Standard"
- Laptop VST Path: "C:/Users/tf2ma/Documents/Ableton/VST Plugins"