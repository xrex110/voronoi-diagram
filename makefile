all:
	g++ src/main.cpp src/cell.cpp -Iinclude/ -lmingw32 -lSDL2main -lSDL2 -o voronoi.exe