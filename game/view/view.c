/*

    SimpleCiv is simple clone of Civilization game, using ncurses library.
    Copyright (C) 2012 by K. Mingulov, A. Sapronov.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include <stdlib.h>

#include "../../modules/player/player.h"
#include "../../modules/city/city.h"
#include "../../modules/unit/unit.h"
#include "../../modules/unit/unit_common_info.h"
#include "../../modules/technology/technology.h"
#include "../../modules/graph/graph.h"
#include "../world/definitions.h"
#include "../message/message.h"

#include "view_settings.h"
#include "view.h"

/*
    Initialize colour pairs.
*/
void initColours()
{
    // Default colour pair.
    init_pair(0, COLOR_WHITE, COLOR_BLACK);

    // Colour pairs for cells.
    init_pair(CELL_TYPE_WATER,    COLOR_BLUE,  COLOR_BLACK);
    init_pair(CELL_TYPE_GRASS,    COLOR_GREEN, COLOR_BLACK);
    init_pair(CELL_TYPE_HILL,     COLOR_WHITE, COLOR_BLACK);
    init_pair(CELL_TYPE_TREE,     COLOR_GREEN, COLOR_BLACK);
    init_pair(CELL_TYPE_MOUNTAIN, COLOR_WHITE, COLOR_BLACK);

    // Players.
    init_pair(PLAYER_COLOURS_START + 0, 1, 0);
    init_pair(PLAYER_COLOURS_START + 1, 3, 0);
    init_pair(PLAYER_COLOURS_START + 2, 5, 0);
    init_pair(PLAYER_COLOURS_START + 3, 6, 0);
    init_pair(PLAYER_COLOURS_START + 4, 7, 0);
}

View * createView(World * world)
{
    if(world == NULL)
    {
        return NULL;
    }

    View * result = malloc(sizeof(View));

    // Initialize curses.
    initscr();
    raw();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    start_color();
    attroff(A_BOLD);

    // Colour pairs.
    initColours();
    attron(COLOR_PAIR(0));

    // Get sizes.
    getmaxyx(stdscr, result -> rows, result -> columns);

    // Set sidebar.
    result -> sidebar = 0.8 * result -> columns;

    // Min size of sidepanel is 20 chars.
    if(result -> sidebar > result -> columns - 20)
    {
        result -> sidebar = result -> columns - 20;
    }

    result -> chooser = NULL;
    result -> textbox = NULL;

    // Go through player's list and init his view.
    for(int i = 0; i < world -> properties -> players_count; i++)
    {
        // Get player.
        Player * player = (Player *) listGetHead(world -> players);
        // Init his view settings.
        initPlayerSettings(world, result, player);
        // Go on.
        listScrollNext(world -> players);
    }

    // Load first player settings.
    Player * player = (Player *) listGetHead(world -> players);
    loadPlayerSettings(result, player);

    return result;
}

void destroyView(View * view)
{
    endwin();
    free(view);
}

void focusOn(World * world, View * view, int r, int c)
{
    // Get node with cell.
    Node * cell = getMapCell(world -> map, r, c);

    // Remember it.
    view -> current_cell = cell;
    view -> map_r = r;
    view -> map_c = c;

    // Set cursor to the middle of the screen.
    view -> cur_r = view -> rows / 2;
    view -> cur_c = view -> sidebar / 2;

    // Scroll player's graph_map.
    Player * player = (Player *) listGetHead(world -> players);
    // Map in the window starts from (1, 1) point, so add 1 to r and c
    // coordinates.
    player -> graph_map = getCell(view -> current_cell, 1 - view -> cur_r, 1 - view -> cur_c);
}

void drawBasicMapUI(World * world, View * view)
{
    erase();

    // Copying rows, columns and sidebar.
    int r = view -> rows;
    int c = view -> columns;
    int s = view -> sidebar;

    // Drawing main interface.
    drawBox(0, 0, r, c);
    drawVertLine(0, r, s);
    drawHorizLine(s, c - s, SIDEBAR_CELL_BLOCK);
    drawHorizLine(s, c - s, r - 3);

    // Some static strings.
    putInMiddle(r - 4, s + 1, c - s - 2, "Press h for help");
}

