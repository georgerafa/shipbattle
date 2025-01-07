#include <math.h>
#include "raylib.h"
#include "gameCalculations.h"
typedef struct ShipStruct Ship;
typedef struct ProjectileStruct Projectile;
const int PROJECTILE_SPEED = 500; //The initial projectile speed
const int GRAVITY = 90; //Gravitational acceleration

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


void initializeProjectiles(Projectile *projectiles, int projectileCount) { //Initialize the provided projectiles by setting calculating their speed on each axis based on where they are looking
    for (int i = 0; i < projectileCount; i++) {
        projectiles[i].speed.x = cos(projectiles[i].heading)*cos(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.y = sin(projectiles[i].heading)*cos(projectiles[i].angle)*PROJECTILE_SPEED;
        projectiles[i].speed.z = sin(projectiles[i].angle)*PROJECTILE_SPEED;
    }
}

//Update the projectiles' speeds and positions while also applying gravity
void updateProjectiles(Projectile *projectiles, int projectileCount, double deltaT) {
    for (int i = 0; i < projectileCount; i++) {
        Projectile projectile = projectiles[i];
        if (projectiles[i].position.z > 0) {
            projectiles[i].position = (Vector3){projectile.position.x+projectile.speed.x*deltaT, projectile.position.y+projectile.speed.y*deltaT, projectile.position.z+projectile.speed.z*deltaT};
            projectiles[i].speed.z -= GRAVITY*deltaT;
        }
    }
};