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
#include <SDL2/SDL.h>

#include "main.hpp"

SDL_Window* window;
SDL_Renderer* renderer;

//Look up remove/erase idiom for explanation for this macro
#define REMOVE_ELEM_FROM_VECTOR(vec, elem) vec.erase(std::remove(vec.begin(), vec.end(), elem), vec.end())

//main must have this signature for SDL2.0 to work properly
int main(int arc, char* argv[]) {
    //rigorDelauney(512, 512, 128, 2500, true);

    std::vector<Point> sites = randomPoints(512, 512, 128);
    /*std::vector<Point> sites;
    sites.push_back(Point(263, 415));
    sites.push_back(Point(218, 340));
    sites.push_back(Point(471, 296));
    sites.push_back(Point(154, 66));
    sites.push_back(Point(191, 155));
    sites.push_back(Point(510, 35));
    sites.push_back(Point(312, 34));
    sites.push_back(Point(212, 156));
    sites.push_back(Point(286, 437));
    sites.push_back(Point(138, 369));*/
    std::cout << "SITES:\n"; 
    //printVector(sites);

    std::vector<Triangle> triangles = delauney(sites);
    
    std::cout << "Verify delauney:\n";
    std::cout << verifyDelauney(sites, triangles) << "\n";

    std::vector<Cell> voronoi = delauneyToVoronoi(sites, triangles);
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
    std::cout << "Start converting Delauney triangulation to Voronoi Diagram\n";

    std::vector<Cell> voronoiCells;
    for(Point p : sites) {
        voronoiCells.push_back(Cell(p));
    }

    Line bound1(0, 0, false, 0); //Top line
    Line bound2(0, 0, true, 0); //left line
    Line bound3(0, 0, true, 512); //right line
    Line bound4(0, 512, false, 0); //bottom line

    for(Triangle tri : triangles) {
        std::vector<LineSegment> edgesShared;
        for(Triangle bri : triangles) {
            if(tri == bri) continue;
            LineSegment* shared = new LineSegment(Point(0, 0), Point(0,0));
            if(tri.sharedEdge(bri, shared)) {
                LineSegment shared_obj = *shared;
                LineSegment vEdge(tri.circumcircle().center, bri.circumcircle().center);
                for(auto it = voronoiCells.begin(); it != voronoiCells.end(); ++it) {
                    if(it->site == shared_obj.a || it->site == shared_obj.b) {
                        //std::cout << "Added edge " << vEdge << "\n";
                        it->addEdge(vEdge);
                    }
                }
                edgesShared.push_back(*shared);
            }
            delete shared;
        }
        //std::cout << "Triangle " << tri << " has " << edgesShared.size() << " shared edges\n";
        //edgesShared holds all the shared edges
        std::vector<LineSegment> unsharedEdges;
        if(std::find(edgesShared.begin(), edgesShared.end(), tri.sideA) == edgesShared.end()) unsharedEdges.push_back(tri.sideA);
        if(std::find(edgesShared.begin(), edgesShared.end(), tri.sideB) == edgesShared.end()) unsharedEdges.push_back(tri.sideB);
        if(std::find(edgesShared.begin(), edgesShared.end(), tri.sideC) == edgesShared.end()) unsharedEdges.push_back(tri.sideC);
        //std::cout << "Triangle " << tri << " has " << unsharedEdges.size() << " UNshared edges\n";

        for(LineSegment uEdge : unsharedEdges) {
            Point inter1 = uEdge.perpendicularBisector().intersection(bound1);
            Point inter2 = uEdge.perpendicularBisector().intersection(bound2);
            Point inter3 = uEdge.perpendicularBisector().intersection(bound3);
            Point inter4 = uEdge.perpendicularBisector().intersection(bound4);
            
            LineSegment minDist(tri.circumcircle().center, inter1);
            LineSegment dist2(tri.circumcircle().center, inter2);
            LineSegment dist3(tri.circumcircle().center, inter3);
            LineSegment dist4(tri.circumcircle().center, inter4);
            if(dist2.lengthSqr() < minDist.lengthSqr()) minDist = dist2;
            if(dist3.lengthSqr() < minDist.lengthSqr()) minDist = dist3;
            if(dist4.lengthSqr() < minDist.lengthSqr()) minDist = dist4;

            for(auto it = voronoiCells.begin(); it != voronoiCells.end(); ++it) {
                if(it->site == uEdge.a || it->site == uEdge.b) {
                    //std::cout << "Added edge " << minDist << "\n";
                    it->addEdge(minDist);
                }
            }
        }
    }

    return voronoiCells;
}

