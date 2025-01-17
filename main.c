#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include "gameCalculations.h"
#include "raymath.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

float countdownTimer = 3.0f; // Countdown timer for 3-2-1-Go
float roundTimer = 10.0f;
bool isMidGame = false;

typedef enum GameState {DIRECTION_INSTR, MOVEMENT_A, FIRE_INSTR, MOVEMENT_B, FIRE} GameState;
typedef enum GameScreen {TITLE, PLAYER_SELECT, COUNTDOWN, GAME, SETTINGS, HOW_TO_PLAY, END} GameScreen; //All screen states

typedef struct Line {
    Vector2 start;
    Vector2 end;
} Line;

struct CollisionSection {
    Vector2 centerPosition;
    int minimumDistance;
    Line Lines[10];
};

//Declare function prototypes
void checkShipCollisions(Ship *ships);
int checkProjectileCollision(Ship ship, Projectile projectiles[]);
int checkTerrainCollision(Ship ship, struct CollisionSection[], int sectionCount);
int playersAlive(Ship ships[]);
Line getTargetLine(Ship ships[], int shipA, int shipB);
void saveGame(Ship *ships, Projectile *projectiles, int selectedPlayers, int targetPlayer, int picking, float roundTimer, GameState gameState);
bool loadGame(Ship *ships, Projectile *projectiles, int *selectedPlayers, int *targetPlayer, int *picking, float *roundTimer, GameState *gameState);

// Sound variables
Music backgroundMusic;
Music gameMusic;
Sound selectionSound;
Sound confirmSound;

//Texture variables
Texture2D shipTexture;
Texture2D gameMapTexture;
Texture2D backgroundTexture;
Texture2D cannonBallTexture;
Texture2D endTexture;

GameScreen currentScreen = TITLE; //Initial screen state
GameState currentState = DIRECTION_INSTR;

GameScreen previousScreen = TITLE;
GameScreen nextScreen = PLAYER_SELECT;

int selectedPlayers = 2; //The number of players to use
int picking = 0; //The player currently picking movement or firing instructions

float musicVolume = 0.4f;
float soundVolume = 0.5f;

int screenWidth;
int screenHeight;
Camera2D camera = {0}; //Initialize 2D top down camera

