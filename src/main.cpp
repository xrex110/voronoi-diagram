#include <cstdio>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <vector>
#include <set>
#include <unordered_set>
#include <limits>
#include <ctime>
#include <chrono>
#include <SDL2/SDL.h>

#include "main.hpp"

SDL_Window* window;
SDL_Renderer* renderer;

//Look up remove/erase idiom for explanation for this macro
#define REMOVE_ELEM_FROM_VECTOR(vec, elem) vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end())

//main must have this signature for SDL2.0 to work properly
int main(int arc, char* argv[]) {
    //rigorDelauney(512, 512, 128, 2500, true);

    std::vector<Point> sites = randomPoints(512, 512, 256);
    std::cout << "SITES:\n"; 
    //printVector(sites);

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<Triangle> triangles = delauney(sites);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Delauney took " << duration.count() << "ms to run\n";
    
    std::cout << "Verify delauney:\n";
    std::cout << verifyDelauney(sites, triangles) << "\n";

    start = std::chrono::high_resolution_clock::now();
    std::vector<Cell> voronoi = delauneyToVoronoi(sites, triangles);
    end = std::chrono::high_resolution_clock::now();

    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Delauney to Voronoi took " << duration.count() << "ms to run\n";

    createWindow(512, 512, sites, triangles, voronoi);
    presentWindow();

    return 0;
}

template <class T>
void printVector(std::vector<T> &vec) {
    for(auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << "\n";
    }
}

//Rigor tests delauney triangulation
bool rigorDelauney(int rangeX, int rangeY, int numPoints, int numRuns, bool verbose) {
    std::cout << "Begin rigor testing Delauney\n";
    int failedRuns = 0;
    for(int i = 0; i < numRuns; i++) {
        std::vector<Point> sites = randomPoints(rangeX, rangeY, numPoints);
        std::vector<Triangle> triangles = delauney(sites);
        if(!verifyDelauney(sites, triangles)) {
            std::cout << "Run " << i << "failed! Points that failed:" << std::endl;
            for(Point pt : sites) {
                std::cout << "\t" << pt << std::endl;
            }
            failedRuns++;
        }
        else {
            if(verbose) {
                std::cout << "Run " << i << " passed\n";
            }
        }
    }
    if(!failedRuns) {
        std::cout << "Rigor testing Delauney: ALL SUCCESS\n";
        return true;
    }
    else {
        std::cout << "Rigor testing Delauney: " << failedRuns << " runs failed!\n";
        return false;
    }
}

bool verifyDelauney(std::vector<Point> sites, std::vector<Triangle> triangles) {
    bool soon = true;
    for(Point pt : sites) {
        for(Triangle tri : triangles) {
            Circle c = tri.circumcircle();
            if(c.isPointInside2(pt)) {
                std::cout << "\tViolating " << tri << "\n"; 
                soon = false;
            }
        }
    }
    return soon;
}

template <typename T>
bool vectorSetInsert(std::vector<T>& vec, T elem) {
    for(T i : vec) {
        if(i == elem) return false;
    }
    vec.push_back(elem);
    return true;
}

std::vector<Cell> delauneyToVoronoi(std::vector<Point> sites, std::vector<Triangle> triangles) {

    Line boundingLines[] = {
        Line(0, 0, false, 0), //Top line,
        Line(0, 0, true, 0), //left line
        Line(0, 0, true, 512), //right line
        Line(0, 512, false, 0), //bottom line
    };

    std::vector<Cell> voronoiCells;
    for(Point p : sites) {
        voronoiCells.push_back(Cell(p));
    }

    for(Triangle tri : triangles) {
        std::vector<LineSegment> unsharedEdges = {tri.sideA, tri.sideB, tri.sideC};
        for(Triangle bri : triangles) {
            if(tri == bri) continue;
            LineSegment shared(Point(0, 0), Point(0, 0)); //= new LineSegment(Point(0, 0), Point(0,0));
            if(tri.sharedEdge(bri, shared)) {
                //if tri shares this an edge with bri
                LineSegment shared_obj = shared;
                LineSegment vEdge(tri.circumcircle().center, bri.circumcircle().center);
                for(auto it = voronoiCells.begin(); it != voronoiCells.end(); ++it) {
                    if(it->site == shared_obj.a || it->site == shared_obj.b) {
                        it->addEdge(vEdge);
                    }
                }
                REMOVE_ELEM_FROM_VECTOR(unsharedEdges, shared);
            }
        }

        for(LineSegment uEdge : unsharedEdges) {
            LineSegment minBoundingEdge(tri.circumcircle().center, uEdge.perpendicularBisector().intersection(boundingLines[0]));
            for(int i = 1; i < 4; i++) {
                Point intersection = uEdge.perpendicularBisector().intersection(boundingLines[i]);
                LineSegment circumToIntersection(tri.circumcircle().center, intersection);
                if(circumToIntersection.lengthSqr() < minBoundingEdge.lengthSqr()) minBoundingEdge = circumToIntersection;
            }

            for(auto it = voronoiCells.begin(); it != voronoiCells.end(); ++it) {
                if(it->site == uEdge.a || it->site == uEdge.b) {
                    it->addEdge(minBoundingEdge);
                }
            }
        }
    }

    return voronoiCells;
}

