#include "raylib.h"
#include <math.h>
#include "gameCalculations.h"

// Define obstacles
typedef struct Obstacle {
    Vector2 position;
    int width;
    int height;
} Obstacle;

#define MAX_OBSTACLES 11
Obstacle obstacles[MAX_OBSTACLES];

void initObstacles();
void drawObstacles();
int checkCollision(Ship ship);


int main(void)
{
    InitWindow(800, 800, "raylib [core] example - basic window");
    const int display = GetCurrentMonitor();
    const int screenWidth = GetMonitorWidth(display);
    const int screenHeight = GetMonitorHeight(display);
    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();
    Camera2D camera = {0};
    camera.target = (Vector2){0, 0};
    camera.rotation = 0.0f;
    camera.zoom = 1;
    SetTargetFPS(GetMonitorRefreshRate(display));
    Ship ships[] = {{0, {1000, 500}, 100, 0, 1}};
    initObstacles();
    while (!WindowShouldClose())
    {
        updateShipPositions(ships, 1, GetFrameTime());
        ships[0].heading = atan2f(ships[0].position.y - GetMouseY() +50, ships[0].position.x - GetMouseX()+50) + M_PI;
        Ship ship = ships[0];
        ships[0].isAlive = checkCollision(ship);
        BeginDrawing();
        ClearBackground(DARKBLUE);
        DrawText(TextFormat("FPS: %d",GetFPS()), 10, 10, 20, GRAY);
        drawObstacles();
        BeginMode2D(camera);
        DrawRectangle(ship.position.x, ship.position.y, 50, 50,  RED);
        DrawLine(ship.position.x+25, ship.position.y+25, ship.speed*cos(ship.heading)+ship.position.x+25, ship.speed*sin(ship.heading)+ship.position.y+25, BLACK);
        EndMode2D();
        EndDrawing();

    }

    CloseWindow();

    return 0;
}


void initObstacles() {
    obstacles[0] = (Obstacle){{200, 200}, 160, 100};
    obstacles[1] = (Obstacle){{500, 550}, 50, 50};
    obstacles[2] = (Obstacle){{700, 120}, 50, 200};
    obstacles[3] = (Obstacle){{100, 600}, 200, 50};
    obstacles[4] = (Obstacle){{1600, 600}, 120, 70};
    obstacles[5] = (Obstacle){{700, 850}, 150, 50};
    obstacles[6] = (Obstacle){{1050, 220}, 200, 50};
    obstacles[7] = (Obstacle){{1250, 800}, 200, 70};
    obstacles[8] = (Obstacle){{300, 850}, 70,140};
    obstacles[9] = (Obstacle){{1500, 100}, 200, 50};
    obstacles[10] = (Obstacle){{1550, 300}, 40,160};
}

// Draw obstacles
void drawObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        DrawRectangle(obstacles[i].position.x, obstacles[i].position.y, obstacles[i].width, obstacles[i].height,DARKGREEN);
    }
}
int checkCollision(Ship ship) {
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