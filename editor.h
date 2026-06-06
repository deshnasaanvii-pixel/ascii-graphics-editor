#ifndef EDITOR_H
#define EDITOR_H

#include "shapes.h"

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

editor_t* create_editor(int initial_capacity);
void free_editor(editor_t* editor);
int add_shape(editor_t* editor, Shape shape);
int delete_shape(editor_t* editor, int id);
Shape* find_shape(editor_t* editor, int id);
void render_scene(const editor_t* editor, canvas_t* canvas);

#endif // EDITOR_H
