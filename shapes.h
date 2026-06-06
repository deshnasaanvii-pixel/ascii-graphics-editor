#ifndef SHAPES_H
#define SHAPES_H

#include "canvas.h"

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point p1;
    Point p2;
} Line;

typedef struct {
    Point top_left;
    int width;
    int height;
    int filled;
} Rectangle;

typedef struct {
    Point center;
    int radius;
    int filled;
} Circle;

typedef struct {
    Point p1;
    Point p2;
    Point p3;
} Triangle;

typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

void draw_line(canvas_t* canvas, Point p1, Point p2, char ch);
void draw_rectangle(canvas_t* canvas, Point top_left, int width, int height, int filled, char ch);
void draw_circle(canvas_t* canvas, Point center, int radius, int filled, char ch);
void draw_triangle(canvas_t* canvas, Point p1, Point p2, Point p3, char ch);

#endif // SHAPES_H
