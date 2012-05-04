#include <stdlib.h>
#include <math.h>

#include "../../game/world/definitions.h"
#include "../graph/graph.h"
#include "../technology/technology.h"
#include "unit.h"

void destroyUnitCommonInfo(void * data)
{
    UnitCommonInfo * unit = (UnitCommonInfo *) data;
    free(unit -> name);
    free(unit);
}

IntArray * createUnitStatus(IntArray * techs_status, DynArray * techs_info, DynArray * units_info)
{
    IntArray * result = iaLengthCreate(units_info -> length);

    for(int i = 0; i < techs_status -> length; i++)
    {
        if( iaGetByIndex(techs_status, i) == TECH_RESEARCHED )
        {
            IntArray * units = ((Technology *) ((Node *) daGetByIndex(techs_info , i)) -> data) -> provides_units;
            if(units != NULL)
            {
                for(int j = 0; j < units -> length; j++)
                {
                    int id = iaGetByIndex(units, j);
                    iaSetByIndex(result, id, UNIT_AVAILABLE);
                }
            }
        }
    }

    return result;
}

Unit * createUnit(DynArray * units_info, int unit_id, Player * player)
{
    Unit * unit = malloc(sizeof(Unit));
    UnitCommonInfo * info = (UnitCommonInfo *) daGetByIndex(units_info, unit_id);

    unit -> unit_id = unit_id;
    unit -> owner = player;
    unit -> x = -1;
    unit -> y = -1;
    unit -> health = info -> max_health;
    unit -> moves = info -> max_moves;

    // TODO add to player -> units

    return unit;
}

void unitsFight(DynArray * units_info, Unit * unit1, Unit * unit2)
{
    // Getting units' info.
    UnitCommonInfo * u1 = (UnitCommonInfo *) daGetByIndex(units_info, unit1 -> unit_id);
    UnitCommonInfo * u2 = (UnitCommonInfo *) daGetByIndex(units_info, unit2 -> unit_id);

    // Calculating max damage.
    double damage1 = (double) unit1 -> health / u1 -> max_health * u1 -> max_damage;
    double damage2 = (double) unit2 -> health / u2 -> max_health * u2 -> max_damage;

    // Random damage between 70% and 100% of damage1/damage2.
    damage1 *= ((double) (rand() % 7 + 3.0f)) / 10.0f;
    damage2 *= ((double) (rand() % 7 + 3.0f)) / 10.0f;

    // Fight!
    unit1 -> health -= ceil(damage2);
    unit2 -> health -= ceil(damage1);
}
