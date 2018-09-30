#include "qni-connector-context.h"
#include "qni-alloc.h"
#include "impls/qni-hashmap.h"
#include <string.h>
#include <stdio.h>

QniConnectorContext *qni_connector_context_new(QniConsoleContext *ctx, QniSharedConsoleContext *shared_ctx)
{
    QniConnectorContext *ret = qni_alloc(sizeof(QniConnectorContext));
    ret->ctx = ctx;
    ret->shared_ctx = shared_ctx;
    return ret;
}

void qni_connector_context_free(QniConnectorContext *ctx)
{
    qni_free(ctx);
}

int qni_connector_context_insert_ctx(QniConnectorContext *ctx, char *key, QniConsoleContext *value, bool overwrite)
{
    while (atomic_flag_test_and_set(&ctx->shared_ctx->lock))
    {
    }

    if (!overwrite)
    {
        int prev = hashmap_get(ctx->shared_ctx->ctxs, key, (void *)value);

        if (prev == MAP_OK)
            return -1;
    }

    int ret = hashmap_put(ctx->shared_ctx->ctxs, key, value);

    atomic_flag_clear(&ctx->shared_ctx->lock);
    return ret == MAP_OK;
}

void qni_connector_context_set_request(QniConnectorContext *ctx, Qni__Api__ProgramRequest *req)
{
    ctx->ctx->request = req;
}

size_t pack_message(Qni__Api__ProgramMessage *msg, void **buf)
{
    size_t len = qni__api__program_message__get_packed_size(msg);
    *buf = qni_alloc(len);
    protobuf_c_message_pack(msg, *buf);
    return len;
}

size_t qni_connector_context_process_request(QniConnectorContext *ctx, Qni__Api__ConsoleRequest *req, void **buf)
{
    Qni__Api__ProgramMessage msg = QNI__API__PROGRAM_MESSAGE__INIT;
    Qni__Api__ProgramResponse res = QNI__API__PROGRAM_RESPONSE__INIT;
    size_t len = 0;

    msg.data_case = QNI__API__PROGRAM_MESSAGE__DATA_RES;
    msg.res = &res;

    switch (req->data_case)
    {
    case QNI__API__CONSOLE_REQUEST__DATA_GET__STATE:
    {
        size_t pos = (size_t)req->get_state;

        if (ctx->ctx->commands->pos <= pos)
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_ERR;
            Qni__Api__ErrorResponse err = QNI__API__ERROR_RESPONSE__INIT;
            err.reason = "pos is too big";
            err.req_type = req->data_case;
            res.err = &err;
            len = pack_message(&msg, buf);
        }
        else
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_OK__GET__STATE;
            Qni__Api__ProgramCommandArray arr = QNI__API__PROGRAM_COMMAND_ARRAY__INIT;
            qni_command_buf_export(ctx->ctx->commands, &arr);
            res.ok_get_state = &arr;
            len = pack_message(&msg, buf);
            qni_command_buf_export_end(ctx->ctx->commands);
        }

        break;
    }

    case QNI__API__CONSOLE_REQUEST__DATA_LOAD__STATE:
    {
        while (atomic_flag_test_and_set(&ctx->shared_ctx->lock))
        {
        }

        int ret = hashmap_get(ctx->shared_ctx->ctxs, req->load_state, (any_t *)&ctx->ctx);

        if (ret != MAP_OK)
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_ERR;
            Qni__Api__ErrorResponse err = QNI__API__ERROR_RESPONSE__INIT;
            err.reason = "failed to load state";
            err.req_type = req->data_case;
            res.err = &err;
        }
        else
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_OK__LOAD__STATE;
            res.ok_load_state = qni_get_global_empty();
        }

        len = pack_message(&msg, buf);
        atomic_flag_clear(&ctx->shared_ctx->lock);
        break;
    }

    case QNI__API__CONSOLE_REQUEST__DATA_SHARE__STATE:
    case QNI__API__CONSOLE_REQUEST__DATA_SHARE__STATE__OVERWRITE:
    {
        while (atomic_flag_test_and_set(&ctx->shared_ctx->lock))
        {
        }

        bool overwrite = req->data_case == QNI__API__CONSOLE_REQUEST__DATA_SHARE__STATE__OVERWRITE;

        int ret = qni_connector_context_insert_ctx(ctx, req->share_state, ctx->ctx, overwrite);

        if (ret > 0)
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_ERR;
            Qni__Api__ErrorResponse err = QNI__API__ERROR_RESPONSE__INIT;
            err.reason = "failed to insert: already exists";
            err.req_type = req->data_case;
            res.err = &err;
        }
        else if (ret < 0)
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_ERR;
            Qni__Api__ErrorResponse err = QNI__API__ERROR_RESPONSE__INIT;
            err.reason = "failed to insert: map_full";
            err.req_type = req->data_case;
            res.err = &err;
        }
        else
        {
            res.data_case = QNI__API__PROGRAM_RESPONSE__DATA_OK__SHARE__STATE;

            //Move share state var
            res.ok_share_state = req->share_state;
        }

        len = pack_message(&msg, buf);
        atomic_flag_clear(&ctx->shared_ctx->lock);
        break;
    }
    }

    return len;
}

size_t qni_connector_context_process_response(QniConnectorContext *ctx, Qni__Api__ConsoleResponse *res, void **buf)
{
    Qni__Api__ProgramRequest *_Atomic req;

    req = NULL;

    // take request
    while (!atomic_compare_exchange_strong(&ctx->ctx->request, &req, NULL))
    {
        req = ctx->ctx->request;
    }

    // no request to react
    if (req == NULL)
    {
        return 0;
    }

    size_t len = 0;

    Qni__Api__ProgramMessage msg = QNI__API__PROGRAM_MESSAGE__INIT;
    Qni__Api__ProgramRequest ret_req = QNI__API__PROGRAM_REQUEST__INIT;

    msg.data_case = QNI__API__PROGRAM_MESSAGE__DATA_REQ;
    msg.req = &ret_req;

    switch (res->data_case)
    {
    case QNI__API__CONSOLE_RESPONSE__DATA_OK__INPUT:
        if (req->data_case != QNI__API__PROGRAM_REQUEST__DATA_INPUT || !qni_check_input(req->input, res->ok_input))
            break;

        ret_req.data_case = QNI__API__PROGRAM_REQUEST__DATA_ACCEPT__INPUT;
        ret_req.accept_input = req->input->tag;
        len = pack_message(&msg, buf);

        break;
    case QNI__API__CONSOLE_RESPONSE__DATA_OK__ACCEPT__INPUT:
        break;
    case QNI__API__CONSOLE_RESPONSE__DATA_ERR:
        printf("Console encount err: req[%d] reason[%s]", res->err->req_type, res->err->reason);
        break;
    }

    qni_free_prog_req(req);
    return len;
}
