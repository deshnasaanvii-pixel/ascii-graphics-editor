#include "shapes.h"
#include <stdlib.h>
#include <math.h>

static void draw_circle_points(canvas_t* canvas, Point center, int x, int y, int filled, char ch) {
    if (filled) {
        for (int i = center.x - x; i <= center.x + x; i++) {
            draw_pixel(canvas, i, center.y + y, ch);
            draw_pixel(canvas, i, center.y - y, ch);
        }
        for (int i = center.x - y; i <= center.x + y; i++) {
            draw_pixel(canvas, i, center.y + x, ch);
            draw_pixel(canvas, i, center.y - x, ch);
        }
    } else {
        draw_pixel(canvas, center.x + x, center.y + y, ch);
        draw_pixel(canvas, center.x - x, center.y + y, ch);
        draw_pixel(canvas, center.x + x, center.y - y, ch);
        draw_pixel(canvas, center.x - x, center.y - y, ch);
        draw_pixel(canvas, center.x + y, center.y + x, ch);
        draw_pixel(canvas, center.x - y, center.y + x, ch);
        draw_pixel(canvas, center.x + y, center.y - x, ch);
        draw_pixel(canvas, center.x - y, center.y - x, ch);
    }
}

void draw_line(canvas_t* canvas, Point p1, Point p2, char ch) {
    if (!canvas) return;

    int x1 = p1.x;
    int y1 = p1.y;
    int x2 = p2.x;
    int y2 = p2.y;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_pixel(canvas, x1, y1, ch);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void draw_rectangle(canvas_t* canvas, Point top_left, int width, int height, int filled, char ch) {
    if (!canvas || width <= 0 || height <= 0) return;

    int x_end = top_left.x + width - 1;
    int y_end = top_left.y + height - 1;

    if (filled) {
        for (int y = top_left.y; y <= y_end; y++) {
            for (int x = top_left.x; x <= x_end; x++) {
                draw_pixel(canvas, x, y, ch);
            }
        }
    } else {
        for (int x = top_left.x; x <= x_end; x++) {
            draw_pixel(canvas, x, top_left.y, ch);
            draw_pixel(canvas, x, y_end, ch);
        }
        for (int y = top_left.y; y <= y_end; y++) {
            draw_pixel(canvas, top_left.x, y, ch);
            draw_pixel(canvas, x_end, y, ch);
        }
    }
}

void draw_circle(canvas_t* canvas, Point center, int radius, int filled, char ch) {
    if (!canvas || radius < 0) return;

    if (radius == 0) {
        draw_pixel(canvas, center.x, center.y, ch);
        return;
    }

    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;

    draw_circle_points(canvas, center, x, y, filled, ch);

    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        draw_circle_points(canvas, center, x, y, filled, ch);
    }
}

void draw_triangle(canvas_t* canvas, Point p1, Point p2, Point p3, char ch) {
    if (!canvas) return;

    draw_line(canvas, p1, p2, ch);
    draw_line(canvas, p2, p3, ch);
    draw_line(canvas, p3, p1, ch);
}
