#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include "gameCalculations.h"
#include "raymath.h"
#include <stddef.h>
#include <stdlib.h>
#define maxShipSpeed 100
#define MAX_PLAYERS 6
float countdownTimer = 3.0f; // Countdown timer for 3-2-1-Go
float roundTimer = 10.0f;
typedef enum GameState {DIRECTION_INSTR, MOVEMENT_A, FIRE_INSTR, MOVEMENT_B, FIRE} GameState;
typedef enum GameScreen {TITLE, PLAYER_SELECT, COUNTDOWN, GAME } GameScreen; //All screen states

struct Line {
    Vector2 start;
    Vector2 end;
};
struct CollisionSection {
    Vector2 centerPosition;
    int minimumDistance;
    struct Line Lines[10];
};
//Declare function prototypes
int checkCollision(Ship ship, struct CollisionSection[], int sectionCount);

// Sound variables
Music backgroundMusic;
Music gameMusic;
Sound selectionSound;
Sound confirmSound;

Texture2D shipTexture;
Texture2D turretTexture;
Texture2D gameMapTexture;
Texture2D backgroundTexture;
GameScreen currentScreen = TITLE; //Initial screen state
GameState currentState = DIRECTION_INSTR;
int selectedPlayers = 2;
int picking = 0;
void main(void)
{
    FILE *f = fopen("collisions.dat", "rb");

    if (f == NULL) {
        perror("collisions.dat file is missing");
        return;
    }
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    if (length <= 0) {
        perror("collisions.dat file is corrupted");
        return;
    }
    struct CollisionSection readSections[length/sizeof(struct CollisionSection)];
    rewind(f);
    fread(&readSections, sizeof(readSections), 1, f);
    fclose(f);

    InitWindow(800, 800, "POLYNAYMAXIA"); //Initialize the game window
    const int display = GetCurrentMonitor(); //Get which display the game is running on
    const int screenWidth = GetMonitorWidth(display); //Get screen width
    const int screenHeight = GetMonitorHeight(display); //Get screen height
    SetWindowSize(screenWidth, screenHeight); //Set the game window size to be the same as the screen size
    ToggleFullscreen(); //Set window mode to fullscreen

    // Initialize audio
    InitAudioDevice();
    backgroundMusic = LoadMusicStream("assets/background_music.mp3");
    gameMusic = LoadMusicStream("assets/game_music.mp3");
    selectionSound = LoadSound("assets/selection.wav");
    confirmSound = LoadSound("assets/confirm.wav");
    SetMusicVolume(gameMusic,0.2f);
    SetSoundVolume(selectionSound, 0.5f);
    SetSoundVolume(confirmSound, 0.5f);
    PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.6f);

    Image gameMapImage = LoadImage("assets/gameMap.png"); //Load ocean background image
    ImageResize(&gameMapImage, 2048, 2048);
    gameMapTexture = LoadTextureFromImage(gameMapImage); //Create ocean texture
    UnloadImage(gameMapImage);  //Unload the original image after creating the texture

    Image backgroundImage = LoadImage("assets/background.png"); //Load main menu background image
    backgroundTexture = LoadTextureFromImage(backgroundImage); //Create main menu background texture
    UnloadImage(backgroundImage);  // Unload the original image after creating the texture

    Image shipImage = LoadImage("assets/ship.png"); //Load ship image
    shipTexture = LoadTextureFromImage(shipImage); //Create ship texture
    UnloadImage(shipImage); //Unload original image after creating the texture

    Camera2D camera = {0}; //Initialize 2D top down camera
    camera.zoom = (float)screenWidth/2048.0f; //Set camera zoom to 1
    SetTargetFPS(GetMonitorRefreshRate(display)); //Set target fps to monitor refresh rate

    Ship ships[MAX_PLAYERS]; //Create array for storing ships
    Projectile projectiles[MAX_PLAYERS];
    Vector2 posBuffer;

    double selectAnimation = 0;
    while (!WindowShouldClose()) //While the window open
    {
        UpdateMusicStream(backgroundMusic);
        UpdateMusicStream(gameMusic);

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
                (Rectangle){0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
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

            if (IsKeyPressed(KEY_ENTER)) { //If enter is pressed, transition to countdown
                PlaySound(confirmSound); // Confirm sound
                currentScreen = COUNTDOWN;
                PlayMusicStream(gameMusic); // Start game music
                StopMusicStream(backgroundMusic); // Stop menu music
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexturePro(
                backgroundTexture,
                (Rectangle){0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                (Vector2){0, 0}, 0.0f,
                WHITE);
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

        case COUNTDOWN: // Countdown screen
            countdownTimer -= GetFrameTime()*1.1f;
            if (countdownTimer <= 0) {
                currentScreen = GAME; // Transition to game screen
                initializeShips(ships, selectedPlayers);
            }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText(TextFormat("%.0f", countdownTimer > 0 ? ceilf(countdownTimer) : 0), screenWidth / 2 - 50, screenHeight / 2 - 50, 100, WHITE);
            EndDrawing();
            break;
        case GAME: //Game screen
            BeginDrawing();
            ClearBackground(DARKBLUE);
            BeginMode2D(camera);
            DrawTexture(gameMapTexture, 0, 0, WHITE);
            switch (currentState) {
                case DIRECTION_INSTR: {
                    selectAnimation = fmod(selectAnimation + GetFrameTime()*M_PI, M_PI*2);
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
                    ships[picking].heading = atan2(mousePos.y-ships[picking].position.y, mousePos.x-ships[picking].position.x);
                    if (picking >= selectedPlayers) {
                        currentState = MOVEMENT_A;
                        picking = 0;
                    }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        float mouseDist = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ships[picking].position));
                        mouseDist = fmin(mouseDist, maxShipSpeed*2);
                        ships[picking].speed = mouseDist/2;
                        picking ++;
                    }
                    break;
                }
                case MOVEMENT_A: {
                    updateShipPositions(ships, selectedPlayers, GetFrameTime());
                    roundTimer -=GetFrameTime();
                    if (roundTimer <= 5) {
                        currentState = FIRE_INSTR;
                        posBuffer = ships[0].position;
                        ships[0].position = Vector2Add(ships[0].position, ships[0].distanceMoved);
                    }
                    break;
                }
                case FIRE_INSTR: {
                    selectAnimation = fmod(selectAnimation + GetFrameTime()*M_PI, M_PI*2);
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
                    projectiles[picking].heading = atan2(mousePos.y-ships[picking].position.y, mousePos.x-ships[picking].position.x);
                    if (picking >= selectedPlayers) {
                        currentState = MOVEMENT_B;
                        picking = 0;
                    }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        float mouseDist = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ships[picking].position)) ;
                        mouseDist = fmax(fmin(mouseDist, maxShipSpeed*2+10), 10);
                        projectiles[picking].angle = M_PI/2*(1-(mouseDist-10)/200);
                        projectiles[picking].position.x = ships[picking].position.x;
                        projectiles[picking].position.y = ships[picking].position.y;
                        projectiles[picking].position.z = 10;
                        ships[picking].position = posBuffer;
                        picking ++;
                        if (picking<selectedPlayers) {
                            posBuffer = ships[picking].position;
                            ships[picking].position = Vector2Add(ships[picking].position, ships[picking].distanceMoved);
                        }
                    }
                    break;
                }
                case MOVEMENT_B: {
                    updateShipPositions(ships, selectedPlayers, GetFrameTime());
                    roundTimer -=GetFrameTime();
                    if (roundTimer <= 0) {
                        currentState = FIRE;
                        initializeProjectiles(projectiles, selectedPlayers);
                    }
                    break;
                }
                case FIRE: {
                    updateProjectiles(projectiles, selectedPlayers, GetFrameTime());
                }

            }
            for (int i = 0; i < selectedPlayers; i++) {
                Ship ship = ships[i];
                if (ship.isAlive) {
                    Vector2 lineStart = ship.position;
                    if (i==picking && (currentState==DIRECTION_INSTR||currentState==FIRE_INSTR)) {
                        float mouseDist = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ship.position));
                        mouseDist = (float)fmin(mouseDist, maxShipSpeed*2);
                        DrawRectanglePro((Rectangle){ship.position.x, ship.position.y, 10, mouseDist}, (Vector2){5,0}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading) * RAD2DEG + 270, WHITE);
                        DrawTriangle(Vector2Add(lineStart, Vector2Rotate((Vector2){mouseDist, -10}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), Vector2Add(lineStart, Vector2Rotate((Vector2){mouseDist, 10}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), Vector2Add(lineStart, Vector2Rotate((Vector2){mouseDist+40, 0}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), WHITE);
                    }
                    DrawTexturePro(
                        shipTexture,
                        (Rectangle){0, 0, (float)shipTexture.width, (float)shipTexture.height},
                        (Rectangle){ship.position.x, ship.position.y, 100, 100},
                        (Vector2){50, 50},
                        ship.heading * RAD2DEG + 270,
                        (Color){255, 255, 255, (currentState==DIRECTION_INSTR||currentState==FIRE_INSTR)&&i==picking?205-50*cos(selectAnimation) : 255});


                }
                if (currentState == FIRE) DrawCircle(projectiles[i].position.x, projectiles[i].position.y, 5+0.05*projectiles[i].position.z, WHITE);
            }



            EndMode2D();
            EndDrawing();
            break;

        }
    }
    //Unload all textures
    UnloadTexture(gameMapTexture);
    UnloadTexture(backgroundTexture);
    UnloadTexture(shipTexture);

    // Unload audio resources
    UnloadMusicStream(backgroundMusic);
    UnloadMusicStream(gameMusic);
    UnloadSound(selectionSound);
    UnloadSound(confirmSound);
    CloseAudioDevice();

    //Close the window
    CloseWindow();
}

