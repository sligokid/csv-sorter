/**
 * Various utilities, and implementation of PCRE class.
 * @todo This file and logger.cc need a good merge
 * @file util.cc
 */

/**
 * Simple OOP wraper around the pcre API.
 * @class PCRE
 */

#include <errno.h>
#include <sysexits.h>

#include <iostream>
#include "headers/util.h"

char* BASE16_TO_10_DATA = "\0\0\0\0\0\0\0\0\0\0" // 0-9
"\0\0\0\0\0\0\0\0\0\0" // 10-19
"\0\0\0\0\0\0\0\0\0\0" // 20-29
"\0\0\0\0\0\0\0\0\0\0" // 30-39
"\0\0\0\0\0\0\0\0\000\001" // 40-49
"\002\003\004\005\006\007\010\011\0\0" // 50-59
"\0\0\0\0\0\012\013\014\015\016" // 60-69
"\017\0\0\0\0\0\0\0\0\0" // 70-79
"\0\0\0\0\0\0\0\0\0\0" // 80-89
"\0\0\0\0\0\0\0\012\013\014" // 90-99
"\015\016\017\0\0\0\0\0\0\0" // 100-109
"\0\0\0\0\0\0\0\0\0\0" // 110-119
"\0\0\0\0\0\0\0\0\0\0" // 120-129
"\0\0\0\0\0\0\0\0\0\0" // 130-139
"\0\0\0\0\0\0\0\0\0\0" // 140-149
"\0\0\0\0\0\0\0\0\0\0" // 150-159
"\0\0\0\0\0\0\0\0\0\0" // 160-169
"\0\0\0\0\0\0\0\0\0\0" // 170-179
"\0\0\0\0\0\0\0\0\0\0" // 180-189
"\0\0\0\0\0\0\0\0\0\0" // 190-199
"\0\0\0\0\0\0\0\0\0\0" // 200-209
"\0\0\0\0\0\0\0\0\0\0" // 210-219
"\0\0\0\0\0\0\0\0\0\0" // 220-229
"\0\0\0\0\0\0\0\0\0\0" // 230-239
"\0\0\0\0\0\0\0\0\0\0" // 240-249
"\0\0\0\0\0\0"; // 250-255

char* ISXDIGIT_DATA = "\0\0\0\0\0\0\0\0\0\0" // 0-9
"\0\0\0\0\0\0\0\0\0\0" // 10-19
"\0\0\0\0\0\0\0\0\0\0" // 20-29
"\0\0\0\0\0\0\0\0\0\0" // 30-39
"\0\0\0\0\0\0\0\0\001\001" // 40-49
"\001\001\001\001\001\001\001\001\0\0" // 50-59
"\0\0\0\0\0\001\001\001\001\001" // 60-69
"\001\0\0\0\0\0\0\0\0\0" // 70-79
"\0\0\0\0\0\0\0\0\0\0" // 80-89
"\0\0\0\0\0\0\0\001\001\001" // 90-99
"\001\001\001\0\0\0\0\0\0\0" // 100-109
"\0\0\0\0\0\0\0\0\0\0" // 110-119
"\0\0\0\0\0\0\0\0\0\0" // 120-129
"\0\0\0\0\0\0\0\0\0\0" // 130-139
"\0\0\0\0\0\0\0\0\0\0" // 140-149
"\0\0\0\0\0\0\0\0\0\0" // 150-159
"\0\0\0\0\0\0\0\0\0\0" // 160-169
"\0\0\0\0\0\0\0\0\0\0" // 170-179
"\0\0\0\0\0\0\0\0\0\0" // 180-189
"\0\0\0\0\0\0\0\0\0\0" // 190-199
"\0\0\0\0\0\0\0\0\0\0" // 200-209
"\0\0\0\0\0\0\0\0\0\0" // 210-219
"\0\0\0\0\0\0\0\0\0\0" // 220-229
"\0\0\0\0\0\0\0\0\0\0" // 230-239
"\0\0\0\0\0\0\0\0\0\0" // 240-249
"\0\0\0\0\0\0"; // 250-255

