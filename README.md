# Final Project, Simulating the Hunts
This program runs a simulation of hunters trying to identify a ghost's type with certain devices. Based on the game Phasmaphobia

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




## Bonus Items
- Ghost will not leave the same type of evidence in a room more than once
- Used coloured text to make success/failure more visually distinct and appealing

## Sources
- Discussions with a peer
- Course notes

## Assumptions
- User will not enter invalid hunter names or hunter IDs
- If a hunter enters the van by chance, they do not switch devices
- "Hunters exited after identifying the ghost" refers to how many hunters discovered evidence

### Credits
- Developed individually by Nailah Abel (101339862)