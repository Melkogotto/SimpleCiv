#include "world.h"

#include "../../modules/parser/xml.h"

World * createWorld()
{
    // Creating world.
    World * world = malloc(sizeof(World));

    // Parsing config.xml.
    printf("Parsing config.xml… ");
    world -> properties = parseXML(XML_CONFIG);
    if(world -> properties == NULL)
    {
        printf("Failed\n\033[1;31mError:\033[0m config.xml doesn't exist or corrupted.\n");
        return NULL;
    }
    printf("Done\n");

    // Parsing units.xml.
    printf("Parsing resources/units.xml… ");
    world -> units_info = parseXML(XML_UNITS);
    if(world -> units_info == NULL)
    {
        printf("Failed\n\033[1;31mError:\033[0m resources/units.xml doesn't exist or corrupted.\n");
        return NULL;
    }
    printf("%d units loaded\n", world -> units_info -> length);

    // Parsing technologies.xml.
    printf("Parsing resources/technologies.xml… ");
    DynArray * techs_data = parseXML(XML_TECHNOLOGIES);
    if(techs_data == NULL)
    {
        printf("Failed\n\033[1;31mError:\033[0m resources/technologies.xml doesn't exist or corrupted.\n");
        return NULL;
    }
    printf("%d technologies loaded\n", techs_data -> length);

    // Going through techs_data and creating tech tree.
    printf("Creating technology tree… ");
    // Creates edges in technology tree (we already have nodes).
    world -> tech_tree = createEdgesInTechnologyTree(techs_data);
    printf("Done\n");

    // Creating techs_info table.
    world -> techs_info = daCreate();
    for(int i = 0; i < techs_data -> length; i++)
    {
        TechnologyParseInfo * t = (TechnologyParseInfo *) daGetByIndex(techs_data, i);
        daPrepend(world -> techs_info, t -> tech_in_tree);
    }

    // Creating tech tables.
    printf("Creating technology table… ");
    IntArray * techs_status = createTechnologyStatus(world -> techs_info);
    printf("Done\n");

    // Creating unit tables.
    printf("Creating units table… ");
    IntArray * units_status = createUnitStatus(techs_status, world -> techs_info, world -> units_info);
    printf("Done\n");

    // Free techs_data.
    printf("Freeing auxiliary data… ");
    daDestroy(techs_data, &destroyTechnologyParseInfo);
    printf("Done\n");

    // Creating players list.
    printf("Creating list of %d players… ", world -> properties -> players_count);
    world -> graph_players = NULL; // Graph head.
    Node * temp = NULL;            // Temporary variable.
    for(int i = 0; i < world -> properties -> players_count; i++)
    {
        // Creating new player.
        char * name = (char *) daGetByIndex(world -> properties -> player_names, i);
        Player * player = createPlayer(name, iaCopy(units_status), iaCopy(techs_status));
        temp = addNode(temp, EDGE_NEXT_PLAYER, NODE_PLAYER, player);
        // Remembering head.
        if(world -> graph_players == NULL)
        {
            world -> graph_players = temp;
        }
    }
    addEdge(temp, world -> graph_players, EDGE_NEXT_PLAYER);
    iaDestroy(units_status);
    iaDestroy(techs_status);
    printf("Done\n");

    // Creating map.
    printf("Creating map %dx%d… ", world -> properties -> map_w, world -> properties -> map_h);
    world -> graph_map = createMap(world -> properties -> map_w, world -> properties -> map_h);
    //generateMap(world -> map_head, properties -> map_w, properties -> map_h);
    printf("Done\n");


    printf("All done!\n");

    // Returning world.
    return world;
}

/*
    Deletes graph's node.
*/
void destroyGraphNode(unsigned char type, void * data)
{
    switch(type)
    {
        case NODE_PLAYER:
            iaDestroy( ((Player *) data) -> available_units );
            iaDestroy( ((Player *) data) -> available_techs );
            free( ((Player *) data) -> name );
        break;
    }

    free(data);
}

void destroyWorld(World * world)
{
    // Auxiliary array for destroyGraph();
    DynArray * deleted = daCreate();

    // Destroy map.
    destroyMap(world -> graph_map, world -> properties -> map_w, world -> properties -> map_h);

    // Destroy properties.
    daDestroy(world -> properties -> player_names, NULL);
    free(world -> properties);

    // Destroy array of UnitCommonInfos.
    daDestroy(world -> units_info, &destroyUnitCommonInfo);

    // Destroy array of pointers to tech_in_tree.
    daDestroy(world -> techs_info, &destroyTechnologyCommonInfo);

    // Destroy world.
    destroyGraph(world -> graph_players, deleted, &destroyGraphNode);
    free(world);

    // Destroy auxiliary array.
    daDestroy(deleted, NULL);
}

void saveWorld(World * world, WorldProperties * properties, FILE * map_file)
{
    fprintf(map_file, "<?xml version=\"1.0\" encoding=\"UTF-8\">\n<map>\n\t<width>%d</width>\n\t<height>%d</height>\n", properties -> map_w, properties -> map_h);

    Node * current = world -> graph_map;

    for(int i = 0; i < properties -> map_w; i++)
    {
        for(int j = 0; j < properties -> map_h; j++)
        {
            fprintf(map_file, "\t<cell x=\"%d\" y=\"%d\" territory=\"%d\">\n", i, j, ( (Cell *) current -> data ) -> territory);
            current = getNeighbour(current, EDGE_CELL_RIGHT);
        }
        current = getNeighbour(current, EDGE_CELL_BOTTOM);
    }

    fprintf(map_file, "</map>");
}
