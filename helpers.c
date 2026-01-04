#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <stdint.h>
#include "helpers.h"
#include <unistd.h>

// ---- House layout ----
void house_populate_rooms(struct House* house) {
    // Willow House layout from Phasmaphobia, DO NOT MODIFY HOUSE LAYOUT
    house->roomCount = 13;

    room_init(house->rooms+0, "Van", true);
    room_init(house->rooms+1, "Hallway", false);
    room_init(house->rooms+2, "Master Bedroom", false);
    room_init(house->rooms+3, "Boy's Bedroom", false);
    room_init(house->rooms+4, "Bathroom", false);
    room_init(house->rooms+5, "Basement", false);
    room_init(house->rooms+6, "Basement Hallway", false);
    room_init(house->rooms+7, "Right Storage Room", false);
    room_init(house->rooms+8, "Left Storage Room", false);
    room_init(house->rooms+9, "Kitchen", false);
    room_init(house->rooms+10, "Living Room", false);
    room_init(house->rooms+11, "Garage", false);
    room_init(house->rooms+12, "Utility Room", false);

    room_connect(house->rooms+0, house->rooms+1);    // Van - Hallway
    room_connect(house->rooms+1, house->rooms+2);    // Hallway - Master Bedroom
    room_connect(house->rooms+1, house->rooms+3);    // Hallway - Boy's Bedroom
    room_connect(house->rooms+1, house->rooms+4);    // Hallway - Bathroom
    room_connect(house->rooms+1, house->rooms+9);    // Hallway - Kitchen
    room_connect(house->rooms+1, house->rooms+5);    // Hallway - Basement
    room_connect(house->rooms+5, house->rooms+6);    // Basement - Basement Hallway
    room_connect(house->rooms+6, house->rooms+7);    // Basement Hallway - Right Storage Room
    room_connect(house->rooms+6, house->rooms+8);    // Basement Hallway - Left Storage Room
    room_connect(house->rooms+9, house->rooms+10);   // Kitchen - Living Room
    room_connect(house->rooms+9, house->rooms+11);   // Kitchen - Garage
    room_connect(house->rooms+11, house->rooms+12);  // Garage - Utility Room

    house->starting_room = house->rooms; // Van is at index 0
}

// ---- to_string functions ----
const char* evidence_to_string(enum EvidenceType evidence) {
    switch (evidence) {
        case EV_EMF:
            return "emf";
        case EV_ORBS:
            return "orbs";
        case EV_RADIO:
            return "radio";
        case EV_TEMPERATURE:
            return "temp";
        case EV_FINGERPRINTS:
            return "prints";
        case EV_WRITING:
            return "writing";
        case EV_INFRARED:
            return "infrared";
        default:
            return "unknown";
    }
}

const char* ghost_to_string(enum GhostType ghost) {
    switch (ghost) {
        case GH_POLTERGEIST:
            return "poltergeist";
        case GH_THE_MIMIC:
            return "the_mimic";
        case GH_HANTU:
            return "hantu";
        case GH_JINN:
            return "jinn";
        case GH_PHANTOM:
            return "phantom";
        case GH_BANSHEE:
            return "banshee";
        case GH_GORYO:
            return "goryo";
        case GH_BULLIES:
            return "bullies";
        case GH_MYLING:
            return "myling";
        case GH_OBAKE:
            return "obake";
        case GH_YUREI:
            return "yurei";
        case GH_ONI:
            return "oni";
        case GH_MOROI:
            return "moroi";
        case GH_REVENANT:
            return "revenant";
        case GH_SHADE:
            return "shade";
        case GH_ONRYO:
            return "onryo";
        case GH_THE_TWINS:
            return "the_twins";
        case GH_DEOGEN:
            return "deogen";
        case GH_THAYE:
            return "thaye";
        case GH_YOKAI:
            return "yokai";
        case GH_WRAITH:
            return "wraith";
        case GH_RAIJU:
            return "raiju";
        case GH_MARE:
            return "mare";
        case GH_SPIRIT:
            return "spirit";
        default:
            return "unknown";
    }
}