//Algorithm description taken from http://paulbourke.net/papers/triangulate/
//The paper has an AMAZING explanation of how this algorithm works
std::vector<Triangle> delauney(std::vector<Point> sites) {
    //printf("\nEntered delauney triangulation\n");

    //Init triangle list, which will be final output
    std::vector<Triangle> triangle_list;

    //Sort all points by x coordinate
    //This improves the runtime from O(n^2) to O(n^1.5) for reasons yet to be understood
    std::sort(sites.begin(), sites.end());   //Uses operator< for comparision, implemented for Point

    double minX = sites.at(0).x;
    double maxX = sites.at(sites.size() - 1).x;
    double minY = sites.at(0).y;
    double maxY = minY;
    for(auto it = sites.begin(); it != sites.end(); ++it) {
        if(it->y < minY) minY = it->y;
        if(it->y > maxY) maxY = it->y;
    }

    double diffX = maxX - minX;
    double diffY = maxY - minY;

    //These three are the vertices of the supertriangle
    Point pointA((int) diffX / 2, -2 * diffY);
    Point pointB(-2 * diffX, diffY);
    Point pointC(2 * diffX, 2 * diffY);

    //Add the vertexes of super triangle to the sites list
    sites.push_back(pointA);
    sites.push_back(pointB);
    sites.push_back(pointC);

    //Add supertriangle to triangle list
    Triangle super_triangle(pointA, pointB, pointC);
    triangle_list.push_back(super_triangle);

    Triangle debug_tri(Point(341, 213), Point(214, 5), Point(224, 305));
    LineSegment cursedLine(Point(214, 5), Point(224, 305));

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
            LineSegment e = *it;
            auto new_end = std::remove(it + 1, end, e);
            if(new_end != end) {
                end = std::remove(edges.begin(), new_end, e);
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

    //std::cout << "Delauney triangulation finished" << std::endl;
    //remove supertriangle vertices from sites list

    REMOVE_ELEM_FROM_VECTOR(sites, pointA);
    REMOVE_ELEM_FROM_VECTOR(sites, pointB);
    REMOVE_ELEM_FROM_VECTOR(sites, pointC);

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
    auto seed = std::time(NULL);

    std::cout << "Generating " << num_points << " points in (" << width << ", " << height << ") with seed = " << seed << "\n";

    std::srand(seed);    //Current systime as seed
    for(int i = 0; i < num_points; i++) {
        int x_pt = (std::rand() % width) + 1;
        int y_pt = (std::rand() % height) + 1;
        Point new_point((double) x_pt, (double) y_pt);
        points.push_back(new_point);
    }
    return points;
}

void createWindow(int width, int height, std::vector<Point> sites, std::vector<Triangle> triangles,
                    std::vector<Cell> voronoi) {
    window = NULL;
    renderer = NULL;

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
    /*for(Triangle tri : triangles) {
        Circle circum = tri.circumcircle();
        //Below two lines draw all the circumcircles as well
        //SDL_SetRenderDrawColor(renderer, 0x0, 0xff, 0x0, SDL_ALPHA_OPAQUE);
        //DrawCircle(renderer, (int) circum.center.x, (int) circum.center.y, (int) circum.radius);
        SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0xff, SDL_ALPHA_OPAQUE);
        DrawTriangle(renderer, tri);
    }*/

    for(Cell c : voronoi) {
        SDL_SetRenderDrawColor(renderer, 0x0, 0xff, 0x0, SDL_ALPHA_OPAQUE);
        DrawCell(renderer, c);
    }

    SDL_SetRenderDrawColor(renderer, 0xff, 0x0, 0x0, SDL_ALPHA_OPAQUE);
    for(Point pt : sites) {
        SDL_RenderDrawPoint(renderer, pt.x, pt.y);
    }
}

void presentWindow() {
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

void DrawCell(SDL_Renderer* renderer, Cell cell) {
    for(LineSegment ln : cell.edges) {
        SDL_RenderDrawLineF(renderer, ln.a.x, ln.a.y, ln.b.x, ln.b.y);
    }
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