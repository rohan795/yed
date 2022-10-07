void yed_assert_fail(const char *msg, const char *fname, int line, const char *cond_str) {
    volatile int *trap;

    yed_term_exit();

    printf("Assertion failed -- %s\n"
           "at  %s :: line %d\n"
           "    Condition: '%s'\n",
           msg, fname, line, cond_str);

    trap = 0;
    (void)*trap;
}


uint64_t next_power_of_2(uint64_t x) {
    if (x == 0) {
        return 2;
    }

    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x |= x >> 32;
    x++;
    return x;
}



char * pretty_bytes(uint64_t n_bytes) {
    uint64_t    s;
    double      count;
    char       *r;
    const char *suffixes[]
        = { " B", " KB", " MB", " GB", " TB", " PB", " EB" };

    s     = 0;
    count = (double)n_bytes;

    while (count >= 1024 && s < 7) {
        s     += 1;
        count /= 1024;
    }

    r = calloc(64, 1);

    if (count - floor(count) == 0.0) {
        sprintf(r, "%d", (int)count);
    } else {
        sprintf(r, "%.2f", count);
    }

    strcat(r, suffixes[s]);

    return r;
}

int output_buff_len(void) { return array_len(ys->output_buffer); }

static char *itoa(char *p, unsigned x) {
    p += 3*sizeof(int);
    *--p = 0;
    do {
        *--p = '0' + x % 10;
        x /= 10;
    } while (x);
    return p;
}

int yed_check_version_breaking(void) {
    int   breaks;
    char *env;
    char  cmd_buff[1024];
    FILE *p;
    char  ver_buff[32];
    int   new_ver;

    breaks = 1;

    if ((env = getenv("LD_LIBRARY_PATH"))) {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "LD_LIBRARY_PATH='%s' %s --no-init --version",
                 env, ys->argv0);
    } else if ((env = getenv("DYLD_LIBRARY_PATH"))) {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "DYLD_LIBRARY_PATH='%s' %s --no-init --version",
                 env, ys->argv0);
    } else {
        snprintf(cmd_buff, sizeof(cmd_buff),
                 "%s --version",
                 ys->argv0);
    }

    p = popen(cmd_buff, "r");
    if (p == NULL) { goto out; }

    if (fgets(ver_buff, sizeof(ver_buff), p) == NULL) { goto out; }

    pclose(p);

    sscanf(ver_buff, "%d", &new_ver);

    breaks = (new_ver / 100) > (yed_version / 100);

out:;
    return breaks;
}

