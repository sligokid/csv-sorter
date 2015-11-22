/**
 * Declaration of PCRE class and of util.cc functions.
 * @file util.h
 */

#ifndef UTIL_H
#define UTIL_H

#include <pcre.h>

#include "std.h"

/// @see http://www.oreillynet.com/pub/a/network/excerpt/spcookbook_chap03/index2.html
//#define BASE16_TO_10(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : (toupper((x)) - 'A' + 10))
#define BASE16_TO_10(x) (BASE16_TO_10_DATA[(unsigned char)(x)])
//#define ISXDIGIT(x) (isxdigit(x))
#define ISXDIGIT(x) (ISXDIGIT_DATA[(unsigned char)(x)])
//#define ISDIGIT(x) (isdigit(x))
#define ISDIGIT(x) (ISDIGIT_DATA[(unsigned char)(x)])

class PCRE
{
public:
   PCRE(const char* const pattern, const int options = 0);
   PCRE(string pattern, const int options = 0) {PCRE(pattern.c_str(), options);};
   PCRE(const PCRE& p);
   ~PCRE();
   int exec(const char* const subject, const int length, const int options = 0);
   int exec(const string& subject, const int options = 0);
   int exec_r(const string& subject, int* matches, size_t mymatchSize, const int option = 0);
   int exec_retrieve(const string& subject, vector<string>& captures, const int option = 0);
   size_t getMatchSize();
   int* matches;

private:
   pcre* regex;
   pcre_extra* study;
   size_t matchSize;
};

void split(vector<string>& v, const string& s, const string& sep, size_t offset = 0, size_t reserve = 0);
size_t split(string* v, const string& s, const string& sep, size_t offset = 0, size_t max = 1000);
vector<string>	split(const string& s, const string& sep, size_t offset = 0, size_t reserve = 0);

bool startswith(const string& s, const string& substring);

bool endswith(const string& s, const string& substring);

void erasedups(string &str, char c = '/');

bool is_utf8(const string& s);

void error(const string& message, const int status);

string strftime(const size_t maxsize, const char* const format, const time_t t = time(NULL));

void array_dump(const string* myarray, int length);
#endif // UTIL_H
