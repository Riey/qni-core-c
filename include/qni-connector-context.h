#pragma once
#include "./qni-console-context.h"

//thread-unsafe
typedef struct _QniConnectorContext
{
    QniConsoleContext *ctx;
    QniSharedConsoleContext *shared_ctx;
} QniConnectorContext;

QniConnectorContext *qni_connector_context_new(QniConsoleContext *ctx, QniSharedConsoleContext *shared_ctx);
void qni_connector_context_free(QniConnectorContext *ctx);
void qni_connector_context_set_request(QniConnectorContext *ctx, Qni__Api__ProgramRequest *req);
int qni_connector_context_insert_ctx(QniConnectorContext *ctx, char *key, QniConsoleContext *value, bool overwrite);
size_t qni_connector_context_process_request(QniConnectorContext *ctx, Qni__Api__ConsoleRequest *req, void **buf);
size_t qni_connector_context_process_response(QniConnectorContext *ctx, Qni__Api__ConsoleResponse *res, void **buf);
