#include <string.h> 
#include <stdlib.h> 
#include <err.h>

#include "object.h"
#include "env.h"

struct env_pool {
    struct environment *head;
};

struct env_pool env_pool = {
    .head = NULL,
};

static unsigned long djb2(char *str)
{
    unsigned long hash = 5381;
    int c;

    // hash * 33 + c
    // shifting bits for performance
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }

    return hash;
}

struct environment *make_environment(unsigned int cap) {
    struct environment *env;

    // try to get pre-allocated object from pool
    if (!env_pool.head) {
        env = malloc(sizeof *env);
        if (!env) {
            err(EXIT_FAILURE, "out of memory");
        }
        env->table = malloc(sizeof *env->table * cap);
        env->cap = cap;
        if (!env->table) {
            err(EXIT_FAILURE, "out of memory");
        }

    } else {
        env = env_pool.head;
        env_pool.head = env->next;

        // increase capacity of existing env if needed
        if (env->cap < cap) {
            env->table = realloc(env->table, sizeof *env->table * cap);
            env->cap = cap;
            if (!env->table) {
                err(EXIT_FAILURE, "out of memory");
            }
        }
    }
    
    env->ref_count = 1;
    env->outer = NULL;
    for (int i = 0; i < env->cap; i++)
    {
        env->table[i] = NULL;
    }
    return env;
}

struct environment *make_closed_environment(struct environment *parent, unsigned int cap) {
    struct environment *env = make_environment(cap);
    env->outer = parent;
    env->outer->ref_count++;
    return env;
}

struct object *environment_get_with_pos(struct environment *env, char *key, unsigned int pos) {
    struct environment_node *node = env->table[pos];

    while (node) {
        if (strncmp(node->key, key, MAX_KEY_LENGTH) == 0) {
            return node->value;
        }

        node = node->next;
    }

    // try parent environment (bubble up scope)
    if (env->outer) {
        return environment_get_with_pos(env->outer, key, pos);
    }

    return NULL;
}

struct object *environment_get(struct environment *env, char *key) {
   unsigned int pos = djb2(key) % env->cap;
   return environment_get_with_pos(env, key, pos);
}

void environment_set(struct environment *env, char *key, struct object *value) {
    unsigned int pos = djb2(key) % env->cap;
    struct environment_node *node = env->table[pos];

    // find existing node with that key
    while (node) {
        if (strncmp(node->key, key, MAX_KEY_LENGTH) == 0 && value != node->value) {
            node->value->ref_count--;
            node->value = value;
            node->value->ref_count++;
            return;
        }      

        node = node->next;
    }

    // insert new node at start of list
    struct environment_node *new_node = malloc(sizeof *node);
    new_node->next = env->table[pos];
    new_node->key = key;
    new_node->value = value;
    env->table[pos] = new_node;

    // increase ref count by 1 so we know not to release this object
    value->ref_count++;
}

void free_environment(struct environment *env) {
    if (--env->ref_count > 0) {
        return;
    }

    struct environment_node *node;
    struct environment_node *next;

    // free objects in env
    for (int i=0; i < env->cap; i++) {
        node = env->table[i];

        while (node) {
            next = node->next;
            free_object(node->value);
            free(node);
            node = next;
        }
    }

    if (env->outer != NULL) {
        free_environment(env->outer);
    }

    // return env to pool so future calls of make_environment can use it
    env->outer = NULL;
    env->next = env_pool.head;
    env_pool.head = env;
}

void free_environment_pool() {
    struct environment *node = env_pool.head;
    struct environment *next = node;

    while (node) {
        next = node->next;
        free(node->table);
        free(node);
        node = next;
    }

    env_pool.head = NULL;
}