void main(void){
    //Open collisions.dat file in read binary mode
    FILE *f = fopen("collisions.dat", "rb");
    //Check if file was opened
    if (f == NULL) {
        perror("collisions.dat file is missing");
        return;
    }
    //Measure file length
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    //Check if file has contentes
    if (length <= 0) {
        perror("collisions.dat file is corrupted");
        return;
    }
    int segmentCount = length/sizeof(struct CollisionSection);
    //Create variable to store the read collision sections from the file
    struct CollisionSection readSections[segmentCount];
    rewind(f);
    //Get the data from the file and close it
    fread(&readSections, sizeof(readSections), 1, f);
    fclose(f);

    InitWindow(800, 800, "POLYNAYMAXIA"); //Initialize the game window
    SetExitKey(0); //Remove exit key

    const int display = GetCurrentMonitor(); //Get which display the game is running on
    screenWidth = GetMonitorWidth(display); //Get screen width
    screenHeight = GetMonitorHeight(display); //Get screen height

    //Set target fps to monitor refresh rate
    SetTargetFPS(GetMonitorRefreshRate(display));
    SetWindowSize(screenWidth, screenHeight); //Set the game window size to be the same as the screen size
    ToggleFullscreen(); //Set window mode to fullscreen

    //Set camera zoom based on screen size
    camera.zoom = (float)screenWidth/2048.0f;
    const Vector2 mapBounds = GetScreenToWorld2D((Vector2){screenWidth, screenHeight}, camera);


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
    Image endImage = LoadImage("assets/end.png");

    //Create textures
    gameMapTexture = LoadTextureFromImage(gameMapImage);
    backgroundTexture = LoadTextureFromImage(backgroundImage);
    shipTexture = LoadTextureFromImage(shipImage);
    cannonBallTexture = LoadTextureFromImage(cannonBall);
    endTexture = LoadTextureFromImage(endImage);

    //Unload images
    UnloadImage(gameMapImage);
    UnloadImage(backgroundImage);
    UnloadImage(shipImage);
    UnloadImage(cannonBall);
    UnloadImage(endImage);

    Ship ships[MAX_PLAYERS]; //Create array for storing ships
    Projectile projectiles[MAX_PLAYERS]; //Create array for storing projectiles

    //Counter variable for showing selected ship
    double selectAnimation = 0;
    int targetPlayer = 1;
    bool shouldExit = 0;
    while (!(WindowShouldClose()||shouldExit)){ //While the window open
        UpdateMusicStream(backgroundMusic);
        UpdateMusicStream(gameMusic);
        switch (currentScreen){//Change what is displayed based on current screen state
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
            if (IsKeyPressed(KEY_ESCAPE)){ // Open settings menu
                PlaySound(confirmSound);
                previousScreen = TITLE;
                currentScreen = SETTINGS;
            }
            if (IsKeyPressed(KEY_ENTER)) {
                switch (selectedOption) {
                    case 0://New game
                        PlaySound(confirmSound);
                        currentScreen = PLAYER_SELECT;
                        break;
                    case 1://Load game
                        PlaySound(confirmSound);
                        loadGame(ships, projectiles, &selectedPlayers, &targetPlayer, &picking, &roundTimer, &currentState);
                        isMidGame = true;
                        currentScreen = GAME;
                        break;
                    case 2://Settings
                        PlaySound(confirmSound);
                        previousScreen = TITLE;
                        currentScreen = SETTINGS;
                        selectedOption = 0;
                        break;
                    case 3://Quit
                        shouldExit = true;
                        break;
                }
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
            DrawText(TextFormat("New Game"), 100, 250, 30, selectedOption == 0 ? WHITE : BLACK);
            DrawText(TextFormat("Resume game"), 100, 300, 30, selectedOption == 1 ? WHITE : BLACK);
            DrawText("Options", 100, 350, 30, selectedOption == 2 ? WHITE : BLACK);
            DrawText("Exit", 100, 400, 30, selectedOption == 3 ? WHITE : BLACK);

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
                    //Reset all game variables
                    selectAnimation = 0;
                    countdownTimer = 3;
                    roundTimer = 10.0f;
                    targetPlayer = 1;
                    currentState = DIRECTION_INSTR;
                } else if (selectedPlayers == totalOptions) {
                    currentScreen = TITLE;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)){
                PlaySound(confirmSound);
                previousScreen = PLAYER_SELECT;
                currentScreen = SETTINGS;
            }

            BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexturePro( //Draw background
                backgroundTexture,
                (Rectangle){0, 0, (float)backgroundTexture.width, (float)backgroundTexture.height},
                (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                (Vector2){0, 0}, 0.0f,
                WHITE
            );

            //Draw navigation instructions on screen
            DrawText("Select Number of Players (2 to 6)", 100, 100, 40, WHITE);
            DrawText("Press UP/DOWN arrows to choose", 100, 160, 30, WHITE);
            DrawText("Press ENTER to select", 100, 200, 30, WHITE);

            for (int i = 2; i <= MAX_PLAYERS; i++) {
                const char *text = TextFormat("%s%d Players", i == selectedPlayers ? "> " : "", i);
                DrawText(text,300-MeasureText(text, 40), 250 + (i - 1) * 40, 40, i==selectedPlayers ? GREEN : WHITE);
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
                isMidGame = true;
                currentScreen = GAME; // Transition to game screen
                initializeShips(ships, selectedPlayers);
            }
            BeginDrawing();
            ClearBackground(BLACK);
            if (countdownTimer <= 0) {
                DrawText("GO!", screenWidth / 2 - 50, screenHeight /2 -25 , 100, GREEN);
            }else {
                DrawText(TextFormat("%.0f", countdownTimer > 0 ? ceilf(countdownTimer) : 0), screenWidth / 2 - 50, screenHeight / 2 -25 , 100, WHITE);
            }
            EndDrawing();
            break;
        case GAME:
            // Handle pause and switch to settings menu
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
                        ships[picking].speed = fminf(Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ships[picking].position)), maxShipSpeed*2)/2;
                        picking ++;
                    }
                    break;
                }
                case MOVEMENT_A: {
                    updateShipPositions(ships, selectedPlayers, GetFrameTime());
                    roundTimer -= GetFrameTime();
                    checkShipCollisions(ships);
                    for (int i = 0; i < selectedPlayers; i++) {
                        if (ships[i].position.x > mapBounds.x || ships[i].position.y > mapBounds.y) ships[i].isAlive = 0;
                        ships[i].isAlive = (1 - checkTerrainCollision(ships[i], readSections, segmentCount))*ships[i].isAlive;
                    }
                    if (playersAlive(ships) == 0) {
                        currentScreen = END;
                        StopMusicStream(gameMusic);
                        PlayMusicStream(backgroundMusic);
                    }
                    if (roundTimer <= 5 && playersAlive(ships) > 1) {
                        currentState = FIRE_INSTR;
                        resetProjectiles(projectiles, selectedPlayers);
                        GetMouseWheelMove();
                    }
                    else if (roundTimer <= 0){
                        currentScreen = END;
                        StopMusicStream(gameMusic);
                        PlayMusicStream(backgroundMusic);
                    }
                    break;
                }
                case FIRE_INSTR: {
                    selectAnimation = fmod(selectAnimation + GetFrameTime()*M_PI, M_PI*2);
                    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
                    while (ships[picking].isAlive == 0) picking++;
                    if (picking >= selectedPlayers) {
                        currentState = MOVEMENT_B;
                        picking = 0;
                        break;
                    }

                    projectiles[picking].heading = atan2f(mousePos.y-ships[picking].position.y, mousePos.x-ships[picking].position.x);
                    projectiles[picking].angle = fmaxf(fminf(GetMouseWheelMove()*0.01f+projectiles[picking].angle, M_PI/2), 0);

                    Line targetLine;

                    if (playersAlive(ships) > 1) {
                        if (IsKeyPressed(KEY_DOWN)) {
                            if (--targetPlayer<0) targetPlayer = selectedPlayers-1;
                            while (ships[targetPlayer].isAlive == 0 || picking == targetPlayer) --targetPlayer < 0 ? targetPlayer = selectedPlayers-1 : targetPlayer;
                        }
                        if (IsKeyPressed(KEY_UP)) {
                            if (++targetPlayer>=selectedPlayers) targetPlayer = 0;
                            while (ships[targetPlayer].isAlive == 0 || picking == targetPlayer) ++targetPlayer >= selectedPlayers ? targetPlayer = 0 : targetPlayer;
                        }
                        targetLine = getTargetLine(ships, picking,  targetPlayer);
                    }

                    DrawLineV(targetLine.start, targetLine.end, RED);

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        projectiles[picking].position.x = ships[picking].position.x + ships[picking].distanceMoved.x;
                        projectiles[picking].position.y = ships[picking].position.y + ships[picking].distanceMoved.y;
                        targetPlayer = (++picking + 1) % selectedPlayers;
                    }
                    break;
                }
                case MOVEMENT_B: {
                    updateShipPositions(ships, selectedPlayers, GetFrameTime());
                    roundTimer -=GetFrameTime();
                    checkShipCollisions(ships);
                    for (int i = 0; i < selectedPlayers; i++) {
                        if (ships[i].position.x > mapBounds.x || ships[i].position.y > mapBounds.y) ships[i].isAlive = 0;
                        ships[i].isAlive = (1 - checkTerrainCollision(ships[i], readSections, segmentCount))*ships[i].isAlive;
                    }
                    if (playersAlive(ships) == 0) {
                        currentScreen = END;
                        StopMusicStream(gameMusic); // Start game music
                        PlayMusicStream(backgroundMusic); // Stop menu music
                    }
                    if (roundTimer <= 0) {
                        currentState = FIRE;
                        initializeProjectiles(projectiles, ships, selectedPlayers);
                    }
                    break;
                }
                case FIRE: {
                    updateProjectiles(projectiles, ships, selectedPlayers, GetFrameTime());
                    int projectilesAlive = 0;
                    for (int i = 0; i<selectedPlayers; i++) {
                        int aliveState = (1-checkProjectileCollision(ships[i], projectiles))*ships[i].isAlive;
                        ships[i].isAlive = aliveState;
                        if (projectiles[i].position.z>0)  projectilesAlive++;
                    }
                    if (projectilesAlive==0) {
                        resetProjectiles(projectiles, selectedPlayers);
                        if (playersAlive(ships) <= 1) {
                            isMidGame = false;
                            currentScreen = END;
                            StopMusicStream(gameMusic); // Start game music
                            PlayMusicStream(backgroundMusic); // Stop menu music
                        }
                        else {
                            currentState = DIRECTION_INSTR;
                            roundTimer = 10.0f;
                            picking = 0;
                            targetPlayer = 1;
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
                        float arrowLength = Vector2Length(Vector2Subtract(GetScreenToWorld2D(GetMousePosition(), camera), ship.position));
                        arrowLength = currentState == FIRE_INSTR ? 200*(M_PI/2 - projectiles[i].angle)/(M_PI/2) : fminf(arrowLength, maxShipSpeed*2); //HERE
                        DrawRectanglePro((Rectangle){ship.position.x, ship.position.y, 10, arrowLength}, (Vector2){5,0}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading) * RAD2DEG + 270, WHITE);
                        DrawTriangle(Vector2Add(lineStart, Vector2Rotate((Vector2){arrowLength, -10}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), Vector2Add(lineStart, Vector2Rotate((Vector2){arrowLength, 10}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), Vector2Add(lineStart, Vector2Rotate((Vector2){arrowLength+40, 0}, (currentState==DIRECTION_INSTR?ship.heading:projectiles[i].heading))), WHITE);
                    }
                    DrawTexturePro(
                        shipTexture,
                        (Rectangle){0, 0, (float)shipTexture.width, (float)shipTexture.height},
                        (Rectangle){ship.position.x, ship.position.y, 100, 100},
                        (Vector2){50, 50},
                        ship.heading * RAD2DEG + 270,
                        (Color){255, i==targetPlayer&&currentState==FIRE_INSTR? 128 : 255, i==targetPlayer&&currentState==FIRE_INSTR? 128 : 255, (currentState==DIRECTION_INSTR||currentState==FIRE_INSTR)&&i==picking?205-50*cos(selectAnimation) : 255});
                }
            }
            for (int i = 0 ; i<selectedPlayers; i++) {
                if (currentState == FIRE&&projectiles[i].position.z>0) DrawTexturePro(cannonBallTexture, (Rectangle){0,0, cannonBallTexture.width, cannonBallTexture.height}, (Rectangle){projectiles[i].position.x, projectiles[i].position.y, 10+0.1f*projectiles[i].position.z, 10+0.1f*projectiles[i].position.z,},(Vector2){(10+0.1f*projectiles[i].position.z)/2, (10+0.1f*projectiles[i].position.z)/2}, 0, WHITE);
                if (currentState == FIRE_INSTR) {
                    float initialZspeed = PROJECTILE_SPEED*sinf(projectiles[picking].angle); //HERE
                    float maxDistance = PROJECTILE_SPEED*cosf(projectiles[picking].angle)*((initialZspeed+sqrtf(20*GRAVITY+initialZspeed*initialZspeed))/GRAVITY); //HERE
                    for (float p  = 0; p <= 30; p++) {
                        float xPos = p*maxDistance/30;
                        Ship ship = ships[picking];
                        float initialX = xPos;
                        float initialY = getLinePoint(projectiles[picking], xPos);
                        float projectileHeading = projectiles[picking].heading;
                        float rotatedX = initialX*cosf(projectileHeading) - initialY*sinf(projectileHeading);
                        float rotatedY = initialX*sinf(projectileHeading) + initialY*cosf(projectileHeading);
                        DrawCircle(rotatedX+ship.position.x+ship.distanceMoved.x, rotatedY+ship.position.y+ship.distanceMoved.y, 3, initialY<=15 ? RED : BLACK);
                    }
                }
            }

            EndMode2D();
            EndDrawing();
            break;

        case END: {
                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawTexturePro( //Draw the background texture
                    endTexture,
                    (Rectangle){0, 0, (float)endTexture.width, (float)endTexture.height},
                    (Rectangle){0, 0, (float)screenWidth, (float)screenHeight},
                    (Vector2){0, 0},
                    0.0f,
                    WHITE);


                if (playersAlive(ships) == 1)
                {
                    DrawText("Victory!", screenWidth / 2 - MeasureText("Victory!", 100) / 2, 150, 100, BLACK);

                } else if (playersAlive(ships) == 0)
                {
                    DrawText("Draw", screenWidth / 2 - MeasureText("Draw", 100) / 2, 150, 100, BLACK);
                }

                DrawText("Press Enter to return to the main menu.", screenWidth / 2 - MeasureText("Press Enter to return to the main menu.", 30) / 2, 50, 30, WHITE);

                if (IsKeyPressed(KEY_ENTER)) {
                    currentScreen =TITLE;
                    currentState = DIRECTION_INSTR;

                    for (int i = 0; i < selectedPlayers; i++) {
                        ships[i].isAlive = 1;
                        ships[i].position = (Vector2){0, 0};
                        ships[i].distanceMoved = (Vector2){0};
                        projectiles[i].position = (Vector3){-10, -10, -10};
                    }
                }
                if (IsKeyPressed(KEY_ESCAPE))
                {
                    shouldExit = 1;
                }

                EndDrawing();
                break;
        }

            case SETTINGS: {
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
                        if (isMidGame) saveGame(ships, projectiles, selectedPlayers, targetPlayer, picking, roundTimer, currentState);
                        isMidGame = false;
                        PlaySound(selectionSound);
                        currentScreen = TITLE;
                        StopMusicStream(gameMusic); // Start game music
                        PlayMusicStream(backgroundMusic); // Stop menu music
                        selectedOption=0;

                    } else if (selectedOption == 5 && IsKeyPressed(KEY_ENTER))
                    {
                        if (isMidGame) saveGame(ships, projectiles, selectedPlayers, targetPlayer, picking, roundTimer, currentState);
                        shouldExit = 1;
                    }

                    if (IsKeyPressed(KEY_ESCAPE)) {
                        PlaySound(confirmSound);
                        currentScreen = previousScreen;
                    }

                    BeginDrawing();

                    ClearBackground((Color){255, 255, 255, 100});

                    DrawText("SETTINGS MENU", 100, 100, 50, BLACK);
                    DrawText(TextFormat("Return"), 100, 200, 30, selectedOption == 0 ? RED : BLACK);
                    DrawText(TextFormat("Music Volume: %.1f", musicVolume), 100, 250, 30, selectedOption == 1 ? RED : BLACK);
                    DrawText(TextFormat("Sound Volume: %.1f", soundVolume), 100, 300, 30, selectedOption == 2 ? RED : BLACK);
                    DrawText("How to Play Instructions", 100, 350, 30, selectedOption == 3 ? RED : BLACK);
                    DrawText(TextFormat("Go To Main Menu"), 100, 400, 30, selectedOption == 4 ? RED : BLACK);
                    DrawText(TextFormat("Exit to desktop"), 100, 450, 30, selectedOption == 5 ? RED : BLACK);
                    DrawText("Use UP/DOWN to navigate, LEFT/RIGHT to adjust", 100, 550, 20, BLACK);
                    DrawText("Press ENTER to select, ESC to return", 100, 600, 20, BLACK);

                    EndDrawing();
                    break;
                }


             case HOW_TO_PLAY: {
                    BeginDrawing();
                    ClearBackground((Color){255, 255, 255, 100});
                    DrawText("HOW TO PLAY", 100, 100, 50, BLACK);
                    DrawText("Control the ship movement by setting up the direction with your mouse and left click to perform the movement.",100, 200, 30, BLACK);
                    DrawText ("Avoid obstacles and other ships, as any collision or hit can knock you out.",100, 250, 30, BLACK );
                    DrawText ("The ships movement is paused mid-game and you are given the ability to fire a shot ",100, 350, 30, BLACK) ;
                    DrawText ("as well as a helpful red line for each pair of ships indicating their final positions between them.",100, 400, 30, BLACK );
                    DrawText ("You can wander around each opponent's final position relative to yours by pressing the up and down arrows.",100, 500, 30, BLACK );
                    DrawText("To attack, aim based on the final positions of the ships and fire using Left click.", 100, 550, 30, BLACK);
                    DrawText ("Adjust the firing angle with the scroll wheel. After the pause, the ships movement continues, ",100, 650, 30, BLACK );
                    DrawText ("while at the end of the movement the shots are fired. ",100, 700, 30, BLACK );
                    DrawText ("The process repeats until a player is crowned the winner of the game or until all players are eliminated.", 100, 800, 30, BLACK  );
                    DrawText("Press ESC to return to the Settings Menu", 100, 900, 20, BLACK);

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

    if (isMidGame) saveGame(ships, projectiles, selectedPlayers, targetPlayer, picking, roundTimer, currentState);

    //Close the window
    CloseWindow();
}

