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
typedef enum GameScreen {TITLE, PLAYER_SELECT, COUNTDOWN, GAME, SETTINGS, HOW_TO_PLAY} GameScreen; //All screen states

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
int checkProjectileCollision(Ship ship, Projectile projectiles[]);
int playersAlive(Ship ships[]);

// Sound variables
Music backgroundMusic;
Music gameMusic;
Sound selectionSound;
Sound confirmSound;

Texture2D shipTexture;
Texture2D gameMapTexture;
Texture2D backgroundTexture;
Texture2D cannonBallTexture;

GameScreen currentScreen = TITLE; //Initial screen state
GameState currentState = DIRECTION_INSTR;

GameScreen previousScreen = TITLE;
GameScreen nextScreen = PLAYER_SELECT;

int selectedPlayers = 2;
int picking = 0;

float musicVolume = 0.4f;
float soundVolume = 0.5f;

int screenWidth;
int screenHeight;
Camera2D camera;

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
    SetExitKey(0);
    const int display = GetCurrentMonitor(); //Get which display the game is running on
    screenWidth = GetMonitorWidth(display); //Get screen width
    screenHeight = GetMonitorHeight(display); //Get screen height
    SetWindowSize(screenWidth, screenHeight); //Set the game window size to be the same as the screen size
    ToggleFullscreen(); //Set window mode to fullscreen

    // Initialize audio
    InitAudioDevice();

    //Load main menu music
    backgroundMusic = LoadMusicStream("assets/background_music.mp3");
    //Load in-game music
    gameMusic = LoadMusicStream("assets/game_music.mp3");
    //Load selection sound effect
    selectionSound = LoadSound("assets/selection.wav");
    //Load confirm selection sound effect
    confirmSound = LoadSound("assets/confirm.wav");

    //Set volume
    SetMusicVolume(gameMusic,0.2f);
    SetSoundVolume(selectionSound, 0.5f);
    SetSoundVolume(confirmSound, 0.5f);
    SetMusicVolume(backgroundMusic, 0.4f);

    //Start playing main menu music
    PlayMusicStream(backgroundMusic);

    //Load images
    Image gameMapImage = LoadImage("assets/gameMap.png");
    Image backgroundImage = LoadImage("assets/background.png");
    Image shipImage = LoadImage("assets/ship.png");
    Image cannonBall = LoadImage("assets/cannonBall.png");

    //Create textures
    gameMapTexture = LoadTextureFromImage(gameMapImage);
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    shipTexture = LoadTextureFromImage(shipImage);
    cannonBallTexture = LoadTextureFromImage(cannonBall);

    //Unload images
    UnloadImage(gameMapImage);
    UnloadImage(backgroundImage);
    UnloadImage(shipImage);
    UnloadImage(cannonBall);


    camera = {0}; //Initialize 2D top down camera
    camera.zoom = (float)screenWidth/2048.0f; //Set camera zoom to 1

    //Set target fps to monitor refresh rate
    SetTargetFPS(GetMonitorRefreshRate(display));

    Ship ships[MAX_PLAYERS]; //Create array for storing ships
    Projectile projectiles[MAX_PLAYERS]; //Create array for storing projectiles

    Vector2 posBuffer; //Temporary position storage

    //Counter variable for showing selected ship
    double selectAnimation = 0;
    int targetPlayer = 0;
    bool shouldExit = 0;
    while (!(WindowShouldClose()||shouldExit)) //While the window open
    {
        UpdateMusicStream(backgroundMusic);
        UpdateMusicStream(gameMusic);

        switch (currentScreen) //Change what is displayed based on current screen state
        {
        case TITLE: //Startup screen
            static int selectedOption = 0;
            if (IsKeyPressed(KEY_UP)) {
                PlaySound(selectionSound);
                selectedOption = (selectedOption - 1 + 4) % 4;
            }
            if (IsKeyPressed(KEY_DOWN)) {
                PlaySound(selectionSound);
                selectedOption = (selectedOption + 5) % 4;
            }
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)){ //If enter is pressed or LMB is clicked move to player selection screen

            }else if (IsKeyPressed(KEY_ESCAPE)){ // Open settings menu
                PlaySound(confirmSound);
                previousScreen = TITLE;
                currentScreen = SETTINGS;
            }

            if (selectedOption == 0 && (IsKeyPressed(KEY_ENTER))) {
                PlaySound(confirmSound); // Confirm sound
                currentScreen = PLAYER_SELECT;
            }else if (selectedOption == 1 && (IsKeyPressed(KEY_ENTER)))
            {
                PlaySound(confirmSound);
            } else if (selectedOption == 2 && (IsKeyPressed(KEY_ENTER)))
            {
                PlaySound(confirmSound);
                previousScreen = TITLE;
                currentScreen = SETTINGS;

            } else if (selectedOption == 3 && (IsKeyPressed(KEY_ENTER)))
            {
                shouldExit = 1;
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
            DrawText("Welcome to Mononaumaxia!", 100, 150, 50, WHITE);
            DrawText(TextFormat("New Game"), 100, 250, 30,
         selectedOption == 0 ? WHITE : BLACK);
            DrawText(TextFormat("Resume game"), 100, 300, 30,
                     selectedOption == 1 ? WHITE : BLACK);
            DrawText("Options", 100, 350, 30,
                     selectedOption == 2 ? WHITE : BLACK);
            DrawText("Exit", 100, 400, 30,
         selectedOption == 3 ? WHITE : BLACK);


            EndDrawing();//Stop drawing screen
            break;

        case PLAYER_SELECT: // Player selection screen
                const int totalOptions = MAX_PLAYERS + 1;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {
                PlaySound(selectionSound);
                if (IsKeyPressed(KEY_UP) && selectedPlayers > 2) {
                    selectedPlayers--;
                } else if (IsKeyPressed(KEY_DOWN) && selectedPlayers < totalOptions) {
                    selectedPlayers++;
                }
            }

            if (IsKeyPressed(KEY_ENTER)) {
                PlaySound(confirmSound);
                if (selectedPlayers <= MAX_PLAYERS) {
                    currentScreen = COUNTDOWN;
                    PlayMusicStream(gameMusic);
                    StopMusicStream(backgroundMusic);
                } else if (selectedPlayers == totalOptions) {
                    currentScreen = TITLE;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)){
                PlaySound(confirmSound);
                previousScreen = PLAYER_SELECT;
                currentScreen =SETTINGS;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexturePro(
                backgroundTexture,
                (Rectangle){0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                (Vector2){0, 0}, 0.0f,
                WHITE);

            DrawText("Select Number of Players (2 to 6)", 100, 100, 40, WHITE);
            DrawText("Press UP/DOWN arrows to choose", 100, 160, 30, WHITE);
            DrawText("Press ENTER to select", 100, 200, 30, WHITE);

            for (int i = 2; i <= MAX_PLAYERS; i++) {
                if (i == selectedPlayers) {
                    DrawText(TextFormat("> %d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 2) * 40, 40, GREEN);
                } else {
                    DrawText(TextFormat("%d Player%s", i, i > 1 ? "s" : ""), 100, 250 + (i - 2) * 40, 40, WHITE);
                }
            }
            if (selectedPlayers == totalOptions) {
                DrawText("> Return to Main Menu", 100, 300 + (MAX_PLAYERS - 1) * 40, 40, GREEN);
            } else {
                DrawText("Return to Main Menu", 100, 300 + (MAX_PLAYERS - 1) * 40, 40, WHITE);
            }

            EndDrawing();
            break;


        case COUNTDOWN: // Countdown screen
            countdownTimer -= GetFrameTime()*1.1f;
            if (countdownTimer <= -1) {
                currentScreen = GAME; // Transition to game screen
                initializeShips(ships, selectedPlayers);
            }

            BeginDrawing();
            ClearBackground(BLACK);
            if (countdownTimer <= 0) {
                DrawText("GO!", screenWidth / 2 - 50, screenHeight / 2+50 , 100, GREEN);
            }else {
                DrawText(TextFormat("%.0f", countdownTimer > 0 ? ceilf(countdownTimer) : 0), screenWidth / 2 - 50, screenHeight / 2+50 , 100, WHITE);
            }
            EndDrawing();
            break;
        case GAME:
            if (IsKeyPressed(KEY_ESCAPE)) {
                PlaySound(confirmSound);
                previousScreen = GAME;
                currentScreen = SETTINGS;
            }
            BeginDrawing();
            ClearBackground(DARKBLUE);

            BeginMode2D(camera);
            DrawTexture(gameMapTexture, 0, 0, WHITE);
            switch (currentState) {
                case DIRECTION_INSTR: {
                    selectAnimation = fmod(selectAnimation + GetFrameTime()*M_PI, M_PI*2);
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
                    while (ships[picking].isAlive == 0) picking ++;
                    ships[picking].heading = atan2f(mousePos.y-ships[picking].position.y, mousePos.x-ships[picking].position.x);
                    if (picking >= selectedPlayers) {
                        currentState = MOVEMENT_A;
                        picking = 0;
                    }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        float mouseDist = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ships[picking].position));
                        mouseDist = (float)fmin(mouseDist, maxShipSpeed*2);
                        ships[picking].speed = mouseDist/2;
                        picking ++;
                    }
                    break;
                }
                case MOVEMENT_A: {
                    updateShipPositions(ships, selectedPlayers, GetFrameTime());
                    roundTimer -=GetFrameTime();
                    for (int i = 0; i < selectedPlayers; i++) {
                        ships[i].isAlive = (1 - checkCollision(ships[i], readSections, length/sizeof(struct CollisionSection)))*ships[i].isAlive;
                        projectiles[i].position.z = ships[i].isAlive == 1 ? 10 : -10;
                    }
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
                    projectiles[picking].heading = atan2f(mousePos.y-ships[picking].position.y, mousePos.x-ships[picking].position.x);
                    DrawRectangleV(GetScreenToWorld2D((Vector2){20,screenHeight - 40}, camera), GetScreenToWorld2D((Vector2){screenWidth-40, 20}, camera), (Color){255,255,255,255});
                    if (picking >= selectedPlayers) {
                        currentState = MOVEMENT_B;
                        picking = 0;
                    }
                    while (ships[picking].isAlive == 0) picking ++;
                    projectiles[picking].angle = fmaxf(fminf(GetMouseWheelMove()*0.01f+projectiles[picking].angle, M_PI/2), 0);
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        projectiles[picking].position.x = ships[picking].position.x;
                        projectiles[picking].position.y = ships[picking].position.y;
                        projectiles[picking].position.z = 10;
                        ships[picking].position = posBuffer;
                        picking++;
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
                    for (int i = 0; i < selectedPlayers; i++) {
                        ships[i].isAlive = (1 - checkCollision(ships[i], readSections, length/sizeof(struct CollisionSection)))*ships[i].isAlive;
                        projectiles[i].position.z = ships[i].isAlive == 1 ? 10 : -10;
                    }
                    if (roundTimer <= 0) {
                        currentState = FIRE;
                        initializeProjectiles(projectiles, selectedPlayers);
                    }
                    break;
                }
                case FIRE: {
                    updateProjectiles(projectiles, ships, selectedPlayers, GetFrameTime());
                    int projectilesAreAlive = 0;
                    for (int i = 0; i<selectedPlayers; i++) {
                        ships[i].isAlive = (1-checkProjectileCollision(ships[i], projectiles))*ships[i].isAlive;
                        if (projectiles[i].position.z>0)  projectilesAreAlive = 1;
                    }
                    if (projectilesAreAlive==0) {
                        if (playersAlive(ships) == 1) {
                        }
                        else if (playersAlive(ships) == 0) {
                        }
                        else {
                            currentState = DIRECTION_INSTR;
                            roundTimer = 10.0f;
                            for (int i = 0 ; i<selectedPlayers; i++) {
                                ships[i].distanceMoved = (Vector2){0};
                            }
                        }
                    }
                }

            }
            for (int i = 0; i < selectedPlayers; i++) {
                Ship ship = ships[i];
                if (ship.isAlive) {

                    Vector2 lineStart = ship.position;
                    if (i==picking && (currentState==DIRECTION_INSTR||currentState==FIRE_INSTR)) {
                        float mouseDist = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ship.position));
                        mouseDist = currentState == FIRE_INSTR ? 200*(M_PI/2 - projectiles[i].angle)/(M_PI/2) : fminf(mouseDist, maxShipSpeed*2);
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

            }
            for (int i = 0 ; i<selectedPlayers; i++) {
                if (currentState == FIRE&&projectiles[i].position.z>0) DrawTexturePro(cannonBallTexture, (Rectangle){0,0, cannonBallTexture.width, cannonBallTexture.height}, (Rectangle){projectiles[i].position.x, projectiles[i].position.y, 10+0.1f*projectiles[i].position.z, 10+0.1f*projectiles[i].position.z,},(Vector2){(10+0.1f*projectiles[i].position.z)/2, (10+0.1f*projectiles[i].position.z)/2}, 0, WHITE);
            }

            EndMode2D();
            EndDrawing();
            break;

            case SETTINGS: {
                    static int selectedOption = 0;
                    if (IsKeyPressed(KEY_UP)) {
                        PlaySound(selectionSound);
                        selectedOption = (selectedOption - 1 + 6) % 6;
                    }
                    if (IsKeyPressed(KEY_DOWN)) {
                        PlaySound(selectionSound);
                        selectedOption = (selectedOption + 1) % 6;
                    }
                    if (selectedOption == 0 && IsKeyPressed(KEY_ENTER))
                    {
                        PlaySound(confirmSound);
                        currentScreen= previousScreen;
                    }
                    if (selectedOption == 1 && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) { // Music Volume
                        PlaySound(selectionSound);
                        musicVolume = IsKeyPressed(KEY_RIGHT)
                                        ? fminf(musicVolume + 0.1f, 1.0f)
                                        : fmaxf(musicVolume - 0.1f, 0.0f);
                        SetMusicVolume(backgroundMusic, musicVolume);
                        SetMusicVolume(gameMusic, musicVolume);
                    } else if (selectedOption == 2 && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) { // Sound Volume
                        PlaySound(selectionSound);
                        soundVolume = IsKeyPressed(KEY_RIGHT)
                                        ? fminf(soundVolume + 0.1f, 1.0f)
                                        : fmaxf(soundVolume - 0.1f, 0.0f);
                        SetSoundVolume(selectionSound, soundVolume);
                        SetSoundVolume(confirmSound, soundVolume);
                    } else if (selectedOption == 3 && IsKeyPressed(KEY_ENTER)) {
                        PlaySound(confirmSound);
                        currentScreen = HOW_TO_PLAY;
                    } else if (selectedOption == 4 && IsKeyPressed(KEY_ENTER))
                    {
                        PlaySound(selectionSound);
                        currentScreen = TITLE;
                        StopMusicStream(gameMusic); // Start game music
                        PlayMusicStream(backgroundMusic); // Stop menu music

                    } else if (selectedOption == 5 && IsKeyPressed(KEY_ENTER))
                    {
                        shouldExit = 1;
                    }

                    if (IsKeyPressed(KEY_ESCAPE)) {
                        PlaySound(confirmSound);
                        currentScreen = previousScreen;
                    }

                    BeginDrawing();
                    ClearBackground((Color){255, 255, 255, 100});

                    DrawText("SETTINGS MENU", 100, 100, 50, BLACK);
                    DrawText(TextFormat("Return"), 100, 200, 30,
                             selectedOption == 0 ? RED : BLACK);
                    DrawText(TextFormat("Music Volume: %.1f", musicVolume), 100, 250, 30,
                             selectedOption == 1 ? RED : BLACK);
                    DrawText(TextFormat("Sound Volume: %.1f", soundVolume), 100, 300, 30,
                             selectedOption == 2 ? RED : BLACK);
                    DrawText("How to Play Instructions", 100, 350, 30,
                             selectedOption == 3 ? RED : BLACK);
                    DrawText(TextFormat("Go To Main Menu"), 100, 400, 30,
                        selectedOption == 4 ? RED : BLACK);
                    DrawText(TextFormat("Exit to desktop"), 100, 450, 30,
                        selectedOption == 5 ? RED : BLACK);
                    DrawText("Use UP/DOWN to navigate, LEFT/RIGHT to adjust", 100, 550, 20, BLACK);
                    DrawText("Press ENTER to select, ESC to return", 100, 600, 20, BLACK);

                    EndDrawing();
                    break;
                }


             case HOW_TO_PLAY: {
                    BeginDrawing();
                    ClearBackground((Color){255, 255, 255, 100});
                    DrawText("HOW TO PLAY", 100, 100, 50, BLACK);
                    DrawText("Press ESC to return to the Settings Menu", 100, 450, 20, BLACK);

                    EndDrawing();

                    if (IsKeyPressed(KEY_ESCAPE)) {
                        PlaySound(confirmSound);
                        currentScreen = previousScreen;
                    }
                    break;
                }
        }
    }
    //Unload all textures
    UnloadTexture(gameMapTexture);
    UnloadTexture(backgroundTexture);
    UnloadTexture(shipTexture);
    UnloadTexture(cannonBallTexture);

    // Unload audio resources
    UnloadMusicStream(backgroundMusic);
    UnloadMusicStream(gameMusic);
    UnloadSound(selectionSound);
    UnloadSound(confirmSound);
    CloseAudioDevice();

    //Close the window
    CloseWindow();
}

