#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <vector>
#include <set>
#include <ctime>
#include <SDL2/SDL.h>

#include "GEOM.hpp"

//Look up remove/erase idiom for explanation for this macro
#define REMOVE_ELEM_FROM_VECTOR(vec, elem) vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end())

void createWindow(int width, int height, std::vector<Point> sites, std::vector<Triangle> triangles);
std::vector<Point> randomPoints(int width, int height, int num_points);
void DrawTriangle(SDL_Renderer* renderer, Triangle tri);
void DrawCircle(SDL_Renderer * renderer, int centreX, int centreY, int radius);
std::vector<Triangle> delauney(std::vector<Point> sites);

//main must have this signature for SDL2.0 to work properly
int main(int arc, char* argv[]) {
    printf("Hello World!\n");

    std::vector<Point> sites = randomPoints(512, 512, 10);

    std::cout << "Points before delauney:" << std::endl;
    for(Point pt : sites) {
        std::cout << "\t" << pt << std::endl;
    }

    std::vector<Triangle> triangles = delauney(sites);

    //std::vector<LineSegment> edges = voronoi(512, 512, sites);
    //printf("LMAO\n");
    createWindow(512, 512, sites, triangles);

    return 0;
}

//Algorithm description taken from http://paulbourke.net/papers/triangulate/
//The paper has an AMAZING explanation of how this algorithm works
std::vector<Triangle> delauney(std::vector<Point> sites) {
    printf("\nEntered delauney triangulation\n");

    //Init triangle list, which will be final output
    std::vector<Triangle> triangle_list;

    //Sort all points by x coordinate
    //This improves the runtime from O(n^2) to O(n^1.5) for reasons yet to be understood
    std::sort(sites.begin(), sites.end());   //Uses operator< for comparision, implemented for Point

    //Determine supertriangle
    /*double minX = sites.at(0).x;
    double minY = sites.at(0).y;
    double maxX = minX, maxY = minY;
    for(std::vector<Point>::iterator it = sites.begin(); it != sites.end(); it++) {
        if(it->x > maxX) maxX = it->x;
        if(it->x < minX) minX = it->x;
        if(it->y > maxY) maxY = it->y;
        if(it->y < minY) minY = it->y;
    }
    //Now we have the smallest and largest x and y values
    double diffX = maxX - minX;
    double diffY = maxY - minY;
    double diffMax = (diffX > diffY ? diffX : diffY) * 20;
    double midX = (maxX + minX) / 2;
    double midY = (maxY + minY) / 2;*/

    //Now we setup the super triangle so that all points
    //are contained within it
    //Point pointA(midX - diffMax, midY - diffMax);
    //Point pointB(midX, midY + diffMax);
    //Point pointC(midX + diffMax, midY - diffMax);

    Point pointA(256, -2*512);
    Point pointB(-2*512, 512);
    Point pointC(2*512, 2*512);

    //Add the vertexes of super triangle to the sites list
    sites.push_back(pointA);
    sites.push_back(pointB);
    sites.push_back(pointC);

    //Add supertriangle to triangle list
    Triangle super_triangle(pointA, pointB, pointC);
    triangle_list.push_back(super_triangle);

    //For each point in the sites list
    for(Point p : sites) {
        std::vector<LineSegment> edges;
        auto tri_end = triangle_list.end();
        for(auto it = triangle_list.begin(); it != tri_end; ++it) {
            Triangle tri = *it;
            Circle c = tri.circumcircle();
            if(c.isPointInside(p)) {
                edges.push_back(tri.sideA);
                edges.push_back(tri.sideB);
                edges.push_back(tri.sideC);
                tri_end = std::remove(triangle_list.begin(), tri_end, tri);
                it--;
                if(triangle_list.begin() == tri_end) break;
            }
        }
        triangle_list.erase(tri_end, triangle_list.end());

        //Now we check for any edges that occured more than once, and remove
        //ALL occurances of any such edge
        auto end = edges.end();
        for(auto it = edges.begin(); it != end; ++it) {
            auto new_end = std::remove(it + 1, end, *it);
            if(new_end != end) {
                end = std::remove(edges.begin(), new_end, *it);
                it--;
                if(edges.begin() == end) break;
            }
        }
        edges.erase(end, edges.end());

        //Add all triangles formed between point p and the remaining edges in
        //edges vector
        for(LineSegment edge : edges) {
            Triangle new_t(p, edge.a, edge.b);
            triangle_list.push_back(new_t);
        }
    }

    //Before returning, we must remove from the traingle
    // any triangle list that use vertices from the supertriangle
    auto tri_end = triangle_list.end();
    for(auto it = triangle_list.begin(); it != tri_end; ++it) {
        Triangle t = *it;
        if(t.pointMatch(pointA) || t.pointMatch(pointB) || t.pointMatch(pointC)) {
            tri_end = std::remove(triangle_list.begin(), tri_end, t);
            it--;
            if(tri_end == triangle_list.begin()) break;
        }
    }
    triangle_list.erase(tri_end, triangle_list.end());

    std::cout << "Delauney triangulation finished" << std::endl;
    //remove supertriangle vertices from sites list
    return triangle_list;
}

