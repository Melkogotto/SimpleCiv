# common modules
OBJ_COMMON = list.o dyn_array.o int_array.o

# graph and map
OBJ_GRAPH = graph.o map.o landscape.o

# unit
OBJ_UNIT = unit.o unit_common_info.o unit_hiring.o unit_table.o

# city
OBJ_CITY = city.o

# technology
OBJ_TECH = technology.o technology_parse_info.o technology_research.o technology_table.o

# player
OBJ_PLAYER = player.o

# parsing
OBJ_PARSE = string_functions.o expat_handlers.o xml.o

# helpful for ncurses and view
OBJ_NCURSES = draw_functions.o view_chooser.o view_textbox.o

# main modules
OBJ_MAIN = world.o view.o control.o message.o main.o

# all objects
OBJ = $(OBJ_COMMON) $(OBJ_GRAPH) $(OBJ_UNIT) $(OBJ_CITY) $(OBJ_TECH) $(OBJ_PLAYER) $(OBJ_PARSE) $(OBJ_NCURSES) $(OBJ_MAIN)

# test mode
ifeq ($(t), 1)
	TEST_MODE = -DTEST_MODE=1
else
	TEST_MODE = -DTEST_MODE=0
endif

# compilation target
ifeq ($(win32), 1)
	CC = i686-w64-mingw32-gcc
	LFLIB = -lexpat -lpdcurses -lm
	BIN = simpleciv.exe
	TARGET = -DWIN32
else
	CC = gcc
	LFLIB = -lexpat -lncurses -lm
	BIN = simpleciv
	TARGET =
endif

# debug flag
DEBUG = -g

# common flags
FLAGS = -std=c99 -Wall -pedantic

# flags for compilation object files
CFLAGS = $(FLAGS) -c $(DEBUG) $(TARGET) $(TEST_MODE)

# flags for linking
LFLAGS = $(FLAGS) $(DEBUG)

# target
simpleciv: $(OBJ)
	$(CC) $(LFLAGS) -o $(BIN) $(OBJ) $(LFLIB)

# common modules
list.o: modules/list/list.c
	$(CC) $(CFLAGS) modules/list/list.c

dyn_array.o: modules/dyn_array/dyn_array.c
	$(CC) $(CFLAGS) modules/dyn_array/dyn_array.c

int_array.o: modules/int_array/int_array.c
	$(CC) $(CFLAGS) modules/int_array/int_array.c

# graph and map
graph.o: modules/graph/graph.c
	$(CC) $(CFLAGS) modules/graph/graph.c

map.o: modules/map/map.c
	$(CC) $(CFLAGS) modules/map/map.c

landscape.o: modules/landscape/landscape.c
	$(CC) $(CFLAGS) modules/landscape/landscape.c

# unit
unit.o: modules/unit/unit.c
	$(CC) $(CFLAGS) modules/unit/unit.c

unit_common_info.o: modules/unit/unit_common_info.c
	$(CC) $(CFLAGS) modules/unit/unit_common_info.c

unit_hiring.o: modules/unit/unit_hiring.c
	$(CC) $(CFLAGS) modules/unit/unit_hiring.c

unit_table.o: modules/unit/unit_table.c
	$(CC) $(CFLAGS) modules/unit/unit_table.c

# city
city.o: modules/city/city.c
	$(CC) $(CFLAGS) modules/city/city.c

# technology
technology.o: modules/technology/technology.c
	$(CC) $(CFLAGS) modules/technology/technology.c

technology_parse_info.o: modules/technology/technology_parse_info.c
	$(CC) $(CFLAGS) modules/technology/technology_parse_info.c

technology_research.o: modules/technology/technology_research.c
	$(CC) $(CFLAGS) modules/technology/technology_research.c

technology_table.o: modules/technology/technology_table.c
	$(CC) $(CFLAGS) modules/technology/technology_table.c

# player
player.o: modules/player/player.c
	$(CC) $(CFLAGS) modules/player/player.c

# parsing
string_functions.o: modules/parser/string_functions.c
	$(CC) $(CFLAGS) modules/parser/string_functions.c

expat_handlers.o: modules/parser/expat_handlers.c
	$(CC) $(CFLAGS) modules/parser/expat_handlers.c

xml.o: modules/parser/xml.c
	$(CC) $(CFLAGS) modules/parser/xml.c

# helpful for ncurses and view
draw_functions.o: game/view/draw_functions.c
	$(CC) $(CFLAGS) game/view/draw_functions.c

view_chooser.o: game/view/view_chooser.c
	$(CC) $(CFLAGS) game/view/view_chooser.c

view_textbox.o: game/view/view_textbox.c
	$(CC) $(CFLAGS) game/view/view_textbox.c

# main
world.o: game/world/world.c
	$(CC) $(CFLAGS) game/world/world.c

view.o: game/view/view.c
	$(CC) $(CFLAGS) game/view/view.c

control.o: game/control/control.c
	$(CC) $(CFLAGS) game/control/control.c

message.o: game/message/message.c
	$(CC) $(CFLAGS) game/message/message.c

main.o: main.c
	$(CC) $(CFLAGS) main.c

# clean
clean:
	rm -f *.o simpleciv simpleciv.exe tests/*/*.o tests/*/test
