#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defs.h"
#include "helpers.h"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define RESET   "\x1b[0m"

int main() {

    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

    struct House house; // initialize house structure
    house_populate_rooms(&house); // populate rooms
    house.casefile.collected = 0;
    house.casefile.solved = false;
    sem_init(&house.casefile.mutex, 0, 1);

    
    // choose random ghost type
    const enum GhostType* ghostTypes;
    int count = get_all_ghost_types(&ghostTypes);
    int selected = ghostTypes[rand_int_threadsafe(0, count)];
    house.ghost.type = selected;
    
    // create ghost
    house.ghost.id = DEFAULT_GHOST_ID; 
    house.ghost.room = &house.rooms[rand_int_threadsafe(0, house.roomCount)];
    house.ghost.room->ghost = &house.ghost;
    house.ghost.boredom = 0;
    house.ghost.exited = false;
    pthread_mutex_init(&house.ghost.boredom_mutex, NULL);
    log_ghost_init(house.ghost.id, house.ghost.room->name, house.ghost.type); // initialize ghost data

    // prepare hunter array
    int huntersArrSize = 8;
    house.hunters = calloc(huntersArrSize, sizeof(struct Hunter*));
    house.hunterCount = 0;

    enum EvidenceType devices[] = {EV_EMF, EV_ORBS, EV_RADIO, EV_TEMPERATURE, EV_FINGERPRINTS, EV_WRITING, EV_INFRARED};

    // menu messages
    printf("==========================\nWillow House Investigation\n==========================\n");
    printf("Enter hunters one at a time. Type 'done' as the name to finish.");

    char hunterName [MAX_HUNTER_NAME];
    int hunterID;
    bool inputtingHunters = true;
    
    while (inputtingHunters) {     
        // enter hunter's name
        while (true) {
            printf("\nEnter hunter's name (max 63 characters) or 'done' to finish: ");
        
            if (fgets(hunterName, sizeof(hunterName), stdin) == NULL) {
                fprintf(stderr, "Error reading input.\n");
                exit(1); // or handle gracefully
            }
            hunterName[strcspn(hunterName, "\n")] = '\0';
            
            // do not exit if there are 0 hunters
            if (strcmp("done", hunterName) == 0 && house.hunterCount == 0){
                printf("\nError: Add at least 1 hunter");
            }

            // do not exit if the name is blank
            if (strlen(hunterName) == 0) {
                printf("\nError: Enter a valid name");
            }
            
            // exit loop when "done is entered"
            else if (strcmp("done", hunterName) == 0 && house.hunterCount > 0){
                inputtingHunters = false;
                break;
            }
            else break;
        }

        if (!inputtingHunters) break;
    
        // enter hunter's ID
        while (true) {
            printf("Enter hunter ID (integer): ");

            // user enters a non int
            if (scanf("%d", &hunterID) == 1) break;
            printf("Error: Value was not an integer\n");
            
            // clear invalid input so the loop doesn’t infinite-loop
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {}
        }
        
        // clear invalid input so the loop doesn’t infinite loop
        int d;
        while ((d = getchar()) != '\n' && d != EOF) {}
        
        // if hunter array is full, resize
        if (house.hunterCount == huntersArrSize) {
            huntersResize(&house, &huntersArrSize);
        }
        
        // dynamically add space in the array
        house.hunters[house.hunterCount] = malloc(sizeof(struct Hunter));

        // initialize fields
        strcpy(house.hunters[house.hunterCount]->name, hunterName); // name
        house.hunters[house.hunterCount]->id = hunterID; // id
        house.hunters[house.hunterCount]->room = house.starting_room; // room
        house.hunters[house.hunterCount]->casefile = &house.casefile; // casefile
        house.hunters[house.hunterCount]->device = devices[rand_int_threadsafe(0, 7)]; // device
        house.hunters[house.hunterCount]->fear= 0; // fear
        house.hunters[house.hunterCount]->boredom = 0; // boredom
        house.hunters[house.hunterCount]->exited = false; // exited
        house.hunters[house.hunterCount]->returning = false; // returning to van
        stackInit(&house.hunters[house.hunterCount]->path); // room stack creation
        pthread_mutex_init(&house.hunters[house.hunterCount]->mutex, NULL);

        // log hunter initialization
        log_hunter_init(hunterID, house.starting_room->name, hunterName, house.hunters[house.hunterCount]->device);
        
        // check if there's room in the starting room 
        if (house.starting_room->numHunters < MAX_ROOM_OCCUPANCY) {
            int index = house.starting_room->numHunters;
            house.starting_room->hunters[index] = house.hunters[house.hunterCount];
            house.starting_room->numHunters++; // increase # of hunters in starting room
        }        
        house.hunterCount++;
    }

    // create ghost thread
    pthread_t ghostThread;
    pthread_create(&ghostThread, NULL, ghostFunction, &house.ghost);

    // create one thread for each hunter
    pthread_t *hunterThreads = calloc(house.hunterCount, sizeof(pthread_t));
    for (int i = 0; i < house.hunterCount; i++) {
        pthread_create(&hunterThreads[i], NULL, hunterFunction, house.hunters[i]);
    }

    // join threads
    pthread_join(ghostThread, NULL);
    for (int i = 0; i < house.hunterCount; i++) {
        pthread_join(hunterThreads[i], NULL);
    }

    for (int i = 0; i < house.hunterCount; i++) {
        pthread_mutex_destroy(&house.hunters[i]->mutex);
    }

    pthread_mutex_destroy(&house.ghost.boredom_mutex);

    printf("==========================\nInvestigation Results:\n==========================\n");
    
    for (int i = 0; i < house.hunterCount; i++) {
        const char* reason = exit_reason_to_string(house.hunters[i]->exitReason);

        // if hunter left because of evidence add a check
        if (house.hunters[i]->exitReason == LR_EVIDENCE) {
            printf("[" GREEN "✔" RESET "] Hunter %s (ID %d) exited because of [%s] (bored=%d fear=%d)\n", house.hunters[i]->name, house.hunters[i]->id, reason, house.hunters[i]->boredom, house.hunters[i]->fear);
        }

        else {
            printf("[" RED "✖" RESET "] Hunter %s (ID %d) exited because of [%s] (bored=%d fear=%d)\n", house.hunters[i]->name, house.hunters[i]->id, reason, house.hunters[i]->boredom, house.hunters[i]->fear);
        }

        
    }
    
    int num = 0;
    printf("\nShared Case File Checklist:\n");
    
    // case file checklist
    if (house.casefile.collected & EV_EMF) {
        num++;
        printf("  - [" GREEN "✔" RESET "] emf\n");
    }

    else printf("  - [" RED "✖" RESET "] emf\n");

    if (house.casefile.collected & EV_ORBS) {
        num++;
        printf("  - [" GREEN "✔" RESET "] orbs\n");
    }

    else printf("  - [" RED "✖" RESET "] orbs\n");

    if (house.casefile.collected & EV_RADIO) {
        num++;
        printf("  - [" GREEN "✔" RESET "] radio\n");
    }

    else printf("  - [" RED "✖" RESET "] radio\n");

    if (house.casefile.collected & EV_TEMPERATURE) {
        num++;
        printf("  - [" GREEN "✔" RESET "] temp\n");
    }

    else printf("  - [" RED "✖" RESET "] temp\n");

    if (house.casefile.collected & EV_FINGERPRINTS) {
        num++;
        printf("  - [" GREEN "✔" RESET "] prints\n");
    }

    else printf("  - [" RED "✖" RESET "] prints\n");

    if (house.casefile.collected & EV_WRITING) {
        num++;
        printf("  - [" GREEN "✔" RESET "] writing\n");
    }

    else printf("  - [" RED "✖" RESET "] writing\n");

    if (house.casefile.collected & EV_INFRARED) {
        num++;
        printf("  - [" GREEN "✔" RESET "] infrared\n\n");
    }

    else printf("  - [" RED "✖" RESET "] infrared\n\n");

    printf("Victory Results:\n--------------------------\n");
    printf("- Hunters exited after identifying the ghost: %d/%d\n", num, house.hunterCount);
    
    // check if collected evidence can make a ghost
    if (!evidence_is_valid_ghost(house.casefile.collected)) 
        printf("- Ghost Guess: N/A\n");
    else 
        printf("- Ghost Guess: %s\n", ghost_to_string(mask_to_ghost(house.casefile.collected)));

    printf("- Actual Ghost Type: %s\n", ghost_to_string(house.ghost.type));
    
    // if guess = ghost type, hunters win
    if (house.ghost.type == house.casefile.collected) 
        printf("Overall Result: " GREEN "Hunters Win!\n" RESET);

    // otherwise ghost wins
    else
        printf("Overall Result: " RED "Ghost Wins!\n" RESET);

    // cleanup
    houseCleanup(&house);
    free(hunterThreads);
    
    return 0;
}
