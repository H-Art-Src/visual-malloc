#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct blockLL{
    void* mallocAddr;
    int size;
    struct blockLL* next;
} blockLL;

struct blockLL root = {0};
struct blockLL* tail;

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
        if(ptr == next->mallocAddr)
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

int main()
{

    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [models] example - cubicmap rendering");

    // Define the camera to look into our 3d world
    Camera camera = { 0 };
    camera.position = (Vector3){ 16.0f, 14.0f, 16.0f };     // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };          // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };              // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                    // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;                 // Camera projection type

    bool pause = false;     // Pause camera orbital rotation (and zoom)

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    Vector3 mapPosition = { -16.0f, 0.0f, -8.0f };          // Set model position

    int selectBlock = 0;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (IsKeyPressed(KEY_P)) pause = !pause;
        if (IsKeyPressed(KEY_LEFT)) selectBlock--;
        if (IsKeyPressed(KEY_RIGHT)) selectBlock++;

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
                    cubePosition.x = (float){referenceAddressPos - (long)(uintptr_t)root.mallocAddr};
                    DrawCubeWires(cubePosition, cubeScale, cubeScale, cubeScale, RED);

                    iterator = iterator->next;
                    if(index == selectBlock)
                    {
                        //camera.position = cubePosition;
                        //camera.position.x += 0.5f;
                        camera.target = cubePosition;
                    }
                    index++;
                }
            EndMode3D();
            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}