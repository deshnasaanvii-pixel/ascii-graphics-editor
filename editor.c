#include "editor.h"
#include <stdio.h>
#include <stdlib.h>

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
        if (!new_shapes) {
            return -1;
        }
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

    int index = -1;
    for (int i = 0; i < editor->count; i++) {
        if (editor->shapes[i].id == id) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return 0;
    }

    for (int i = index; i < editor->count - 1; i++) {
        editor->shapes[i] = editor->shapes[i + 1];
    }

    editor->count--;
    return 1;
}

Shape* find_shape(editor_t* editor, int id) {
    if (!editor) return NULL;

    for (int i = 0; i < editor->count; i++) {
        if (editor->shapes[i].id == id) {
            return &editor->shapes[i];
        }
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
