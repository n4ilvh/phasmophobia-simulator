# Phasmophobia Simulator
This program runs a simulation of hunters trying to identify a ghost's type with certain devices. Based on the game Phasmaphobia by Kinetic Games.

## Building and Running
1. To compile all source files and create an executable called `huntSimulation`, type:
   `make`
2. Run the program with:
    `./huntSimulation`
3. Follow the on screen prompts to add hunters to the investigation. Type `done` when finished to begin the simulation.
4. When done, you can remove all object riles and the executable with:
    `make clean`

## File Descriptions
`defs.h`
- Contains all type definitions, constants and structure declarations

`helpers.h`
- Header file containing all function declarations

`helpers.c`
- Implementation of all helper functions and simulation logic

`main.c`
- Main program entry point and simulation control

`Makefile`
- Allows easy building of the project
