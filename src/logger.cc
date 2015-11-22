/**
 * Various error reporting and logging utilities.
 * @todo This file and utils.cc need a good merge
 * @file logger.cc
 */

#include <stdarg.h>
#include <sys/time.h> 
#include "headers/std.h"
#include "headers/Config.h"

#include <fstream>
#include <iostream>
extern Config conf_obj;
extern FILE *logF_p, *errF_p, *dbgF_p, *logerrF_p, *logfailF_p, *repF_p;

static pthread_mutex_t mutex_logger = PTHREAD_MUTEX_INITIALIZER;

/**
 * log a message (currently very simple-minded but improvements will come as needed)
 */
void  log(const char *function, const char *file, const int line, const char *severity, const char *msg, ...)
{
   pthread_mutex_lock(&mutex_logger);
   va_list ap;
   va_start(ap, msg); //ap points to 1st element after msg
   char* msgInFull;
   vasprintf(&msgInFull, msg, ap);
   va_end(ap);

   /// @todo stop using that, throw errors instead
   if (!strcmp("ERROR_DIE", severity))
   {
      printf("%s: %s\n", timestring(time(NULL)).c_str(), msgInFull);
      exit(1);
   }
   // cmd line switch
   if (conf_obj.is_optShowDebug())
   {
      printf("%s: %s: %s:%d %s: %s\n", timestring(time(NULL)).c_str(), severity, file, line, function, msgInFull);
   }
   if (!strcmp("LOGF", severity) || !strcmp("TIME", severity))
   {
      fprintf(logF_p, "%s: %s: %s\n", timestring(time(NULL)).c_str(), severity, msgInFull);
   }
#ifdef DO_DEBUG
   else if (!strcmp("DEBUG", severity))
   {
      fprintf(dbgF_p, "%s: %s: %s:%d %s: %s\n", timestring(time(NULL)).c_str(), severity, file, line, function, msgInFull);
   }
   else if (!strcmp("MEMUSED", severity))
   {
      fprintf(dbgF_p, "%s: %s: %s\n", timestring(time(NULL)).c_str(), severity, msgInFull);
   }
   else if (!strcmp("REPLAY", severity))
   {
      fprintf(repF_p, "%s\n", msgInFull);
   }
#endif
   else if (!strcmp("ERROR", severity) || !strcmp("ERRF", severity) || !strcmp("WARNING", severity))
   {
      fprintf((strstr(file, "CSVLine.cc") != NULL ? logerrF_p : errF_p) , "%s: %s: %s:%d %s: %s\n",timestring(time(NULL)).c_str(), severity, file, line, function, msgInFull);
   }
   else // unknown severity
   {
      fprintf(stderr, "%s: %s???: %s:%d %s: %s\n", timestring(time(NULL)).c_str(), severity, file, line, function, msgInFull);
   }
   free(msgInFull);
   fflush(0);
   pthread_mutex_unlock(&mutex_logger);
}

/**
 * Return the time as a formated string.
 * @param t unix timestamp
 * @param localtime wether to use local time or UTC (default)
 */
string timestring(time_t t, bool localtime)
{
  char timestring_buf[20];
  tm now;
  if (localtime)
     localtime_r(&t, &now);
  else
     gmtime_r(&t, &now);
  snprintf(timestring_buf, 20, "%d-%02d-%02d %02d:%02d:%02d",
           now.tm_year+1900, now.tm_mon+1, now.tm_mday,
           now.tm_hour, now.tm_min, now.tm_sec);
  return string(timestring_buf);
}

/*
static map<string, struct timeval[2] > > bench_map;
void bench(string section, bool begin)
{
  //pthread_mutex_lock/g(&mutex_logger);

  struct timeval tv;
  gettimeofday(&tv, NULL);

  map<string, struct timeval[2]> >::iterator i = bench_map.find(section);
  if (i == bench_map.end())
    {
      i = bench_map.insert(section);
    }

  //thread_mutex_unlock(&mutex_logger);
}
*/

/**
 * Exception constructor.
 * Note we keep behaviour as close to STL's runtime_error, and avoid using memory-allocating functions.
 */
runtime_error_pf::runtime_error_pf(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vsnprintf(_M_msg, sizeof(_M_msg)-1, format, ap);
  va_end(ap);
}

runtime_error_pf::~runtime_error_pf() throw() { }

const char* runtime_error_pf::what() const throw()
{
  return _M_msg;
}

void log_fail_line(const string& line) 
{
   fprintf(logfailF_p, "%s\n", line.c_str());
}
