#ifndef GEOM_H
#define GEOM_H

#include <vector>
#include <iostream>

#define EPSILON  0.00001

bool compareDoubles(double x, double y);    //returns true if x == y

class Point {
    public:
    double x, y;
    Point();
    Point(const Point& a);
    Point(double x, double y);
    bool operator==(Point pt);
    bool operator<(Point pt);
    friend std::ostream& operator<<(std::ostream& os, Point p);
    double distance(Point b);
    double distanceSqr(Point b);
};

class Line {
    public:
    double slope;
    double y_intercept;
    //Can be union???
    bool is_vertical;
    double vert_x;
    Line(Point a, Point b);
    Line(double slope, double y_intercept, bool is_vert, double vert_x);
    bool doesPointSatisfy(Point a);
    bool operator==(Line l);
    friend std::ostream& operator<<(std::ostream& os, Line p);
    Point intersection(Line l);
};

//Rename to Line Segment
class LineSegment {
    public:
    Point a, b;    
    Line line;
    LineSegment(Point a, Point b);
    LineSegment(double x1, double y1, double x2, double y2);
    double lengthSqr();
    Line perpendicularBisector();
    bool operator==(LineSegment e);
    friend std::ostream& operator<<(std::ostream& os, LineSegment p);
};

class Circle {
    public:
    Point center;
    double radius;
    Circle(Point cent, double rad);
    bool isPointInside(Point a);    //circumference inclusive
    bool isPointInside2(Point a);   //circumference exclusive
    friend std::ostream& operator<<(std::ostream& os, Circle p);
};

class Triangle {
    public:
    Point a, b, c;
    LineSegment sideA, sideB, sideC;
    Triangle(Point one, Point two, Point three);
    bool operator==(Triangle t);
    Circle circumcircle();
    bool pointMatch(Point p);
    bool sharedEdge(Triangle t, LineSegment* shared);
    friend std::ostream& operator<<(std::ostream& os, Triangle p);
};

class Cell {
    public:
    Point site;
    std::vector<LineSegment> edges;

    Cell(double x, double y);
    Cell(Point site);
    void addEdge(LineSegment edge);
    void addEdge(Point a, Point b);
    void addEdge(double x1, double y1, double x2, double y2);        
};

#endif