const char* exit_reason_to_string(enum LogReason reason) {
    switch (reason) {
        case LR_EVIDENCE:
            return "evidence";
        case LR_BORED:
            return "bored";
        case LR_AFRAID:
            return "afraid";
        default:
            return "unknown";
    }
}

// ---- enum retrieval functions ----
int get_all_evidence_types(const enum EvidenceType** list) {
    // Stored in the data segment so that we can point to it safely
    static const enum EvidenceType evidence_types[] = {
        EV_EMF,
        EV_ORBS,
        EV_RADIO,
        EV_TEMPERATURE,
        EV_FINGERPRINTS,
        EV_WRITING,
        EV_INFRARED
    };

    if (list) {
        *list = evidence_types;
    }
    return (int)(sizeof(evidence_types) / sizeof(evidence_types[0]));
}

int get_all_ghost_types(const enum GhostType** list) {
    // Stored in the data segment so that we can point to it safely
    static const enum GhostType ghost_types[] = {
        GH_POLTERGEIST,
        GH_THE_MIMIC,
        GH_HANTU,
        GH_JINN,
        GH_PHANTOM,
        GH_BANSHEE,
        GH_GORYO,
        GH_BULLIES,
        GH_MYLING,
        GH_OBAKE,
        GH_YUREI,
        GH_ONI,
        GH_MOROI,
        GH_REVENANT,
        GH_SHADE,
        GH_ONRYO,
        GH_THE_TWINS,
        GH_DEOGEN,
        GH_THAYE,
        GH_YOKAI,
        GH_WRAITH,
        GH_RAIJU,
        GH_MARE,
        GH_SPIRIT
    };

    if (list) {
        *list = ghost_types;
    }
    return (int)(sizeof(ghost_types) / sizeof(ghost_types[0]));
}

// ---- Thread-safe random number generation ----
int rand_int_threadsafe(int lower_inclusive, int upper_exclusive) {
    static _Thread_local unsigned seed = 0;

    if (upper_exclusive <= lower_inclusive) {
        return lower_inclusive;
    }

    if (seed == 0) {
        seed = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)pthread_self();
        if (seed == 0) {
            seed = 0xA5A5A5A5u;
        }
    }

    unsigned span = (unsigned)(upper_exclusive - lower_inclusive);
    unsigned value = (unsigned)rand_r(&seed) % span;
    return lower_inclusive + (int)value;
}

// ---- Evidence helpers ----
bool evidence_is_valid_ghost(EvidenceByte mask) {
    const enum GhostType* ghost_types = NULL;
    int ghost_count = get_all_ghost_types(&ghost_types);

    for (int index = 0; index < ghost_count; index++) {
        if (mask == (EvidenceByte)ghost_types[index]) {
            return true;
        }
    }

    return false;
}

// ---- Logging (Writes CSV logs, DO NOT MODIFY the file outputs: timestamp,type,id,room,device,boredom,fear,action,extra) ----

// These enums are just for logging purposes, not needed elsewhere
enum LogEntityType {
    LOG_ENTITY_HUNTER = 0,
    LOG_ENTITY_GHOST = 1
};

struct LogRecord {
    enum LogEntityType entity_type;
    int                entity_id;
    const char*        room;
    const char*        device;
    int                boredom;
    int                fear;
    const char*        action;
    const char*        extra;
};

static const char* log_entity_type_to_string(enum LogEntityType type) {
    switch (type) {
        case LOG_ENTITY_HUNTER:
            return "hunter";
        case LOG_ENTITY_GHOST:
            return "ghost";
        default:
            return "unknown";
    }
}

