#ifndef CANVAS_H
#define CANVAS_H
typedef struct {
    char** grid;
    int width;
    int height;
    char bg_char;
} canvas_t;
canvas_t* create_canvas(int width, int height, char bg_char);
void free_canvas(canvas_t* canvas);
void clear_canvas(canvas_t* canvas);
void draw_pixel(canvas_t* canvas, int x, int y, char ch);
void print_canvas(const canvas_t* canvas);
#endif 
