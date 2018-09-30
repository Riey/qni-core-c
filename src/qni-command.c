#define QNI_DEFINE_GLOBAL_EMPTY
#include "qni-command.h"
#include "qni-alloc.h"
#include <string.h>

Qni__Api__Empty global_empty = QNI__API__EMPTY__INIT;

Qni__Api__Empty *qni_get_global_empty()
{
    return &global_empty;
}

void resize_array(QniCommandBuffer *buf, size_t new_size)
{
    size_t old_cap = buf->cap;
    Qni__Api__ProgramCommand **old_ptr = buf->ptr;
    buf->cap = new_size;
    buf->ptr = qni_alloc(sizeof(Qni__Api__ProgramCommand *) * new_size);
    memcpy(buf->ptr, old_ptr, sizeof(Qni__Api__ProgramCommand *) * old_cap);
    qni_free(old_ptr);
}

void init_array(QniCommandBuffer *buf)
{
    buf->pos = 0;
    buf->cap = 100;
    buf->ptr = qni_alloc(sizeof(Qni__Api__ProgramCommand *) * 100);
    atomic_flag_clear(&buf->lock);
}

void append_command(QniCommandBuffer *buf, Qni__Api__ProgramCommand *command)
{
    while (atomic_flag_test_and_set(&buf->lock))
    {
    }

    if (buf->pos == buf->cap)
    {
        resize_array(buf, buf->cap * 2);
    }

    buf->ptr[buf->pos++] = command;

    atomic_flag_clear(&buf->lock);
}

QniCommandBuffer *qni_command_buf_new()
{
    QniCommandBuffer *buf = qni_alloc(sizeof(QniCommandBuffer));

    if (buf != NULL)
    {
        init_array(buf);
    }

    return buf;
}

void qni_command_buf_array_clear(QniCommandBuffer *buf)
{
    //spin lock
    while (atomic_flag_test_and_set(&buf->lock))
    {
    }

    for (size_t i = 0; i < buf->pos; i++)
    {
        qni_free_prog_com(buf->ptr[i]);
    }

    buf->pos = 0;

    atomic_flag_clear(&buf->lock);
}

void qni_command_buf_free(QniCommandBuffer *buf)
{
    qni_command_buf_array_clear(buf);
    qni_free(buf->ptr);
    qni_free(buf);
}

void qni_command_buf_export(QniCommandBuffer *buf, Qni__Api__ProgramCommandArray *msg)
{
    while (atomic_flag_test_and_set(&buf->lock))
    {
    }

    msg->n_commands = buf->pos;
    msg->commands = buf->ptr;
}

void qni_command_buf_export_end(QniCommandBuffer *buf)
{
    atomic_flag_clear(&buf->lock);
}

bool qni_check_input(Qni__Api__InputRequest *req, Qni__Api__InputResponse *res)
{
    switch (req->data_case)
    {
    case QNI__API__INPUT_REQUEST__DATA_ANYKEY:
    case QNI__API__INPUT_REQUEST__DATA_ENTER:
    case QNI__API__INPUT_REQUEST__DATA_TOUCH:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_EMPTY;

    case QNI__API__INPUT_REQUEST__DATA_BOOLEAN:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_BOOLEAN;

    case QNI__API__INPUT_REQUEST__DATA_INT:
    case QNI__API__INPUT_REQUEST__DATA_INT__MAX__LEN:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_INT;

    case QNI__API__INPUT_REQUEST__DATA_STR:
    case QNI__API__INPUT_REQUEST__DATA_STR__MAX__LEN:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_STR;

    case QNI__API__INPUT_REQUEST__DATA_STR__SELECT:

        if (res->data_case != QNI__API__INPUT_RESPONSE__DATA_STR)
            return false;

        for (size_t i = 0; i < req->str_select->n_data; i++)
        {
            if (strcmp(req->str_select->data[i], res->str) == 0)
                return true;
        }

        return false;

    case QNI__API__INPUT_REQUEST__DATA_FLOAT:
    case QNI__API__INPUT_REQUEST__DATA_FLOAT__MAX__LEN:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_FLOAT;

    case QNI__API__INPUT_REQUEST__DATA_DATE:
        return res->data_case == QNI__API__INPUT_REQUEST__DATA_DATE;

    case QNI__API__INPUT_REQUEST__DATA_TIME:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_TIME;

    case QNI__API__INPUT_REQUEST__DATA_DATETIME:
        return res->data_case == QNI__API__INPUT_REQUEST__DATA_DATETIME;

    case QNI__API__INPUT_REQUEST__DATA_COLOR:
        return res->data_case == QNI__API__INPUT_RESPONSE__DATA_COLOR;
    }

    return false;
}