void drawTechView(World * world, View * view)
{
    erase();

    // Drawing interface.
    drawBox(0, 0, view -> rows, view -> columns);

    // Drawing player name and other info.
    int line = 2;
    Player * player = (Player *) listGetHead(world -> players);
    attron(A_BOLD); mvprintw(line++, 3, "%s's research", player -> name); attroff(A_BOLD);
    mvprintw(line++, 3, "%d gold, %d gold spents on reasearches every turn", player -> gold, player -> research -> delta);

    // Drawing space.
    line++;

    // Drawing research.
    attron(A_BOLD); mvprintw(line++, 3, "Current research:"); attroff(A_BOLD);
    if(player -> research -> id == -1)
    {
        mvprintw(line++, 3, "No research");
    }
    else
    {
        // Getting technology.
        Node * n = (Node *) daGetByIndex(world -> techs_info, player -> research -> id);
        Technology * t = (Technology *) n -> data;
        // Printing it.
        mvprintw(line++, 3, "%s (%d/%d turns)", t -> name, player -> research -> turns, t -> turns);
        // If player doesn't have enought money…
        if(player -> gold < player -> research -> delta)
        {
            mvprintw(line++, 3, "(Your research is suspended: you don't have enough gold!)");
        }
    }

    // Drawing space.
    line++;

    // Drawing available technologies.
    attron(A_BOLD); mvprintw(line++, 3, "Available for research:"); attroff(A_BOLD);
    // Remembering start_r line.
    view -> chooser -> start_r = line;
    mvprintw(line++, 3, "[ ] Do not explore anything");
    // Getting ids.
    IntArray * ids = view -> chooser -> ids;
    for(int i = 0; i < ids -> length; i++)
    {
        Technology * t = (Technology *) ((Node *) daGetByIndex(world -> techs_info, iaGetByIndex(ids, i))) -> data;
        mvprintw(line++, 3, "[ ] %s (%d turns)", t -> name, t -> turns);
    }

    // Drawing researched technologies.
    line++;
    attron(A_BOLD); mvprintw(line++, 3, "Already researched:"); attroff(A_BOLD);
    for(int i = 0; i < player -> available_techs -> length; i++)
    {
        int value = iaGetByIndex(player -> available_techs, i);
        if(value == TECH_RESEARCHED)
        {
            if(line < view -> rows - 3)
            {
                Technology * t = (Technology *) ((Node *) daGetByIndex(world -> techs_info, i)) -> data;
                mvprintw(line++, 3, "%s", t -> name);
            }
        }

        if(line == view -> rows - 3)
        {
            mvprintw(line++, 3, "And others...");
        }
    }

    mvaddch(view -> chooser -> start_r + view -> chooser -> current + 1, 4, '*');
    move(view -> chooser -> start_r + view -> chooser -> current + 1, 4);
}

void drawCityView(World * world, View * view)
{
    erase();

    // Drawing interface.
    drawBox(0, 0, view -> rows, view -> columns);

    // Drawing player name and other info.
    int line = 2;
    Player * player = (Player *) listGetHead(world -> players);
    City * city = (City * ) getNeighbour(view -> current_cell, EDGE_CELL_CITY) -> data;
    attron(A_BOLD); mvprintw(line++, 3, "%s's city %s", player -> name, city -> name); attroff(A_BOLD);
    mvprintw(line++, 3, "You have %d gold, %d gold spents on hiring every turn", player -> gold, city -> hiring -> delta);

    // Drawing space.
    line++;

    // Drawing unit.
    attron(A_BOLD); mvprintw(line++, 3, "Current hiring:"); attroff(A_BOLD);
    if(city -> hiring -> id == -1)
    {
        mvprintw(line++, 3, "No hiring");
    }
    else
    {
        // Getting unit.
        UnitCommonInfo * u_info = daGetByIndex(world -> units_info, city -> hiring -> id);
        mvprintw(line++, 3, "%s (%d/%d turns)", u_info -> name, city -> hiring -> turns, u_info -> hiring_turns);
        // If player doesn't have enought money…
        if(player -> gold < city -> hiring -> delta)
        {
            mvprintw(line++, 3, "(Your hiring is suspended: you don't have enough gold!)");
        }
    }

    // Drawing space.
    line++;

    // Drawing available technologies.
    attron(A_BOLD); mvprintw(line++, 3, "Available for hiring:"); attroff(A_BOLD);
    // Remembering start_r line.
    view -> chooser -> start_r = line;
    mvprintw(line++, 3, "[ ] Don't hire anyone");
    // Getting ids.
    IntArray * ids = view -> chooser -> ids;
    for(int i = 0; i < ids -> length; i++)
    {
        int id = iaGetByIndex(ids, i);
        UnitCommonInfo * u_info = (UnitCommonInfo *) daGetByIndex(world -> units_info, id);
        mvprintw(line++, 3, "[ ] %s (%d turns)", u_info -> name, u_info -> hiring_turns);
    }

    mvaddch(view -> chooser -> start_r + view -> chooser -> current + 1, 4, '*');
    move(view -> chooser -> start_r + view -> chooser -> current + 1, 4);
}

