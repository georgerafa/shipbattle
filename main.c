#include "raylib.h"
#include <math.h>
#include <stdio.h>

#include "gameCalculations.h"
#include "raymath.h"

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
struct CollisionSection {
    Vector2 centerPosition;
    int minimumDistance;
    struct Line Lines[10];
};

#define MAX_OBSTACLES 8
#define MAX_PLAYERS 6
Obstacle obstacles[MAX_OBSTACLES]; //Array of obstacles

//Declare function prototypes
void initObstacles();
void drawObstacles();
int checkCollision(Ship ship, struct CollisionSection[], int sectionCount);

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
    FILE *f = fopen("collisions.dat", "rb");
    fseek(f, 0, SEEK_END);
    int length = ftell(f);
    struct CollisionSection readSections[length];
    rewind(f);
    fread(&readSections, sizeof(readSections), 1, f);
    fclose(f);

    InitWindow(800, 800, "POLYNAYMAXIA"); //Initialize the game window
    const int display = GetCurrentMonitor(); //Get which display the game is running on
    const int screenWidth = GetMonitorWidth(display); //Get screen width
    const int screenHeight = GetMonitorHeight(display); //Get screen height
    SetWindowSize(screenWidth, screenHeight); //Set the game window size to be the same as the screen size
    ToggleFullscreen(); //Set window mode to fullscreen

    Image oceanImage = LoadImage("assets/gameMap.png"); //Load ocean background image
    ImageResize(&oceanImage, 2048, 2048);
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
    camera.zoom = (screenWidth)/2048.0f; //Set camera zoom to 1
    SetTargetFPS(GetMonitorRefreshRate(display)); //Set target fps to monitor refresh rate

    Ship ships[MAX_PLAYERS]; //Create array for storing ships
    for (int i = 0; i < MAX_PLAYERS; i++) {
        ships[i] = (Ship){0, {1000 + (i * 120), 500}, 100, 0, 1};  // Initialize ships with offset positions
    }
    initObstacles(); //Initialize obstacles

    while (!WindowShouldClose()) //While the window open
    {
        switch (currentScreen) //Change what is displayed based on current screen state
        {
            case TITLE: //Startup scren
                if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) //If enter is pressed or LMB is clicked move to player selection screen
                {
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
                DrawText("Welcome to Mononaumaxia!", 100, 100, 50, WHITE);
                DrawText("Press ENTER or click to start", 100, 200, 30, WHITE);

                EndDrawing();//Stop drawing screen
                break;

            case PLAYER_SELECT: //Player selection screen
                if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) { //Navigate through options
                    if (IsKeyPressed(KEY_UP) && selectedPlayers > 1) {
                        selectedPlayers--;
                    } else if (IsKeyPressed(KEY_DOWN) && selectedPlayers < MAX_PLAYERS) {
                        selectedPlayers++;
                    }
                }

                if (IsKeyPressed(KEY_ENTER)) {//If enter is pressed start game
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
                //Im not commenting these they will change
                updateShipPositions(ships, selectedPlayers, GetFrameTime());
                for (int i = 0; i < selectedPlayers; i++) {
                    Vector2 mousePosWorld = GetScreenToWorld2D(GetMousePosition(), camera);
                    ships[i].heading = atan2f(mousePosWorld.y - ships[i].position.y, mousePosWorld.x - ships[i].position.x);
                }
                for (int i = 0; i < selectedPlayers; i++) {
                    //ships[i].isAlive = 1 - checkCollision(ships[i]);
                }

                BeginDrawing();
                ClearBackground(DARKBLUE);


                BeginMode2D(camera);

                DrawTexture(oceanTexture, 0, 0, WHITE);

                //drawObstacles();
                Ship ship = ships[0];
                // Draw the ship
                if (ship.isAlive) {
                    //checkCollision(ships[0]);
                    DrawTexturePro(
                    shipTexture,
                    (Rectangle){0, 0, shipTexture.width, shipTexture.height},
                    (Rectangle){ship.position.x, ship.position.y, 100, 100},
                    (Vector2){50, 50},
                    (ships[0].heading * RAD2DEG) + 270,
                    WHITE);
                }
                EndMode2D();
                //DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GRAY);
                if (checkCollision(ship, readSections, length/sizeof(struct CollisionSection))) {
                   DrawText("COLLISION", 10, 10, 30, RED);
                }
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


int checkCollision(Ship ship, struct CollisionSection sections[], int sectionCount){//Checks if the provided ship is colliding with any obstacle. Returns 1 if it detects collision and 0 if it doesn't
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
    for (int i = 0; i < sectionCount; i++) {//Iterate through all obstacles
        struct CollisionSection section = sections[i]; //Current obstacle
        Vector2 centerPos = section.centerPosition; //Obstacle offset from (0,0)
        //If the distance, between the ship and the obstacle, is less than 200 pixels check for collision
        if(Vector2Length(Vector2Subtract(centerPos,ship.position)) < section.minimumDistance) {
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
