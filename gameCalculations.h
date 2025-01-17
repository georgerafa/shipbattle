#ifndef GAMECALCULATIONS_H
#define GAMECALCULATIONS_H
#define PROJECTILE_SPEED  200.0f //The initial projectile speed
#define GRAVITY  45.0f //Gravitational acceleration
#define maxShipSpeed 75
#define MAX_PLAYERS 6
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

typedef struct Line {
    Vector2 start;
    Vector2 end;
} Line;

struct CollisionSection {
    Vector2 centerPosition;
    int minimumDistance;
    Line Lines[10];
};

int playersAlive(Ship ships[], int playerCount);
void checkShipCollisions(Ship *ships, int playerCount);
int checkProjectileCollision(Ship ship, Projectile *projectiles, int playerCount);
int checkTerrainCollision(Ship ship, struct CollisionSection[], int sectionCount);
int getLinePoint(Projectile p, int x);
void updateShipPositions(Ship *ships, int shipCount, float deltaT);
void updateProjectiles(Projectile *projectiles, Ship *ships, int projectileCount, float deltaT);
void initializeProjectiles(Projectile *projectiles, Ship ships[], int playerCount);
void resetProjectiles(Projectile *projectiles, int projectileCount);
void initializeShips(Ship *ships, int shipCount);
#endif //GAMECALCULATIONS_H
