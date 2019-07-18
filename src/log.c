/* log.c - Log functions. */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "log.h"

#define LOG_FILE "plato.log"

void _log_out(char* buffer)
{
#ifdef LOGGING
  FILE *fp;
  time_t time_of_day;
  char tod_buffer[32];
  memset(tod_buffer,0,sizeof(tod_buffer));
  sprintf(tod_buffer,"%s - ",ctime(&time_of_day));
  fp=fopen(LOG_FILE,"a");
  fwrite(tod_buffer,sizeof(char),strlen(tod_buffer),fp);
  fwrite(buffer,sizeof(char),strlen(buffer),fp);
  fclose(fp);
#endif
}

void log(const char* format, ...)
{
#ifdef LOGGING
  char buffer[256];
  va_list args;
  va_start(args,format);
  vsprintf(buffer,format,args);
  _log_out(buffer);
  va_end(args);
#endif
}