void drawPlayerInfo(World * world, View * view)
{
    int s = view -> sidebar;
    int len = view -> columns - view -> sidebar - 2;

    clearBlock(SIDEBAR_PLAYER_BLOCK + 1, s + 1, 7, len);

    // Player.
    Player * player = (Player *) listGetHead(world -> players);

    // Player info.
    attron(COLOR_PAIR(PLAYER_COLOURS_START + player -> colour));
    attron(A_BOLD);
    putInMiddle(SIDEBAR_PLAYER_BLOCK + 1, s + 1, len, "%s", player -> name);
    attroff(A_BOLD);
    attroff(COLOR_PAIR(PLAYER_COLOURS_START + player -> colour));
    // Researching.
    if(player -> research -> id == -1)
    {
        mvprintw(SIDEBAR_PLAYER_BLOCK + 2, s + 1, "No researches");
    }
    else
    {
        // Getting technology.
        Node * n = (Node *) daGetByIndex(world -> techs_info, player -> research -> id);
        Technology * t = (Technology *) n -> data;
        // Printing it.
        putInLeft(SIDEBAR_PLAYER_BLOCK + 2, s + 1, len, "%d/%d %s", player -> research -> turns, t -> turns, t -> name);
    }
    // Gold.
    mvprintw(SIDEBAR_PLAYER_BLOCK + 3, s + 1, "Gold");
    putInRight(SIDEBAR_PLAYER_BLOCK + 3, s + 1, len, "%d-%d", player -> gold, player -> research -> delta);
    // Resources.
    const char res_names[][10] = {"", "Bronze", "Iron", "Coal", "Gunpowder", "Horses"};
    for(int i = 1; i < CELL_RES_COUNT; i++)
    {
        mvprintw(SIDEBAR_PLAYER_BLOCK + 3 + i, s + 1, res_names[i]);
        putInRight(SIDEBAR_PLAYER_BLOCK + 3 + i, s + 1, len, "%d", iaGetByIndex(player -> resources, i));
    }
}

void drawCellInfo(World * world, View * view)
{
    int s = view -> sidebar;
    int r = view -> rows;
    int len = view -> columns - view -> sidebar - 2;

    Player * player = (Player *) listGetHead(world -> players);

    clearBlock(SIDEBAR_CELL_BLOCK + 1, s + 1, 11, len);

    if(isKnownCell(player -> fog, view -> map_r, view -> map_c))
    {
        // Cell.
        Cell * c = (Cell *) view -> current_cell -> data;
        attron(A_BOLD);

        // Type of territory.
        switch(c -> territory)
        {
            case CELL_TYPE_WATER:    mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Water    "); break;
            case CELL_TYPE_GRASS:    mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Grass    "); break;
            case CELL_TYPE_TREE:     mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Forest   "); break;
            case CELL_TYPE_HILL:     mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Hill     "); break;
            case CELL_TYPE_MOUNTAIN: mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Mountains"); break;
        }

        // Resources. Hindi code.
        int delta = 0;
        if(c -> mine == CELL_MINE)
        {
            mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1, "[Mine] ");
            delta = 7;
        }
        switch(c -> resources)
        {
            case CELL_RES_BRONZE:    mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Bronze"); break;
            case CELL_RES_IRON:      mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Iron"); break;
            case CELL_RES_COAL:      mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Coal"); break;
            case CELL_RES_GUNPOWDER: mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Gunpowder"); break;
            case CELL_RES_HORSES:    mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Horses"); break;
            case CELL_RES_MUSHROOMS: mvprintw(SIDEBAR_CELL_BLOCK + 2, s + 1 + delta, "Mushrooms :O"); break;
        }

        attroff(A_BOLD);

        // City.
        Node * city = getNeighbour(view -> current_cell, EDGE_CELL_CITY);
        if(city != NULL)
        {
            City * c = (City *) city -> data;
            attron(A_BOLD); mvprintw(SIDEBAR_CELL_BLOCK + 3, s + 1, "City:  "); attroff(A_BOLD);
            putInLeft(SIDEBAR_CELL_BLOCK + 4, s + 2, len - 2, c -> name);
            attron(COLOR_PAIR(PLAYER_COLOURS_START + c -> owner -> colour));
            putInLeft(SIDEBAR_CELL_BLOCK + 5, s + 2, len - 2, c -> owner -> name);
            attroff(COLOR_PAIR(PLAYER_COLOURS_START + c -> owner -> colour));
            mvprintw(SIDEBAR_CELL_BLOCK + 6, s + 2, "People");
            putInRight(SIDEBAR_CELL_BLOCK + 6, s + 2, len - 2, "%d", c -> population);
        }

        // Unit.
        Node * unit = getNeighbour(view -> current_cell, EDGE_CELL_UNIT);
        if(unit != NULL)
        {
            Unit * u = (Unit *) unit -> data;
            UnitCommonInfo * u_info = (UnitCommonInfo *) daGetByIndex(world -> units_info, u -> unit_id);

            attron(A_BOLD); mvprintw(SIDEBAR_CELL_BLOCK + 7, s + 1, "Unit:  "); attroff(A_BOLD);
            putInLeft(SIDEBAR_CELL_BLOCK + 8, s + 2, len - 2, u_info -> name);

            if(u -> owner != NULL)
            {
                attron(COLOR_PAIR(PLAYER_COLOURS_START + u -> owner -> colour));
                putInLeft(SIDEBAR_CELL_BLOCK + 9, s + 2, len - 2, u -> owner -> name);
                attroff(COLOR_PAIR(PLAYER_COLOURS_START + u -> owner -> colour));
            }
            else
            {
                putInLeft(SIDEBAR_CELL_BLOCK + 9, s + 2, len - 2, "Neutral");
            }
            mvprintw(SIDEBAR_CELL_BLOCK + 10, s + 2, "HP");
            putInRight(SIDEBAR_CELL_BLOCK + 10, s + 2, len - 2, "%d/%d", u -> health, u_info -> max_health);
            mvprintw(SIDEBAR_CELL_BLOCK + 11, s + 2, "Moves");
            putInRight(SIDEBAR_CELL_BLOCK + 11, s + 2, len - 2, "%d/%d", u -> moves, u_info -> max_moves);
        }
    }
    else
    {
        attron(A_BOLD);
        mvprintw(SIDEBAR_CELL_BLOCK + 1, s + 1, "Fog of war");
        attroff(A_BOLD);
    }

    // Other info.
    clearBlock(r - 2, s + 1, 1, len);
    putInMiddle(r - 2, s + 1, len, "(%d,%d)", view -> map_r, view -> map_c);
}

