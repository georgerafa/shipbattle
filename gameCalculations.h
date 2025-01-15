#ifndef GAMECALCULATIONS_H
#define GAMECALCULATIONS_H
typedef struct ShipStruct {
    int team; //Ship team
    Vector2 position; //Current ship position
    float speed; //Current ship speed
    float heading; //Direction in radians
    int isAlive;
    Vector2 distanceMoved;
} Ship; //Ship object

typedef struct ProjectileStruct {
    int team; //Origin ship team
    Vector3 position; //Current projectile position
    Vector3 speed; //Current projectile speed
    float heading; //Direction in radians
    float angle; //Elevation angle in radians
} Projectile;

float getLinePoint(Projectile p, float x);
void updateShipPositions(Ship *ships, int shipCount, float deltaT);
void updateProjectiles(Projectile *projectiles, Ship *ships, int projectileCount, float deltaT);
void initializeProjectiles(Projectile *projectiles, int projectileCount);
void initializeShips(Ship *ships, int shipCount);
#endif //GAMECALCULATIONS_H