Line getTargetLine(Ship ships[], int shipA, int shipB) {
    Ship shipOrigin = ships[shipA];
    Ship shipTarget = ships[shipB];
    shipOrigin.position = Vector2Add(shipOrigin.position, shipOrigin.distanceMoved);
    shipTarget.position = Vector2Add(shipTarget.position, shipTarget.distanceMoved);
    Vector2 targetVector = Vector2Subtract(shipOrigin.position,shipTarget.position);
    targetVector = Vector2Scale(targetVector, 4000/Vector2Length(targetVector));
    Line checkLine = {
        Vector2Add(targetVector, shipOrigin.position),
        Vector2Add(Vector2Negate(targetVector), shipOrigin.position),
    };
    DrawLineV(checkLine.start, checkLine.end, BLUE);
    Line line  = {(Vector2){-1, -1}, (Vector2){-1, -1}};
    CheckCollisionLines(checkLine.start, checkLine.end, (Vector2){0,0}, GetScreenToWorld2D((Vector2){screenWidth, 0}, camera), &line.start);
    CheckCollisionLines(checkLine.start, checkLine.end, GetScreenToWorld2D((Vector2){screenWidth,0}, camera), GetScreenToWorld2D((Vector2){screenWidth, screenWidth}, camera), line.start.x==-1 ? & line.start : &line.end);
    CheckCollisionLines(checkLine.start, checkLine.end, GetScreenToWorld2D((Vector2){0,screenHeight}, camera), GetScreenToWorld2D((Vector2){screenWidth, screenHeight}, camera), line.start.x==-1 ? & line.start : &line.end);
    CheckCollisionLines(checkLine.start, checkLine.end, GetScreenToWorld2D((Vector2){0,0}, camera), GetScreenToWorld2D((Vector2){0, screenHeight}, camera), line.start.x==-1 ? & line.start : &line.end);
    return line;
}