static void write_log_record(const struct LogRecord* record) {
    static _Thread_local unsigned line_count = 0;

    if (line_count >= 100000) {
        fprintf(stderr, "Log capped for entity %d; stopping to prevent infinite growth.\n", record->entity_id);
        exit(1);
    }

    char filename[64];
    snprintf(filename, sizeof(filename), "log_%d.csv", record->entity_id);

    FILE* log_file = fopen(filename, "a");

    if (!log_file) {
        return;
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long timestamp = (long long)tv.tv_sec * 1000LL + (long long)tv.tv_usec / 1000LL;

    const char* entity = log_entity_type_to_string(record->entity_type);
    const char* room = record->room ? record->room : "";
    const char* device = record->device ? record->device : "";
    const char* action = record->action ? record->action : "";
    const char* extra = record->extra ? record->extra : "";

    fprintf(log_file,
            "%lld,%s,%d,%s,%s,%d,%d,%s,%s\n",
            timestamp,
            entity,
            record->entity_id,
            room,
            device,
            record->boredom,
            record->fear,
            action,
            extra);

    fclose(log_file);
    line_count++;

    // Short pause helps ensure successive logs receive distinct timestamps.
    struct timespec pause = {0, 2 * 1000 * 1000}; // 2 ms
    nanosleep(&pause, NULL);
}

void log_move(int hunter_id, int boredom, int fear, const char* from_room, const char* to_room, enum EvidenceType device) {
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = from_room,
        .device = evidence_to_string(device),
        .boredom = boredom,
        .fear = fear,
        .action = "MOVE",
        .extra = to_room
    };

    write_log_record(&record);

    printf("Hunter %d using %s moved from %s to %s (bored=%d fear=%d)\n",
           hunter_id,
           evidence_to_string(device),
           from_room ? from_room : "",
           to_room ? to_room : "",
           boredom,
           fear);
}

void log_evidence(int hunter_id, int boredom, int fear, const char* room_name, enum EvidenceType device) {
    const char* evidence = evidence_to_string(device);
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = room_name,
        .device = evidence,
        .boredom = boredom,
        .fear = fear,
        .action = "EVIDENCE",
        .extra = evidence
    };

    write_log_record(&record);

    printf("Hunter %d using %s gathered evidence in %s (bored=%d fear=%d)\n",
           hunter_id,
           evidence,
           room_name ? room_name : "",
           boredom,
           fear);
}

void log_swap(int hunter_id, int boredom, int fear, enum EvidenceType from_device, enum EvidenceType to_device) {
    char extra[64];
    const char* from_text = evidence_to_string(from_device);
    const char* to_text = evidence_to_string(to_device);
    snprintf(extra, sizeof(extra), "%s->%s", from_text, to_text);

    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = NULL,
        .device = to_text,
        .boredom = boredom,
        .fear = fear,
        .action = "SWAP",
        .extra = extra
    };

    write_log_record(&record);

    printf("Hunter %d swapped devices: %s -> %s (bored=%d fear=%d)\n",
           hunter_id,
           from_text,
           to_text,
           boredom,
           fear);
}

void log_exit(int hunter_id, int boredom, int fear, const char* room_name, enum EvidenceType device, enum LogReason reason) {
    const char* device_text = evidence_to_string(device);
    const char* reason_text = exit_reason_to_string(reason);

    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = room_name,
        .device = device_text,
        .boredom = boredom,
        .fear = fear,
        .action = "EXIT",
        .extra = reason_text
    };

    write_log_record(&record);

    printf("Hunter %d using %s exited at %s (reason=%s, bored=%d fear=%d)\n",
           hunter_id,
           device_text,
           room_name ? room_name : "",
           reason_text,
           boredom,
           fear);
}

void log_return_to_van(int hunter_id, int boredom, int fear, const char* room_name, enum EvidenceType device, bool heading_home) {
    const char* device_text = evidence_to_string(device);
    const char* extra = heading_home ? "start" : "complete";
    const char* action = heading_home ? "RETURN_START" : "RETURN_COMPLETE";

    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = room_name,
        .device = device_text,
        .boredom = boredom,
        .fear = fear,
        .action = action,
        .extra = extra
    };

    write_log_record(&record);

    if (heading_home) {
        printf("Hunter %d using %s heading to van from %s (bored=%d fear=%d)\n",
               hunter_id,
               device_text,
               room_name ? room_name : "",
               boredom,
               fear);
    } else {
        printf("Hunter %d using %s finished return at %s (bored=%d fear=%d)\n",
               hunter_id,
               device_text,
               room_name ? room_name : "",
               boredom,
               fear);
    }
}

