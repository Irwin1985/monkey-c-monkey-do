#include "gc.h"
#include <stdio.h>

void gc_init(struct environment *env) {
    gc = malloc(sizeof *gc);
    gc->root = env;
    gc->list = NULL;
}

void gc_add(struct gc *gc, struct object *obj) {
    if (obj->type == OBJ_BOOL || obj->type == OBJ_NULL || obj->type == OBJ_BUILTIN) {
        return;
    }

    struct gc_node *node = malloc(sizeof *node);
    node->object = obj;
    node->next = gc->list;
    gc->list = node;   
}

void gc_mark_env(struct gc *gc, struct environment *env) {
    struct object *node;
    
    for (int i=0; i < env->cap; i++) {
        node = env->table[i];

        while (node) {
            node->gc_mark = 1;
            if (node->type == OBJ_FUNCTION && node->function.env != env) {
                gc_mark_env(gc, node->function.env);
            }
            node = node->next;
        }
    }
};


void gc_sweep(struct gc *gc) {
    
    struct gc_node *node = gc->list;
    struct gc_node *next = NULL;
    struct gc_node *prev = NULL;

    while (node) {
        // skip marked objects
        if (node->object->gc_mark) {
            prev = node;
            node = node->next;
            continue;
        }


        if (prev) {
            prev->next = node->next;
        } else {
            gc->list = node->next;
        }

        next = node->next;
        free_object(node->object);
        free(node);
        node = next;
    }

   
}

void gc_run(struct gc *gc) {
    gc_mark_env(gc, gc->root);
    gc_sweep(gc);
}

void gc_destroy(struct gc *gc, struct object *except) {
    struct gc_node *node = gc->list;
    struct gc_node *next = NULL;

    while (node) {
        next = node->next;
        if (node->object != except) {
            free_object(node->object);
        }
        free(node);
        node = next;
    }

    free(gc);
    gc = NULL;
}