char* ISDIGIT_DATA = "\0\0\0\0\0\0\0\0\0\0" // 0-9
"\0\0\0\0\0\0\0\0\0\0" // 10-19
"\0\0\0\0\0\0\0\0\0\0" // 20-29
"\0\0\0\0\0\0\0\0\0\0" // 30-39
"\0\0\0\0\0\0\0\0\001\001" // 40-49
"\001\001\001\001\001\001\001\001\0\0" // 50-59
"\0\0\0\0\0\0\0\0\0\0" // 60-69
"\0\0\0\0\0\0\0\0\0\0" // 70-79
"\0\0\0\0\0\0\0\0\0\0" // 80-89
"\0\0\0\0\0\0\0\0\0\0" // 90-99
"\0\0\0\0\0\0\0\0\0\0" // 100-109
"\0\0\0\0\0\0\0\0\0\0" // 110-119
"\0\0\0\0\0\0\0\0\0\0" // 120-129
"\0\0\0\0\0\0\0\0\0\0" // 130-139
"\0\0\0\0\0\0\0\0\0\0" // 140-149
"\0\0\0\0\0\0\0\0\0\0" // 150-159
"\0\0\0\0\0\0\0\0\0\0" // 160-169
"\0\0\0\0\0\0\0\0\0\0" // 170-179
"\0\0\0\0\0\0\0\0\0\0" // 180-189
"\0\0\0\0\0\0\0\0\0\0" // 190-199
"\0\0\0\0\0\0\0\0\0\0" // 200-209
"\0\0\0\0\0\0\0\0\0\0" // 210-219
"\0\0\0\0\0\0\0\0\0\0" // 220-229
"\0\0\0\0\0\0\0\0\0\0" // 230-239
"\0\0\0\0\0\0\0\0\0\0" // 240-249
"\0\0\0\0\0\0"; // 250-255

/**
 * Standard constructor.
 * @param pattern regular expression
 * @param options passed to pcre_compile()
 */
PCRE::PCRE(const char* const pattern, const int options)
{
   const char    *errmsg;
   int           erroffset;
   int           numCapt;

   // Compile and study regexp
   this->regex = pcre_compile(pattern, options, &errmsg, &erroffset, NULL);
   if (errmsg != NULL)
      throw runtime_error(errmsg);
   this->study = pcre_study(this->regex, 0, &errmsg);
   if (errmsg != NULL)
      throw runtime_error(errmsg);

   // Allocate work space
   if (pcre_fullinfo(this->regex, this->study, PCRE_INFO_CAPTURECOUNT, &numCapt) != 0)
      throw runtime_error("Can't get pattern info");
   this->matchSize = (numCapt + 1) * 3;
   this->matches = new int[this->matchSize];
   
   // Keeping a referrence count makes sure we can pass a PCRE along without running into memory problems
   pcre_refcount(this->regex, 1);
}

/**
 * Copy constructor taking care of refference-counting.
 * This makes sure we can pass a PCRE along without running into memory problems.
 */
PCRE::PCRE(const PCRE& p)
{
   this->regex = p.regex;
   this->study = p.study;
   this->matchSize = p.matchSize;
   this->matches = p.matches;
   pcre_refcount(this->regex, 1);
}

/**
 * Destructor.
 * Frees memory if this is the last PCRE instance using it.
 */
PCRE::~PCRE()
{
   if (pcre_refcount(this->regex, -1) == 0)
   {
      delete[] this->matches;
      free(this->regex);
      free(this->study);
   }
}

/**
 * Execute regexp.
 * @warning if you want to retreive captured substrings and the PCRE is shared between threads, use exec_r().
 */
int PCRE::exec(const char* const subject, const int length, const int option)
{
   return pcre_exec(this->regex, this->study, subject, length, 0, option, this->matches, this->matchSize);
}

/**
 * Execute regexp.
 * @warning if you want to retreive captured substrings and the PCRE is shared between threads, use exec_r().
 */
int PCRE::exec(const string& subject, const int option)
{
   return pcre_exec(this->regex, this->study, subject.c_str(), subject.length(), 0, option, this->matches, this->matchSize);
}

/**
 * Execute regexp (reentrant version).
 */
int PCRE::exec_r(const string& subject, int* mymatches, size_t mymatchSize, const int option)
{
   return pcre_exec(this->regex, this->study, subject.c_str(), subject.length(), 0, option, mymatches, mymatchSize);
}