struct Line getTargetLine(Ship ships[], int shipA, int shipB) {
    Ship shipOrigin = ships[shipA];
    Ship shipTarget = ships[shipB];
    Vector2 targetVector = Vector2Subtract(shipOrigin.position, shipTarget.position);
    targetVector = Vector2Scale(targetVector, 2000/Vector2Length(Vector2Scale(targetVector, Vector2Length(targetVector))));
    struct Line line;
    if (!CheckCollisionLines(targetVector, Vector2Negate(targetVector), (Vector2){0,0}, GetScreenToWorld2D({screenWidth, 0}, camera), &line.start)) {
        CheckCollisionLines(targetVector, Vector2Negate(targetVector), GetScreenToWorld2D((Vector2){screenWidth,0}, camera), GetScreenToWorld2D({screenWidth, screenWidth}, camera), &line.start);
    }
    if (!CheckCollisionLines(targetVector, Vector2Negate(targetVector), GetScreenToWorld2D((Vector2){0,screenHeight}, camera), GetScreenToWorld2D({screenWidth, screenHeight}, camera), &line.end)) {
        CheckCollisionLines(targetVector, Vector2Negate(targetVector), GetScreenToWorld2D((Vector2){0,0}, camera), GetScreenToWorld2D({0, screenHeight}, camera), &line.end);
    }
    return line;
}

int playersAlive(Ship ships[]) {
    int playersAlive = 0;
    for (int i = 0; i < selectedPlayers; i++) {
        if (ships[i].isAlive == 1) playersAlive++;
    }
    return playersAlive;
}

int checkProjectileCollision(Ship ship, Projectile projectiles[]) {
    Vector2 shipPos = ship.position;//Position of the provided ship
    if (ship.isAlive==0) return 0;
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
    for (int i = 0; i < selectedPlayers; i++) {
        Projectile projectile = projectiles[i];
        for (int j = 0; j < 4; j++) {
            if (CheckCollisionCircleLine((Vector2){projectile.position.x, projectile.position.y}, 15, shipLines[i].start, shipLines[i].end)&&projectile.position.z<15&&projectile.position.z>0&&projectile.team!=ship.team) {
                return 1;
            }
        }
    }
    return 0;
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
