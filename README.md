# Picbacktrack

## General info
Picbacktrack is a Raspberry Pi Pico based gps backtracker.

## Hardware
This project requires:
- Raspberry Pi Pico or RP2040-based analogue
- UART GPS module with NMEA (beitian be-220 in my case)
- SSD1306 128x64 display
- A tact button

## Current features
- Current position and time (only UTC)
- Saving of current position as destination point with the following information:
  * Coordinates
  * Distance
  * Absolute (cardinal) direction
  * Relative direction (according to movement direction) 
