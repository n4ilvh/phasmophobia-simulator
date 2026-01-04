#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
    char name[MAX_ROOM_NAME]; // name string
    struct Room* connectedRooms[MAX_CONNECTIONS]; // array of pointers to connected rooms
    int numConnections;
    struct Ghost* ghost; // pointer to the ghost (null if ghost isn't there)
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY]; // array of hunter pointers
    int numHunters; // count of the number of hunters in the room
    bool is_exit; // flag if the room is the exit
    EvidenceByte evidence; // evidence currently in the room
    sem_t mutex; // semaphore;
};

struct RoomNode {
    struct Room* room; // current room
    struct RoomNode* next;
};

struct RoomStack {
    struct RoomNode* head;
};

// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
    int id;
    enum GhostType type;
    struct Room* room; // room the ghost is in
    int boredom;
    bool exited; // has the ghost exited the house
    pthread_mutex_t boredom_mutex;
};

// Can be either stack or heap allocated
struct House {
    struct Room* starting_room; // Needed by house_populate_rooms, but can be adjusted to suit your needs.
    struct Room rooms[MAX_ROOMS]; // all rooms
    int roomCount;
    struct Hunter** hunters; //array of hunters
    int hunterCount;
    struct CaseFile casefile; // collected evidence
    struct Ghost ghost;
};

struct Hunter {
    char name[MAX_HUNTER_NAME];
    int id;
    struct Room* room; // current room the hunter is in
    struct CaseFile *casefile; // the shared case file
    enum EvidenceType device; // device the hunter is using
    struct RoomStack path; // the path from the house's starting room
    int fear;
    int boredom;
    enum LogReason exitReason; // why/if the hunter exited
    bool exited; // if the hunter has left the simulation
    bool returning; // if hunter is retuning to the van
    pthread_mutex_t mutex;
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void room_init(struct Room* room, const char* name, bool is_exit);

void room_connect(struct Room* a, struct Room* b); // Bidirectional connection

#endif // DEFS_H
