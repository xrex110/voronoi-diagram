#ifndef MAIN_H
#define MAIN_H

#include "geom.hpp"

void createWindow(int width, int height, std::vector<Point> sites, std::vector<Triangle> triangles);
std::vector<Point> randomPoints(int width, int height, int num_points);
void DrawTriangle(SDL_Renderer* renderer, Triangle tri);
void DrawCircle(SDL_Renderer * renderer, int centreX, int centreY, int radius);
std::vector<Triangle> delauney(std::vector<Point> sites);
bool verifyDelauney(std::vector<Point> sites, std::vector<Triangle> triangles);
template <class T> void printVector(std::vector<T> &vec);
bool rigorDelauney(int rangeX, int rangeY, int numPoints, int numRuns, bool verbose);
void presentWindow();

#endif