void log_hunter_init(int hunter_id, const char* room_name, const char* hunter_name, enum EvidenceType device) {
    const char* device_text = evidence_to_string(device);
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_HUNTER,
        .entity_id = hunter_id,
        .room = room_name,
        .device = device_text,
        .boredom = 0,
        .fear = 0,
        .action = "INIT",
        .extra = hunter_name ? hunter_name : ""
    };

    write_log_record(&record);
    printf("Hunter %d (%s) initialized in %s with %s\n",
           hunter_id,
           hunter_name ? hunter_name : "unknown",
           room_name ? room_name : "",
           device_text);
}

void log_ghost_init(int ghost_id, const char* room_name, enum GhostType type) {
    const char* type_text = ghost_to_string(type);
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_GHOST,
        .entity_id = ghost_id,
        .room = room_name,
        .device = NULL,
        .boredom = 0,
        .fear = 0,
        .action = "INIT",
        .extra = type_text
    };

    write_log_record(&record);
    printf("Ghost %d (%s) initialized in %s\n",
           ghost_id,
           type_text,
           room_name ? room_name : "");
}

void log_ghost_move(int ghost_id, int boredom, const char* from_room, const char* to_room) {
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_GHOST,
        .entity_id = ghost_id,
        .room = from_room,
        .device = NULL,
        .boredom = boredom,
        .fear = 0,
        .action = "MOVE",
        .extra = to_room
    };

    write_log_record(&record);

    printf("Ghost %d [bored=%d] MOVE %s -> %s\n",
           ghost_id,
           boredom,
           from_room ? from_room : "",
           to_room ? to_room : "");
}

void log_ghost_evidence(int ghost_id, int boredom, const char* room_name, enum EvidenceType evidence) {
    const char* evidence_text = evidence_to_string(evidence);

    struct LogRecord record = {
        .entity_type = LOG_ENTITY_GHOST,
        .entity_id = ghost_id,
        .room = room_name,
        .device = NULL,
        .boredom = boredom,
        .fear = 0,
        .action = "EVIDENCE",
        .extra = evidence_text
    };

    write_log_record(&record);

    printf("Ghost %d [bored=%d] EVIDENCE %s in %s\n",
           ghost_id,
           boredom,
           evidence_text,
           room_name ? room_name : "");
}

void log_ghost_exit(int ghost_id, int boredom, const char* room_name) {
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_GHOST,
        .entity_id = ghost_id,
        .room = room_name,
        .device = NULL,
        .boredom = boredom,
        .fear = 0,
        .action = "EXIT",
        .extra = ""
    };

    write_log_record(&record);

    printf("Ghost %d [bored=%d] EXIT %s\n",
           ghost_id,
           boredom,
           room_name ? room_name : "");
}

void log_ghost_idle(int ghost_id, int boredom, const char* room_name) {
    struct LogRecord record = {
        .entity_type = LOG_ENTITY_GHOST,
        .entity_id = ghost_id,
        .room = room_name,
        .device = NULL,
        .boredom = boredom,
        .fear = 0,
        .action = "IDLE",
        .extra = ""
    };

    write_log_record(&record);

    printf("Ghost %d [bored=%d] IDLE in %s\n",
           ghost_id,
           boredom,
           room_name ? room_name : "");
}

void room_init(struct Room* room, const char* name, bool is_exit) {
    strcpy(room->name, name); // copies name
    room->is_exit = is_exit; // copies exit status
    
    room->numConnections = 0;
    room->ghost = NULL;
    room->numHunters = 0;
    room->evidence = 0;

    // create space for connected rooms
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        room->connectedRooms[i] = NULL;
    }

    // create space for hunters
    for (int i = 0; i < MAX_ROOM_OCCUPANCY; i++) {
        room->hunters[i] = NULL;
    }

    sem_init(&room->mutex, 0, 1);
}

void room_connect(struct Room* a, struct Room* b) { // Bidirectional connection
    if (a->numConnections == MAX_CONNECTIONS || b->numConnections == MAX_CONNECTIONS) return; // check connections capacity

    // check for duplicate rooms
    for (int i = 0; i < a->numConnections; i++) {
        if (a->connectedRooms[i] == b) return;
    }

    a->connectedRooms[a->numConnections] = b;
    b->connectedRooms[b->numConnections] = a;
    a->numConnections++;
    b->numConnections++;
}

