#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
// Platform-specific console setups (Windows ANSI Escape code enabling)
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
void enable_ansi() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}
#else
void enable_ansi() {}
#endif
// ANSI Color Escape Sequences
#define COLOR_TITLE "\033[1;36m" // Bold Cyan
#define COLOR_MENU  "\033[1;32m" // Bold Green
#define COLOR_ALERT "\033[1;31m" // Bold Red
#define COLOR_INFO  "\033[1;33m" // Bold Yellow
#define COLOR_SHAPE "\033[1;35m" // Bold Magenta
#define COLOR_RESET "\033[0m"
// ==========================================
// 1. CANVAS COMPONENT (Declarations & Defs)
// ==========================================
typedef struct {
    char** grid;
    int width;
    int height;
    char bg_char;
} canvas_t;
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
            for (int j = 0; j < i; j++) free(canvas->grid[j]);
            free(canvas->grid);
            free(canvas);
            return NULL;
        }
    }
    // Clear canvas
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            canvas->grid[y][x] = bg_char;
        }
    }
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
    // Print top border
    printf("┌");
    for (int x = 0; x < canvas->width; x++) printf("─");
    printf("┐\n");
    // Print content
    for (int y = 0; y < canvas->height; y++) {
        printf("│");
        for (int x = 0; x < canvas->width; x++) {
            printf("%c", canvas->grid[y][x]);
        }
        printf("│\n");
    }
    // Print bottom border
    printf("└");
    for (int x = 0; x < canvas->width; x++) printf("─");
    printf("┘\n");
}
// ==========================================
// 2. SHAPES COMPONENT (Declarations & Defs)
// ==========================================
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
// Drawing implementations
void draw_line(canvas_t* canvas, Point p1, Point p2, char ch) {
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
    if (width <= 0 || height <= 0) return;
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
void draw_circle(canvas_t* canvas, Point center, int radius, int filled, char ch) {
    if (radius < 0) return;
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
    draw_line(canvas, p1, p2, ch);
    draw_line(canvas, p2, p3, ch);
    draw_line(canvas, p3, p1, ch);
}
// ==========================================
// 3. EDITOR COMPONENT (Declarations & Defs)
// ==========================================
typedef struct {
    int id;
    ShapeType type;
    union {
        Line line;
        Rectangle rect;
        Circle circle;
        Triangle tri;
    } data;
    char draw_char;
} Shape;
typedef struct {
    Shape* shapes;
    int capacity;
    int count;
    int next_id;
} editor_t;
editor_t* create_editor(int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = 10;
    editor_t* editor = (editor_t*)malloc(sizeof(editor_t));
    if (!editor) return NULL;
    editor->shapes = (Shape*)malloc(initial_capacity * sizeof(Shape));
    if (!editor->shapes) {
        free(editor);
        return NULL;
    }
    editor->capacity = initial_capacity;
    editor->count = 0;
    editor->next_id = 1;
    return editor;
}
void free_editor(editor_t* editor) {
    if (!editor) return;
    free(editor->shapes);
    free(editor);
}
int add_shape(editor_t* editor, Shape shape) {
    if (!editor) return -1;
    if (editor->count >= editor->capacity) {
        int new_capacity = editor->capacity * 2;
        Shape* new_shapes = (Shape*)realloc(editor->shapes, new_capacity * sizeof(Shape));
        if (!new_shapes) return -1;
        editor->shapes = new_shapes;
        editor->capacity = new_capacity;
    }
    shape.id = editor->next_id++;
    editor->shapes[editor->count] = shape;
    editor->count++;
    return shape.id;
}
int delete_shape(editor_t* editor, int id) {
    if (!editor) return 0;
    int idx = -1;
    for (int i = 0; i < editor->count; i++) {
        if (editor->shapes[i].id == id) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return 0;
    for (int i = idx; i < editor->count - 1; i++) {
        editor->shapes[i] = editor->shapes[i + 1];
    }
    editor->count--;
    return 1;
}
Shape* find_shape(editor_t* editor, int id) {
    if (!editor) return NULL;
    for (int i = 0; i < editor->count; i++) {
        if (editor->shapes[i].id == id) return &editor->shapes[i];
    }
    return NULL;
}
void render_scene(const editor_t* editor, canvas_t* canvas) {
    if (!editor || !canvas) return;
    clear_canvas(canvas);
    for (int i = 0; i < editor->count; i++) {
        const Shape* s = &editor->shapes[i];
        switch (s->type) {
            case SHAPE_LINE:
                draw_line(canvas, s->data.line.p1, s->data.line.p2, s->draw_char);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(canvas, s->data.rect.top_left, s->data.rect.width, s->data.rect.height, s->data.rect.filled, s->draw_char);
                break;
            case SHAPE_CIRCLE:
                draw_circle(canvas, s->data.circle.center, s->data.circle.radius, s->data.circle.filled, s->draw_char);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(canvas, s->data.tri.p1, s->data.tri.p2, s->data.tri.p3, s->draw_char);
                break;
        }
    }
}
// ==========================================
// 4. PERSISTENCE & HELPERS & MAIN LOOP
// ==========================================
int save_scene(const editor_t* editor, const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return 0;
    fprintf(f, "%d\n", editor->count);
    for (int i = 0; i < editor->count; i++) {
        const Shape* s = &editor->shapes[i];
        fprintf(f, "%d %c ", s->type, s->draw_char);
        switch (s->type) {
            case SHAPE_LINE:
                fprintf(f, "%d %d %d %d\n", s->data.line.p1.x, s->data.line.p1.y, s->data.line.p2.x, s->data.line.p2.y);
                break;
            case SHAPE_RECTANGLE:
                fprintf(f, "%d %d %d %d %d\n", s->data.rect.top_left.x, s->data.rect.top_left.y, s->data.rect.width, s->data.rect.height, s->data.rect.filled);
                break;
            case SHAPE_CIRCLE:
                fprintf(f, "%d %d %d %d\n", s->data.circle.center.x, s->data.circle.center.y, s->data.circle.radius, s->data.circle.filled);
                break;
            case SHAPE_TRIANGLE:
                fprintf(f, "%d %d %d %d %d %d\n", s->data.tri.p1.x, s->data.tri.p1.y, s->data.tri.p2.x, s->data.tri.p2.y, s->data.tri.p3.x, s->data.tri.p3.y);
                break;
        }
    }
    fclose(f);
    return 1;
}
int load_scene(editor_t* editor, const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;
    editor->count = 0;
    editor->next_id = 1;
    int count = 0;
    if (fscanf(f, "%d\n", &count) != 1) {
        fclose(f);
        return 0;
    }
    for (int i = 0; i < count; i++) {
        Shape s;
        int type_val = 0;
        char draw_ch = ' ';
        if (fscanf(f, "%d %c ", &type_val, &draw_ch) != 2) break;
        s.type = (ShapeType)type_val;
        s.draw_char = draw_ch;
        int read_ok = 0;
        switch (s.type) {
            case SHAPE_LINE:
                read_ok = (fscanf(f, "%d %d %d %d\n", &s.data.line.p1.x, &s.data.line.p1.y, &s.data.line.p2.x, &s.data.line.p2.y) == 4);
                break;
            case SHAPE_RECTANGLE:
                read_ok = (fscanf(f, "%d %d %d %d %d\n", &s.data.rect.top_left.x, &s.data.rect.top_left.y, &s.data.rect.width, &s.data.rect.height, &s.data.rect.filled) == 5);
                break;
            case SHAPE_CIRCLE:
                read_ok = (fscanf(f, "%d %d %d %d\n", &s.data.circle.center.x, &s.data.circle.center.y, &s.data.circle.radius, &s.data.circle.filled) == 4);
                break;
            case SHAPE_TRIANGLE:
                read_ok = (fscanf(f, "%d %d %d %d %d %d\n", &s.data.tri.p1.x, &s.data.tri.p1.y, &s.data.tri.p2.x, &s.data.tri.p2.y, &s.data.tri.p3.x, &s.data.tri.p3.y) == 6);
                break;
        }
        if (read_ok) {
            add_shape(editor, s);
        }
    }
    fclose(f);
    return 1;
}
int get_input_int(const char* prompt, int* val) {
    char buffer[256];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    return sscanf(buffer, "%d", val) == 1;
}
int get_input_char(const char* prompt, char* val) {
    char buffer[256];
    printf("%s", prompt);
    if (!fgets(buffer, sizeof(buffer), stdin)) return 0;
    int idx = 0;
    while (buffer[idx] == ' ' || buffer[idx] == '\t') idx++;
    if (buffer[idx] != '\n' && buffer[idx] != '\0') {
        *val = buffer[idx];
        return 1;
    }
    return 0;
}
int get_input_string(const char* prompt, char* buffer, int max_len) {
    printf("%s", prompt);
    if (!fgets(buffer, max_len, stdin)) return 0;
    int len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';
    return strlen(buffer) > 0;
}
void print_shapes_list(const editor_t* editor) {
    if (editor->count == 0) {
        printf("No active shapes. Draw something!\n");
        return;
    }
    printf(COLOR_INFO "ID   %-12s %-30s %-8s\n" COLOR_RESET, "Type", "Parameters", "Char");
    printf("-------------------------------------------------------------\n");
    for (int i = 0; i < editor->count; i++) {
        const Shape* s = &editor->shapes[i];
        char detail[128];
        char type_str[16];
        switch (s->type) {
            case SHAPE_LINE:
                strcpy(type_str, "Line");
                sprintf(detail, "(%d, %d) -> (%d, %d)", s->data.line.p1.x, s->data.line.p1.y, s->data.line.p2.x, s->data.line.p2.y);
                break;
            case SHAPE_RECTANGLE:
                strcpy(type_str, s->data.rect.filled ? "Rectangle (F)" : "Rectangle");
                sprintf(detail, "TL(%d, %d), W:%d, H:%d", s->data.rect.top_left.x, s->data.rect.top_left.y, s->data.rect.width, s->data.rect.height);
                break;
            case SHAPE_CIRCLE:
                strcpy(type_str, s->data.circle.filled ? "Circle (F)" : "Circle");
                sprintf(detail, "C(%d, %d), R:%d", s->data.circle.center.x, s->data.circle.center.y, s->data.circle.radius);
                break;
            case SHAPE_TRIANGLE:
                strcpy(type_str, "Triangle");
                sprintf(detail, "A(%d,%d), B(%d,%d), C(%d,%d)", s->data.tri.p1.x, s->data.tri.p1.y, s->data.tri.p2.x, s->data.tri.p2.y, s->data.tri.p3.x, s->data.tri.p3.y);
                break;
        }
        printf(COLOR_SHAPE "%-4d" COLOR_RESET " %-12s %-30s '%c'\n", s->id, type_str, detail, s->draw_char);
    }
}
int main() {
    enable_ansi();
    int canvas_w = 60;
    int canvas_h = 20;
    char bg_char = '_';
    canvas_t* canvas = create_canvas(canvas_w, canvas_h, bg_char);
    if (!canvas) return 1;
    editor_t* editor = create_editor(20);
    if (!editor) {
        free_canvas(canvas);
        return 1;
    }
    char message[256] = "";
    int running = 1;
    while (running) {
        printf("\033[H\033[J");
        printf(COLOR_TITLE "=========================================================\n");
        printf("            2D TERMINAL GRAPHICS EDITOR (C)            \n");
        printf("=========================================================\n" COLOR_RESET);
        render_scene(editor, canvas);
        print_canvas(canvas);
        printf("\n");
        print_shapes_list(editor);
        printf("\n");
        if (strlen(message) > 0) {
            printf(COLOR_INFO "Message: %s\n" COLOR_RESET, message);
            message[0] = '\0';
        }
        printf(COLOR_MENU "Menu Options:\n");
        printf(" [1] Add Shape      [2] Modify Shape    [3] Delete Shape\n");
        printf(" [4] Canvas Config  [5] Clear Canvas    [6] Save / Load Scene\n");
        printf(" [0] Exit\n" COLOR_RESET);
        int choice = -1;
        if (!get_input_int("Enter selection: ", &choice)) {
            sprintf(message, COLOR_ALERT "Invalid selection format!" COLOR_RESET);
            continue;
        }
        switch (choice) {
            case 0:
                running = 0;
                break;
            case 1: {
                printf("\nSelect Shape:\n [1] Line\n [2] Rectangle\n [3] Circle\n [4] Triangle\n");
                int t_choice = 0;
                if (!get_input_int("Enter selection: ", &t_choice) || t_choice < 1 || t_choice > 4) {
                    sprintf(message, COLOR_ALERT "Invalid shape type!" COLOR_RESET);
                    break;
                }
                Shape new_s;
                char d_ch = '*';
                if (t_choice == 1) {
                    new_s.type = SHAPE_LINE;
                    printf("Line Details:\n");
                    get_input_int("  Start X: ", &new_s.data.line.p1.x);
                    get_input_int("  Start Y: ", &new_s.data.line.p1.y);
                    get_input_int("  End X:   ", &new_s.data.line.p2.x);
                    get_input_int("  End Y:   ", &new_s.data.line.p2.y);
                } else if (t_choice == 2) {
                    new_s.type = SHAPE_RECTANGLE;
                    printf("Rectangle Details:\n");
                    get_input_int("  Top-Left X: ", &new_s.data.rect.top_left.x);
                    get_input_int("  Top-Left Y: ", &new_s.data.rect.top_left.y);
                    get_input_int("  Width:      ", &new_s.data.rect.width);
                    get_input_int("  Height:     ", &new_s.data.rect.height);
                    int f = 0;
                    get_input_int("  Filled? (1=Y, 0=N): ", &f);
                    new_s.data.rect.filled = (f == 1);
                } else if (t_choice == 3) {
                    new_s.type = SHAPE_CIRCLE;
                    printf("Circle Details:\n");
                    get_input_int("  Center X: ", &new_s.data.circle.center.x);
                    get_input_int("  Center Y: ", &new_s.data.circle.center.y);
                    get_input_int("  Radius:   ", &new_s.data.circle.radius);
                    int f = 0;
                    get_input_int("  Filled? (1=Y, 0=N): ", &f);
                    new_s.data.circle.filled = (f == 1);
                } else {
                    new_s.type = SHAPE_TRIANGLE;
                    printf("Triangle Details:\n");
                    get_input_int("  V1 X: ", &new_s.data.tri.p1.x);
                    get_input_int("  V1 Y: ", &new_s.data.tri.p1.y);
                    get_input_int("  V2 X: ", &new_s.data.tri.p2.x);
                    get_input_int("  V2 Y: ", &new_s.data.tri.p2.y);
                    get_input_int("  V3 X: ", &new_s.data.tri.p3.x);
                    get_input_int("  V3 Y: ", &new_s.data.tri.p3.y);
                }
                get_input_char("  Draw character (default '*'): ", &d_ch);
                new_s.draw_char = d_ch;
                int id = add_shape(editor, new_s);
                sprintf(message, "Added Shape #%d!", id);
                break;
            }
            case 2: {
                int mod_id = 0;
                if (!get_input_int("Enter ID to modify: ", &mod_id)) {
                    sprintf(message, COLOR_ALERT "Invalid format!" COLOR_RESET);
                    break;
                }
                Shape* s = find_shape(editor, mod_id);
                if (!s) {
                    sprintf(message, COLOR_ALERT "Shape #%d not found!" COLOR_RESET, mod_id);
                    break;
                }
                printf("Modifying Shape #%d:\n", s->id);
                if (s->type == SHAPE_LINE) {
                    get_input_int("  New Start X: ", &s->data.line.p1.x);
                    get_input_int("  New Start Y: ", &s->data.line.p1.y);
                    get_input_int("  New End X:   ", &s->data.line.p2.x);
                    get_input_int("  New End Y:   ", &s->data.line.p2.y);
                } else if (s->type == SHAPE_RECTANGLE) {
                    get_input_int("  New Top-Left X: ", &s->data.rect.top_left.x);
                    get_input_int("  New Top-Left Y: ", &s->data.rect.top_left.y);
                    get_input_int("  New Width:      ", &s->data.rect.width);
                    get_input_int("  New Height:     ", &s->data.rect.height);
                    int f = s->data.rect.filled;
                    get_input_int("  Filled? (1=Y, 0=N): ", &f);
                    s->data.rect.filled = (f == 1);
                } else if (s->type == SHAPE_CIRCLE) {
                    get_input_int("  New Center X: ", &s->data.circle.center.x);
                    get_input_int("  New Center Y: ", &s->data.circle.center.y);
                    get_input_int("  New Radius:   ", &s->data.circle.radius);
                    int f = s->data.circle.filled;
                    get_input_int("  Filled? (1=Y, 0=N): ", &f);
                    s->data.circle.filled = (f == 1);
                } else if (s->type == SHAPE_TRIANGLE) {
                    get_input_int("  New V1 X: ", &s->data.tri.p1.x);
                    get_input_int("  New V1 Y: ", &s->data.tri.p1.y);
                    get_input_int("  New V2 X: ", &s->data.tri.p2.x);
                    get_input_int("  New V2 Y: ", &s->data.tri.p2.y);
                    get_input_int("  New V3 X: ", &s->data.tri.p3.x);
                    get_input_int("  New V3 Y: ", &s->data.tri.p3.y);
                }
                char ch = s->draw_char;
                get_input_char("  New draw character: ", &ch);
                s->draw_char = ch;
                sprintf(message, "Modified Shape #%d!", mod_id);
                break;
            }
            case 3: {
                int del_id = 0;
                if (!get_input_int("Enter ID to delete: ", &del_id)) {
                    sprintf(message, COLOR_ALERT "Invalid format!" COLOR_RESET);
                    break;
                }
                if (delete_shape(editor, del_id)) {
                    sprintf(message, "Deleted Shape #%d!", del_id);
                } else {
                    sprintf(message, COLOR_ALERT "Shape #%d not found!" COLOR_RESET, del_id);
                }
                break;
            }
            case 4: {
                printf("\nConfig Options:\n [1] Resize Canvas\n [2] Change Background Char\n");
                int cfg = 0;
                get_input_int("Enter selection: ", &cfg);
                if (cfg == 1) {
                    int nw = canvas_w, nh = canvas_h;
                    get_input_int("  New Width (5-150):  ", &nw);
                    get_input_int("  New Height (5-80):  ", &nh);
                    if (nw >= 5 && nw <= 150 && nh >= 5 && nh <= 80) {
                        canvas_t* temp = create_canvas(nw, nh, bg_char);
                        if (temp) {
                            free_canvas(canvas);
                            canvas = temp;
                            canvas_w = nw;
                            canvas_h = nh;
                            sprintf(message, "Resized canvas to %dx%d!", canvas_w, canvas_h);
                        }
                    } else {
                        sprintf(message, COLOR_ALERT "Dimensions out of bounds!" COLOR_RESET);
                    }
                } else if (cfg == 2) {
                    char n_bg = bg_char;
                    if (get_input_char("  New Background character: ", &n_bg)) {
                        bg_char = n_bg;
                        canvas->bg_char = bg_char;
                        sprintf(message, "Changed background character to '%c'!", bg_char);
                    }
                }
                break;
            }
            case 5:
                editor->count = 0;
                sprintf(message, "Cleared canvas!");
                break;
            case 6: {
                printf("\n [1] Save\n [2] Load\n");
                int opt = 0;
                get_input_int("Enter selection: ", &opt);
                char fname[128] = "scene.txt";
                get_input_string("  Enter filename (default 'scene.txt'): ", fname, sizeof(fname));
                if (opt == 1) {
                    if (save_scene(editor, fname)) {
                        sprintf(message, "Saved scene successfully to '%s'!", fname);
                    } else {
                        sprintf(message, COLOR_ALERT "Save failed!" COLOR_RESET);
                    }
                } else if (opt == 2) {
                    if (load_scene(editor, fname)) {
                        sprintf(message, "Loaded scene successfully from '%s'!", fname);
                    } else {
                        sprintf(message, COLOR_ALERT "Load failed!" COLOR_RESET);
                    }
                }
                break;
            }
            default:
                sprintf(message, COLOR_ALERT "Invalid option!" COLOR_RESET);
                break;
        }
    }
    free_editor(editor);
    free_canvas(canvas);
    printf("Goodbye!\n");
    return 0;
}
