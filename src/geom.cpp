#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <vector>

#include <SDL2/SDL.h>

#include "geom.hpp"

bool compareDoubles(double x, double y) {
    return abs(x - y) <= EPSILON;
}

//Point class
Point::Point() : x(0), y(0) {}

Point::Point(double a, double b) : x(a), y(b) {}

Point::Point(const Point& a) : x(a.x), y(a.y) {}

bool Point::operator==(Point pt) {
    return (compareDoubles(x, pt.x) && compareDoubles(y, pt.y));
}

bool Point::operator<(Point pt) {
    return x < pt.x;
}

double Point::distance(Point b) {
    return sqrt(pow(x - b.x, 2) + pow(y - b.y, 2));
}

double Point::distanceSqr(Point b) {
    return pow(x - b.x, 2) + pow(y - b.y, 2);
}

std::ostream& operator<<(std::ostream& os, Point p) {
    os << "Point = (" << p.x << ", " << p.y << ")"; 
    return os;
}

//Line class
Line::Line(Point a, Point b) {
    //Equation of a line is y = mx + c
    //=> c = y - mx
    //Slope = y1 - y2 / x1 - x2
    if(compareDoubles(a.x, b.x)) {
        is_vertical = true;
        vert_x = a.x;
        this->slope = 0;
        this->y_intercept = 0;
    }
    else {
        this->is_vertical = false;
        this->vert_x = 0;
        this->slope = (double) ((a.y - b.y)) / ((a.x - b.x));
        this->y_intercept = a.y - (this->slope * a.x);
    }
}

Line::Line(double slope, double y_intercept, bool is_vert, double vert_x) {
    if(is_vert) {
        this->is_vertical = true;
        this->vert_x = vert_x;
        this->slope = 0;
        this->y_intercept = 0;
    }
    else {
        this->is_vertical = false;
        this->vert_x = 0;
        this->slope = slope;
        this->y_intercept = y_intercept;
    }
}

bool Line::operator==(Line l) {
    if(is_vertical && l.is_vertical) {  //both are vertical
        return compareDoubles(vert_x, l.vert_x);
    }
    else if(!is_vertical && !l.is_vertical) {   //both are not vertical
        return (compareDoubles(slope, l.slope) && compareDoubles(y_intercept, l.y_intercept));
    }
    else {  //One is vertical and the other isn't, it's not possible for them to be equal
        return false;
    }
}

bool Line::doesPointSatisfy(Point a) {
    if(is_vertical) {
        if(compareDoubles(a.x, vert_x)) return true;
        else return false;
    }
    else {
        return (compareDoubles(a.y, (a.x * slope + y_intercept)));
    }
}

//Case for parallelism
Point Line::intersection(Line l) {
    double intersection_x, intersection_y;
    if(this->is_vertical && l.is_vertical) {
        printf("Whack\n");
        return Point(0, 0);
    }
    else if(this->is_vertical) {
        intersection_x = this->vert_x;
        intersection_y = (l.slope * intersection_x) + l.y_intercept;
    }
    else if(l.is_vertical) {
        intersection_x = l.vert_x;
        intersection_y = (this->slope * intersection_x) + this->y_intercept;
    }
    else {
        double a = this->slope;
        double b = l.slope;
        double c = this->y_intercept;
        double d = l.y_intercept;

        intersection_x = (d - c) / (a - b);
        intersection_y = (a * (intersection_x)) + c;
    }
    return Point((double) intersection_x, (double) intersection_y);
}

std::ostream& operator<<(std::ostream& os, Line p) {
    if(p.is_vertical) {
        os << "Line equation: vertical at x = " << p.vert_x;
    }
    else {
        os << "Line equation: y = " << p.slope << "x + " << p.y_intercept << ", vertical: " << p.is_vertical << ", vert_x: " << p.vert_x;
    }
    return os;
}

//LineSegment class
LineSegment::LineSegment(Point x, Point y) : a(x), b(y), line(Line(a, b)) {}

LineSegment::LineSegment(double x1, double y1, double x2, double y2) : LineSegment(Point(x1, y1), Point(x2, y2)) {}

double LineSegment::lengthSqr() {
    return a.distanceSqr(b);
}

