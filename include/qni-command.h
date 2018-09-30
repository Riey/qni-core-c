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
