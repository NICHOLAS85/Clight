#pragma once

#include "timer.h"
#include <dlfcn.h>
#include <glob.h>

#define SET_SELF() \
do { \
    modules[self.idx].self = &self; \
    modules[self.idx].init = init; \
    modules[self.idx].check = check; \
    modules[self.idx].poll_cb = callback; \
    modules[self.idx].destroy = destroy; \
    set_self_deps(&self); \
} while (0)

/* Useful macro to call each module's "set_$module_self" functions */
#define SET_MODULES_SELFS() \
do { \
    void (*func)(void); \
    char funcname[40] = {0}; \
    glob_t results; \
    glob("src/*.c", 0, NULL, &results); \
    for (int i = 0; i < results.gl_pathc; i++) { \
        char *name = strndup(results.gl_pathv[i] + 4, strlen(results.gl_pathv[i]) - 6); \
        sprintf(funcname, "set_%s_self", name); \
        free(name); \
        *(void **)(&func) = dlsym(NULL, funcname); \
        if (func) { \
            DEBUG("Function %s found!\n", funcname); \
            func(); \
        } \
    } \
    globfree(& results); \
} while(0)

#define INIT_MOD(fd, ...) init_module(fd, self.idx, ##__VA_ARGS__, (void *)0)

void init_modules(const enum modules module);
void init_module(int fd, enum modules module, ...);
int is_disabled(enum modules module);
int is_inited(enum modules module);
void set_self_deps(struct self_t *self);
void poll_cb(const enum modules module);
void change_dep_type(const enum modules mod, const enum modules mod_dep, const enum dep_type type);
void destroy_modules(void);
