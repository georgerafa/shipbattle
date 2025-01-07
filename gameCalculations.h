#ifndef GAMECALCULATIONS_H
#define GAMECALCULATIONS_H
typedef struct ShipStruct {
    int team; //Ship team
    Vector2 position; //Current ship position
    int speed; //Current ship speed
    double heading; //Direction in radians
    int isAlive;
} Ship; //Ship object

typedef struct ProjectileStruct {
    int team; //Origin ship team
    Vector3 position; //Current projectile position
    Vector3 speed; //Current projectile speed
    double heading; //Direction in radians
    double angle; //Elevation angle in radians
} Projectile;

void updateShipPositions(Ship *ships, int shipCount, double deltaT);
void updateProjectiles(Projectile *projectiles, int projectileCount, double deltaT);
void initializeProjectiles(Projectile *projectiles, int projectileCount);

#endif //GAMECALCULATIONS_H
