# Nintendo Emulator - 2015 style
Target Platform: OS X  
Primary Goals: CPU & PPU Performance, using C++14 features, scanline-accurate CPU<->PPU synchronization  
Stretch Goals: APU emulation, memory-mapper supports, pixel-accurate synchronization

Compatibility Goals:
* Primary: Super Mario Bros & Donkey Kong
* Secondary: other 32kB ROM + 8kB VROM games
* Stretch: Official Nintendo MMC1+ memory mapper powered games

## Current Status
### CPU
* Load ROM & parse header

### GPU

### Other

## Dependencies
``brew cask install biicode``

## Setup
``bii init``

``bii cpp:configure``

``bii cpp:build``
