#include "../inc/bus.h"

static void init(void);
static int check(void);
static void destroy(void);
static void bus_cb(void);
static void free_bus_structs(sd_bus_error *err, sd_bus_message *m, sd_bus_message *reply);
static int check_err(int r, sd_bus_error *err);

static sd_bus *bus;
static struct self_t self = {
    .name = "Bus",
    .idx = BUS,
};

void set_bus_self(void) {
    modules[self.idx].self = &self;
    modules[self.idx].init = init;
    modules[self.idx].check = check;
    modules[self.idx].destroy = destroy;
    set_self_deps(&self);
}

/*
 * Open our bus and start lisetining on its fd
 */
static void init(void) {
    int r = sd_bus_open_system(&bus);
    if (r < 0) {
        return ERROR("Failed to connect to system bus: %s\n", strerror(-r));
    }
    // let main poll listen on bus events
    int bus_fd = sd_bus_get_fd(bus);
    init_module(bus_fd, self.idx, bus_cb);
}

static int check(void) {
    return 0; /* Skeleton function needed for modules interface */
}

/*
 * Close bus.
 */
static void destroy(void) {
    if (bus) {
        sd_bus_flush_close_unref(bus);
    }
}

/*
 * Callback for bus events
 */
static void bus_cb(void) {
    int r;
    do {
        r = sd_bus_process(bus, NULL);
    } while (r > 0);
}

/*
 * Call a method on bus and store its result of type userptr_type in userptr.
 */
void bus_call(void *userptr, const char *userptr_type, const struct bus_args *a, const char *signature, ...) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL, *reply = NULL;

    int r = sd_bus_message_new_method_call(bus, &m, a->service, a->path, a->interface, a->member);
    if (check_err(r, &error)) {
        goto finish;
    }

    va_list args;
    va_start(args, signature);
    
#if LIBSYSTEMD_VERSION >= 234
    sd_bus_message_appendv(m, signature, args);
#else
    int i = 0;
    char *s;
    int val;
    while (signature[i] != '\0') {
        switch (signature[i]) {
            case 's':
                s = va_arg(args, char *);
                r = sd_bus_message_append_basic(m, 's', s);
                if (check_err(r, &error)) {
                    goto finish;
                }
                break;
            case 'i':
                val = va_arg(args, int);
                r = sd_bus_message_append_basic(m, 'i', &val);
                if (check_err(r, &error)) {
                    goto finish;
                }
                break;
            default:
                WARN("Wrong signature in bus call: %c.\n", signature[i]);
                break;
        }
        i++;
    }
#endif
    va_end(args);
    r = sd_bus_call(bus, m, 0, &error, &reply);
    if (check_err(r, &error)) {
        goto finish;
    }

    /* Parse the response message */
    if (userptr != NULL) {
        if (!strncmp(userptr_type, "o", strlen("o"))) {
            const char *obj = NULL;
            r = sd_bus_message_read(reply, userptr_type, &obj);
            if (r >= 0) {
                strncpy(userptr, obj, PATH_MAX);
            }
        } else {
            r = sd_bus_message_read(reply, userptr_type, userptr);
        }
        check_err(r, &error);
    }

finish:
    free_bus_structs(&error, m, reply);
}

/*
 * Add a match on bus on certain signal for cb callback
 */
void add_match(const struct bus_args *a, sd_bus_message_handler_t cb) {
    char match[500] = {0};
    snprintf(match, sizeof(match), "type='signal', sender='%s', interface='%s', member='%s', path='%s'", a->service, a->interface, a->member, a->path);
    int r = sd_bus_add_match(bus, NULL, match, cb, NULL);
    check_err(r, NULL);
}

/*
 * Set property of type "type" value to "value". It correctly handles 'u' and 's' types.
 */
void set_property(const struct bus_args *a, const char type, const char *value) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r = 0;

    switch (type) {
        case 'u':
            r = sd_bus_set_property(bus, a->service, a->path, a->interface, a->member, &error, "u", atoi(value));
            break;
        case 's':
            r = sd_bus_set_property(bus, a->service, a->path, a->interface, a->member, &error, "s", value);
            break;
        default:
            WARN("Wrong signature in bus call: %c.\n", type);
            break;
    }
    check_err(r, &error);
    free_bus_structs(&error, NULL, NULL);
}

/*
 * Get a property of type "type" into userptr.
 */
void get_property(const struct bus_args *a, const char *type, void *userptr) {
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *m = NULL;

    int r = sd_bus_get_property(bus, a->service, a->path, a->interface, a->member, &error, &m, type);
    if (check_err(r, &error)) {
        goto finish;
    }
    if (!strcmp(type, "o")) {
        const char *obj = NULL;
        r = sd_bus_message_read(m, type, &obj);
        if (r >= 0) {
            strncpy(userptr, obj, PATH_MAX);
        }
    } else {
        r = sd_bus_message_read(m, type, userptr);
    }
    check_err(r, NULL);

finish:
    free_bus_structs(&error, m, NULL);
}

/*
 * Free used resources.
 */
static void free_bus_structs(sd_bus_error *err, sd_bus_message *m, sd_bus_message *reply) {
    if (err) {
        sd_bus_error_free(err);
    }
    if (m) {
        sd_bus_message_unref(m);
    }
    if (reply) {
        sd_bus_message_unref(reply);
    }
}

/*
 * Check any error. Do not leave for EBUSY errors.
 */
static int check_err(int r, sd_bus_error *err) {
    if (r < 0) {
        /* Don't leave for ebusy/eperm errors. eperm may mean that a not-active session called a method on clightd */
        if (r == -EBUSY || r == -EPERM) {
            WARN("%s\n", err && err->message ? err->message : strerror(-r));
        } else {
            ERROR("%s\n", err && err->message ? err->message : strerror(-r));
        }
    }
    return r < 0;
}