//Deprecate below in favor of macro REMOVE_ELEM_FROM_VECTOR
//removes all objects equal to elem from list if == is overloaded for T
template <typename T>
void removeFromVector(std::vector<T> list, T elem) {
    for(typename std::vector<T>::iterator it = list.begin(); it != list.end(); ) {
        if(*it == elem) {
            it = list.erase(it);
        }
        else {
            ++it;
        }
    }
}

std::vector<Point> randomPoints(int width, int height, int num_points) {
    std::vector<Point> points;
    std::srand(std::time(NULL));    //Current systime as seed
    for(int i = 0; i < num_points; i++) {
        int x_pt = (std::rand() % width) + 1;
        int y_pt = (std::rand() % height) + 1;
        Point new_point((double) x_pt, (double) y_pt);
        points.push_back(new_point);
    }
    return points;
}

void createWindow(int width, int height, std::vector<Point> sites, std::vector<Triangle> triangles) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL broke, idk why!\n");
        return;
    }

    SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, SDL_ALPHA_OPAQUE);

    //NULL outlines entire rendering target
    SDL_RenderFillRect(renderer, NULL);

    //Draw triangles
    for(Triangle tri : triangles) {
        /*Circle circum = tri.circumcircle();
        SDL_SetRenderDrawColor(renderer, 0x0, 0xff, 0x0, SDL_ALPHA_OPAQUE);
        DrawCircle(renderer, (int) circum.center.x, (int) circum.center.y, (int) circum.radius);*/
        SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0xff, SDL_ALPHA_OPAQUE);
        DrawTriangle(renderer, tri);
    }

    SDL_SetRenderDrawColor(renderer, 0xff, 0x0, 0x0, SDL_ALPHA_OPAQUE);
    for(Point pt : sites) {
        SDL_RenderDrawPoint(renderer, pt.x, pt.y);
    }

    SDL_RenderPresent(renderer);
    
    bool quit = false;
    while(!quit) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                quit = true;
            }
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    printf("Program ended\n");
    SDL_Quit();
}

void DrawTriangle(SDL_Renderer* renderer, Triangle tri) {
    SDL_RenderDrawLineF(renderer, tri.a.x, tri.a.y, tri.b.x, tri.b.y);
    SDL_RenderDrawLineF(renderer, tri.b.x, tri.b.y, tri.c.x, tri.c.y);
    SDL_RenderDrawLineF(renderer, tri.c.x, tri.c.y, tri.a.x, tri.a.y);
}

void DrawCircle(SDL_Renderer * renderer, int centreX, int centreY, int radius) {
   const int diameter = (radius * 2);

   int x = (radius - 1);
   int y = 0;
   int tx = 1;
   int ty = 1;
   int error = (tx - diameter);

   while (x >= y) {
      //  Each of the following renders an octant of the circle
      SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
      SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
      SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

      if (error <= 0)
      {
         ++y;
         error += ty;
         ty += 2;
      }

      if (error > 0)
      {
         --x;
         tx += 2;
         error += (tx - diameter);
      }
   }
}