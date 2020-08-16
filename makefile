all:
	g++ src/main.cpp src/geom.cpp -Iinclude/ -lmingw32 -lSDL2main -lSDL2 -o voronoi.exe