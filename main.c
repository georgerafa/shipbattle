#include "raylib.h"
#include <math.h>
typedef struct Ship {
    int team; //Ship team
    Vector2 position; //Current ship Y coordinate
    int speed; //Current ship speed
    double heading; //Direction in radians
} Ship; //Ship object

struct {
    Vector2 position;
    int radius;
} spawnCircle; //Circle on which ships will spawn

void updateShipPositions(Ship *ships, int shipCount, double deltaT);

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
    Ship ships[] = {{0, {1000, 500}, 100, 0}};
    while (!WindowShouldClose())
    {
        updateShipPositions(ships, 1, GetFrameTime());
        ships[0].heading = atan2f(ships[0].position.y - GetMouseY() +50, ships[0].position.x - GetMouseX()+50) + M_PI;
        Ship ship = ships[0];
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("FPS: %d",GetFPS()), 10, 10, 20, GRAY);
        BeginMode2D(camera);
        DrawRectangle(ship.position.x, ship.position.y, 100, 100,  RED);
        DrawLine(ship.position.x+50, ship.position.y+50, ship.speed*cos(ship.heading)+ship.position.x+50, ship.speed*sin(ship.heading)+ship.position.y+50, BLACK);
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}


//Sets the spawn circle position and radius
void setSpawnCircle(Vector2 pos, int radius) {
    spawnCircle.position = pos;
    spawnCircle.radius = radius;
}

//Sets the initial position of the provided ship on the spawning circle while ensuring the ships are evenly distributed around the spawning circle
void setInitialPosition(Ship ship, int shipCount, int index) {
    double increment = (2*M_PI)/shipCount;
    ship.position = (Vector2){
        (float)(spawnCircle.radius*cos(increment*index) + spawnCircle.position.x),
        (float)(spawnCircle.radius*sin(increment*index) + spawnCircle.position.y)
    };
}

//Updates the positions of the ships provided based on their current position, speed, and time passed (deltaT in seconds) since last update;
void updateShipPositions(Ship *ships, int shipCount, double deltaT) {
    for (int i = 0; i < shipCount; i++) {
        double speedX = cos(ships[i].heading)*ships[i].speed; //Calculate speed on the X axis
        double speedY = sin(ships[i].heading)*ships[i].speed; //Calculate speed on the Y axis
        ships[i].position.x += speedX*deltaT; //Adjust X position
        ships[i].position.y += speedY*deltaT; //Adjust Y position
    }
}

//Initializes the ships provided by setting their heading and speed values to 0 as well as setting their initial position
void initializeShips(Ship *ships, int shipCount) {
    for (int i = 0; i < shipCount; i++) {
        ships[i].heading = 0;
        ships[i].speed = 0;
        setInitialPosition(ships[i], shipCount, i);
    }
}