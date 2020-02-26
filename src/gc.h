#ifndef GC_H 
#define GC_H

#include "env.h"
#include "object.h"
#include <stdlib.h>

struct gc {
    struct environment *root;
    struct gc_node *list;
};

struct gc *gc;

struct gc_node {
    struct object *object;
    struct gc_node *next;
};

void gc_init(struct environment *env);
void gc_add(struct gc *gc, struct object *obj);
void gc_mark_env(struct gc *gc, struct environment *env);
void gc_sweep(struct gc *gc);
void gc_run(struct gc *gc);
void gc_destroy(struct gc *gc, struct object *except);

#endif 