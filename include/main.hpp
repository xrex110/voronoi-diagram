#ifndef MAIN_H
#define MAIN_H

#include "geom.hpp"

void createWindow(int width, int height, std::vector<Point> sites, std::vector<Triangle> triangles, std::vector<Cell> voronoi);
std::vector<Point> randomPoints(int width, int height, int num_points);
void DrawTriangle(SDL_Renderer* renderer, Triangle tri);
void DrawCircle(SDL_Renderer * renderer, int centreX, int centreY, int radius);
void DrawCell(SDL_Renderer* renderer, Cell cell);
std::vector<Triangle> delauney(std::vector<Point> sites);
bool verifyDelauney(std::vector<Point> sites, std::vector<Triangle> triangles);
template <class T> void printVector(std::vector<T> &vec);
template <typename T> bool vectorSetInsert(std::vector<T>& vec, T elem);
bool rigorDelauney(int rangeX, int rangeY, int numPoints, int numRuns, bool verbose);
std::vector<Cell> delauneyToVoronoi(std::vector<Point> sites, std::vector<Triangle> triangles);
void presentWindow();


#endif