void drawNode(World * world, Node * current)
{
    if(getNeighbour(current, EDGE_CELL_CITY) != NULL)
    {
        City * city = (City *) getNeighbour(current, EDGE_CELL_CITY) -> data;
        attron(COLOR_PAIR(PLAYER_COLOURS_START + city -> owner -> colour));
        attron(A_BOLD);
        addch('M');
        attroff(A_BOLD);
        attroff(COLOR_PAIR(PLAYER_COLOURS_START + city -> owner -> colour));
    }
    else if(getNeighbour(current, EDGE_CELL_UNIT) != NULL)
    {
        // Getting unit's char.
        Unit * unit = (Unit *) getNeighbour(current, EDGE_CELL_UNIT) -> data;
        UnitCommonInfo * u_info = (UnitCommonInfo *) daGetByIndex(world -> units_info, unit -> unit_id);
        if(unit -> owner != NULL)
        {
            attron(COLOR_PAIR(PLAYER_COLOURS_START + unit -> owner -> colour));
        }
        attron(A_BOLD);
        addch(u_info -> c);
        attroff(A_BOLD);
        if(unit -> owner != NULL)
        {
            attroff(COLOR_PAIR(PLAYER_COLOURS_START + unit -> owner -> colour));
        }
    }
    else
    {
        unsigned char type = ((Cell *) current -> data) -> territory;
        attron(COLOR_PAIR(type));
        switch ( type )
        {
            case CELL_TYPE_WATER    : addch('.'); break;
            case CELL_TYPE_GRASS    : addch('_'); break;
            case CELL_TYPE_HILL     : addch('-'); break;
            case CELL_TYPE_TREE     : addch('T'); break;
            case CELL_TYPE_MOUNTAIN : addch('^'); break;
            default                 : addch('E'); break;
        }
        attroff(COLOR_PAIR(type));
    }
}

void drawMap(World * world, View * view)
{
    int start_row = 1,    end_row = view -> rows - 2;
    int start_column = 1, end_column = view -> sidebar - 1;

    Player * player = (Player *) listGetHead(world -> players);
    Node * current = player -> graph_map;

    Node * line = current;

    // Calc deltas (for fog).
    int delta_r = view -> map_r - view -> cur_r;
    int delta_c = view -> map_c - view -> cur_c;

    for(int i = start_row; i <= end_row; i++)
    {
        for(int j = start_column; j <= end_column; j++)
        {
            move(i, j);
            if(isKnownCell(player -> fog, delta_r + i, delta_c + j))
            {
                drawNode(world, current);
            }
            else
            {
                addch(' ');
            }

            current = getNeighbour(current, EDGE_CELL_RIGHT);
        }
        line = getNeighbour(line, EDGE_CELL_BOTTOM);
        current = line;
    }
    move(view -> cur_r, view -> cur_c);
}