#define MAKE_COM_AND_APPEND(DATA_CASE, ASSIGN_STMT)                              \
    Qni__Api__ProgramCommand *com = qni_alloc(sizeof(Qni__Api__ProgramCommand)); \
    qni__api__program_command__init(com);                                        \
    com->data_case = DATA_CASE;                                                  \
    ASSIGN_STMT;                                                                 \
    append_command(buf, com);

#define MAKE_PRINT_DATA_AND_APPEND(DATA_CASE, ASSIGN_STMT)                             \
    Qni__Api__ConsolePrintData *print = qni_alloc(sizeof(Qni__Api__ConsolePrintData)); \
    qni__api__console_print_data__init(print);                                         \
    print->data_case = DATA_CASE;                                                      \
    ASSIGN_STMT;                                                                       \
    MAKE_COM_AND_APPEND(QNI__API__PROGRAM_COMMAND__DATA_PRINT, com->print = print);

#define MAKE_CONSOLE_SETTIGN_AND_APPEND(DATA_CASE, ASSIGN_STMT)                              \
    Qni__Api__ConsoleSettingItem *setting = qni_alloc(sizeof(Qni__Api__ConsoleSettingItem)); \
    qni__api__console_setting_item__init(setting);                                           \
    setting->data_case = DATA_CASE;                                                          \
    ASSIGN_STMT;                                                                             \
    MAKE_COM_AND_APPEND(QNI__API__PROGRAM_COMMAND__DATA_UPDATE__SETTING, com->update_setting = setting);

void qni_print(QniCommandBuffer *buf, const char *text, size_t len)
{
    MAKE_PRINT_DATA_AND_APPEND(
        QNI__API__CONSOLE_PRINT_DATA__DATA_PRINT,
        print->print = qni_alloc(sizeof(char) * len);
        memcpy(print->print, text, len););
}

void qni_print_line(QniCommandBuffer *buf, const char *text, size_t len)
{
    MAKE_PRINT_DATA_AND_APPEND(
        QNI__API__CONSOLE_PRINT_DATA__DATA_PRINT__LINE,
        print->print_line = qni_alloc(sizeof(char) * len);
        memcpy(print->print_line, text, len););
}

void qni_new_line(QniCommandBuffer *buf)
{
    MAKE_PRINT_DATA_AND_APPEND(
        QNI__API__CONSOLE_PRINT_DATA__DATA_NEW__LINE,
        print->new_line = &global_empty);
}

void qni_set_font(QniCommandBuffer *buf, const char *font_family, size_t font_family_len, float font_size, Qni__Api__FontStyle font_style)
{
    Qni__Api__Font *font = qni_alloc(sizeof(Qni__Api__Font));
    qni__api__font__init(font);

    font->font_family = qni_alloc(font_family_len);
    memcpy(font->font_family, font_family, font_family_len);
    font->font_size = font_size;
    font->font_style = font_style;

    MAKE_CONSOLE_SETTIGN_AND_APPEND(QNI__API__CONSOLE_SETTING_ITEM__DATA_FONT, setting->font = font);
}

void qni_set_text_align(QniCommandBuffer *buf, Qni__Api__TextAlign text_align)
{
    MAKE_CONSOLE_SETTIGN_AND_APPEND(QNI__API__CONSOLE_SETTING_ITEM__DATA_TEXT__ALIGN, setting->text_align = text_align);
}

void qni_set_text_color(QniCommandBuffer *buf, uint32_t color)
{
    MAKE_CONSOLE_SETTIGN_AND_APPEND(QNI__API__CONSOLE_SETTING_ITEM__DATA_TEXT__COLOR, setting->text_color = color);
}

void qni_set_back_color(QniCommandBuffer *buf, uint32_t color)
{
    MAKE_CONSOLE_SETTIGN_AND_APPEND(QNI__API__CONSOLE_SETTING_ITEM__DATA_BACK__COLOR, setting->back_color = color);
}

void qni_set_highlight_color(QniCommandBuffer *buf, uint32_t color)
{
    MAKE_CONSOLE_SETTIGN_AND_APPEND(QNI__API__CONSOLE_SETTING_ITEM__DATA_HIGHLIGHT__COLOR, setting->highlight_color = color);
}
