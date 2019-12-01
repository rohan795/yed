#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "internal.h"

#define YED_CMD_PROMPT ": "

void yed_init_commands(void);

yed_command yed_get_command(char *name);
void yed_set_command(char *name, yed_command command);
void yed_unset_command(char *name);

void yed_set_default_command(char *name, yed_command command);
void yed_set_default_commands(void);

void yed_clear_cmd_buff(void);
void yed_cmd_buff_push(char c);
void yed_cmd_buff_pop(void);
void yed_append_text_to_cmd_buff(char *s);
void yed_append_non_text_to_cmd_buff(char *s);
void yed_append_int_to_cmd_buff(int i);

void yed_draw_command_line(void);
void yed_do_command(void);
void yed_command_take_key(int key);

int yed_execute_command(char* name, int, char**);
void yed_add_command(char *name, yed_command cmd);

#define DEF_DEFAULT_COMMAND(name) \
    void yed_default_command_##name(int, char**)

DEF_DEFAULT_COMMAND(command_prompt);
DEF_DEFAULT_COMMAND(fill_command_prompt);
DEF_DEFAULT_COMMAND(quit);
DEF_DEFAULT_COMMAND(reload);
DEF_DEFAULT_COMMAND(redraw);
DEF_DEFAULT_COMMAND(set);
DEF_DEFAULT_COMMAND(get);
DEF_DEFAULT_COMMAND(unset);
DEF_DEFAULT_COMMAND(build_and_reload);
DEF_DEFAULT_COMMAND(sh);
DEF_DEFAULT_COMMAND(less);
DEF_DEFAULT_COMMAND(echo);
DEF_DEFAULT_COMMAND(cursor_down);
DEF_DEFAULT_COMMAND(cursor_up);
DEF_DEFAULT_COMMAND(cursor_left);
DEF_DEFAULT_COMMAND(cursor_right);
DEF_DEFAULT_COMMAND(cursor_line_begin);
DEF_DEFAULT_COMMAND(cursor_line_end);
DEF_DEFAULT_COMMAND(cursor_prev_word);
DEF_DEFAULT_COMMAND(cursor_next_word);
DEF_DEFAULT_COMMAND(cursor_prev_paragraph);
DEF_DEFAULT_COMMAND(cursor_next_paragraph);
DEF_DEFAULT_COMMAND(cursor_page_up);
DEF_DEFAULT_COMMAND(cursor_page_down);
DEF_DEFAULT_COMMAND(cursor_buffer_begin);
DEF_DEFAULT_COMMAND(cursor_buffer_end);
DEF_DEFAULT_COMMAND(cursor_line);
DEF_DEFAULT_COMMAND(word_under_cursor);
DEF_DEFAULT_COMMAND(buffer);
DEF_DEFAULT_COMMAND(buffer_delete);
DEF_DEFAULT_COMMAND(buffer_next);
DEF_DEFAULT_COMMAND(buffer_prev);
DEF_DEFAULT_COMMAND(buffer_name);
DEF_DEFAULT_COMMAND(frame_new);
DEF_DEFAULT_COMMAND(frame_delete);
DEF_DEFAULT_COMMAND(frame_vsplit);
DEF_DEFAULT_COMMAND(frame_hsplit);
DEF_DEFAULT_COMMAND(frame_next);
DEF_DEFAULT_COMMAND(frame_prev);
DEF_DEFAULT_COMMAND(frame_move);
DEF_DEFAULT_COMMAND(frame_resize);
DEF_DEFAULT_COMMAND(insert);
DEF_DEFAULT_COMMAND(delete_back);
DEF_DEFAULT_COMMAND(delete_forward);
DEF_DEFAULT_COMMAND(delete_line);
DEF_DEFAULT_COMMAND(write_buffer);
DEF_DEFAULT_COMMAND(plugin_load);
DEF_DEFAULT_COMMAND(plugin_unload);
DEF_DEFAULT_COMMAND(plugins_list);
DEF_DEFAULT_COMMAND(plugins_list_dirs);
DEF_DEFAULT_COMMAND(plugins_add_dir);
DEF_DEFAULT_COMMAND(select);
DEF_DEFAULT_COMMAND(select_lines);
DEF_DEFAULT_COMMAND(select_off);
DEF_DEFAULT_COMMAND(yank_selection);
DEF_DEFAULT_COMMAND(paste_yank_buffer);
DEF_DEFAULT_COMMAND(find_in_buffer);
DEF_DEFAULT_COMMAND(find_next_in_buffer);
DEF_DEFAULT_COMMAND(find_prev_in_buffer);
DEF_DEFAULT_COMMAND(replace_current_search);
DEF_DEFAULT_COMMAND(style);
DEF_DEFAULT_COMMAND(style_off);
DEF_DEFAULT_COMMAND(styles_list);
DEF_DEFAULT_COMMAND(undo);
DEF_DEFAULT_COMMAND(redo);

#endif
