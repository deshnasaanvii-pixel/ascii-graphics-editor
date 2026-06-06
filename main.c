#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 80
#define HEIGHT 24
#define MAX_SHAPES 100

// Define shape types
typedef enum {
    SHAPE_LINE = 1,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Structural parameters for shapes
typedef struct {
    int x1, y1, x2, y2;
} LineData;

typedef struct {
    int x, y, w, h;
} RectData;

typedef struct {
    int cx, cy, r;
} CircleData;

typedef struct {
    int x1, y1, x2, y2, x3, y3;
} TriangleData;

typedef struct {
    int id;
    ShapeType type;
    char draw_char;
    union {
        LineData line;
        RectData rect;
        CircleData circle;
        TriangleData triangle;
    } params;
} Shape;

// Global application state
char canvas[HEIGHT][WIDTH];
Shape shapes[MAX_SHAPES];
int shape_count = 0;
int next_id = 1;
char bg_char = '_';

// Clear input buffer to avoid input loops on errors
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Read integer safely with validation
int read_int(const char *prompt, int min_val, int max_val) {
    int val;
    int items;
    while (1) {
        printf("%s", prompt);
        items = scanf("%d", &val);
        if (items == 1 && val >= min_val && val <= max_val) {
            clear_input_buffer();
            return val;
        }
        printf("Invalid input. Please enter an integer between %d and %d.\n", min_val, max_val);
        clear_input_buffer();
    }
}

// Read character safely with a default option
char read_char(const char *prompt, char default_val) {
    char c;
    printf("%s [%c]: ", prompt, default_val);
    c = getchar();
    if (c == '\n' || c == EOF) {
        return default_val;
    }
    clear_input_buffer();
    return c;
}

// Initialize/clear the canvas with the background character
void init_canvas(char canvas[HEIGHT][WIDTH], char bg) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = bg;
        }
    }
}

// Plot a single pixel on the canvas with boundary checks
void plot_pixel(char canvas[HEIGHT][WIDTH], int x, int y, char c) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = c;
    }
}

// Bresenham's Line Algorithm
void draw_line(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, char c) {
    int dx = abs(x2 - x1);
    int dy = -abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx + dy;
    int e2;

    while (1) {
        plot_pixel(canvas, x1, y1, c);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x1 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y1 += sy;
        }
    }
}

// Draw rectangle boundaries using horizontal and vertical segments
void draw_rectangle(char canvas[HEIGHT][WIDTH], int x, int y, int w, int h, char c) {
    if (w <= 0 || h <= 0) return;
    
    // Top and bottom borders
    for (int i = 0; i < w; i++) {
        plot_pixel(canvas, x + i, y, c);
        plot_pixel(canvas, x + i, y + h - 1, c);
    }
    // Left and right borders
    for (int i = 0; i < h; i++) {
        plot_pixel(canvas, x, y + i, c);
        plot_pixel(canvas, x + w - 1, y + i, c);
    }
}

// Midpoint Circle Drawing Algorithm (Bresenham's Circle)
void draw_circle(char canvas[HEIGHT][WIDTH], int cx, int cy, int r, char c) {
    if (r < 0) return;
    if (r == 0) {
        plot_pixel(canvas, cx, cy, c);
        return;
    }

    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    while (y >= x) {
        plot_pixel(canvas, cx + x, cy + y, c);
        plot_pixel(canvas, cx - x, cy + y, c);
        plot_pixel(canvas, cx + x, cy - y, c);
        plot_pixel(canvas, cx - x, cy - y, c);
        plot_pixel(canvas, cx + y, cy + x, c);
        plot_pixel(canvas, cx - y, cy + x, c);
        plot_pixel(canvas, cx + y, cy - x, c);
        plot_pixel(canvas, cx - y, cy - x, c);

        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
    }
}

// Draw triangle boundaries by connecting three points
void draw_triangle(char canvas[HEIGHT][WIDTH], int x1, int y1, int x2, int y2, int x3, int y3, char c) {
    draw_line(canvas, x1, y1, x2, y2, c);
    draw_line(canvas, x2, y2, x3, y3, c);
    draw_line(canvas, x3, y3, x1, y1, c);
}

