#pragma once
#include "api/qni-api.pb-c.h"
#include <stddef.h>

void *qni_alloc(size_t size);
void qni_free(void *ptr);

void qni_free_print_dat(Qni__Api__ConsolePrintData *dat);
void qni_free_font(Qni__Api__Font *font);
void qni_free_input_req(Qni__Api__InputRequest *req);
void qni_free_input_res(Qni__Api__InputResponse *res);
void qni_free_prog_req(Qni__Api__ProgramRequest *req);
void qni_free_prog_res(Qni__Api__ProgramResponse *res);
void qni_free_prog_com(Qni__Api__ProgramCommand *command);
void qni_free_console_setting(Qni__Api__ConsoleSettingItem *setting);
void qni_free_err(Qni__Api__ErrorResponse *err);
