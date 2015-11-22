/**
 * Declaration of CSVLineBase class.
 * @file CSVLineBase.h
 */

#ifndef CSVLINEBASE_H
#define CSVLINEBASE_H

#include <GeoIP.h>
#include "std.h"
#include "util.h"
#include <errno.h>

class CSVFile;
using namespace std; ///< @todo clean that out

/**
 * @see CSVLineBase::parse_version()
 */
enum CSVVersion 
{
   CSVVERSION_BAD, 
   CSVVERSION_SITE10, 
   CSVVERSION_SITE11, 
   CSVVERSION_SITE13, 
   CSVVERSION_STREAM10 
};

/**
 * We only use CSVLine that have been parsed as VALID.
 * The other values only indicate in what way the parsing failed.
 */
enum CSVValidity
{
   VALID, 
   INVALID_VERSION, 
   INVALID_UNWANTED, 
   INVALID_FIELDCOUNT, 
   INVALID_TIMESTAMP,
   INVALID_LOCATION_URL, 
   INVALID_IP,
   VALIDITY_ENUM_SIZE
};

enum LOG_FIELDS
{
   LOG_URI,
   LOG_TIME,
   LOG_HTTP_REF,
   LOG_IP,
   LOG_UA,
   LOG_LANG,
   LOG_PROXY,
   LOG_SID,
   LOG_CID,
   LOG_ENUM_SIZE
};

enum URI_FIELDS
{
   URI_LOC,
   URI_FURL,
   URI_DOCTITLE,
   URI_REFERRER,
   URI_JAVA,
   URI_SCREEN,
   URI_COLOR,
   URI_TMAC,
   URI_CC,
   URI_ENCODEING,
   URI_JSCRIPT,
   URI_ENUM_SIZE
};

/**
 * These should match exactly the order in the csv file (except the version field, which is excluded).
 * If incompatible csv formats are created, we'll have to create a different enum.
 */
enum CSVFIELDS
{
   CSVFIELD_VERSION,
   CSVFIELD_TIMESTAMP,
   CSVFIELD_LOCATION,
   CSVFIELD_FIXED_URL,
   CSVFIELD_DOCUMENT_TITLE, ///< @bug how come this isnt used anywhere ??
   CSVFIELD_REFERRER,
   CSVFIELD_IP,
   CSVFIELD_USER_AGENT,
   CSVFIELD_LANGUAGE,
   CSVFIELD_SCREEN,
   CSVFIELD_COLOR,
   CSVFIELD_PROXY,
   CSVFIELD_JAVASCRIPT,
   CSVFIELD_COOKIE,
   CSVFIELD_JAVA,
   CSVFIELD_SESSIONSTRING,
   CSVFIELD_USERSTRING,
   CSVFIELD_DATA_END,
   CSVFIELD_TMAC,
   CSVFIELD_CC,        // used for setting TZ
   CSVFIELD_ENCODEING, // FIXME Dont need this after we encode the uri
   CSVFIELD_ENUM_SIZE
};

/*
FixedURL]
[DocumentTitle]
[ReferrerURL]
[AccountName]
[CountryCode]
[EncType]
[TmSec]
*/
enum NOJS_MATCH
{
   NOJS_START,
   NOJS_END,
   NOJS_FURL_S,
   NOJS_FURL_E,
   NOJS_DOCTITLE_S,
   NOJS_DOCTITLE_E,
   NOJS_REFERRER_S,
   NOJS_REFERRER_E,
   NOJS_TMAC_S,
   NOJS_TMAC_E,
   NOJS_CC_S,
   NOJS_CC_E,
   NOJS_ENCODING_S,
   NOJS_ENCODING_E,
   NOJS_TMSEC_S,
   NOJS_TMSEC_E,
   NOJS_ENUM_SIZE
};

class CSVLineBase
{
  public:
   CSVVersion  version;
   CSVValidity validity;
   time_t time;
   time_t microtime;

   // UT Wrappers
   void split_UT();
   void log_parse_version_UT();
   void log_parse_uri_UT();
   void truncate_field_UT();
   void csv_put_cookie_UT();
   void csv_parse_cookies_UT();

  protected:
   CSVLineBase();
   bool ensure_timestamp(const string& timestamp);
   bool log_parse_uri();

   CSVVersion log_parse_version(const string& line, string& LOGV);
   void csv_put_version(const string& v);
   void csv_put_timestamp();
   void csv_put_uri();
   void csv_put_ip();
   void csv_put_user_agent();
   void csv_put_lang();
   void csv_put_proxy();
   void csv_put_cookie();
   void csv_encode();
   void csv_parse_cookies();
   void csv_write_stream();
   string truncate_field(string& s);

   string csvfields_ary[CSVFIELD_ENUM_SIZE];
   string logfields_ary[LOG_ENUM_SIZE];
   string urifields_ary[URI_ENUM_SIZE];
   static PCRE log_uri_regex_js_on;
   static PCRE log_uri_regex_js_off;

   //UT helpers
   bool log_parse_uri(const string& ut);
   bool csv_put_cookie(bool sid_b, bool cid_b);
   bool csv_parse_cookies(string& utUser, string utSession = string());
};

#endif // CSVLINEBASE_H