// Redraw canvas from scratch using current shape list
void render_shapes(char canvas[HEIGHT][WIDTH]) {
    init_canvas(canvas, bg_char);
    for (int i = 0; i < shape_count; i++) {
        Shape *s = &shapes[i];
        switch (s->type) {
            case SHAPE_LINE:
                draw_line(canvas, s->params.line.x1, s->params.line.y1, 
                          s->params.line.x2, s->params.line.y2, s->draw_char);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(canvas, s->params.rect.x, s->params.rect.y, 
                               s->params.rect.w, s->params.rect.h, s->draw_char);
                break;
            case SHAPE_CIRCLE:
                draw_circle(canvas, s->params.circle.cx, s->params.circle.cy, 
                            s->params.circle.r, s->draw_char);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(canvas, s->params.triangle.x1, s->params.triangle.y1, 
                              s->params.triangle.x2, s->params.triangle.y2, 
                              s->params.triangle.x3, s->params.triangle.y3, s->draw_char);
                break;
        }
    }
}

// Display the canvas enclosed in a clean border
void display_canvas(char canvas[HEIGHT][WIDTH]) {
    printf("\n+");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");

    for (int y = 0; y < HEIGHT; y++) {
        printf("|");
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", canvas[y][x]);
        }
        printf("|\n");
    }

    printf("+");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");
}

// List all shapes currently in memory
void list_shapes() {
    if (shape_count == 0) {
        printf("No shapes added to the picture yet.\n");
        return;
    }

    printf("\n=====================================================================\n");
    printf("                         Active Shapes List                          \n");
    printf("=====================================================================\n");
    printf("%-5s | %-10s | %-5s | %-40s\n", "ID", "Type", "Char", "Parameters / Coordinates");
    printf("---------------------------------------------------------------------\n");
    for (int i = 0; i < shape_count; i++) {
        Shape *s = &shapes[i];
        const char *type_name = "Unknown";
        char details[128] = "";

        switch (s->type) {
            case SHAPE_LINE:
                type_name = "Line";
                sprintf(details, "Start: (%d, %d), End: (%d, %d)", 
                        s->params.line.x1, s->params.line.y1, 
                        s->params.line.x2, s->params.line.y2);
                break;
            case SHAPE_RECTANGLE:
                type_name = "Rectangle";
                sprintf(details, "Top-Left: (%d, %d), Size: %dx%d", 
                        s->params.rect.x, s->params.rect.y, 
                        s->params.rect.w, s->params.rect.h);
                break;
            case SHAPE_CIRCLE:
                type_name = "Circle";
                sprintf(details, "Center: (%d, %d), Radius: %d", 
                        s->params.circle.cx, s->params.circle.cy, 
                        s->params.circle.r);
                break;
            case SHAPE_TRIANGLE:
                type_name = "Triangle";
                sprintf(details, "P1:(%d,%d), P2:(%d,%d), P3:(%d,%d)", 
                        s->params.triangle.x1, s->params.triangle.y1, 
                        s->params.triangle.x2, s->params.triangle.y2, 
                        s->params.triangle.x3, s->params.triangle.y3);
                break;
        }

        printf("%-5d | %-10s | %-5c | %-40s\n", s->id, type_name, s->draw_char, details);
    }
    printf("=====================================================================\n\n");
}