int checkCollision(Ship ship, struct CollisionSection sections[], int sectionCount){//Checks if the provided ship is colliding with any obstacle. Returns 1 if it detects collision and 0 if it doesn't
    Vector2 shipPos = ship.position;//Position of the provided ship
    struct Line shipLines[4] = { //The line segments that make up the hitbox of the ship
        {
            {(-40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x),(-40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y)},
           {(40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x),(40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y)}
        },
        {
            {(-40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x), (-40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y)},
            {(40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x), (40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y)}
        },
        {
            {(-40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x), (-40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y)},
            {(-40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x), (-40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y)}
        },
        {
            {40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x, 40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y},
            {(40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x), (40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y)}
        }
    };
    for (int i = 0; i < sectionCount; i++) {//Iterate through all obstacles
        struct CollisionSection section = sections[i]; //Current obstacle
        Vector2 centerPos = section.centerPosition; //Obstacle offset from (0,0)
        //If the distance, between the ship and the obstacle, is less than 200 pixels check for collision
        if(Vector2Length(Vector2Subtract(centerPos,ship.position)) < (float)section.minimumDistance) {
            //Iterate through section hitbox lines
            for (int j = 0; j<10; j++) {
                //Temporary collision point variable
                Vector2 colP;
                //Iterate through ship hitbox lines
                for (int k = 0; k<4; k++) {
                    //If there is a collision between the terrain and ship lines then return 1
                    if (CheckCollisionLines(
                    section.Lines[j].start, //Island line start point
                    section.Lines[j].end, //Island line end point
                    shipLines[k].start, //Ship line start point
                    shipLines[k].end, //Ship line end point
                        &colP)) //Collision point variable
                        return 1;
                }
            }
        }
    }
    return 0; //Return 0 if no collision is detected
}