void lockRooms(struct Room *r1, struct Room *r2) {
    if (r1 == r2) {
        sem_wait(&r1->mutex);
        return;
    }

    if ((uintptr_t)r1 < (uintptr_t)r2) {
        sem_wait(&r1->mutex);
        sem_wait(&r2->mutex);
    }

    else {
        sem_wait(&r2->mutex);
        sem_wait(&r1->mutex);
    }
}

void unlockRooms(struct Room *r1, struct Room *r2) {
    if (r1 == r2) {
        sem_post(&r1->mutex);
        return;
    }

    if ((uintptr_t)r1 < (uintptr_t)r2) {
        sem_post(&r2->mutex);
        sem_post(&r1->mutex);
    }

    else {
        sem_post(&r1->mutex);
        sem_post(&r2->mutex);
    }
}

void huntersResize(struct House* house, int* capacity) {
    *capacity *= 2;
    struct Hunter** newArr = realloc(house->hunters, sizeof(struct Hunter*) * (*capacity));
    house->hunters = newArr;
}

void ghostIdle(struct Ghost *ghost) {
    pthread_mutex_lock(&ghost->boredom_mutex);
    int current_boredom = ghost->boredom;
    pthread_mutex_unlock(&ghost->boredom_mutex);
    log_ghost_idle(ghost->id, current_boredom, ghost->room->name);
}

void ghostHaunt(struct Ghost *ghost) {
    enum EvidenceType ev = get_random_evidence(ghost->type);

    sem_wait(&ghost->room->mutex);

    // Only add if the evidence isn't already there
    if (!(ghost->room->evidence & ev)) {
        ghost->room->evidence |= ev;
        sem_post(&ghost->room->mutex);

        pthread_mutex_lock(&ghost->boredom_mutex);
        int current_boredom = ghost->boredom;
        pthread_mutex_unlock(&ghost->boredom_mutex);

        log_ghost_evidence(ghost->id, current_boredom, ghost->room->name, ev);
    } else {
        sem_post(&ghost->room->mutex);
    }
}

void ghostMove(struct Ghost *ghost) {
    sem_wait(&ghost->room->mutex);
    struct Room *oldRoom = ghost->room;
    int count = oldRoom->numConnections;

    if (count == 0) {
        sem_post(&oldRoom->mutex);
        return;
    }

    int index = rand_int_threadsafe(0, count);
    struct Room *newRoom = oldRoom->connectedRooms[index];
    sem_post(&oldRoom->mutex);
    lockRooms(oldRoom, newRoom);

    if (ghost->room == oldRoom) {
        newRoom->ghost = ghost;
        oldRoom->ghost = NULL;
        ghost->room = newRoom;

        pthread_mutex_lock(&ghost->boredom_mutex);
        int current_boredom = ghost->boredom;
        pthread_mutex_unlock(&ghost->boredom_mutex);
        log_ghost_move(ghost->id, current_boredom, oldRoom->name, newRoom->name);
    }
    unlockRooms(oldRoom, newRoom);
}

enum EvidenceType get_random_evidence(enum GhostType ghostType) {
    const enum EvidenceType* all;
    int total = get_all_evidence_types(&all);  // gives pointer + count

    enum EvidenceType possible[3];
    int count = 0;

    // collect only the evidence bits that match this ghost type
    for (int i = 0; i < total; i++) {
        if (ghostType & all[i]) {
            possible[count++] = all[i];
        }
    }

    // randomly choose one of the 3 allowed evidence types
    if (count == 0) return EV_EMF;
    int idx = rand_int_threadsafe(0, count);
    return possible[idx];
}