/*std::vector<Cell> delauneyToVoronoi(std::vector<Point> sites, std::vector<Triangle> triangles) {
    std::cout << "Start converting Delauney triangulation to Voronoi Diagram\n";

    std::vector<Cell> voronoiCells;

    //define four lines which form the bounds of the canvas
    Line bound1(0, 0, false, 0); //Top line
    Line bound2(0, 0, true, 0); //left line
    Line bound3(0, 0, true, 512); //right line
    Line bound4(0, 512, false, 0); //bottom line

    for(Point p : sites) {
        std::cout << "OPERATING ON " << p << "\n";
        int triangle_count = 0;
        std::vector<Point> vertices;
        std::vector<Line> lines;
        for(Triangle tri : triangles) {
            if(tri.pointMatch(p)) {
                //Intersection of perpendicular bisector of any two sides 
                //of this triangle should form ONE point of the voronoi cell about 
                //this point. So the center of the triangle's circumcircle will work
                std::cout << "\t Triangle: " << tri << "\n";

                Circle c = tri.circumcircle();
                vertices.push_back(c.center);
                Point midA((tri.sideA.a.x + tri.sideA.b.x)/2, (tri.sideA.a.y + tri.sideA.b.y)/2);
                Point midB((tri.sideB.a.x + tri.sideB.b.x)/2, (tri.sideB.a.y + tri.sideB.b.y)/2);
                Point midC((tri.sideC.a.x + tri.sideC.b.x)/2, (tri.sideC.a.y + tri.sideC.b.y)/2);

                //vertices.push_back(midA);
                //vertices.push_back(midB);
                //vertices.push_back(midC);

                vectorSetInsert(lines, tri.sideA.perpendicularBisector());
                vectorSetInsert(lines, tri.sideB.perpendicularBisector());
                vectorSetInsert(lines, tri.sideC.perpendicularBisector());
            }
        }

        std::vector<LineSegment> edges;
        //Find the two closest points to each point, and form edges to them
        for(Line line : lines) {
            std::cout << "\tEvaluating line: " << line << "\n";
            int num_points = 0;
            Point pts[2];
            for(Point vert : vertices) {
                if(line.doesPointSatisfy(vert)) {
                    //vert lines on line somewhere
                    pts[num_points++] = vert;
                    if(num_points >= 2) break;
                }
            }
            if(num_points == 2) {
                edges.push_back(LineSegment(pts[0], pts[1]));
            }
            else if(num_points == 1) {
                std::cout << "\t\tThis is a corner edge!\n";
            }
            else {
                std::cout << "\t\tWHACK? num_points: " << num_points << "\n";
            }
        }

        Cell newCell(p);
        for(LineSegment ls : edges) {
            newCell.addEdge(ls);
        }
        voronoiCells.push_back(newCell);
    }
    return voronoiCells;
}*/

/*std::vector<Cell> delauneyToVoronoi(std::vector<Point> sites, std::vector<Triangle> triangles) {
    std::cout << "Start converting Delauney triangulation to Voronoi Diagram\n";

    std::vector<Cell> voronoiCells;

    //define four lines which form the bounds of the canvas
    Line bound1(0, 0, false, 0); //Top line
    Line bound2(0, 0, true, 0); //left line
    Line bound3(0, 0, true, 512); //right line
    Line bound4(0, 512, false, 0); //bottom line

    for(Point p : sites) {
        std::cout << "OPERATING ON " << p << "\n";
        int triangle_count = 0;
        std::vector<Point> vertices;
        std::vector<Line> lines;
        for(Triangle tri : triangles) {
            if(tri.pointMatch(p)) {
                //Intersection of perpendicular bisector of any two sides 
                //of this triangle should form ONE point of the voronoi cell about 
                //this point. So the center of the triangle's circumcircle will work
                Circle c = tri.circumcircle();
                vertices.push_back(c.center);
                vectorSetInsert(lines, tri.sideA.perpendicularBisector());
                vectorSetInsert(lines, tri.sideB.perpendicularBisector());
                vectorSetInsert(lines, tri.sideC.perpendicularBisector());
            }
        }
        std::vector<LineSegment> edges;
        //Find the two closest points to each point, and form edges to them

        for(auto it = vertices.begin(); it != vertices.end() - 1; ++it) {
            for(auto kt = it + 1; kt != vertices.end(); kt++) {
                LineSegment this_line(*it, *kt); 
                for(Line ln : lines) {
                    if(this_line.line == ln) {
                        edges.push_back(this_line);;
                        break;
                    }
                }
            }
        }

        Cell newCell(p);
        for(LineSegment ls : edges) {
            newCell.addEdge(ls);
        }
        voronoiCells.push_back(newCell);
    }
    return voronoiCells;
}*/

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
    std::srand(std::time(NULL));    //Current systime as seed
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