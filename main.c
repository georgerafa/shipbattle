#include "raylib.h"
#include <math.h>
#include "gameCalculations.h"
#include "raymath.h"

typedef struct Obstacle {
    Vector2 position;
    Texture2D texture; // Texture for the obstacle
    int islandType;
} Obstacle;

struct Line {
    Vector2 start;
    Vector2 end;
};


#define MAX_OBSTACLES 8
#define MAX_PLAYERS 6
Obstacle obstacles[MAX_OBSTACLES];

const struct Line island1HitboxLines[19] =
{{{66,12},{1,82}},
{{1,82},{2,125}},
{{2,125},{17,146}},
{{17,146},{27,180}},
{{27,180},{88,177}},
{{88,177},{118,214}},
{{118,214},{102,237}},
{{102,237},{119,253}},
{{119,253},{162,223}},
{{162,223},{144,191}},
{{144,191},{191,168}},
{{191,168},{217,192}},
{{217,192},{242,159}},
{{242,159},{243,134}},
{{243,134},{228,111}},
{{228,111},{221,80}},
{{221,80},{205,58}},
{{205,58},{128,1}},
{{128,1},{66,12}}
};

const struct Line island2HitboxLines[15] =
{{{72,1},{48,26}},
{{48,26},{29,70}},
{{29,70},{9,85}},
{{9,85},{0,126}},
{{0,126},{13,153}},
{{13,153},{57,178}},
{{57,178},{76,174}},
{{76,174},{153,249}},
{{153,249},{228,179}},
{{228,179},{250,133}},
{{250,133},{219,62}},
{{219,62},{173,32}},
{{173,32},{132,26}},
{{132,26},{111,35}},
{{111,35},{72,1}}
};


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
        Ship ship = ships[0];
        // Draw the ship
        if (ship.isAlive) {
            checkCollision(ships[0]);
            DrawTexturePro(
                shipTexture,
                (Rectangle){0, 0, shipTexture.width, shipTexture.height},  // Source rectangle
                (Rectangle){ship.position.x, ship.position.y, 100, 100},  // Destination rectangle (2x size)
                (Vector2){50, 50},                                       // Origin (center for rotation)
                (ships[0].heading * RAD2DEG) + 270,                      // Rotation in degrees (heading + 3π/2)
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
    // Assign some obstacles to use islandTexture, others to use island2Texture
    obstacles[0] = (Obstacle){{200, 100}, islandTexture, 0};
    obstacles[1] = (Obstacle){{600, 750}, islandTexture, 0};
    obstacles[2] = (Obstacle){{700, 70},  island2Texture, 1};
    obstacles[3] = (Obstacle){{200, 500}, island2Texture,1};
    obstacles[4] = (Obstacle){{1500, 600},  islandTexture, 0};
    obstacles[5] = (Obstacle){{1150, 120},  islandTexture, 0};
    obstacles[6] = (Obstacle){{1050, 700},  island2Texture, 1};
    obstacles[7] = (Obstacle){{1500, 300},  island2Texture, 1};
}

void drawObstacles()
{
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle obstacle = obstacles[i];
        DrawTexture(
            obstacle.texture,
            obstacle.position.x,
            obstacle.position.y,
            WHITE);
    }
}

int checkCollision(Ship ship)
{
    Vector2 shipPos = ship.position;
    struct Line shipLines[4] = {
        {
            {-40*cos(ship.heading)-15*sin(ship.heading)+shipPos.x,-40*sin(ship.heading)+15*cos(ship.heading)+shipPos.y},
           {40*cos(ship.heading)-15*sin(ship.heading)+shipPos.x,40*sin(ship.heading)+15*cos(ship.heading)+shipPos.y}
        },
        {
            {-40*cos(ship.heading)+15*sin(ship.heading)+shipPos.x, -40*sin(ship.heading)-15*cos(ship.heading)+shipPos.y},
            {40*cos(ship.heading)+15*sin(ship.heading)+shipPos.x, 40*sin(ship.heading)-15*cos(ship.heading)+shipPos.y}
        },
        {
            {-40*cos(ship.heading)-15*sin(ship.heading)+shipPos.x, -40*sin(ship.heading)+15*cos(ship.heading)+shipPos.y},
            {-40*cos(ship.heading)+15*sin(ship.heading)+shipPos.x, -40*sin(ship.heading)-15*cos(ship.heading)+shipPos.y}
        },
        {
            {40*cos(ship.heading)-15*sin(ship.heading)+shipPos.x, 40*sin(ship.heading)+15*cos(ship.heading)+shipPos.y},
            {40*cos(ship.heading)+15*sin(ship.heading)+shipPos.x, 40*sin(ship.heading)-15*cos(ship.heading)+shipPos.y}
        }
    };
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle obstacle = obstacles[i];
        Vector2 offset = {obstacle.position.x, obstacle.position.y};
        if(Vector2Length(Vector2Subtract(Vector2Add(obstacle.position, (Vector2){obstacle.texture.width/2, obstacle.texture.height/2}),ship.position)) < 200) {
            if (obstacle.islandType == 0) {
                for (int j = 0; j<19; j++) {
                    Vector2 colP;
                    for (int k = 0; k<4; k++) {
                        DrawLineV(Vector2Add(island1HitboxLines[j].start, offset),
                            Vector2Add(island1HitboxLines[j].end, offset), WHITE);
                        DrawLineV(shipLines[k].start,
                            shipLines[k].end, WHITE);
                        if (CheckCollisionLines(
                            Vector2Add(island1HitboxLines[j].start, offset),
                            Vector2Add(island1HitboxLines[j].end, offset),
                            shipLines[k].start,
                            shipLines[k].end,
                             &colP)) return 0;
                    }
                }
            }
            else {
                for (int j = 0; j<15; j++) {
                    Vector2 colP;
                    for (int k = 0; k<4; k++) {
                        DrawLineV(Vector2Add(island2HitboxLines[j].start, offset),
                                                    Vector2Add(island2HitboxLines[j].end, offset), WHITE);
                        DrawLineV(shipLines[k].start,
                            shipLines[k].end, WHITE);
                        if (CheckCollisionLines(
                        Vector2Add(island2HitboxLines[j].start, offset),
                        Vector2Add(island2HitboxLines[j].end, offset),
                            shipLines[k].start,
                            shipLines[k].end,
                             &colP)) return 0;
                    }
                }
            }
        }


    }
    return 1;// No collision
}
