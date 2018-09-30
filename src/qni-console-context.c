#include "qni-console-context.h"
#include "impls/qni-hashmap.h"

QniConsoleContext *qni_console_context_new()
{
    QniConsoleContext *ctx = qni_alloc(sizeof(QniConsoleContext));

    ctx->exit_flag = false;
    ctx->commands = qni_command_buf_new();
    ctx->request = NULL;

    return ctx;
}

void qni_console_context_free(QniConsoleContext *ctx)
{
    qni_command_buf_free(ctx->commands);

    qni_free((void *)ctx);
}

void qni_console_context_set_exit(QniConsoleContext *ctx)
{
    ctx->exit_flag = true;
}

bool qni_console_context_need_exit(QniConsoleContext *ctx)
{
    return ctx->exit_flag;
}

QniSharedConsoleContext *qni_shared_console_context_new()
{
    QniSharedConsoleContext *ctx = qni_alloc(sizeof(QniSharedConsoleContext));

    atomic_flag_clear(&ctx->lock);
    ctx->ctxs = hashmap_new();

    return ctx;
}

void qni_shared_console_context_delete(QniSharedConsoleContext *ctx)
{
    hashmap_free(ctx->ctxs);
    qni_free((void *)ctx);
}
