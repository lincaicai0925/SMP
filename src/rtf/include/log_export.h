#ifndef LOG_EXPORT_H
#define LOG_EXPORT_H

#include "pages/include/log_manage.h"
#include <stdbool.h>

bool Log_Export_To_Word(const log_data_t *logs, int count, const char *filename);

#endif /* LOG_EXPORT_H */