/**
 * Use this if you manually allocate memory for exec_r().
 * @return the size of the int[] used by exec_r()
 */
size_t PCRE::getMatchSize()
{
   return this->matchSize;
}

/**
 * Execute regexp, and put captured substrings (starting with the full match) in a vector.
 */
int PCRE::exec_retrieve(const string& subject, vector<string>& captures, const int option)
{
   int mymatches[this->matchSize];
   int re_ret = pcre_exec(this->regex, this->study, subject.c_str(), subject.length(), 0, option, mymatches, this->matchSize);
   if (re_ret <= 0 && re_ret != PCRE_ERROR_NOMATCH)
   {
      throw runtime_error_pf("pcre error %d", re_ret);
   }
   else
   {
      captures.reserve(re_ret);
      for (int i = 0; i < re_ret; ++i)
      {
         int start = mymatches[2 * i];
         int stop = mymatches[2 * i + 1];
         captures.push_back((start >= 0 && stop >= 0) ? string(subject, start, stop - start) : "");
      }
   }
   return re_ret;
}

vector<string>  split(const string& s, const string& sep, size_t offset, size_t reserve)
{
  vector<string>  ret;
  split(ret, s, sep, offset, reserve);
  return ret;
}

void split(vector<string>& v, const string& s, const string& sep, size_t offset, size_t reserve)
{
   size_t pos;

   v.reserve(reserve);
   while ((pos = s.find(sep, offset)) != string::npos)
   {
      v.push_back(s.substr(offset, pos - offset));
      offset = pos + sep.length();
   }
   v.push_back(s.substr(offset));
}

/*
 * Added fix to replace blank placeholder char '-' wtih ""
 */
size_t split(string* v, const string& s, const string& sep, size_t offset, size_t max)
{
   size_t pos, i;

   for (i = 0; (pos = s.find(sep, offset)) != string::npos && i < max-1; i++)
   {
      v[i] = s.substr(offset, pos - offset);
      if ("-" == v[i])
         //v[i] = string();
         v[i].clear();
      offset = pos + sep.length();
   }
   //cout << i <<endl;
   v[i] = s.substr(offset);
   if ("-" == v[i])
      v[i] = string();
   return i+1;
}

bool startswith(const string& s, const string& substring) {
  const size_t l2 = substring.length();
  return (s.length() >= l2 && s.compare(0, l2, substring) == 0);
}

bool endswith(const string& s, const string& substring) {
  const size_t l1 = s.length();
  const size_t l2 = substring.length();
  return (l1 >= l2 && s.compare(l1-l2, l2, substring) == 0);
}

/**
 * Reduce multiple adjacent identical char to a single one.
 * erasedup("my/cool/////path//here") == "my/cool/path/here"
 * @param str input string
 * @param c char to reduce (default to '/')
 */
void erasedups(string &str, char c)
{
   int i = 0, j = 0, len = str.size();
   while (i < len)
   {
      if (str[i] == c)
         while (str[i+1] == c)
            ++i;
      str[j++] = str[i++];
   }
   str.resize(j);
}

PCRE anything(".?", PCRE_UTF8);

bool is_utf8(const string& s) {
  return (anything.exec(s) != PCRE_ERROR_BADUTF8);
}

__attribute__ ((deprecated)) // Use log() and / or runtime_error_pf()
void error(const string& message, const int status) {
  //tzsetwall(); //FIXME: bsd-only ?
  if (errno != 0) {
    cerr << strftime(22, "%F %T: ") << message << ": " << strerror(errno) << " (" << errno << ")" << endl;
    errno = 0;
  } else {
    cerr << strftime(22, "%F %T: ") << message << endl;
  }
//   if (status != EX_OK) {
//     term(status);
//   }
  tzset();
}

string strftime(const size_t maxsize, const char* const format, const time_t t) {
  char c[maxsize];
  tm* lt = localtime(&t);
  strftime(c, maxsize, format, lt);
  return string(c);
}

//void array_dump(string* myarray)
void array_dump(const string* myarray, int length)
{
   //string::string* iter = myarray.begin();
   //string myaray[] = {"AAA", "BBB", "CCC"};    
   for (int i = 0; i < length; i++)
   {
      cout << "EL:" << i << "->" << myarray[i] <<endl; 
   }
}