void *ghostFunction(void *arg) {
    struct Ghost *ghost = (struct Ghost *)arg;
    int choice;
    while (!ghost->exited) {
        // protect access to numHunters
        sem_wait(&ghost->room->mutex);
        int huntersInRoom = ghost->room->numHunters;
        sem_post(&ghost->room->mutex);
        
        // at least 1 hunter in the room
        if (huntersInRoom > 0) {
            pthread_mutex_lock(&ghost->boredom_mutex);
            ghost->boredom = 0;
            pthread_mutex_unlock(&ghost->boredom_mutex);
            choice = rand_int_threadsafe(1, 3); // range 1-2 because ghost cannot move
        }
        
        // no hunters in the room
        else {
            pthread_mutex_lock(&ghost->boredom_mutex);
            ghost->boredom++;
            int current_boredom = ghost->boredom;
            pthread_mutex_unlock(&ghost->boredom_mutex);
            choice = rand_int_threadsafe(1, 4); // range 1-3
        }
        
        pthread_mutex_lock(&ghost->boredom_mutex);
        bool shouldExit = (ghost->boredom > ENTITY_BOREDOM_MAX);
        if (shouldExit) {
            ghost->exited = true;
            int final_boredom = ghost->boredom;
            pthread_mutex_unlock(&ghost->boredom_mutex);
            log_ghost_exit(ghost->id, final_boredom, ghost->room->name);
            break;
        }
        pthread_mutex_unlock(&ghost->boredom_mutex);

        // otherwise continue
        switch (choice)
        {
            case 1: // ghost idles
                ghostIdle(ghost);
                break;
            case 2: // ghost haunts
                ghostHaunt(ghost);
                break;
            case 3: // ghost moves
                ghostMove(ghost);
                break;
        }
        usleep(100 * 1000);
    }
    return NULL;
}

void hunterRemove(struct Hunter *hunter, struct Room *room) {
    if (!room) return;
    
    // Room mutex should already be locked by caller (lockRooms)
    for (int i = 0; i < room->numHunters; i++) {
        if (hunter == room->hunters[i]) {
            // Shift array to remove the hunter
            for (int j = i; j < room->numHunters - 1; j++) {
                room->hunters[j] = room->hunters[j + 1];
            }
            room->hunters[room->numHunters - 1] = NULL;
            room->numHunters--;
            hunter->room = NULL;
            return;
        }
    }
}

void hunterAdd(struct Hunter *hunter, struct Room *room) {
    if (!room) return;
    
    // Room mutex should already be locked by caller (lockRooms)
    if (room->numHunters < MAX_ROOM_OCCUPANCY) {
        room->hunters[room->numHunters] = hunter;
        room->numHunters++;
        hunter->room = room;
    }
}

void hunterMove(struct Hunter *hunter) {
    struct Room *oldRoom = hunter->room;

    sem_wait(&oldRoom->mutex);
    int connections = oldRoom->numConnections;
    sem_post(&oldRoom->mutex);

    int index = rand_int_threadsafe(0, connections);
    struct Room *newRoom = oldRoom->connectedRooms[index];

    lockRooms(oldRoom, newRoom);

    if (newRoom->numHunters < MAX_ROOM_OCCUPANCY) {
        stackPush(&hunter->path, oldRoom);
        log_move(hunter->id, hunter->boredom, hunter->fear, oldRoom->name, newRoom->name, hunter->device);
        hunterRemove(hunter, oldRoom);
        hunterAdd(hunter, newRoom);
        
    }

    unlockRooms(oldRoom, newRoom);
}

void stackInit(struct RoomStack *stack) {
    stack->head = NULL;
}

void stackPush(struct RoomStack *stack, struct Room *room) {
    struct RoomNode *node = malloc(sizeof(struct RoomNode));
    node->room = room;
    node->next = stack->head;
    stack->head = node;
}

struct Room* stackPop(struct RoomStack *stack) {
    if (stack->head == NULL) return NULL;

    struct RoomNode *node = stack->head;
    stack->head = node->next;

    struct Room *room = node->room;
    free(node);
    return room;
}

struct Room* stackPeek(struct RoomStack *stack) {
    if (stack->head == NULL) return NULL;
    return stack->head->room;
}

void stackClear(struct RoomStack *stack) {
    while (stackPop(stack) != NULL);
}

