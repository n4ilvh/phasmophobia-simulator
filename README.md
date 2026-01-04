# Phasmophobia Simulator
A multithreaded C simulation inspired by Phasmophobia (Kinetic Games). Hunters explore a house, collect evidence using specialized devices, and attempt to identify a roaming ghost all while managing fear, boredom, and shared state safely across threads. This project was completed as a second year Systems Programming final project and focuses on concurrency, memory management, and low-level data structures.

## Key Features
- Multithreaded simulation using POSIX threads
- Bitwise evidence system
- Dynamic memory & data structures


## Building and Running
1. To compile all source files and create an executable called `huntSimulation`, type:
   `make`
2. Run the program with:
    `./huntSimulation`
3. Follow the on screen prompts to add hunters to the investigation. Type `done` when finished to begin the simulation.
4. When done, you can remove all object files and the executable with:
    `make clean`

