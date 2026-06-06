#include "canvas.h"
#include <stdio.h>
#include <stdlib.h>
canvas_t* create_canvas(int width, int height, char bg_char) {
    if (width <= 0 || height <= 0) return NULL;
    canvas_t* canvas = (canvas_t*)malloc(sizeof(canvas_t));
    if (!canvas) return NULL;
    canvas->width = width;
    canvas->height = height;
    canvas->bg_char = bg_char;
    canvas->grid = (char**)malloc(height * sizeof(char*));
    if (!canvas->grid) {
        free(canvas);
        return NULL;
    }
    for (int i = 0; i < height; i++) {
        canvas->grid[i] = (char*)malloc(width * sizeof(char));
        if (!canvas->grid[i]) {
            for (int j = 0; j < i; j++) {
                free(canvas->grid[j]);
            }
            free(canvas->grid);
            free(canvas);
            return NULL;
        }
    }
    clear_canvas(canvas);
    return canvas;
}
void free_canvas(canvas_t* canvas) {
    if (!canvas) return;
    for (int i = 0; i < canvas->height; i++) {
        free(canvas->grid[i]);
    }
    free(canvas->grid);
    free(canvas);
}
void clear_canvas(canvas_t* canvas) {
    if (!canvas) return;
    for (int y = 0; y < canvas->height; y++) {
        for (int x = 0; x < canvas->width; x++) {
            canvas->grid[y][x] = canvas->bg_char;
        }
    }
}
void draw_pixel(canvas_t* canvas, int x, int y, char ch) {
    if (!canvas) return;
    if (x >= 0 && x < canvas->width && y >= 0 && y < canvas->height) {
        canvas->grid[y][x] = ch;
    }
}
void print_canvas(const canvas_t* canvas) {
    if (!canvas) return;
    // Top border
    printf("┌");
    for (int x = 0; x < canvas->width; x++) {
        printf("─");
    }
    printf("┐\n");
    // Grid contents
    for (int y = 0; y < canvas->height; y++) {
        printf("│");
        for (int x = 0; x < canvas->width; x++) {
            printf("%c", canvas->grid[y][x]);
        }
        printf("│\n");
    }
    // Bottom border
    printf("└");
    for (int x = 0; x < canvas->width; x++) {
        printf("─");
    }
    printf("┘\n");
}
