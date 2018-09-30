#define QNI_DEFINE_GLOBAL_ALLOCATOR
#include "qni-alloc.h"
#include <stdlib.h>

void *qni_alloc(size_t size)
{
    return malloc(size);
}

void qni_free(void *ptr)
{
    free(ptr);
}

void *qni_proto_alloc(void *data, size_t size)
{
    return qni_alloc(size);
}

void qni_proto_free(void *data, void *ptr)
{
    qni_free(ptr);
}

void qni_free_input_req(Qni__Api__InputRequest *req)
{
    switch (req->data_case)
    {
    case QNI__API__INPUT_REQUEST__DATA_STR__SELECT:
        for (size_t i = 0; i < req->str_select->n_data; i++)
        {
            qni_free(req->str_select->data[i]);
        }
        qni_free(req->str_select);
        break;
    }

    qni_free(req);
}

void qni_free_input_res(Qni__Api__InputResponse *res)
{
    switch (res->data_case)
    {
    case QNI__API__INPUT_RESPONSE__DATA_STR:
        qni_free(res->str);
        break;
    }

    qni_free(res);
}

void qni_free_print_dat(Qni__Api__ConsolePrintData *data)
{
    switch (data->data_case)
    {
    case QNI__API__CONSOLE_PRINT_DATA__DATA_PRINT:
        qni_free(data->print);
        break;
    case QNI__API__CONSOLE_PRINT_DATA__DATA_PRINT__LINE:
        qni_free(data->print_line);
        break;
    case QNI__API__CONSOLE_PRINT_DATA__DATA_PRINT__BUTTON:
        qni_free(data->print_button->text);
        qni_free_input_res(data->print_button->value);
        break;
    }

    qni_free(data);
}

void qni_free_font(Qni__Api__Font *font)
{
    qni_free(font->font_family);
    qni_free(font);
}

void qni_free_prog_req(Qni__Api__ProgramRequest *req)
{
    switch (req->data_case)
    {
    case QNI__API__PROGRAM_REQUEST__DATA_INPUT:
        qni_free_input_req(req->input);
        break;
    }

    qni_free(req);
}

void qni_free_prog_res(Qni__Api__ProgramResponse *res)
{
    switch (res->data_case)
    {
    case QNI__API__PROGRAM_RESPONSE__DATA_ERR:
        qni_free_err(res->err);
        break;
    case QNI__API__PROGRAM_RESPONSE__DATA_OK__SHARE__STATE:
        qni_free(res->ok_share_state);
        break;
    }

    qni_free(res);
}

void qni_free_prog_com(Qni__Api__ProgramCommand *command)
{
    switch (command->data_case)
    {
    case QNI__API__PROGRAM_COMMAND__DATA_PRINT:
        qni_free_print_dat(command->print);
        break;
    case QNI__API__PROGRAM_COMMAND__DATA_UPDATE__SETTING:
        qni_free_console_setting(command->update_setting);
        break;
    }

    qni_free(command);
}

void qni_free_console_setting(Qni__Api__ConsoleSettingItem *setting)
{
    switch (setting->data_case)
    {
    case QNI__API__CONSOLE_SETTING_ITEM__DATA_FONT:
        qni_free_font(setting->font);
        break;
    }

    qni_free(setting);
}

void qni_free_err(Qni__Api__ErrorResponse *err)
{
    qni_free(err->reason);
    qni_free(err);
}
