#include "raylib.h"
#include <math.h>
#include "gameCalculations.h"

typedef struct Obstacle {
    Vector2 position;
    int width;
    int height;
    Texture2D texture;
} Obstacle;

#define MAX_OBSTACLES 8
#define MAX_PLAYERS 6
Obstacle obstacles[MAX_OBSTACLES];

void initObstacles(Texture2D islandTexture, Texture2D island2Texture);
void drawObstacles();
int checkCollision(Ship ship);

Texture2D shipTexture;
Texture2D oceanTexture;
Texture2D backgroundTexture;
typedef enum GameScreen { LOGO, TITLE, PLAYER_SELECT, GAME } GameScreen;
GameScreen currentScreen = TITLE;
int selectedPlayers = 2;

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
    oceanTexture = LoadTextureFromImage(oceanImage);
    UnloadImage(oceanImage);  // Unload the original image after creating the texture

    // Load background image
    Image backgroundImage = LoadImage("assets/background.png");
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    UnloadImage(backgroundImage);  // Unload the original image after creating the texture

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
    SetTargetFPS(60);

    Ship ships[MAX_PLAYERS];
    for (int i = 0; i < MAX_PLAYERS; i++) {
        ships[i] = (Ship){0, {1000 + (i * 120), 500}, 100, 0, 1};  // Initialize ships with offset positions
    }
    initObstacles(islandTexture, island2Texture);

    while (!WindowShouldClose())
    {
        switch (currentScreen)
        {
            case TITLE:
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                {
                    currentScreen = PLAYER_SELECT;
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);

            DrawTexturePro(
                backgroundTexture,
                (Rectangle){0, 0, backgroundTexture.width, backgroundTexture.height},
                (Rectangle){0, 0, screenWidth, screenHeight},
                (Vector2){0, 0},
                0.0f,
                WHITE);

                DrawText("Welcome to Mononaumaxia!", 100, 100, 50, WHITE);
                DrawText("Press ENTER or click to start", 100, 200, 30, WHITE);

                EndDrawing();
                break;

            case PLAYER_SELECT:
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
                    if (IsKeyPressed(KEY_UP) && selectedPlayers > 1) {
                        selectedPlayers--;
                    } else if (IsKeyPressed(KEY_DOWN) && selectedPlayers < MAX_PLAYERS) {
                        selectedPlayers++;
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {
                    currentScreen = GAME;
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);

            DrawTexturePro(
             backgroundTexture,
             (Rectangle){0, 0, backgroundTexture.width, backgroundTexture.height},
             (Rectangle){0, 0, screenWidth, screenHeight},
             (Vector2){0, 0},
             0.0f,
             WHITE);

                DrawText("Select Number of Players (2 to 6)", 100, 100, 40, WHITE);
                DrawText("Press UP/DOWN arrows to choose", 100, 160, 30, WHITE);
                DrawText("Press ENTER to start", 100, 200, 30, WHITE);

                for (int i = 2; i <= MAX_PLAYERS; i++) {
                    if (i == selectedPlayers) {
                        DrawText(TextFormat("> %d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 1) * 40, 40, GREEN);
                    } else {
                        DrawText(TextFormat("%d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 1) * 40, 40, WHITE);
                    }
                }

                EndDrawing();
                break;

            case GAME:
                updateShipPositions(ships, selectedPlayers, GetFrameTime());

                for (int i = 0; i < selectedPlayers; i++) {
                    Vector2 mousePosWorld = GetScreenToWorld2D(GetMousePosition(), camera);
                    ships[i].heading = atan2f(mousePosWorld.y - ships[i].position.y, mousePosWorld.x - ships[i].position.x);
                }

                for (int i = 0; i < selectedPlayers; i++) {
                    ships[i].isAlive = checkCollision(ships[i]);
                }

                BeginDrawing();
                ClearBackground(DARKBLUE);

                DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GRAY);

                BeginMode2D(camera);

                DrawTexture(oceanTexture, 0, 0, WHITE);

                drawObstacles();

                for (int i = 0; i < selectedPlayers; i++) {
                    DrawTexturePro(
                        shipTexture,
                        (Rectangle){0, 0, shipTexture.width, shipTexture.height},
                        (Rectangle){ships[i].position.x, ships[i].position.y, 100, 100},
                        (Vector2){50, 50},
                        (ships[i].heading * RAD2DEG) + 270,
                        WHITE);
                }

                EndMode2D();

                EndDrawing();
                break;
        }
    }

    UnloadTexture(oceanTexture);
    UnloadTexture(backgroundTexture);
    UnloadTexture(islandTexture);
    UnloadTexture(island2Texture);
    UnloadTexture(shipTexture);

    CloseWindow();

    return 0;
}

void initObstacles(Texture2D islandTexture, Texture2D island2Texture)
{
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
    Rectangle shipRect = {ship.position.x, ship.position.y, 100, 100};

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Rectangle obstacleRect = {obstacles[i].position.x, obstacles[i].position.y, obstacles[i].width, obstacles[i].height};
        if (CheckCollisionRecs(shipRect, obstacleRect)) {
            return 0;
        }
    }

    return 1;
}