void *hunterFunction(void *arg) {
    struct Hunter *hunter = arg;

    while (!hunter->exited) {
        // ATOMIC OPERATION: Check ghost and update stats in one critical section
        pthread_mutex_lock(&hunter->mutex);
        
        // Check ghost presence
        sem_wait(&hunter->room->mutex);
        int ghostHere = (hunter->room->ghost != NULL);
        sem_post(&hunter->room->mutex);
        
        // Update stats based on ghost presence
        if (ghostHere) {
            hunter->fear++;
            hunter->boredom = 0; // Reset boredom when ghost is present
        } else {
            hunter->boredom++;
        }
        
        // Get current values for exit checks
        int current_boredom = hunter->boredom;
        int current_fear = hunter->fear;
        bool shouldExitBored = (current_boredom > ENTITY_BOREDOM_MAX);
        bool shouldExitFear = (current_fear > HUNTER_FEAR_MAX);
        
        pthread_mutex_unlock(&hunter->mutex);

        // Check exit conditions
        if (shouldExitBored) {
            log_exit(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device, LR_BORED);
            pthread_mutex_lock(&hunter->mutex);
            hunter->exitReason = LR_BORED;
            hunter->exited = true;
            pthread_mutex_unlock(&hunter->mutex);
            hunterRemove(hunter, hunter->room);
            stackClear(&hunter->path);
            return NULL;
        }
        if (shouldExitFear) {
            log_exit(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device, LR_AFRAID);
            pthread_mutex_lock(&hunter->mutex);
            hunter->exitReason = LR_AFRAID;
            hunter->exited = true;
            pthread_mutex_unlock(&hunter->mutex);
            hunterRemove(hunter, hunter->room);
            stackClear(&hunter->path);
            return NULL;
        }

        // RETURNING HUNTER MOVEMENT
        if (hunter->returning) {
            struct Room* next = stackPop(&hunter->path);
            if (next != NULL) {
                struct Room* oldRoom = hunter->room;
                lockRooms(oldRoom, next);

                if (next->numHunters < MAX_ROOM_OCCUPANCY) {
                    hunterRemove(hunter, oldRoom);
                    hunterAdd(hunter, next);

                    // Get updated stats for logging
                    pthread_mutex_lock(&hunter->mutex);
                    current_boredom = hunter->boredom;
                    current_fear = hunter->fear;
                    pthread_mutex_unlock(&hunter->mutex);
                    
                    log_move(hunter->id, current_boredom, current_fear, oldRoom->name, next->name, hunter->device);
                } else {
                    // If target room is full, push the room back onto stack
                    stackPush(&hunter->path, next);
                }
                unlockRooms(oldRoom, next);
            }
            // Skip regular movement if returning
            usleep(100 * 1000);
            continue;
        }

        // VAN ARRIVAL CHECK
        if (hunter->room->is_exit && hunter->returning) {
            log_return_to_van(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device, false);
            hunter->returning = false;
            stackClear(&hunter->path);

            // Check victory
            const enum GhostType* ghostTypes;
            int count = get_all_ghost_types(&ghostTypes);

            sem_wait(&hunter->casefile->mutex);
            bool solved = false;
            for (int i = 0; i < count; i++) {
                if ((hunter->casefile->collected & ghostTypes[i]) == ghostTypes[i]) {
                    hunter->casefile->solved = true;
                    solved = true;
                    break;
                }
            }
            sem_post(&hunter->casefile->mutex);
            
            if (solved) {
                log_exit(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device, LR_EVIDENCE);
                pthread_mutex_lock(&hunter->mutex);
                hunter->exitReason = LR_EVIDENCE;
                hunter->exited = true;
                pthread_mutex_unlock(&hunter->mutex);
                hunterRemove(hunter, hunter->room);
                stackClear(&hunter->path);
                return NULL;
            }

            // Swap device
            enum EvidenceType oldDevice = hunter->device;
            enum EvidenceType devices[] = {EV_EMF, EV_ORBS, EV_RADIO, EV_TEMPERATURE, EV_FINGERPRINTS, EV_WRITING, EV_INFRARED};
            while (true) {
                enum EvidenceType newDevice = devices[rand_int_threadsafe(0, 7)];
                if (newDevice != hunter->device) {
                    log_swap(hunter->id, current_boredom, current_fear, oldDevice, newDevice);
                    hunter->device = newDevice;
                    break;
                }
            }
        }

        // Evidence gathering
        bool matched = false;
        sem_wait(&hunter->room->mutex);
        EvidenceByte ev = hunter->room->evidence;

        if (ev & hunter->device) {
            log_evidence(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device);
            hunter->room->evidence &= ~hunter->device;
            matched = true;
        }
        sem_post(&hunter->room->mutex);

        if (matched) {
            sem_wait(&hunter->casefile->mutex);
            hunter->casefile->collected |= hunter->device;
            sem_post(&hunter->casefile->mutex);

            // Only start returning if not already at van
            if (!hunter->room->is_exit) {
                hunter->returning = true;
                log_return_to_van(hunter->id, current_boredom, current_fear, hunter->room->name, hunter->device, true);
            }
        }
                
        // Regular movement (only for non-returning hunters)
        if (!hunter->returning) {
            hunterMove(hunter);
        }
        
        usleep(100 * 1000);
    }
    return NULL;
}

