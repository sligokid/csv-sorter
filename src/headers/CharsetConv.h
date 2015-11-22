/**
 * Declaration of CharsetConv class.
 * @file CharsetConv.h
 */

#ifndef CHARSETCONV_H
#define CHARSETCONV_H

//#include <pcre.h>
#include <iconv.h>
#include "std.h"

class CharsetConv
{
public:
   CharsetConv(char *fallback_from = "ISO-8859-15");
   ~CharsetConv();
   void setFallback(char *fallback_from);
   void ensure_utf8(string &str);
private:
   iconv_t convDesc;
   void to_utf8(string &str);
   void to_utf8(string &str, iconv_t cd);
};

#endif // CHARSETCONV_H
