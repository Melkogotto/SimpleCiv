#ifndef UNIT
#define UNIT

#include "../int_array/int_array.h"
#include "../dyn_array/dyn_array.h"

/*
    This is struct of unit. It's really similar to technology struct (see
    modules/technology/technology.h file).
    This struct for units' info table (common for all players).
    Each player also will have table with technologies' id and value.
    Also there will be a common for all players table with these values: hp,
    damage, etc.
*/

typedef struct UnitCommonInfo
{
    char * name;
    int max_health;
    int max_damage;
    int max_moves;
} UnitCommonInfo;

/*
    This struct of real unit, who live in our world.
*/
typedef struct Unit
{
    int unit_id;  // Unit id in units' info table.
    int owner_id; // Id of owner of this unit.
    int x, y;     // X and Y coordinates.
    int health;   // Current health.
    int moves;    // Available moves.
} Unit;

/*
    Destroys UnitCommonInfo variable.
*/
void destroyUnitCommonInfo(void * data);

/*
    Creates unit table.
*/
IntArray * createUnitStatus(IntArray * techs_status, DynArray * techs_info, DynArray * units_info);

#endif
