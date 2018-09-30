#pragma once
#include "./qni-alloc.h"
#include <stdatomic.h>
#include <stdbool.h>

Qni__Api__Empty *qni_get_global_empty();

//thread-safe
typedef struct _QniCommandBuffer
{
    Qni__Api__ProgramCommand **ptr;
    size_t pos;
    size_t cap;
    atomic_flag lock;
} QniCommandBuffer;

QniCommandBuffer *qni_command_buf_new();
void qni_command_buf_free(QniCommandBuffer *buf);
void qni_command_buf_clear(QniCommandBuffer *buf);
void qni_command_buf_export(QniCommandBuffer *buf, Qni__Api__ProgramCommandArray *msg);
void qni_command_buf_export_end(QniCommandBuffer *buf);

bool qni_check_input(Qni__Api__InputRequest *req, Qni__Api__InputResponse *res);

void qni_print(QniCommandBuffer *buf, const char *text, size_t len);
void qni_print_line(QniCommandBuffer *buf, const char *text, size_t len);
void qni_new_line(QniCommandBuffer *buf);
void qni_set_font(QniCommandBuffer *buf, const char *font_family, size_t font_family_len, float font_size, Qni__Api__FontStyle font_style);
void qni_set_text_align(QniCommandBuffer *buf, Qni__Api__TextAlign text_align);
void qni_set_text_color(QniCommandBuffer *buf, uint32_t color);
void qni_set_back_color(QniCommandBuffer *buf, uint32_t color);
void qni_set_highlight_color(QniCommandBuffer *buf, uint32_t color);
