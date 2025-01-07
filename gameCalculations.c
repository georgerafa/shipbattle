#include <math.h>
#include "raylib.h"
#include "gameCalculations.h"
typedef struct ShipStruct Ship;
typedef struct ProjectileStruct Projectile;
const int PROJECTILE_SPEED = 500;
const float GRAVITY = 90.81f;
struct {
    Vector2 position;
    int radius;
} spawnCircle; //Circle on which ships will spawn
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

void initializeProjectiles(Projectile *projectiles, int projectileCount) {
    for (int i = 0; i < projectileCount; i++) {
        projectiles[i].speed.x = cos(projectiles[i].heading)*cos(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.y = sin(projectiles[i].heading)*cos(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.z = sin(projectiles[i].angle)*PROJECTILE_SPEED;
    }
}

void updateProjectiles(Projectile *projectiles, int projectileCount, double deltaT) {
    for (int i = 0; i < projectileCount; i++) {
        Projectile projectile = projectiles[i];
        if (projectiles[i].position.z > 0) {
            projectiles[i].position = (Vector3){projectile.position.x+projectile.speed.x*deltaT, projectile.position.y+projectile.speed.y*deltaT, projectile.position.z+projectile.speed.z*deltaT};
            projectiles[i].speed.z -= GRAVITY*deltaT;
        }
    }
};