int playersAlive(Ship ships[]) {
    int playersAlive = 0;
    for (int i = 0; i < selectedPlayers; i++) {
        if (ships[i].isAlive == 1) playersAlive++;
    }
    return playersAlive;
}

void checkShipCollisions(Ship *ships) {
    for (int i = 0; i < selectedPlayers; i++) {
        for (int j = 0; j < selectedPlayers; j++) {
            if (i!=j && ships[i].isAlive == 1 && ships[j].isAlive == 1 && Vector2Length(Vector2Subtract(ships[i].position, ships[j].position))<120) {
                Line shipLinesI[4] = {
                    {
                        {(-40*cosf(ships[i].heading)-15*sinf(ships[i].heading)+ships[i].position.x),(-40*sinf(ships[i].heading)+15*cosf(ships[i].heading)+ships[i].position.y)},
                       {(40*cosf(ships[i].heading)-15*sinf(ships[i].heading)+ships[i].position.x),(40*sinf(ships[i].heading)+15*cosf(ships[i].heading)+ships[i].position.y)}
                    },
                    {
                    {(-40*cosf(ships[i].heading)+15*sinf(ships[i].heading)+ships[i].position.x), (-40*sinf(ships[i].heading)-15*cosf(ships[i].heading)+ships[i].position.y)},
                    {(40*cosf(ships[i].heading)+15*sinf(ships[i].heading)+ships[i].position.x), (40*sinf(ships[i].heading)-15*cosf(ships[i].heading)+ships[i].position.y)}
                    },
                    {
                    {(-40*cosf(ships[i].heading)-15*sinf(ships[i].heading)+ships[i].position.x), (-40*sinf(ships[i].heading)+15*cosf(ships[i].heading)+ships[i].position.y)},
                    {(-40*cosf(ships[i].heading)+15*sinf(ships[i].heading)+ships[i].position.x), (-40*sinf(ships[i].heading)-15*cosf(ships[i].heading)+ships[i].position.y)}
                    },
                    {
                    {40*cosf(ships[i].heading)-15*sinf(ships[i].heading)+ships[i].position.x, 40*sinf(ships[i].heading)+15*cosf(ships[i].heading)+ships[i].position.y},
                    {(40*cosf(ships[i].heading)+15*sinf(ships[i].heading)+ships[i].position.x), (40*sinf(ships[i].heading)-15*cosf(ships[i].heading)+ships[i].position.y)}
                    }
                };

                Line shipLinesJ[4] = {
                    {
                        {(-40*cosf(ships[j].heading)-15*sinf(ships[j].heading)+ships[j].position.x),(-40*sinf(ships[j].heading)+15*cosf(ships[j].heading)+ships[j].position.y)},
                       {(40*cosf(ships[j].heading)-15*sinf(ships[j].heading)+ships[j].position.x),(40*sinf(ships[j].heading)+15*cosf(ships[j].heading)+ships[j].position.y)}
                    },
                    {
                        {(-40*cosf(ships[j].heading)+15*sinf(ships[j].heading)+ships[j].position.x), (-40*sinf(ships[j].heading)-15*cosf(ships[j].heading)+ships[j].position.y)},
                        {(40*cosf(ships[j].heading)+15*sinf(ships[j].heading)+ships[j].position.x), (40*sinf(ships[j].heading)-15*cosf(ships[j].heading)+ships[j].position.y)}
                    },
                    {
                        {(-40*cosf(ships[j].heading)-15*sinf(ships[j].heading)+ships[j].position.x), (-40*sinf(ships[j].heading)+15*cosf(ships[j].heading)+ships[j].position.y)},
                        {(-40*cosf(ships[j].heading)+15*sinf(ships[j].heading)+ships[j].position.x), (-40*sinf(ships[j].heading)-15*cosf(ships[j].heading)+ships[j].position.y)}
                    },
                    {
                        {40*cosf(ships[j].heading)-15*sinf(ships[j].heading)+ships[j].position.x, 40*sinf(ships[j].heading)+15*cosf(ships[j].heading)+ships[j].position.y},
                        {(40*cosf(ships[j].heading)+15*sinf(ships[j].heading)+ships[j].position.x), (40*sinf(ships[j].heading)-15*cosf(ships[j].heading)+ships[j].position.y)}
                    }
                };
                for (int k = 0; k < 4; k++) {
                    for (int l = 0; l < 4; l++) {
                        if (CheckCollisionLines(shipLinesI[k].start, shipLinesI[k].end, shipLinesJ[l].start, shipLinesJ[l].end, NULL)) {
                            ships[i].isAlive = 0;
                            ships[j].isAlive = 0;
                        }
                    }
                }
            }
        }
    }
}
int checkProjectileCollision(Ship ship, Projectile *projectiles) {
    Vector2 shipPos = ship.position;//Position of the provided ship
    if (ship.isAlive==0) return 0;
    Line shipLines[4] = { //The line segments that make up the hitbox of the ship
        {
            {-40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x,-40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y},
           {40*cosf(ship.heading)-15*sinf(ship.heading)+shipPos.x,40*sinf(ship.heading)+15*cosf(ship.heading)+shipPos.y}
        },
        {
                        {-40*cosf(ship.heading)-7*sinf(ship.heading)+shipPos.x,-40*sinf(ship.heading)+7*cosf(ship.heading)+shipPos.y},
                        {40*cosf(ship.heading)-7*sinf(ship.heading)+shipPos.x, 40*sinf(ship.heading)+7*cosf(ship.heading)+shipPos.y}
        },
        {
                        {-40*cosf(ship.heading)+7*sinf(ship.heading)+shipPos.x, -40*sinf(ship.heading)-7*cosf(ship.heading)+shipPos.y},
                        {40*cosf(ship.heading)+7*sinf(ship.heading)+shipPos.x, 40*sinf(ship.heading)-7*cosf(ship.heading)+shipPos.y}
        },
        {
                        {-40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x, -40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y},
                        {40*cosf(ship.heading)+15*sinf(ship.heading)+shipPos.x, 40*sinf(ship.heading)-15*cosf(ship.heading)+shipPos.y}
        }
    };
    for (int i = 0; i < selectedPlayers; i++) {
        Projectile projectile = projectiles[i];
        for (int j = 0; j < 4; j++) {
            if (CheckCollisionCircleLine((Vector2){projectile.position.x, projectile.position.y}, 15, shipLines[i].start, shipLines[i].end)&&projectile.position.z<15&&projectile.position.z>0&&projectile.team!=ship.team) {
                projectiles[i].position.z = -10;
                return 1;
            }
        }
    }
    return 0;
}

