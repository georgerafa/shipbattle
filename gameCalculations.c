#include <math.h>
#include "raylib.h"
#include "gameCalculations.h"

#include <stdio.h>
typedef struct ShipStruct Ship;
typedef struct ProjectileStruct Projectile;

#define MAX_PLAYERS 6
struct SpawnPosition {
    Vector2 position;
    float heading;
} spawnPositions[6] = {
    {
        (Vector2){1432, 1068},
        3*M_PI/2 - M_PI/6
    },{
        (Vector2){730, 297},
        M_PI/3
    },{
        (Vector2){683, 1067},
        -M_PI/4
    },{
        (Vector2){1656, 772},
        M_PI
    },{
        (Vector2){470, 780},

    },{
        (Vector2){1466, 378},
        M_PI-M_PI/6
    }
};


int getLinePoint(Projectile p, int x) {
    float t = x/(PROJECTILE_SPEED*cosf(p.angle));
    return (int)(10 + sinf(p.angle)*PROJECTILE_SPEED*t - 0.5*GRAVITY*t*t);
}

//Updates the positions of the ships provided based on their current position, speed, and time passed (deltaT in seconds) since last update;
void updateShipPositions(Ship *ships, int shipCount, float deltaT) {
    for (int i = 0; i < shipCount; i++) {
        Vector2 oldPos = ships[i].position;
        float speedX = cosf(ships[i].heading)*ships[i].speed; //Calculate speed on the X axis
        float speedY = sinf(ships[i].heading)*ships[i].speed; //Calculate speed on the Y axis
        ships[i].position.x += speedX*deltaT; //Adjust X position
        ships[i].position.y += speedY*deltaT; //Adjust Y position
        ships[i].distanceMoved = Vector2Add(Vector2Subtract(ships[i].position, oldPos), ships[i].distanceMoved);
    }
}

//Initializes the ships provided by setting their heading and speed values to 0 as well as setting their initial position
void initializeShips(Ship *ships, int shipCount) {
    for (int i = 0; i < shipCount; i++) {
        ships[i] = (Ship){i, spawnPositions[i].position, 0, spawnPositions[i].heading, 1, 0};  // Initialize ships with offset positions
    }
}


void resetProjectiles(Projectile *projectiles, int projectileCount) {
    for (int i = 0; i < projectileCount; i++) {
        projectiles[i].angle = 0;
        projectiles[i].heading = 0;
    }
}

void initializeProjectiles(Projectile *projectiles, Ship ships[], int playerCount) { //Initialize the provided projectiles by setting calculating their speed on each axis based on where they are looking
    for (int i = 0; i < playerCount; i++) {
        projectiles[i].speed.x = cosf(projectiles[i].heading)*cosf(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.y = sinf(projectiles[i].heading)*cosf(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.z = sinf(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].position.z = 10-20*(1-ships[i].isAlive);
        projectiles[i].team = i;
    }
}

//Update the projectiles' speeds and positions while also applying gravity
void updateProjectiles(Projectile *projectiles, Ship *ships, int projectileCount, float deltaT) {
    for (int i = 0; i < projectileCount; i++) {
        Projectile projectile = projectiles[i];
        if (projectiles[i].position.z > 0) {
            projectiles[i].position = (Vector3){projectile.position.x+projectile.speed.x*deltaT, projectile.position.y+projectile.speed.y*deltaT, projectile.position.z+projectile.speed.z*deltaT};
            projectiles[i].speed.z -= GRAVITY*deltaT;
        }
    }
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

int checkProjectileCollision(Ship ship, Projectile *projectiles, int playerCount) {
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
    for (int i = 0; i < playerCount; i++) {
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


int playersAlive(Ship ships[], int playerCount) {
    int playersAlive = 0;
    for (int i = 0; i < playerCount; i++) {
        if (ships[i].isAlive == 1) playersAlive++;
    }
    return playersAlive;
}

void checkShipCollisions(Ship *ships, int playerCount) {
    for (int i = 0; i < playerCount; i++) {
        for (int j = 0; j < playerCount; j++) {
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