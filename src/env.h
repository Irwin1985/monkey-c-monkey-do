#ifndef ENV_H 
#define ENV_H 

#include "object.h"

#define MAX_KEY_LENGTH 32

struct environment {
    struct environment_node **table;
    struct environment *outer;

    unsigned int cap;
    unsigned int ref_count;

    // for linking in env_pool
    struct environment *next; 
};

struct environment_node {
    struct object *value;
    char *key;

    struct environment_node *next;
};

struct environment *make_environment(unsigned int cap);
struct environment *make_closed_environment(struct environment *parent, unsigned int cap);
struct object *environment_get(struct environment *env, char *key);
void environment_set(struct environment *env, char *key, struct object *value);
void free_environment(struct environment *env);
void free_environment_pool();

#endif