int viewProcess(World * world, View * view, List * list)
{
    Player * player = (Player *) listGetHead(world -> players);

    if(list != NULL)
    {
        ListElement * le = list -> head;
        for (int i = 0; i < list -> length; i++)
        {
            Message * message = (Message *) le -> data;
            switch(message -> type)
            {
                case VIEW_ESCAPE:
                    listDestroy(list, &free);
                    return 0;
                break;

                case VIEW_REDRAW_ALL:
                    drawBasicMapUI(world, view);
                    drawPlayerInfo(world, view);
                    drawCellInfo(world, view);
                    drawMap(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_REDRAW_INFO:
                    drawPlayerInfo(world, view);
                    drawCellInfo(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_REDRAW_MAP:
                    drawMap(world, view);
                break;

                case VIEW_MOVE_CURSOR_TOP:
                    view -> current_cell = getNeighbour(view -> current_cell, EDGE_CELL_TOP);
                    view -> map_r--; view -> map_r = view -> map_r < 0 ? view -> map_r + world -> properties -> map_r : view -> map_r;
                    if(view -> cur_r > 5)
                    {
                        view -> cur_r--;
                    }
                    else
                    {
                        player -> graph_map = getNeighbour(player -> graph_map, EDGE_CELL_TOP);
                        drawMap(world, view);
                    }
                    drawCellInfo(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_MOVE_CURSOR_BOTTOM:
                    view -> current_cell = getNeighbour(view -> current_cell, EDGE_CELL_BOTTOM);
                    view -> map_r = (view -> map_r + 1) % world -> properties -> map_r;
                    if(view -> cur_r < view -> rows - 5)
                    {
                        view -> cur_r++;
                    }
                    else
                    {
                        player -> graph_map = getNeighbour(player -> graph_map, EDGE_CELL_BOTTOM);
                        drawMap(world, view);
                    }
                    drawCellInfo(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_MOVE_CURSOR_RIGHT:
                    view -> current_cell = getNeighbour(view -> current_cell, EDGE_CELL_RIGHT);
                    view -> map_c = (view -> map_c + 1) % world -> properties -> map_c;
                    if(view -> cur_c < view -> sidebar - 5)
                    {
                        view -> cur_c++;
                    }
                    else
                    {
                        player -> graph_map = getNeighbour(player -> graph_map, EDGE_CELL_RIGHT);
                        drawMap(world, view);
                    }
                    drawCellInfo(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_MOVE_CURSOR_LEFT:
                    view -> current_cell = getNeighbour(view -> current_cell, EDGE_CELL_LEFT);
                    view -> map_c--; view -> map_c = view -> map_c < 0 ? view -> map_c + world -> properties -> map_c : view -> map_c;
                    if(view -> cur_c > 5)
                    {
                        view -> cur_c--;
                        move(view -> cur_r, view -> cur_c);
                    }
                    else
                    {
                        player -> graph_map = getNeighbour(player -> graph_map, EDGE_CELL_LEFT);
                        drawMap(world, view);
                    }
                    drawCellInfo(world, view);
                    move(view -> cur_r, view -> cur_c);
                break;

                case VIEW_REDRAW_TECH_DIALOG:
                    drawTechView(world, view);
                break;

                case VIEW_CHOOSER_MOVE_CURSOR_TOP:
                    if(view -> chooser -> current > -1)
                    {
                        addch(' ');
                        (view -> chooser -> current)--;
                        mvaddch(view -> chooser -> start_r + view -> chooser -> current + 1, 4, '*');
                        move(view -> chooser -> start_r + view -> chooser -> current + 1, 4);
                    }
                break;

                case VIEW_CHOOSER_MOVE_CURSOR_BOTTOM:
                    if(view -> chooser -> current < view -> chooser -> ids -> length - 1)
                    {
                        addch(' ');
                        (view -> chooser -> current)++;
                        mvaddch(view -> chooser -> start_r + view -> chooser -> current + 1, 4, '*');
                        move(view -> chooser -> start_r + view -> chooser -> current + 1, 4);
                    }
                break;

                case VIEW_REDRAW_CITY_DIALOG:
                    drawCityView(world, view);
                break;

                case VIEW_REDRAW_TEXTBOX:
                    drawTextbox(view -> textbox);
                break;
            }
            le = le -> next;
        }
        listDestroy(list, &free);
    }

    // No need in terminating.
    return 1;
}
