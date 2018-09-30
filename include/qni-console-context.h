#pragma once
#include "./qni-command.h"
#include <stdbool.h>

//thread-safe
typedef struct _QniConsoleContext
{
    QniCommandBuffer *commands;
    Qni__Api__ProgramRequest *_Atomic request;
    atomic_bool exit_flag;
} QniConsoleContext;

QniConsoleContext *qni_console_context_new();
void qni_console_context_free(QniConsoleContext *ctx);
void qni_console_context_set_exit(QniConsoleContext *ctx);
bool qni_console_context_need_exit(QniConsoleContext *ctx);

typedef struct _QniSharedConsoleContext
{
    void *ctxs;
    atomic_flag lock;
} QniSharedConsoleContext;

QniSharedConsoleContext *qni_shared_console_context_new();
void qni_shared_console_context_delete(QniSharedConsoleContext *ctx);