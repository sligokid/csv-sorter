/**
 * This file is included by nearly all other files, and contains all the basic stuff.
 * Please try to keep it unbloated.
 * @file std.h
 */

#ifndef STD_H
#define STD_H

////////////////////////////////////////////////////////////////////////////////
// standard includes and namespaces
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>

using std::string;
using std::vector;
using std::deque;
using std::map;
using std::set;
using std::pair;
using std::exception;
using std::runtime_error;
using std::cout;
using std::cerr;
using std::endl;

using boost::shared_ptr;
using boost::weak_ptr;
using boost::format;


////////////////////////////////////////////////////////////////////////////////
// Compile-time options
////////////////////////////////////////////////////////////////////////////////

#define V2_COMPATIBILITY_MODE
//#define USE_DEFINITION_CACHE
//#define DO_DEBUG
//#define DO_ASSERT

////////////////////////////////////////////////////////////////////////////////
// logging
////////////////////////////////////////////////////////////////////////////////

void log(const char *function, const char *file, const int line, const char *severity, const char *msg, ...)
   __attribute__ ((format (printf, 5, 6)));
string  timestring(time_t t, bool localtime = false);

#ifdef DO_DEBUG
   #define DEBUG(s...)	  log(__PRETTY_FUNCTION__, __FILE__, __LINE__, "DEBUG", s)
#else
   #define DEBUG(s...)	  ;
#endif
#define TIME(s...)	log(__func__, __FILE__, __LINE__, "TIME", s)
#define LOGF(s...)	log(NULL, NULL, 0, "LOGF", s)
#define ERRF(s...)	log(__func__, __FILE__, __LINE__, "ERRF", s) ///< @todo What's the difference with ERROR() ?? Merge them.
#define ERROR(s...)	log(__func__, __FILE__, __LINE__, "ERROR", s)
#define WARNING(s...)	log(__func__, __FILE__, __LINE__, "WARNING", s)
#define ERROR(s...)	log(__func__, __FILE__, __LINE__, "ERROR", s)
#define ERROR_DIE(s...)	log(__func__, __FILE__, __LINE__, "ERROR_DIE", s) ///< @todo Get rid of that, throw/catch exceptions instead
#define MEMUSED(s...)   log(NULL, NULL, 0, "MEMUSED", s)

////////////////////////////////////////////////////////////////////////////////
// misc
////////////////////////////////////////////////////////////////////////////////

/**
 * Throw an exception formated using printf
 */
class runtime_error_pf : public std::exception 
{
   char	_M_msg[1024];
   
public:
   explicit runtime_error_pf(const char *format, ...);
   virtual ~runtime_error_pf() throw();
   virtual const char* what() const throw();
};

/**
 * struct for use in sorted STL containers
 */
struct cstringcomp
{
   bool operator()(const char *s1, const char *s2) const
   {
      return strcmp(s1, s2) < 0;
   }
};

#endif // STD_H
