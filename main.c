#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct blockLL{
    void* mallocAddr;
    int size;
    struct blockLL* next;
} blockLL;

struct blockLL root = {0};
struct blockLL* tail = &root;

void* mallocVis(size_t size)
{
    tail->next = malloc(sizeof(blockLL));
    tail = tail->next;

    tail->mallocAddr = malloc(size);
    tail->size = size;
    tail->next = NULL;
    
    return tail->mallocAddr;
}


bool freeVis(void* ptr)
{
    struct blockLL* iterator = &root;
    while(iterator)
    {
        struct blockLL* next = iterator->next;
        if(next && ptr == next->mallocAddr)
        {
            iterator->next = next->next;
            if(tail == next)
                tail = iterator;
            free(ptr);
            free(next);
            return true;
        }
        iterator = next;
    }
    free(ptr);
    return false;
}

//+++TESTING+++

// Structure to keep track of all allocated pointers
typedef struct {
    void** pointers;
    size_t count;
    size_t capacity;
} AllocationTracker;

static AllocationTracker tracker = { NULL, 0, 0 };

// Helper: Add a pointer to the tracker
static void track_pointer(void* ptr) {
    if (tracker.count == tracker.capacity) {
        tracker.capacity = tracker.capacity == 0 ? 16 : tracker.capacity * 2;
        tracker.pointers = realloc(tracker.pointers, tracker.capacity * sizeof(void*));
        if (!tracker.pointers) {
            fprintf(stderr, "Failed to realloc tracker\n");
            exit(EXIT_FAILURE);
        }
    }
    tracker.pointers[tracker.count++] = ptr;
}

// First function: malloc and randomly free some blocks
void random_malloc_and_partial_free(unsigned int seed, size_t num_operations) {
    // Clean any previous state (optional, for multiple calls)
    for (size_t i = 0; i < tracker.count; i++) {
        freeVis(tracker.pointers[i]);
    }
    free(tracker.pointers);
    tracker.pointers = NULL;
    tracker.count = 0;
    tracker.capacity = 0;

    srand(seed);

    for (size_t i = 0; i < num_operations; i++) {
        // Random block size between 1 and 4096 bytes
        size_t size = 1 + (rand() % 4096);

        void* ptr = mallocVis(size);
        if (!ptr) {
            perror("malloc failed");
            continue;
        }

        // Fill with some data so it's not optimized away
        for (size_t j = 0; j < size; j++) {
            ((char*)ptr)[j] = (char)(i + j);
        }

        track_pointer(ptr);

        // Randomly decide whether to free this block immediately (50% chance)
        if (rand() % 2) {
            freeVis(ptr);
            // Remove from tracker (replace with last element)
            for (size_t j = 0; j < tracker.count; j++) {
                if (tracker.pointers[j] == ptr) {
                    tracker.pointers[j] = tracker.pointers[--tracker.count];
                    break;
                }
            }
        }
    }

    printf("After random_malloc_and_partial_free:\n");
    printf("  Operations: %zu\n", num_operations);
    printf("  Still allocated: %zu blocks\n", tracker.count);
}

// Second function: free everything that remains allocated
void free_all_remaining(void) {
    printf("Freeing all remaining %zu blocks...\n", tracker.count);

    for (size_t i = 0; i < tracker.count; i++) {
        freeVis(tracker.pointers[i]);
    }
    freeVis(tracker.pointers);

    tracker.pointers = NULL;
    tracker.count = 0;
    tracker.capacity = 0;

    printf("All memory freed.\n");
}

//+++TESTING+++



int main()
{
    tail = &root;
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Cyberspace");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 16.0f, 14.0f, 16.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    bool pause = false;     // Pause camera orbital rotation (and zoom)
    unsigned int selectBlock = 0;
    unsigned int limitX = 10;
    unsigned int blockCount = 1;
    float cubeScale = 1.0f;
    #define MIN_DIS_SCALE 0.00000001f
    float distanceScale = MIN_DIS_SCALE;
    float transLerp = 0.0f;

    //test
    random_malloc_and_partial_free(12345, 1000);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_P)) pause = !pause;
        if (IsKeyDown(KEY_LEFT)) {selectBlock--; transLerp = 0.0f;}
        if (IsKeyDown(KEY_RIGHT)) {selectBlock++; transLerp = 0.0f;}
        if (IsKeyDown(KEY_UP)) {selectBlock += limitX; transLerp = 0.0f;}
        if (IsKeyDown(KEY_DOWN)) {selectBlock -= limitX; transLerp = 0.0f;}
        if (IsKeyDown(KEY_I) && limitX < UINT_MAX) {limitX++; transLerp = 0.0f;}
        if (IsKeyDown(KEY_K) && limitX > 1) {limitX--; transLerp = 0.0f;}
        if (IsKeyDown(KEY_J) && distanceScale > MIN_DIS_SCALE) {distanceScale/=1.1f; transLerp = 0.999f;}
        if (IsKeyDown(KEY_L) && distanceScale < 1.0f / MIN_DIS_SCALE) {distanceScale*=1.1f; transLerp = 0.999f;}
        if (IsKeyDown(KEY_U)) {cubeScale/=1.1f; transLerp = 0.999f;}
        if (IsKeyDown(KEY_O)) {cubeScale*=1.1f; transLerp = 0.999f;}

        if (IsKeyPressed(KEY_T))//new test
        {
            free_all_remaining();
            random_malloc_and_partial_free(12345, 1000);
        }

        if (!pause) UpdateCamera(&camera, CAMERA_ORBITAL);
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                struct blockLL* iterator = root.next;
                unsigned long referenceAddressPos = (unsigned long)(uintptr_t)root.mallocAddr;
                Vector3 cubePosition = {0};
                Vector3 lastPosition = {0};
                unsigned int index = 0;

                while(iterator)
                {
                    cubePosition.x = (referenceAddressPos - (unsigned long)(uintptr_t)iterator->mallocAddr) * distanceScale * cubeScale;
                    cubePosition.z = (index % limitX) * cubeScale;
                    cubePosition.y = (index / limitX) * cubeScale;
                    DrawCubeWires(cubePosition, cubeScale * iterator->size * distanceScale, cubeScale, cubeScale, RED);
                    DrawCube(cubePosition, cubeScale * iterator->size * distanceScale/10.0f, cubeScale/10.0f, cubeScale/10.0f, BLACK);
                    DrawLine3D(lastPosition , cubePosition , BLUE);
                    lastPosition = cubePosition;
                    iterator = iterator->next;
                    if(index == selectBlock % blockCount && transLerp < 1.0f)
                    {
                        transLerp += 0.1f;
                        if (transLerp > 1.0f)
                            transLerp = 1.0f;
                        camera.target = Vector3Lerp(camera.target , cubePosition , transLerp);
                        cubePosition.y += 4.5f;
                        cubePosition.x += 3.5f;
                        camera.position = Vector3Lerp(camera.position , cubePosition , transLerp);
                    }
                    index++;
                }

                blockCount = index;

            EndMode3D();
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    free_all_remaining();
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}