/**
 * The CharsetConv class provides functions to convert text to utf-8 encoding.
 * It uses the iconv library.
 * @see iconv -l for list of known encodeings.
 * @class CharsetConv
 */

/**
 * Implementation of CharsetConv class.
 * @file CharsetConv.cc
 */

#include "headers/CharsetConv.h"
#include "headers/util.h"
#include "headers/Config.h"
#include <errno.h>
extern Config  conf_obj;

CharsetConv::CharsetConv(char *fallback_from)
{
   if ((this->convDesc = iconv_open("UTF-8//IGNORE", fallback_from)) == (iconv_t)-1)
      throw runtime_error_pf("Can't initialise charset converter to '%s' : %s", fallback_from, strerror(errno));
}

CharsetConv::~CharsetConv()
{
   if (this->convDesc != (iconv_t)-1)
      iconv_close(this->convDesc);
}

/**
 * Set charset to use when we don't know better
 */
void CharsetConv::setFallback(char *fallback_from)
{
   if (this->convDesc != (iconv_t)-1)
      iconv_close(this->convDesc);
   if ((this->convDesc = iconv_open("UTF-8//IGNORE", fallback_from)) == (iconv_t)-1)
   {
      //throw runtime_error_pf("Can't set charset converter to '%s' : %s", fallback_from, strerror(errno));
      ERROR("Can't Set Encodeing to '%s' : %s, using default.", fallback_from, strerror(errno));
      setFallback((char*)conf_obj.get_encodeingFallback().c_str());
   }
      
}

/**
 * Convert the string to UTF-8 if necessary, trying to guess the source encoding
 *
 * @todo add per-domain charset lookup table (essentially for referrer data)
 */
void CharsetConv::ensure_utf8(string &str)
{
   if (!is_utf8(str))
   {
      iconv(this->convDesc, NULL, NULL, NULL, NULL); // Reset conversion descriptor
      this->to_utf8(str, this->convDesc);
   }
}

/**
 * Convert the string to UTF-8, using specified source encoding
 */
void CharsetConv::to_utf8(string &str, iconv_t cd)
{
   size_t insize, outsize, outsize2;
   char *inbuf, *outbuf, *inbuf2, *outbuf2;

   // Prepare buffers
   insize = str.length();
   outsize2 = outsize = str.length() * 4; // utf8 has a maximum of 4 bytes per char, although it usually uses less.
   inbuf2 = inbuf = new char[insize];
   outbuf2 = outbuf = new char[outsize];
   str.copy(inbuf, insize);

   // convert
#ifdef __FreeBSD__
   while (iconv(cd, (const char **)&inbuf, &insize, &outbuf, &outsize) == (size_t)-1)
#else
   while (iconv(cd, &inbuf, &insize, &outbuf, &outsize) == (size_t)-1)
#endif
   {
      DEBUG("CharsetConv chokes on byte %ld of '%s' : %s", (long)insize, str.c_str(), strerror(errno));
      if (errno == E2BIG)
         break;
      ++inbuf;
      ++insize;
   }
   str = string(outbuf2, outsize2 - outsize);

   // cleanup and return
   delete[] inbuf2;
   delete[] outbuf2;
}
