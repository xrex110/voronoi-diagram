#ifndef CELL_H
#define CELL_H

#include <vector>
#include <iostream>

class Point {
    public:
    double x, y;
    Point();
    Point(double x, double y);
    bool operator==(Point pt);
    bool operator<(Point pt);
    friend std::ostream& operator<<(std::ostream& os, Point p);
    double distance(Point b);
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
    Line perpendicularBisector();
    bool operator==(LineSegment e);
    friend std::ostream& operator<<(std::ostream& os, LineSegment p);
};

class Circle {
    public:
    Point center;
    double radius;
    Circle(Point cent, double rad);
    bool isPointInside(Point a);
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