Line LineSegment::perpendicularBisector() {
    Point mid((a.x + b.x)/2, (a.y + b.y)/2);
    double p_slope, p_y_intercept;
    double p_vert_x;
    bool p_is_vertical;

    if(this->line.is_vertical) {
        p_slope = 0;
        p_y_intercept = mid.y;
        p_is_vertical = false;
        p_vert_x = 0;
    }
    else if(line.slope == 0.0) {
        p_is_vertical = true;
        p_vert_x = mid.x;
        p_slope = 0;
        p_y_intercept = 0;
    }
    else {
        p_is_vertical = false;
        p_vert_x = 0;
        p_slope = -(1/this->line.slope);
        p_y_intercept = mid.y - (p_slope * mid.x);
    }
    return Line(p_slope, p_y_intercept, p_is_vertical, p_vert_x);
}

bool LineSegment::operator==(LineSegment e) {
    return ((this->a == e.a && this->b == e.b) || (this->b == e.a && this->a == e.b));
}

std::ostream& operator<<(std::ostream& os, LineSegment p) {
    os << "Line segment from " << p.a << " to " << p.b;
    return os;
}

//Circle
Circle::Circle(Point cent, double rad) : center(cent), radius(rad) {}

bool Circle::isPointInside(Point a) {
    double dist = pow(center.x - a.x, 2) + pow(center.y - a.y, 2);
    return (compareDoubles(dist, pow(radius, 2)) || dist < pow(radius, 2));
}

bool Circle::isPointInside2(Point a) {
    double dist = pow(center.x - a.x, 2) + pow(center.y - a.y, 2);
    double rad2 = pow(radius, 2);
    return (dist < rad2  && !compareDoubles(dist, rad2));
}

std::ostream& operator<<(std::ostream& os, Circle p) {
    os << "Circle radius: " << p.radius << ", center at " << p.center;
    return os;
}

//Triangle
Triangle::Triangle(Point x, Point y, Point z) : a(x), b(y), c(z), sideA(a, b), sideB(b, c), sideC(c, a) {}

bool Triangle::operator==(Triangle t) {
    bool ret = (a == t.a && b == t.b && c == t.c) ||
                (a == t.b && b == t.c && c == t.a) ||
                (a == t.c && b == t.a && c == t.b) ||
                (a == t.b && b == t.a && c == t.c) ||
                (a == t.a && b == t.c && c == t.b) ||
                (a == t.c && b == t.b && c == t.a);
    return ret;
}

bool Triangle::sharedEdge(Triangle t, LineSegment* shared) {
    if (sideA == t.sideA || sideA == t.sideB || sideA == t.sideC) {
        *shared = sideA;
    }
    else if(sideB == t.sideA || sideB == t.sideB || sideB == t.sideC) {
        *shared = sideB;
    }
    else if(sideC == t.sideA || sideC == t.sideB || sideC == t.sideC) {
        *shared = sideC;
    }
    else {
        return false;
    }
    return true;
}

Circle Triangle::circumcircle() {
    //Intersection of the perpendicular bisectors of any
    //two edges of a triangle is the center of the circumcircle of a 
    //triangle. The radius can then be found by calculating the distance
    //between the circumcenter and any vertex of the triangle

    Line sa = sideA.perpendicularBisector();
    Line sb = sideB.perpendicularBisector();

    //If either sa's or sb's perp-bisec is vertical,
    //we simply switch that one over to using sideC's perp-bisec
    //to avoid the problem entirely
    bool flag = false;
    if(sa.is_vertical) {
        flag = true;
        sa = sideC.perpendicularBisector();
    }
    else if(sb.is_vertical) {
        sb = sideC.perpendicularBisector();
    }

    Point circumcenter = sa.intersection(sb);
    double rad = circumcenter.distance(a);

    return Circle(circumcenter, (double) rad);
}

//True if even any one point of this triangle matches p
bool Triangle::pointMatch(Point p) {
    return (a == p || b == p || c == p);
}

std::ostream& operator<<(std::ostream& os, Triangle p) {
    os << "Triangle points: " << p.a << ", " << p.b << ", " << p.c;
    return os;
}

//Cell class
Cell::Cell(double x, double y) {
    Point pt(x, y);
    this->site = pt;
}

Cell::Cell(Point pt) {
    this->site = pt;
}

void Cell::addEdge(LineSegment edge) {
    for(LineSegment ln : edges) {
        if(edge == ln) return;  //Don't add if it already exists
    }
    edges.push_back(edge);
}

void Cell::addEdge(Point a, Point b) {
    LineSegment e(a, b);
    addEdge(e);
}

void Cell::addEdge(double x1, double y1, double x2, double y2) {
    LineSegment e(x1, y1, x2, y2);
    addEdge(e);
}
