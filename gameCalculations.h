//
// Created by VagsS on 06/01/2025.
//

#ifndef GAMECALCULATIONS_H
#define GAMECALCULATIONS_H
typedef struct ShipStruct {
    int team; //Ship team
    Vector2 position; //Current ship Y coordinate
    int speed; //Current ship speed
    double heading; //Direction in radians
    int isAlive;
} Ship; //Ship object

typedef struct ProjectileStruct {
    int team;
    Vector3 position;
    Vector3 speed;
    double heading;
    double angle;
} Projectile;

void setSpawnCircle(Vector2 pos, int radius);
void setInitialPosition(Ship ship, int shipCount, int index);
void updateShipPositions(Ship *ships, int shipCount, double deltaT);

#endif //GAMECALCULATIONS_H
