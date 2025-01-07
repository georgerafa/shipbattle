#include "raylib.h"
#include <math.h>
#include "gameCalculations.h"

// Define obstacles
typedef struct Obstacle {
    Vector2 position;
    int width;
    int height;
    Texture2D texture; // Texture for the obstacle
} Obstacle;

#define MAX_OBSTACLES 8
Obstacle obstacles[MAX_OBSTACLES];

void initObstacles(Texture2D islandTexture, Texture2D island2Texture);
void drawObstacles();
int checkCollision(Ship ship);

Texture2D shipTexture;
Texture2D oceanTexture;  // Ocean background texture

int main(void)
{
    InitWindow(800, 800, "raylib [core] example - basic window");
    const int display = GetCurrentMonitor();
    const int screenWidth = GetMonitorWidth(display);
    const int screenHeight = GetMonitorHeight(display);
    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();

    // Load ocean image and scale it to cover the entire screen
    Image oceanImage = LoadImage("assets/ocean.png");
    Texture2D oceanTexture = LoadTextureFromImage(oceanImage);
    UnloadImage(oceanImage);  // Unload the original image after creating the texture

    // Resize island images to fit the desired size
    Image islandImage = LoadImage("assets/island.png");
    ImageResize(&islandImage, 250, 250);
    Texture2D islandTexture = LoadTextureFromImage(islandImage);
    UnloadImage(islandImage);

    Image island2Image = LoadImage("assets/island2.png");
    ImageResize(&island2Image, 250, 250);
    Texture2D island2Texture = LoadTextureFromImage(island2Image);
    UnloadImage(island2Image);

    Image shipImage = LoadImage("assets/ship.png");
    shipTexture = LoadTextureFromImage(shipImage);
    UnloadImage(shipImage);

    Camera2D camera = {0};
    camera.target = (Vector2){0, 0};
    camera.rotation = 0.0f;
    camera.zoom = 1;
    SetTargetFPS(GetMonitorRefreshRate(display));

    Ship ships[] = {{0, {1000, 500}, 100, 0, 1}};
    initObstacles(islandTexture, island2Texture);

    while (!WindowShouldClose())
    {
        // Update ship positions
        updateShipPositions(ships, 1, GetFrameTime());

        // Calculate heading so that the tip of the ship follows the mouse
        Vector2 mousePosWorld = GetScreenToWorld2D(GetMousePosition(), camera);
        ships[0].heading = atan2f(mousePosWorld.y - ships[0].position.y, mousePosWorld.x - ships[0].position.x);

        Ship ship = ships[0];
        ships[0].isAlive = checkCollision(ship);

        BeginDrawing();
        ClearBackground(DARKBLUE);

        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GRAY);

        BeginMode2D(camera);

        // Draw ocean background scaled to cover the whole screen
        DrawTexture(oceanTexture, 0, 0, WHITE);  // Draw the ocean at (0, 0)

        // Draw obstacles with their respective textures
        drawObstacles();

        // Draw the ship
        DrawTexturePro(
            shipTexture,
            (Rectangle){0, 0, shipTexture.width, shipTexture.height},  // Source rectangle
            (Rectangle){ship.position.x, ship.position.y, 100, 100},  // Destination rectangle (2x size)
            (Vector2){50, 50},                                       // Origin (center for rotation)
            (ships[0].heading * RAD2DEG) + 270,                      // Rotation in degrees (heading + 3π/2)
            WHITE);

        EndMode2D();

        EndDrawing();
    }

    // Unload textures
    UnloadTexture(oceanTexture);
    UnloadTexture(islandTexture);
    UnloadTexture(island2Texture);
    UnloadTexture(shipTexture);

    CloseWindow();

    return 0;
}

void initObstacles(Texture2D islandTexture, Texture2D island2Texture)
{
    // Assign some obstacles to use islandTexture, others to use island2Texture
    obstacles[0] = (Obstacle){{200, 100}, 160, 100, islandTexture};
    obstacles[1] = (Obstacle){{600, 750}, 50, 50, islandTexture};
    obstacles[2] = (Obstacle){{700, 70}, 50, 200, island2Texture};
    obstacles[3] = (Obstacle){{200, 500}, 200, 50, island2Texture};
    obstacles[4] = (Obstacle){{1500, 600}, 120, 70, islandTexture};
    obstacles[5] = (Obstacle){{1150, 120}, 200, 50, islandTexture};
    obstacles[6] = (Obstacle){{1050, 700}, 200, 70, island2Texture};
    obstacles[7] = (Obstacle){{1500, 300}, 200, 50, island2Texture};
}

void drawObstacles()
{
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        DrawTexture(
            obstacles[i].texture,
            obstacles[i].position.x,
            obstacles[i].position.y,
            WHITE);
    }
}

int checkCollision(Ship ship)
{
    // Ship's rectangle
    Rectangle shipRect = {ship.position.x, ship.position.y, 100, 100};

    // Check for collisions with all obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Rectangle obstacleRect = {obstacles[i].position.x, obstacles[i].position.y, obstacles[i].width, obstacles[i].height};
        if (CheckCollisionRecs(shipRect, obstacleRect)) {
            return 0; // Collision detected
        }
    }

    return 1; // No collision
}
