#include "raylib.h"
#include <math.h>
#include "gameCalculations.h"
#include "raymath.h"
#include <stddef.h>

//Obstacle type definition
typedef struct Obstacle {
    Vector2 position;
    Texture2D texture; // Texture for the obstacle
    int islandType; //The type of island based on texture used
} Obstacle;

//Create struct for saving line segments
struct Line {
    Vector2 start;
    Vector2 end;
};

#define MAX_OBSTACLES 8
#define MAX_PLAYERS 6
Obstacle obstacles[MAX_OBSTACLES]; //Array of obstacles

const struct Line islandHitboxLines[2][20] ={ //All hitbox line segments for both island types
{
    {{66,12},{1,82}},
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
    {{128,1},{66,12}},
    {{0,0}, {0,0}},
    },{
    {{72,1},{48,26}},
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
    {{111,35},{72,1}},
    {{0,0}, {0,0}},
    {{0,0}, {0,0}},
    {{0,0}, {0,0}},
    {{0,0}, {0,0}},
    {{0,0}, {0,0}}
    }
};

//Declare function prototypes
void initObstacles();
void drawObstacles();
int checkCollision(Ship ship);

// Sound variables
Music backgroundMusic;
Sound selectionSound;
Sound confirmSound;

Texture2D shipTexture;
Texture2D oceanTexture;
Texture2D backgroundTexture;
Texture2D island2Texture;
Texture2D islandTexture;
typedef enum GameScreen { LOGO, TITLE, PLAYER_SELECT, GAME } GameScreen; //All screen states
GameScreen currentScreen = TITLE; //Initial screen state
int selectedPlayers = 2;