int checkTerrainCollision(Ship ship, struct CollisionSection sections[], int sectionCount){//Checks if the provided ship is colliding with any obstacle. Returns 1 if it detects collision and 0 if it doesn't
    Vector2 shipPos = ship.position;//Position of the provided ship
    Line shipLines[4] = { //The line segments that make up the hitbox of the ship
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

void saveGame(Ship *ships, Projectile *projectiles, int selectedPlayers, int targetPlayer, int picking, float roundTimer, GameState gameState) {
    FILE *f;
    f = fopen("save.dat", "wb");
    if (f == NULL) {
        printf("Failed to save game!\n");
    }
    else {
        struct {
            Ship shipsArray[MAX_PLAYERS];
            Projectile prjectileArray[MAX_PLAYERS];
            int numPlayers;
            int trgtPlayer;
            int pickingPlayer;
            float rndTimer;
            GameState state;
        } saveStruct;

        saveStruct.numPlayers  = selectedPlayers;
        saveStruct.trgtPlayer = targetPlayer;
        saveStruct.pickingPlayer = picking;
        saveStruct.rndTimer = roundTimer;
        saveStruct.state = gameState;

        for (int i = 0; i < MAX_PLAYERS; i++) {
            saveStruct.shipsArray[i] = ships[i];
            saveStruct.prjectileArray[i] = projectiles[i];
        }
        if (fwrite(&saveStruct, sizeof(saveStruct), 1, f)!=1) {
            printf("Failed to save game!\n");
        }
    }
    fclose(f);
}

bool loadGame(Ship *ships, Projectile *projectiles, int *selectedPlayers, int *targetPlayer, int *picking, float *roundTimer, GameState *gameState) {
    FILE *f = fopen("save.dat", "rb");
    if (f == NULL) {
        printf("Failed to load game!\n");
        fclose(f);
        return false;
    }
    struct {
            Ship shipsArray[MAX_PLAYERS];
            Projectile prjectileArray[MAX_PLAYERS];
            int numPlayers;
            int trgtPlayer;
            int pickingPlayer;
            float rndTimer;
            GameState state;
        } saveStruct;
    if (fread(&saveStruct, sizeof(saveStruct), 1, f)!=1) {
        fclose(f);
        return false;
    }
    for (int i = 0; i < MAX_PLAYERS; i++) {
        ships[i] = saveStruct.shipsArray[i];
        projectiles[i] = saveStruct.prjectileArray[i];
    }
    *selectedPlayers = saveStruct.numPlayers;
    *targetPlayer = saveStruct.trgtPlayer;
    *picking = saveStruct.pickingPlayer;
    *roundTimer = saveStruct.rndTimer;
    *gameState = saveStruct.state;
    fclose(f);
    return true;
}