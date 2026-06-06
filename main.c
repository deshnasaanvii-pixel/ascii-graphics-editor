#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "canvas.h"
#include "shapes.h"
#include "editor.h"

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

#define COLOR_TITLE "\033[1;36m"
#define COLOR_MENU  "\033[1;32m"
#define COLOR_ALERT "\033[1;31m"
#define COLOR_INFO  "\033[1;33m"
#define COLOR_SHAPE "\033[1;35m"
#define COLOR_RESET "\033[0m"

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
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return strlen(buffer) > 0;
}

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
    if (!canvas) {
        fprintf(stderr, "Failed to initialize canvas.\n");
        return 1;
    }

    editor_t* editor = create_editor(20);
    if (!editor) {
        free_canvas(canvas);
        fprintf(stderr, "Failed to initialize editor scene.\n");
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
            sprintf(message, COLOR_ALERT "Invalid menu option format!" COLOR_RESET);
            continue;
        }

        switch (choice) {
            case 0:
                running = 0;
                break;
            case 1: {
                printf("\nSelect Shape to Add:\n");
                printf(" [1] Line\n [2] Rectangle\n [3] Circle\n [4] Triangle\n");
                int type_choice = 0;
                if (!get_input_int("Enter type: ", &type_choice) || type_choice < 1 || type_choice > 4) {
                    sprintf(message, COLOR_ALERT "Invalid shape type choice!" COLOR_RESET);
                    break;
                }

                Shape new_shape;
                char d_ch = '*';
                
                if (type_choice == 1) {
                    new_shape.type = SHAPE_LINE;
                    printf("Drawing Line:\n");
                    if (!get_input_int("  Start X: ", &new_shape.data.line.p1.x) ||
                        !get_input_int("  Start Y: ", &new_shape.data.line.p1.y) ||
                        !get_input_int("  End X:   ", &new_shape.data.line.p2.x) ||
                        !get_input_int("  End Y:   ", &new_shape.data.line.p2.y)) {
                        sprintf(message, COLOR_ALERT "Invalid inputs for Line coords!" COLOR_RESET);
                        break;
                    }
                } else if (type_choice == 2) {
                    new_shape.type = SHAPE_RECTANGLE;
                    printf("Drawing Rectangle:\n");
                    if (!get_input_int("  Top-Left X: ", &new_shape.data.rect.top_left.x) ||
                        !get_input_int("  Top-Left Y: ", &new_shape.data.rect.top_left.y) ||
                        !get_input_int("  Width:      ", &new_shape.data.rect.width) ||
                        !get_input_int("  Height:     ", &new_shape.data.rect.height)) {
                        sprintf(message, COLOR_ALERT "Invalid inputs for Rectangle dimensions!" COLOR_RESET);
                        break;
                    }
                    int filled = 0;
                    get_input_int("  Filled? (1 = Yes, 0 = No): ", &filled);
                    new_shape.data.rect.filled = (filled == 1);
                } else if (type_choice == 3) {
                    new_shape.type = SHAPE_CIRCLE;
                    printf("Drawing Circle:\n");
                    if (!get_input_int("  Center X: ", &new_shape.data.circle.center.x) ||
                        !get_input_int("  Center Y: ", &new_shape.data.circle.center.y) ||
                        !get_input_int("  Radius:   ", &new_shape.data.circle.radius)) {
                        sprintf(message, COLOR_ALERT "Invalid inputs for Circle dimensions!" COLOR_RESET);
                        break;
                    }
                    int filled = 0;
                    get_input_int("  Filled? (1 = Yes, 0 = No): ", &filled);
                    new_shape.data.circle.filled = (filled == 1);
                } else {
                    new_shape.type = SHAPE_TRIANGLE;
                    printf("Drawing Triangle:\n");
                    if (!get_input_int("  Vertex 1 X: ", &new_shape.data.tri.p1.x) ||
                        !get_input_int("  Vertex 1 Y: ", &new_shape.data.tri.p1.y) ||
                        !get_input_int("  Vertex 2 X: ", &new_shape.data.tri.p2.x) ||
                        !get_input_int("  Vertex 2 Y: ", &new_shape.data.tri.p2.y) ||
                        !get_input_int("  Vertex 3 X: ", &new_shape.data.tri.p3.x) ||
                        !get_input_int("  Vertex 3 Y: ", &new_shape.data.tri.p3.y)) {
                        sprintf(message, COLOR_ALERT "Invalid inputs for Triangle vertices!" COLOR_RESET);
                        break;
                    }
                }

                get_input_char("  Draw character (default '*'): ", &d_ch);
                new_shape.draw_char = d_ch;

                int shape_id = add_shape(editor, new_shape);
                if (shape_id > 0) {
                    sprintf(message, "Successfully added Shape #%d!", shape_id);
                } else {
                    sprintf(message, COLOR_ALERT "Error: Failed to add shape!" COLOR_RESET);
                }
                break;
            }
            case 2: {
                int modify_id = 0;
                if (!get_input_int("Enter Shape ID to modify: ", &modify_id)) {
                    sprintf(message, COLOR_ALERT "Invalid ID format!" COLOR_RESET);
                    break;
                }

                Shape* s = find_shape(editor, modify_id);
                if (!s) {
                    sprintf(message, COLOR_ALERT "Shape ID #%d not found!" COLOR_RESET, modify_id);
                    break;
                }

                printf("\nModifying Shape #%d:\n", s->id);
                if (s->type == SHAPE_LINE) {
                    printf("Current coordinates: (%d, %d) -> (%d, %d)\n",
                           s->data.line.p1.x, s->data.line.p1.y, s->data.line.p2.x, s->data.line.p2.y);
                    get_input_int("  New Start X: ", &s->data.line.p1.x);
                    get_input_int("  New Start Y: ", &s->data.line.p1.y);
                    get_input_int("  New End X:   ", &s->data.line.p2.x);
                    get_input_int("  New End Y:   ", &s->data.line.p2.y);
                } else if (s->type == SHAPE_RECTANGLE) {
                    printf("Current rect: TL(%d, %d), W:%d, H:%d, Filled:%d\n",
                           s->data.rect.top_left.x, s->data.rect.top_left.y, s->data.rect.width, s->data.rect.height, s->data.rect.filled);
                    get_input_int("  New Top-Left X: ", &s->data.rect.top_left.x);
                    get_input_int("  New Top-Left Y: ", &s->data.rect.top_left.y);
                    get_input_int("  New Width:      ", &s->data.rect.width);
                    get_input_int("  New Height:     ", &s->data.rect.height);
                    int filled = s->data.rect.filled;
                    get_input_int("  Filled? (1 = Yes, 0 = No): ", &filled);
                    s->data.rect.filled = (filled == 1);
                } else if (s->type == SHAPE_CIRCLE) {
                    printf("Current circle: C(%d, %d), R:%d, Filled:%d\n",
                           s->data.circle.center.x, s->data.circle.center.y, s->data.circle.radius, s->data.circle.filled);
                    get_input_int("  New Center X: ", &s->data.circle.center.x);
                    get_input_int("  New Center Y: ", &s->data.circle.center.y);
                    get_input_int("  New Radius:   ", &s->data.circle.radius);
                    int filled = s->data.circle.filled;
                    get_input_int("  Filled? (1 = Yes, 0 = No): ", &filled);
                    s->data.circle.filled = (filled == 1);
                } else if (s->type == SHAPE_TRIANGLE) {
                    printf("Current triangle: A(%d,%d), B(%d,%d), C(%d,%d)\n",
                           s->data.tri.p1.x, s->data.tri.p1.y, s->data.tri.p2.x, s->data.tri.p2.y, s->data.tri.p3.x, s->data.tri.p3.y);
                    get_input_int("  New Vertex 1 X: ", &s->data.tri.p1.x);
                    get_input_int("  New Vertex 1 Y: ", &s->data.tri.p1.y);
                    get_input_int("  New Vertex 2 X: ", &s->data.tri.p2.x);
                    get_input_int("  New Vertex 2 Y: ", &s->data.tri.p2.y);
                    get_input_int("  New Vertex 3 X: ", &s->data.tri.p3.x);
                    get_input_int("  New Vertex 3 Y: ", &s->data.tri.p3.y);
                }

                char ch = s->draw_char;
                get_input_char("  New draw character: ", &ch);
                s->draw_char = ch;

                sprintf(message, "Successfully modified Shape #%d!", modify_id);
                break;
            }
            case 3: {
                int delete_id = 0;
                if (!get_input_int("Enter Shape ID to delete: ", &delete_id)) {
                    sprintf(message, COLOR_ALERT "Invalid ID format!" COLOR_RESET);
                    break;
                }

                if (delete_shape(editor, delete_id)) {
                    sprintf(message, "Successfully deleted Shape #%d!", delete_id);
                } else {
                    sprintf(message, COLOR_ALERT "Shape ID #%d not found!" COLOR_RESET, delete_id);
                }
                break;
            }
            case 4: {
                printf("\nCanvas Settings:\n");
                printf("  [1] Resize Grid\n  [2] Change Background Character\n");
                int choice_cfg = 0;
                if (!get_input_int("Enter selection: ", &choice_cfg)) {
                    sprintf(message, COLOR_ALERT "Invalid config option!" COLOR_RESET);
                    break;
                }

                if (choice_cfg == 1) {
                    int nw = canvas_w, nh = canvas_h;
                    if (!get_input_int("  New Width (5-150):  ", &nw) ||
                        !get_input_int("  New Height (5-80):  ", &nh)) {
                        sprintf(message, COLOR_ALERT "Invalid canvas dimensions!" COLOR_RESET);
                        break;
                    }
                    if (nw < 5 || nw > 150 || nh < 5 || nh > 80) {
                        sprintf(message, COLOR_ALERT "Dimensions out of bounds!" COLOR_RESET);
                        break;
                    }
                    canvas_t* new_cv = create_canvas(nw, nh, bg_char);
                    if (new_cv) {
                        free_canvas(canvas);
                        canvas = new_cv;
                        canvas_w = nw;
                        canvas_h = nh;
                        sprintf(message, "Resized canvas to %dx%d!", canvas_w, canvas_h);
                    } else {
                        sprintf(message, COLOR_ALERT "Failed to resize canvas!" COLOR_RESET);
                    }
                } else if (choice_cfg == 2) {
                    char n_bg = bg_char;
                    if (get_input_char("  New background character (e.g. '_', '.', ' '): ", &n_bg)) {
                        bg_char = n_bg;
                        canvas->bg_char = bg_char;
                        sprintf(message, "Changed background character to '%c'!", bg_char);
                    }
                }
                break;
            }
            case 5:
                editor->count = 0;
                sprintf(message, "Canvas cleared! All shapes deleted.");
                break;
            case 6: {
                printf("\nSave or Load Scene:\n");
                printf("  [1] Save to file\n  [2] Load from file\n");
                int choice_sl = 0;
                if (!get_input_int("Enter selection: ", &choice_sl)) {
                    sprintf(message, COLOR_ALERT "Invalid selection!" COLOR_RESET);
                    break;
                }

                char filename[128] = "scene.txt";
                get_input_string("  Enter filename (default 'scene.txt'): ", filename, sizeof(filename));

                if (choice_sl == 1) {
                    if (save_scene(editor, filename)) {
                        sprintf(message, "Saved scene successfully to '%s'!", filename);
                    } else {
                        sprintf(message, COLOR_ALERT "Failed to save scene to '%s'!" COLOR_RESET, filename);
                    }
                } else if (choice_sl == 2) {
                    if (load_scene(editor, filename)) {
                        sprintf(message, "Loaded scene successfully from '%s'!", filename);
                    } else {
                        sprintf(message, COLOR_ALERT "Failed to load scene from '%s' (check if file exists)!" COLOR_RESET, filename);
                    }
                }
                break;
            }
            default:
                sprintf(message, COLOR_ALERT "Invalid option '%d'!" COLOR_RESET, choice);
                break;
        }
    }

    free_editor(editor);
    free_canvas(canvas);
    printf("Goodbye!\n");
    return 0;
}