// Add a shape to the editor
void menu_add_shape() {
    if (shape_count >= MAX_SHAPES) {
        printf("Error: Maximum shape limit (%d) reached. Delete some shapes first.\n", MAX_SHAPES);
        return;
    }

    printf("\n--- Add a Shape ---\n");
    printf("1. Line\n");
    printf("2. Rectangle\n");
    printf("3. Circle\n");
    printf("4. Triangle\n");
    printf("5. Cancel\n");
    int type_choice = read_int("Enter shape type (1-5): ", 1, 5);

    if (type_choice == 5) {
        printf("Operation cancelled.\n");
        return;
    }

    char c = read_char("Enter character to draw with", '*');

    Shape new_shape;
    new_shape.id = next_id++;
    new_shape.type = (ShapeType)type_choice;
    new_shape.draw_char = c;

    switch (new_shape.type) {
        case SHAPE_LINE:
            printf("Enter Line Coordinates:\n");
            new_shape.params.line.x1 = read_int("  x1 (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.line.y1 = read_int("  y1 (0 to 23): ", 0, HEIGHT - 1);
            new_shape.params.line.x2 = read_int("  x2 (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.line.y2 = read_int("  y2 (0 to 23): ", 0, HEIGHT - 1);
            break;
        case SHAPE_RECTANGLE:
            printf("Enter Rectangle Position & Size:\n");
            new_shape.params.rect.x = read_int("  Top-Left x (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.rect.y = read_int("  Top-Left y (0 to 23): ", 0, HEIGHT - 1);
            new_shape.params.rect.w = read_int("  Width (1 to 80): ", 1, WIDTH);
            new_shape.params.rect.h = read_int("  Height (1 to 24): ", 1, HEIGHT);
            break;
        case SHAPE_CIRCLE:
            printf("Enter Circle Center & Radius:\n");
            new_shape.params.circle.cx = read_int("  Center x (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.circle.cy = read_int("  Center y (0 to 23): ", 0, HEIGHT - 1);
            new_shape.params.circle.r = read_int("  Radius (1 to 40): ", 1, 40);
            break;
        case SHAPE_TRIANGLE:
            printf("Enter Triangle Vertices:\n");
            new_shape.params.triangle.x1 = read_int("  P1 x (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.triangle.y1 = read_int("  P1 y (0 to 23): ", 0, HEIGHT - 1);
            new_shape.params.triangle.x2 = read_int("  P2 x (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.triangle.y2 = read_int("  P2 y (0 to 23): ", 0, HEIGHT - 1);
            new_shape.params.triangle.x3 = read_int("  P3 x (0 to 79): ", 0, WIDTH - 1);
            new_shape.params.triangle.y3 = read_int("  P3 y (0 to 23): ", 0, HEIGHT - 1);
            break;
    }

    shapes[shape_count++] = new_shape;
    printf("Shape successfully added to the picture!\n");

    // Redraw and display canvas
    render_shapes(canvas);
    display_canvas(canvas);
}

// Delete a shape by its ID
void menu_delete_shape() {
    if (shape_count == 0) {
        printf("No shapes available to delete.\n");
        return;
    }

    list_shapes();
    int del_id = read_int("Enter the ID of the shape to delete (0 to cancel): ", 0, next_id - 1);
    if (del_id == 0) {
        printf("Operation cancelled.\n");
        return;
    }

    int found_idx = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == del_id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Shape ID %d not found.\n", del_id);
        return;
    }

    // Shift shapes to the left to delete the item
    for (int i = found_idx; i < shape_count - 1; i++) {
        shapes[i] = shapes[i + 1];
    }
    shape_count--;
    printf("Shape ID %d successfully deleted from the picture.\n", del_id);

    // Redraw and display canvas
    render_shapes(canvas);
    display_canvas(canvas);
}

// Modify a shape in the editor
void menu_modify_shape() {
    if (shape_count == 0) {
        printf("No shapes available to modify.\n");
        return;
    }

    list_shapes();
    int mod_id = read_int("Enter the ID of the shape to modify (0 to cancel): ", 0, next_id - 1);
    if (mod_id == 0) {
        printf("Operation cancelled.\n");
        return;
    }

    int found_idx = -1;
    for (int i = 0; i < shape_count; i++) {
        if (shapes[i].id == mod_id) {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1) {
        printf("Shape ID %d not found.\n", mod_id);
        return;
    }

    Shape *s = &shapes[found_idx];
    printf("\nModifying Shape ID %d (%s):\n", s->id, 
           s->type == SHAPE_LINE ? "Line" :
           s->type == SHAPE_RECTANGLE ? "Rectangle" :
           s->type == SHAPE_CIRCLE ? "Circle" : "Triangle");

    printf("1. Change drawing character (current: '%c')\n", s->draw_char);
    printf("2. Change shape parameters/coordinates\n");
    printf("3. Cancel\n");
    int choice = read_int("Enter choice (1-3): ", 1, 3);

    if (choice == 3) {
        printf("Operation cancelled.\n");
        return;
    }

    if (choice == 1) {
        s->draw_char = read_char("Enter new character to draw with", '*');
    } else if (choice == 2) {
        switch (s->type) {
            case SHAPE_LINE:
                printf("Enter New Line Coordinates:\n");
                s->params.line.x1 = read_int("  x1 (0 to 79): ", 0, WIDTH - 1);
                s->params.line.y1 = read_int("  y1 (0 to 23): ", 0, HEIGHT - 1);
                s->params.line.x2 = read_int("  x2 (0 to 79): ", 0, WIDTH - 1);
                s->params.line.y2 = read_int("  y2 (0 to 23): ", 0, HEIGHT - 1);
                break;
            case SHAPE_RECTANGLE:
                printf("Enter New Rectangle Position & Size:\n");
                s->params.rect.x = read_int("  Top-Left x (0 to 79): ", 0, WIDTH - 1);
                s->params.rect.y = read_int("  Top-Left y (0 to 23): ", 0, HEIGHT - 1);
                s->params.rect.w = read_int("  Width (1 to 80): ", 1, WIDTH);
                s->params.rect.h = read_int("  Height (1 to 24): ", 1, HEIGHT);
                break;
            case SHAPE_CIRCLE:
                printf("Enter New Circle Center & Radius:\n");
                s->params.circle.cx = read_int("  Center x (0 to 79): ", 0, WIDTH - 1);
                s->params.circle.cy = read_int("  Center y (0 to 23): ", 0, HEIGHT - 1);
                s->params.circle.r = read_int("  Radius (1 to 40): ", 1, 40);
                break;
            case SHAPE_TRIANGLE:
                printf("Enter New Triangle Vertices:\n");
                s->params.triangle.x1 = read_int("  P1 x (0 to 79): ", 0, WIDTH - 1);
                s->params.triangle.y1 = read_int("  P1 y (0 to 23): ", 0, HEIGHT - 1);
                s->params.triangle.x2 = read_int("  P2 x (0 to 79): ", 0, WIDTH - 1);
                s->params.triangle.y2 = read_int("  P2 y (0 to 23): ", 0, HEIGHT - 1);
                s->params.triangle.x3 = read_int("  P3 x (0 to 79): ", 0, WIDTH - 1);
                s->params.triangle.y3 = read_int("  P3 y (0 to 23): ", 0, HEIGHT - 1);
                break;
        }
    }

    printf("Shape ID %d successfully modified!\n", s->id);
    render_shapes(canvas);
    display_canvas(canvas);
}

// Configure editor background style
void menu_configure_bg() {
    printf("\n--- Canvas Background Style ---\n");
    printf("1. Empty spaces ' '\n");
    printf("2. Dots '.' (Great for alignments)\n");
    printf("3. Custom character\n");
    int choice = read_int("Enter choice (1-3): ", 1, 3);
    
    if (choice == 1) {
        bg_char = ' ';
    } else if (choice == 2) {
        bg_char = '.';
    } else {
        bg_char = read_char("Enter custom background character", '_');
    }
    
    printf("Background updated!\n");
    render_shapes(canvas);
    display_canvas(canvas);
}

int main() {
    init_canvas(canvas, bg_char);

    while (1) {
        printf("\n=========================================\n");
        printf("      2D TERMINAL GRAPHICS EDITOR        \n");
        printf("=========================================\n");
        printf(" Canvas Grid: %dx%d (X: 0-%d, Y: 0-%d)\n", WIDTH, HEIGHT, WIDTH - 1, HEIGHT - 1);
        printf(" Background Character: '%c'\n", bg_char);
        printf(" Total Objects: %d / %d\n", shape_count, MAX_SHAPES);
        printf("-----------------------------------------\n");
        printf(" 1. Add a Shape\n");
        printf(" 2. Delete a Shape\n");
        printf(" 3. Modify a Shape\n");
        printf(" 4. Display Canvas\n");
        printf(" 5. List Active Shapes\n");
        printf(" 6. Configure Background Style\n");
        printf(" 7. Reset Canvas (Delete All)\n");
        printf(" 8. Exit\n");
        printf("=========================================\n");

        int choice = read_int("Enter choice (1-8): ", 1, 8);

        switch (choice) {
            case 1:
                menu_add_shape();
                break;
            case 2:
                menu_delete_shape();
                break;
            case 3:
                menu_modify_shape();
                break;
            case 4:
                render_shapes(canvas);
                display_canvas(canvas);
                break;
            case 5:
                list_shapes();
                break;
            case 6:
                menu_configure_bg();
                break;
            case 7:
                shape_count = 0;
                printf("Canvas reset. All shapes cleared.\n");
                render_shapes(canvas);
                display_canvas(canvas);
                break;
            case 8:
                printf("Thank you for using the 2D Graphics Editor. Goodbye!\n");
                return 0;
        }
    }
}