void houseCleanup(struct House* house) {
    // Destroy room semaphores
    for (int i = 0; i < house->roomCount; i++) {
        sem_destroy(&house->rooms[i].mutex);
    }

    // Destroy hunter mutexes and cleanup hunters
    if (house->hunters) {
        for (int i = 0; i < house->hunterCount; i++) {
            pthread_mutex_destroy(&house->hunters[i]->mutex); 
            stackClear(&house->hunters[i]->path);
            free(house->hunters[i]);
        }
        free(house->hunters);
        house->hunters = NULL;
    }

    // Destroy casefile semaphore
    sem_destroy(&house->casefile.mutex); 

    // destroy ghost mutex
    pthread_mutex_destroy(&house->ghost.boredom_mutex); 
}




enum GhostType mask_to_ghost(EvidenceByte mask) {
    switch (mask) {
        case EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING:   return GH_POLTERGEIST;
        case EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO:     return GH_THE_MIMIC;
        case EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS:      return GH_HANTU;
        case EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF:       return GH_JINN;
        case EV_FINGERPRINTS | EV_INFRARED | EV_RADIO:        return GH_PHANTOM;
        case EV_FINGERPRINTS | EV_INFRARED | EV_ORBS:         return GH_BANSHEE;
        case EV_FINGERPRINTS | EV_INFRARED | EV_EMF:          return GH_GORYO;
        case EV_FINGERPRINTS | EV_WRITING | EV_RADIO:         return GH_BULLIES;
        case EV_FINGERPRINTS | EV_WRITING | EV_EMF:           return GH_MYLING;
        case EV_FINGERPRINTS | EV_ORBS | EV_EMF:              return GH_OBAKE;
        case EV_TEMPERATURE | EV_INFRARED | EV_ORBS:          return GH_YUREI;
        case EV_TEMPERATURE | EV_INFRARED | EV_EMF:           return GH_ONI;
        case EV_TEMPERATURE | EV_WRITING | EV_RADIO:          return GH_MOROI;
        case EV_TEMPERATURE | EV_WRITING | EV_ORBS:           return GH_REVENANT;
        case EV_TEMPERATURE | EV_WRITING | EV_EMF:            return GH_SHADE;
        case EV_TEMPERATURE | EV_RADIO | EV_ORBS:             return GH_ONRYO;
        case EV_TEMPERATURE | EV_RADIO | EV_EMF:              return GH_THE_TWINS;
        case EV_INFRARED | EV_WRITING | EV_RADIO:             return GH_DEOGEN;
        case EV_INFRARED | EV_WRITING | EV_ORBS:              return GH_THAYE;
        case EV_INFRARED | EV_RADIO | EV_ORBS:                return GH_YOKAI;
        case EV_INFRARED | EV_RADIO | EV_EMF:                 return GH_WRAITH;
        case EV_INFRARED | EV_ORBS | EV_EMF:                  return GH_RAIJU;
        case EV_WRITING | EV_RADIO | EV_ORBS:                 return GH_MARE;
        case EV_WRITING | EV_RADIO | EV_EMF:                  return GH_SPIRIT;
        default:                                             return 0;
    }
}