void yed_service_reload(int core) {
    yed_key_map_list                                *list;
    tree_it(yed_command_name_t, yed_command)         cmd_it;
    tree_it(yed_completion_name_t, yed_completion)   compl_it;
    tree_it(yed_style_name_t, yed_style_ptr_t)       style_it;
    char                                            *key;
    yed_style                                       *style;
    char                                           **ft_name_it;

    if (core) {
        tree_reset_fns(yed_style_name_t,      yed_style_ptr_t,       ys->styles);
        tree_reset_fns(yed_var_name_t,        yed_var_val_t,         ys->vars);
        tree_reset_fns(yed_buffer_name_t,     yed_buffer_ptr_t,      ys->buffers);
        for (list = ys->keymap_list; list != NULL; list = list->next) {
            tree_reset_fns(int, yed_key_binding_ptr_t, list->map->binding_map);
        }
        tree_reset_fns(yed_command_name_t,    yed_command,           ys->commands);
        tree_reset_fns(yed_command_name_t,    yed_command,           ys->default_commands);
        tree_reset_fns(yed_completion_name_t, yed_completion,        ys->completions);
        tree_reset_fns(yed_completion_name_t, yed_completion,        ys->default_completions);
        tree_reset_fns(yed_plugin_name_t,     yed_plugin_ptr_t,      ys->plugins);
    }

    ys->cur_log_name = NULL; /* This could be memory from a plugin that got unloaded. */

    array_traverse(ys->ft_array, ft_name_it) {
        free(*ft_name_it);
    }
    array_clear(ys->ft_array);

    /*
     * Clear out all of the old styles.
     */
    while (tree_len(ys->styles)) {
        style_it = tree_begin(ys->styles);
        key   = tree_it_key(style_it);
        style = tree_it_val(style_it);
        tree_delete(ys->styles, key);
        free(key);
        free(style);
    }

    ys->active_style = NULL;
    /*
     * Reset the defaults.
     */
    yed_set_default_styles();

    /*
     * Clear out all of the old commands.
     */
    while (tree_len(ys->commands)) {
        cmd_it = tree_begin(ys->commands);
        key = tree_it_key(cmd_it);
        tree_delete(ys->commands, key);
        free(key);
    }
    while (tree_len(ys->default_commands)) {
        cmd_it = tree_begin(ys->default_commands);
        key = tree_it_key(cmd_it);
        tree_delete(ys->default_commands, key);
        free(key);
    }
    /*
     * Reset the defaults.
     */
    yed_set_default_commands();
    /*
     * Clear out all of the old completions.
     */
    while (tree_len(ys->completions)) {
        compl_it = tree_begin(ys->completions);
        key      = tree_it_key(compl_it);
        tree_delete(ys->completions, key);
        free(key);
    }
    while (tree_len(ys->default_completions)) {
        compl_it = tree_begin(ys->default_completions);
        key      = tree_it_key(compl_it);
        tree_delete(ys->default_completions, key);
        free(key);
    }
    /*
     * Reset the defaults.
     */
    yed_set_default_completions();

    yed_reload_default_event_handlers();
    yed_reload_plugins();

    if (core) {
        yed_register_sigwinch_handler();
        yed_register_sigstop_handler();
        yed_register_sigcont_handler();
    }
}

static void start_update_forcer(void);

void yed_force_update(void) {
    yed_signal(YED_SIG_FORCE_UPDATE);
}

int yed_get_update_hz(void) { return ys->update_hz; }

void yed_set_update_hz(int hz) {
    int need_to_start_updater;

    if      (hz < MIN_UPDATE_HZ) { hz = 0;             }
    else if (hz > MAX_UPDATE_HZ) { hz = MAX_UPDATE_HZ; }

    need_to_start_updater = ys->update_hz < MIN_UPDATE_HZ && hz;

    ys->update_hz = hz;

    if (need_to_start_updater) {
        start_update_forcer();
    }
}

void yed_signal(char sig) {
    int write_ret;

    write_ret = write(ys->signal_pipe_fds[1], &sig, 1);
    (void)write_ret;
}

void yed_handle_signal(char sig) {
    switch (sig) {
        case YED_SIG_FORCE_UPDATE:
            break;
        default:;
            yed_log("unrecognized signal received: 0x%x", sig);
            break;
    }
}

int s_to_i(const char *s) {
    int i;

    sscanf(s, "%d", &i);

    return i;
}

const char *u8_to_s(u8 u) { return _u8_to_s[u]; }

#include "install.c"
#include "array.c"
#include "bucket_array.c"
#include "term.c"
#include "screen.c"
#include "key.c"
#include "wcwidth.c"
#include "utf8.c"
#include "undo.c"
#include "buffer.c"
#include "attrs.c"
#include "ft.c"
#include "frame.c"
#include "log.c"
#include "command.c"
#include "getRSS.c"
#include "measure_time.c"
#include "default_event_handlers.c"
#include "event.c"
#include "plugin.c"
#include "boyer_moore.c"
#include "find.c"
#include "var.c"
#include "util.c"
#include "style.c"
#include "subproc.c"
#include "complete.c"
#include "direct_draw.c"
#include "frame_tree.c"
#include "version.c"
#include "print_backtrace.c"
#include "cmd_line.c"
#include "status_line.c"