int main(void)
{
    InitWindow(800, 800, "POLYNAYMAXIA"); //Initialize the game window
    const int display = GetCurrentMonitor(); //Get which display the game is running on
    const int screenWidth = GetMonitorWidth(display); //Get screen width
    const int screenHeight = GetMonitorHeight(display); //Get screen height
    SetWindowSize(screenWidth, screenHeight); //Set the game window size to be the same as the screen size
    ToggleFullscreen(); //Set window mode to fullscreen

    // Initialize audio
    InitAudioDevice();
    backgroundMusic = LoadMusicStream("assets/background_music.mp3");
    selectionSound = LoadSound("assets/selection.wav");
    confirmSound = LoadSound("assets/confirm.wav");
    PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.5f);

    Image oceanImage = LoadImage("assets/ocean.png"); //Load ocean background image
    oceanTexture = LoadTextureFromImage(oceanImage); //Create ocean texture
    UnloadImage(oceanImage);  //Unload the original image after creating the texture

    Image backgroundImage = LoadImage("assets/background.png"); //Load main menu background image
    backgroundTexture = LoadTextureFromImage(backgroundImage); //Create main menu background texture
    UnloadImage(backgroundImage);  // Unload the original image after creating the texture

    Image islandImage = LoadImage("assets/island.png"); //Load island type 0 image
    ImageResize(&islandImage, 250, 250); //Resize image
    islandTexture = LoadTextureFromImage(islandImage); //Create island type 0 texture
    UnloadImage(islandImage); //Unload original image after creating the texture

    Image island2Image = LoadImage("assets/island2.png"); //Load island type 1 image
    ImageResize(&island2Image, 250, 250); //Resize image
    island2Texture = LoadTextureFromImage(island2Image); //Create island type 1 texture
    UnloadImage(island2Image); //Unload original image after creating the texture

    Image shipImage = LoadImage("assets/ship.png"); //Load ship image
    shipTexture = LoadTextureFromImage(shipImage); //Create ship texture
    UnloadImage(shipImage); //Unload original image after creating the texture

    Camera2D camera = {0}; //Initialize 2D top down camera
    camera.zoom = 1; //Set camera zoom to 1
    SetTargetFPS(GetMonitorRefreshRate(display)); //Set target fps to monitor refresh rate

    Ship ships[MAX_PLAYERS]; //Create array for storing ships
    for (int i = 0; i < MAX_PLAYERS; i++) {
        ships[i] = (Ship){0, {1000 + (i * 120), 500}, 100, 0, 1};  // Initialize ships with offset positions
    }
    initObstacles(); //Initialize obstacles

    while (!WindowShouldClose()) //While the window open
    {
        UpdateMusicStream(backgroundMusic); // Update music stream

        switch (currentScreen) //Change what is displayed based on current screen state
        {
            case TITLE: //Startup screen
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) //If enter is pressed or LMB is clicked move to player selection screen
                {
                    PlaySound(confirmSound); // Confirm sound
                    currentScreen = PLAYER_SELECT;
                }

                BeginDrawing(); //Start rendering the screen
                ClearBackground(RAYWHITE); //Clear the background

                DrawTexturePro( //Draw the background texture
                backgroundTexture,
                (Rectangle){0, 0, backgroundTexture.width, backgroundTexture.height},
                (Rectangle){0, 0, screenWidth, screenHeight},
                (Vector2){0, 0},
                0.0f,
                WHITE);
                //Write some text on the screen
                DrawText("Welcome to Polunaumaxia!", 100, 100, 50, WHITE);
                DrawText("Press ENTER or click to start", 100, 200, 30, WHITE);

                EndDrawing();//Stop drawing screen
                break;

            case PLAYER_SELECT: //Player selection screen
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) { //Navigate through options
                    PlaySound(selectionSound); // Play navigation sound
                    if (IsKeyPressed(KEY_UP) && selectedPlayers > 1) {
                        selectedPlayers--;
                    } else if (IsKeyPressed(KEY_DOWN) && selectedPlayers < MAX_PLAYERS) {
                        selectedPlayers++;
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {//If enter is pressed start game
                    PlaySound(confirmSound); // Confirm sound
                    currentScreen = GAME;
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);

                DrawTexturePro(
                backgroundTexture,
                (Rectangle){0, 0, backgroundTexture.width, backgroundTexture.height},
                (Rectangle){0, 0, screenWidth, screenHeight},
                (Vector2){0, 0},0.0f,
                WHITE
                );
                //Draw navigation instructions on screen
                DrawText("Select Number of Players (2 to 6)", 100, 100, 40, WHITE);
                DrawText("Press UP/DOWN arrows to choose", 100, 160, 30, WHITE);
                DrawText("Press ENTER to start", 100, 200, 30, WHITE);
                //Draw possible options
                for (int i = 2; i <= MAX_PLAYERS; i++) {
                    if (i == selectedPlayers) {
                        DrawText(TextFormat("> %d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 1) * 40, 40, GREEN);
                    } else {
                        DrawText(TextFormat("%d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 1) * 40, 40, WHITE);
                    }
                }
                EndDrawing();
                break;

            case GAME: //Game screen
                updateShipPositions(ships, selectedPlayers, GetFrameTime());
                for (int i = 0; i < selectedPlayers; i++) {
                    Vector2 mousePosWorld = GetScreenToWorld2D(GetMousePosition(), camera);
                    ships[i].heading = atan2f(mousePosWorld.y - ships[i].position.y, mousePosWorld.x - ships[i].position.x);
                }
                for (int i = 0; i < selectedPlayers; i++) {
                    ships[i].isAlive = 1 - checkCollision(ships[i]);
                }

                BeginDrawing();
                ClearBackground(DARKBLUE);

                BeginMode2D(camera);

                DrawTexture(oceanTexture, 0, 0, WHITE);

                drawObstacles();
                Ship ship = ships[0];
                // Draw the ship
                if (ship.isAlive) {
                    checkCollision(ships[0]);
                    DrawTexturePro(
                    shipTexture,
                    (Rectangle){0, 0, shipTexture.width, shipTexture.height},
                    (Rectangle){ship.position.x, ship.position.y, 100, 100},
                    (Vector2){50, 50},
                    (ships[0].heading * RAD2DEG) + 270,
                    WHITE);
                }
                EndMode2D();
                DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GRAY);
                EndDrawing();
                break;
        }
    }
    //Unload all textures
    UnloadTexture(oceanTexture);
    UnloadTexture(backgroundTexture);
    UnloadTexture(islandTexture);
    UnloadTexture(island2Texture);
    UnloadTexture(shipTexture);

    // Unload audio resources
    UnloadMusicStream(backgroundMusic);
    UnloadSound(selectionSound);
    UnloadSound(confirmSound);
    CloseAudioDevice();

    //Close the window
    CloseWindow();
    return 0;
}

void initObstacles()
{
    // Initialize obstacles by setting their position on the map and which texture to use
    obstacles[0] = (Obstacle){{200, 100}, islandTexture, 0};
    obstacles[1] = (Obstacle){{600, 750}, islandTexture, 0};
    obstacles[2] = (Obstacle){{700, 70},  island2Texture, 1};
    obstacles[3] = (Obstacle){{200, 500}, island2Texture,1};
    obstacles[4] = (Obstacle){{1500, 600},  islandTexture, 0};
    obstacles[5] = (Obstacle){{1150, 120},  islandTexture, 0};
    obstacles[6] = (Obstacle){{1050, 700},  island2Texture, 1};
    obstacles[7] = (Obstacle){{1500, 300},  island2Texture, 1};
}

void drawObstacles(){ //Draws all the obstacles in the obstacles array
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle obstacle = obstacles[i];
        DrawTexture(
            obstacle.texture,
            obstacle.position.x,
            obstacle.position.y,
            WHITE);
    }
}

int checkCollision(Ship ship){//Checks if the provided ship is colliding with any obstacle. Returns 1 if it detects collision and 0 if it doesn't
    Vector2 shipPos = ship.position;//Position of the provided ship
    struct Line shipLines[4] = { //The line segments that make up the hitbox of the ship
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
    for (int i = 0; i < MAX_OBSTACLES; i++) { //Check each obstacle
        struct Line hitbox = islandHitboxLines[obstacles[i].islandType][0];
        for (int j = 0; (hitbox.start.x != 0 || hitbox.start.y != 0) && (hitbox.end.x != 0 || hitbox.end.y != 0); j++) { //Loop through all the hitboxes of the obstacle
            hitbox = islandHitboxLines[obstacles[i].islandType][j];
            for (int k = 0; k < 4; k++) { //Loop through all the hitboxes of the ship
                if (CheckCollisionLines(shipLines[k].start, shipLines[k].end, (Vector2){hitbox.start.x+obstacles[i].position.x, hitbox.start.y+obstacles[i].position.y}, (Vector2){hitbox.end.x+obstacles[i].position.x, hitbox.end.y+obstacles[i].position.y}, NULL)) return 1; //Check if the two lines are intersecting
            }
        }
    }
    return 0;
}
