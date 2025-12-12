#include "raylib.h"
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
    freeVis(tracker.pointers);
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

    Vector3 mapPosition = { -16.0f, 0.0f, -8.0f };          // Set model position
    bool pause = false;     // Pause camera orbital rotation (and zoom)
    int selectBlock = 0;
    int blockCount = 1;
    float distanceScale = 0.00000001f;

    //test
    random_malloc_and_partial_free(12345, 1000);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_P)) pause = !pause;
        if (IsKeyPressed(KEY_LEFT)) selectBlock--;
        if (IsKeyPressed(KEY_RIGHT)) selectBlock++;
        if (IsKeyPressed(KEY_UP)) distanceScale *= 10.0f;
        if (IsKeyPressed(KEY_DOWN)) distanceScale /= 10.0f;
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
                long referenceAddressPos = (long)(uintptr_t)root.mallocAddr;
                Vector3 cubePosition = mapPosition;
                float cubeScale = 1.0f;
                int index = 0;

                while(iterator)
                {
                    cubePosition.x = (float){referenceAddressPos} - distanceScale * (float)(long)(uintptr_t)iterator->mallocAddr;
                    DrawCubeWires(cubePosition, cubeScale, cubeScale, cubeScale, RED);

                    iterator = iterator->next;
                    if(index == selectBlock % blockCount)
                    {
                        //camera.position = cubePosition;
                        //camera.position.x += 0.5f;
                        camera.target = cubePosition;
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