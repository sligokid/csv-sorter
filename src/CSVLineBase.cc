/**
 * Implementation of CSVLineBase class.
 * The CSVLineBase class does most of the actual CSV parsing work.
 * @class CSVLineBase
 * @todo get rid of url_fragment handling code; we're not collecting it in the first place.
 */

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "headers/CSVLineBase.h"
#include "headers/CharsetConv.h"
#include <pcrecpp.h>
#include <math.h>
#include "headers/Config.h"
extern Config conf_obj;
extern CharsetConv gCharsetConv;
extern char* BASE16_TO_10_DATA;
extern char* ISXDIGIT_DATA;
extern char* ISDIGIT_DATA;

const static string EMPTY("");
const static string BLANK("BLANK");
const static string NOJSCRIPT("NOJSCRIPT");
const static string OTHER("OTHER");

using boost::lexical_cast;

CSVLineBase::CSVLineBase()
{
}

/**
 * Detect protocol version of this csv line
 */
CSVVersion CSVLineBase::log_parse_version(const string& line, string& LOGV)
{
   //FIXME The line is not be needed id uri does not lead with GET/...
   string uri = line.substr(0, line.find("\t", 0));
   string::size_type loc = uri.find("V", 0);
   if( loc != string::npos )
   {
      LOGV = line.substr(loc, 3);
      if ("V13" == LOGV)
      {
         this->version = CSVVERSION_SITE13;
      }
      else
      {
         this->version = CSVVERSION_BAD;
      }
   }
   else
   {
      this->version = CSVVERSION_BAD;
   }
   return this->version;
}

/**
 * Regexp to cut a url into sheme, hostname, path, file, query, and fragment.
 * Port and parameter are excluded.
 * Most of the CSV parsing time goes into executing this regexp.
 * @todo test url-parsing libraries, see if they're much faster
 */
//                                       /V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/
//FIXME The .* in ^.*V13 is not be needed id uri does not lead with GET/...
PCRE CSVLineBase::log_uri_regex_js_on("^.*V13.([^\\*]*)\\*([^\\*]*)\\*([^\\*]*)\\*([^\\*]*)\\*([^/]*)/([^/]*)/([^/]*)/([^/]*)/([^/]*)/([^/]*)/.*", PCRE_UTF8);
//                                        /V13a*[FixedURL]*[DocumentTitle]*[ReferrerURL]*[AccountName]/[CountryCode]/[EncType]/[TmSec]/
PCRE CSVLineBase::log_uri_regex_js_off("^.*V13.\\*([^\\*]*)\\*([^\\*]*)\\*([^\\*]*)\\*([^/]*)/([^/]*)/([^/]*)/([^/]*)/.*", PCRE_UTF8);

// This should be set to whatevert log_uri_regex_js_on.getMatchSize() returns
//int LOG_URI_REGEX_JS_MATCHSIZE = log_uri_regex_js_on.getMatchSize();
#define LOG_URI_REGEX_JS_ON_MATCHSIZE 33
#define LOG_URI_REGEX_JS_OFF_MATCHSIZE 24
#define MAX_FIELD_LENGTH 256

string CSVLineBase::truncate_field(string& s)
{
   if (s.length() > MAX_FIELD_LENGTH)
   {
      s = s.substr(0, MAX_FIELD_LENGTH -3);
      s += "...";
   }
   return s;
}

/**
 * Parse a url into its subcomponents (scheme, hostname, path, file, query, fragment)
 *
 * @param url_in the string to parse
 * @param offset tells where in this->fields to put the results
 * @todo The referrer special-cases are weird... They should be handled in a more generic way.
 *
 * TEST URI
 * see log_parse_UT()
 * V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/
 * logfields_ary[LOG_URI] = "/V13a*R>MyRefNewLoc**R>http://www.yandex.ru/yandsearch?text=%CC%E8%F1%F1+%E2%F1%E5%EB%E5%ED%ED%E0%FF+2006&stype=www*yandex_ru/ru/CP1251/tmsec=images_search/";
 */
bool CSVLineBase::log_parse_uri()
{
   int urlmatch[LOG_URI_REGEX_JS_ON_MATCHSIZE];
   const int num_matches_js_on = log_uri_regex_js_on.exec_r(logfields_ary[LOG_URI], urlmatch, LOG_URI_REGEX_JS_ON_MATCHSIZE) * 2;
   if (num_matches_js_on > 0)
   {
      urifields_ary[URI_JSCRIPT] = "true";
      // urlmatch is now an array of indexes we need to pull from string
      // 0 - begin
      // 1 - end
      // 2 - 22 index keys
      // substring form index1 -> (index2 - index1)
      int uri_fields_index = URI_LOC; 
      for (int i = 2;  i < num_matches_js_on; i+=2, uri_fields_index++)
      {
         //cout << logfields_ary[LOG_URI].substr(urlmatch[i], urlmatch[i+1] - urlmatch[i]) << endl;
         urifields_ary[uri_fields_index] = logfields_ary[LOG_URI].substr(urlmatch[i], urlmatch[i+1] - urlmatch[i]);
      }
   }
   else  //no js case
   {
      int urlmatch_js_off[LOG_URI_REGEX_JS_OFF_MATCHSIZE];
      // /V13a*[FixedURL]*[DocumentTitle]*[ReferrerURL]*[AccountName]/[CountryCode]/[EncType]/[TmSec]/
      const int num_matches_js_off = log_uri_regex_js_off.exec_r(logfields_ary[LOG_URI], urlmatch_js_off, LOG_URI_REGEX_JS_OFF_MATCHSIZE) * 2;
      //const int num_matches_js_off= log_uri_regex_js_off.getMatchSize();
      if (num_matches_js_off > 0)
      {
         urifields_ary[URI_JSCRIPT] = "false";
         //get furl
         //urifields_ary[URI_FURL] = logfields_ary[LOG_URI].substr(urlmatch_js_off[myindex], urlmatch_js_off[myindex+1] - urlmatch_js_off[myindex]);
         urifields_ary[URI_FURL] = logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_FURL_S], urlmatch_js_off[NOJS_FURL_E] - urlmatch_js_off[NOJS_FURL_S]);
         // fixedurl starts with "R>", so set location to furl
         if ("R>" == urifields_ary[URI_FURL].substr(0, 2))
         {
            urifields_ary[URI_FURL].erase(0, 2);
            urifields_ary[URI_LOC] = urifields_ary[URI_FURL];
         }
         else 
         {
            if (logfields_ary[LOG_HTTP_REF].empty())  // Browser blocked the referer
            {
               urifields_ary[URI_LOC] = "http://BLANK/";
            }
            // Normal case
            else
            {
               // Truncate location to MAX_FIELD_LENGTH
               urifields_ary[URI_LOC] = truncate_field(logfields_ary[LOG_HTTP_REF]);
               //cout << "AFT:" << urifields_ary[URI_LOC] << endl;
            }
         }

         urifields_ary[URI_DOCTITLE] 	= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_DOCTITLE_S], urlmatch_js_off[NOJS_DOCTITLE_E] - urlmatch_js_off[NOJS_DOCTITLE_S]);
         // Truncate location to MAX_FIELD_LENGTH
         //urifields_ary[URI_REFERRER] 	= truncate_field(logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_REFERRER_S], urlmatch_js_off[NOJS_REFERRER_E] - urlmatch_js_off[NOJS_REFERRER_S]));
         urifields_ary[URI_REFERRER] 	= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_REFERRER_S], urlmatch_js_off[NOJS_REFERRER_E] - urlmatch_js_off[NOJS_REFERRER_S]);
         // Referrer can be overriden by the user
         if ("R>" == urifields_ary[URI_REFERRER].substr(0, 2))
         {
            urifields_ary[URI_REFERRER].erase(0, 2);
         }
         else if (urifields_ary[URI_REFERRER].empty())
         {
            urifields_ary[URI_REFERRER] = "NOJSCRIPT";
         }
         //cout << "ITMP:"<<urifields_ary[URI_REFERRER] <<endl;
         urifields_ary[URI_TMAC]     	= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_TMAC_S], urlmatch_js_off[NOJS_TMAC_E] - urlmatch_js_off[NOJS_TMAC_S]);
         urifields_ary[URI_CC] 		= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_CC_S], urlmatch_js_off[NOJS_CC_E] - urlmatch_js_off[NOJS_CC_S]);
         urifields_ary[URI_ENCODEING] 	= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_ENCODING_S], urlmatch_js_off[NOJS_ENCODING_E] - urlmatch_js_off[NOJS_ENCODING_S]);
         string tmsec			= logfields_ary[LOG_URI].substr(urlmatch_js_off[NOJS_TMSEC_S], urlmatch_js_off[NOJS_TMSEC_E] - urlmatch_js_off[NOJS_TMSEC_S]);
         //Append the tmsec 
         if (! tmsec.empty() )
         {
            string::size_type loc = urifields_ary[URI_LOC].find("?");
            if ( loc != string::npos )
            {
               urifields_ary[URI_LOC] += "&" + tmsec;
            }
            else
            {
               urifields_ary[URI_LOC] += "?" + tmsec;
            }
         }
      }
      else
      {
         //Invalid Request
         return false;
      }
   }
/*   
   // chop strings larger than 256 chars ('URI_LOC' is handled separately)
   urifields_ary[URI_FURL]     = truncate_field(urifields_ary[URI_FURL]);
   urifields_ary[URI_DOCTITLE] = truncate_field(urifields_ary[URI_DOCTITLE]);
   urifields_ary[URI_REFERRER] = truncate_field(urifields_ary[URI_REFERRER]);
   cout << urifields_ary[URI_LOC] <<endl;
   cout << urifields_ary[URI_FURL] <<endl;
   cout << urifields_ary[URI_DOCTITLE] <<endl;
   cout << urifields_ary[URI_REFERRER] <<endl;
   cout << urifields_ary[URI_TMAC] <<endl;
   cout << urifields_ary[URI_CC] <<endl;
   cout << urifields_ary[URI_ENCODEING] <<endl;
*/
   return true;
}

void CSVLineBase::csv_put_version(const string& v)
{
   csvfields_ary[CSVFIELD_VERSION] = v;
   //cout << timestamp << endl;
}

void CSVLineBase::csv_put_timestamp()
{
   csvfields_ary[CSVFIELD_TIMESTAMP] = logfields_ary[LOG_TIME];
   //cout << timestamp << endl;
}

void CSVLineBase::csv_put_uri()
{
   csvfields_ary[CSVFIELD_JAVASCRIPT] 		= urifields_ary[URI_JSCRIPT]; 
   csvfields_ary[CSVFIELD_LOCATION] 		= urifields_ary[URI_LOC]; 
   csvfields_ary[CSVFIELD_FIXED_URL]		= urifields_ary[URI_FURL]; 
   csvfields_ary[CSVFIELD_DOCUMENT_TITLE]	= urifields_ary[URI_DOCTITLE]; 
   csvfields_ary[CSVFIELD_REFERRER]		= urifields_ary[URI_REFERRER]; 
   csvfields_ary[CSVFIELD_JAVA]			= urifields_ary[URI_JAVA]; 
   csvfields_ary[CSVFIELD_SCREEN]		= urifields_ary[URI_SCREEN]; 
   csvfields_ary[CSVFIELD_COLOR]		= urifields_ary[URI_COLOR]; 
   csvfields_ary[CSVFIELD_SCREEN]		= urifields_ary[URI_SCREEN]; 
   csvfields_ary[CSVFIELD_TMAC]			= urifields_ary[URI_TMAC]; 
   csvfields_ary[CSVFIELD_CC]			= urifields_ary[URI_CC]; 
   csvfields_ary[CSVFIELD_ENCODEING]		= urifields_ary[URI_ENCODEING]; 
}

void CSVLineBase::csv_put_ip()
{
   csvfields_ary[CSVFIELD_IP] = logfields_ary[LOG_IP];
   //cout << csvfields_ary[CSVFIELD_IP] <<endl;
}

void CSVLineBase::csv_put_user_agent()
{
  csvfields_ary[CSVFIELD_USER_AGENT] = logfields_ary[LOG_UA];
// cout << csvfields_ary[CSVFIELD_USER_AGENT] << endl;
}

void CSVLineBase::csv_put_lang()
{
   csvfields_ary[CSVFIELD_LANGUAGE] = logfields_ary[LOG_LANG];
//   cout << csvfields_ary[CSVFIELD_LANGUAGE] << endl;
}

void CSVLineBase::csv_put_proxy()
{
   csvfields_ary[CSVFIELD_PROXY] = logfields_ary[LOG_PROXY];
//   cout << csvfields_ary[CSVFIELD_PROXY] << endl;
}
//
//FIXME: do i need to validate the cookie length here?
void CSVLineBase::csv_put_cookie()
{
   if ( !logfields_ary[LOG_SID].empty() && !logfields_ary[LOG_CID].empty() )
   {
      csvfields_ary[CSVFIELD_SESSIONSTRING]	= logfields_ary[LOG_SID];
      csvfields_ary[CSVFIELD_USERSTRING]	= logfields_ary[LOG_CID];
      csvfields_ary[CSVFIELD_COOKIE]        	= "true:true";
   }
   else if ( !logfields_ary[LOG_SID].empty() )
   {
      csvfields_ary[CSVFIELD_SESSIONSTRING] 	= logfields_ary[LOG_SID];  
      csvfields_ary[CSVFIELD_USERSTRING] 	= string();
      csvfields_ary[CSVFIELD_COOKIE]        	= "true:false";
   }
   else if ( !logfields_ary[LOG_CID].empty() )
   {
      csvfields_ary[CSVFIELD_SESSIONSTRING] 	= string();
      csvfields_ary[CSVFIELD_USERSTRING]    	= logfields_ary[LOG_CID];
      csvfields_ary[CSVFIELD_COOKIE]        	= "false:true";
   }
   else
   {
      csvfields_ary[CSVFIELD_SESSIONSTRING] 	= string();
      csvfields_ary[CSVFIELD_USERSTRING] 	= string();
      csvfields_ary[CSVFIELD_COOKIE]        	= "false:false";   
   }
}

/**
 * Encode the elements with the supplied Encodeing
 * Ensure utf8
 */
void CSVLineBase::csv_encode()
{
   string encodeing = urifields_ary[URI_ENCODEING];
   if (! encodeing.empty() )
   {
      gCharsetConv.setFallback((char*)encodeing.c_str());
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_LOCATION]);
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_FIXED_URL]);
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_DOCUMENT_TITLE]);
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_REFERRER]);
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_USER_AGENT]);
      gCharsetConv.ensure_utf8(csvfields_ary[CSVFIELD_LANGUAGE]);
   }
}

unsigned long long debugstat_getSessionint(string& s, unsigned int start, unsigned int stop)
{
   unsigned long long sessionInt;
   char c[8];
   for (int k = 0; k < 8; k++)
      c[k] = '\0';
   int i = 0;
   stop = (stop > s.size() ? s.size() : stop);
   for (unsigned int j = start; j < stop; j+=2)
   {
      char x1 = s[j];
      char x2 = s[j+1];
      if (!(ISXDIGIT(x1) && ISXDIGIT(x2)))
      {
         sessionInt = 1;
         return sessionInt;
      }
      c[i] = (BASE16_TO_10(x1) * 16) + (BASE16_TO_10(x2));
      ++i;
   }
   sessionInt = *((unsigned long long*)c);
   if (sessionInt == 1)
   {
      sessionInt = 2;
   }
   return sessionInt;
}

void cookie_as_hex(char* buf, unsigned long long &tmp)
{
   //char buf[16];
   char * dict = "0123456789abcdef";
   for (int i = 0; i < 16; ++i)
   {
      buf[i] = dict[tmp%16];
      tmp /= 16;
   }
   buf[16] = '\0';
   //cout << buf<<endl ;
}

void CSVLineBase::csv_parse_cookies()
{
   // new 8 byte strings, strip time from user &  ensure user / session lengths
   //user
   if  ( csvfields_ary[CSVFIELD_USERSTRING].length() < 32 ) 
   {
      csvfields_ary[CSVFIELD_USERSTRING] = csvfields_ary[CSVFIELD_USERSTRING].substr(0,16);
   }
   else
   {
      string str_user = csvfields_ary[CSVFIELD_USERSTRING].substr(0,32);
      char buf[17];
      unsigned long long tmp;
      tmp = (debugstat_getSessionint(str_user, 0, 16) ^ debugstat_getSessionint(str_user, 16, 32));
      cookie_as_hex(buf, tmp);
      csvfields_ary[CSVFIELD_USERSTRING] = string(buf);
   }
   //session
   if ( csvfields_ary[CSVFIELD_SESSIONSTRING].length() < 32 )
   {
      csvfields_ary[CSVFIELD_SESSIONSTRING] = csvfields_ary[CSVFIELD_SESSIONSTRING].substr(0,16);
   }
   else
   {
      string str_sess = csvfields_ary[CSVFIELD_SESSIONSTRING];
      char buf[17];
      unsigned long long tmp;
      tmp = (debugstat_getSessionint(str_sess, 0, 16) ^ debugstat_getSessionint(str_sess, 16, 32));
      cookie_as_hex(buf, tmp);
      csvfields_ary[CSVFIELD_SESSIONSTRING] = string(buf);
   }
}
/*
buf[16]
char * dict = "0123456789abcdef";
for (int i = 16; i >= 0; --i)
{
  buf[i] = dict[tmp%16];
  tmp /= 16;
}
*/

#include <iostream>
#include <fstream>
map<string, fstream*> client_stream;

void CSVLineBase::csv_write_stream()
{
   //TODO: Close the streams celanly on timeblock rotation
   //Get the default TZ
   string tz = conf_obj.get_TZ(); 
   string cc = csvfields_ary[CSVFIELD_CC];
   Date date;
   //map<string, string>::iterator pos =  conf_obj.timezone_map.find(cc);
   if ( conf_obj.timezone_map.find(cc) != conf_obj.timezone_map.end() )
   {
      tz = conf_obj.timezone_map[cc];
   }
   //FIXME: This is not threadsafe, we wanna init the boundaries for all known timezones, rather than change this on every line. 
   //Try set the TZ
   if ( setenv("TZ", tz.c_str(), true) != 0 )
   {
      runtime_error_pf("Couldn't set timezone to '%s' (%s)", tz.c_str(), strerror(errno));
   }
   tzset(); 
   date.setDate(this->time, "epoch");
   
   string csv_name = conf_obj.get_csv_path() + csvfields_ary[CSVFIELD_TMAC] + "_" + date.str() + ".csv";
   if ( client_stream.end() == client_stream.find(csv_name) )
   {//FIXME: Test return from the open
      client_stream[csv_name] = new fstream;      
      client_stream[csv_name]->open(csv_name.c_str(), fstream::out |fstream::app);
   }
   if ( client_stream[csv_name]->is_open() )
   {
      string myline = csvfields_ary[CSVFIELD_VERSION];
      for (int i = CSVFIELD_TIMESTAMP; i < CSVFIELD_DATA_END; i++)
      {
         myline += "\t" + csvfields_ary[i];
      }
      //TODO: we wanna remove the flush below when we close files statically
      (*client_stream[csv_name]) << myline << endl;
      //(*client_stream[csv_name]) << myline << "\n"; //endl;
   }
   else
   {
      cout << "Error opening CSV file" << endl;
   }
}

bool CSVLineBase::ensure_timestamp(const string& timestamp)
{
   try
   {
      this->time = lexical_cast<time_t>(timestamp.substr(0, 10));
      this->microtime = lexical_cast<int>(timestamp.substr(10, 5));
   }
   catch (exception)
   {
      return false;
   }
   return true;
}

////////////////////////////////////////////////////////////////////////// UT methods////////////////////////////////////////////////////////////////
const string P = "PASS";
const string F = "*FAIL*";

/**
 * Pretty format for UT results()
 */
void print_UT(const char* fnc, const string& msg)
{
      cout << msg << ":\t" << fnc << endl;
}

/*
 * split_UT()
 * An empty field is represented by a '-' char
 * PASS on correct split 
 * PASS when replace '-' char with empty string
 */
void CSVLineBase::split_UT()
{
   string my_line ="[LOG_URI]	[LOG_TIME]	[LOG_HTTP_REF]	[LOG_IP]	[LOG_UA]	[LOG_LANG]	[LOG_PROXY]	[LOG_SID]	[LOG_CID]";
   size_t field_count;
   size_t reserve = 9;
   field_count = split(logfields_ary, my_line, "\t", 0, reserve);
   if ( "[LOG_URI]" 	== logfields_ary[LOG_URI]	&& 
	"[LOG_TIME]" 	== logfields_ary[LOG_TIME]	&& 
	"[LOG_HTTP_REF]"== logfields_ary[LOG_HTTP_REF] 	&& 
	"[LOG_IP]" 	== logfields_ary[LOG_IP] 	&& 
	"[LOG_UA]" 	== logfields_ary[LOG_UA]	&& 
	"[LOG_LANG]" 	== logfields_ary[LOG_LANG]	&& 
	"[LOG_PROXY]"	== logfields_ary[LOG_PROXY]	&& 
	"[LOG_SID]" 	== logfields_ary[LOG_SID]	&& 
	"[LOG_CID]" 	== logfields_ary[LOG_CID]
	)
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
   my_line ="[LOG_URI]	[LOG_TIME]	-	-	-	-	-	-	-";

   field_count = split(logfields_ary, my_line, "\t", 0, reserve);
   if ( "[LOG_URI]"     == logfields_ary[LOG_URI]       &&
        "[LOG_TIME]"    == logfields_ary[LOG_TIME]      &&
        EMPTY		== logfields_ary[LOG_HTTP_REF]  &&
        EMPTY		== logfields_ary[LOG_IP]        &&
        EMPTY		== logfields_ary[LOG_UA]        &&
        EMPTY		== logfields_ary[LOG_LANG]      &&
        EMPTY		== logfields_ary[LOG_PROXY]     &&
        EMPTY		== logfields_ary[LOG_SID]       &&
        EMPTY		== logfields_ary[LOG_CID]
        )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);

}

/**
 * log_parse_version_UT()
 * Test Version String parsing
 * Wrapper to CSVVersion log_parse_version(const string& line, string& LOGV)
 * - PASS when correct version returns
 * - FAIL on unsupported Version string
 */
void CSVLineBase::log_parse_version_UT()
{
   string pass_13a  = "V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/123454321 Http"; //JS TEST
   string pass_13b  = "V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/123454321 Http"; //JS TEST
   string fail_13a  = "V14a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/ Http"; //JS TEST
   string fail_13b  = "v14a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/ Http"; //JS TEST
   string fail_13c  = "-";
   string pass_a, pass_b, fail_a, fail_b, fail_c; 

   if ( CSVVERSION_SITE13 == log_parse_version(pass_13a, pass_a) && CSVVERSION_SITE13 == log_parse_version(pass_13a, pass_b) )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);

   if ( CSVVERSION_BAD == log_parse_version(fail_13a, fail_a) && CSVVERSION_BAD == log_parse_version(fail_13b, fail_b) && CSVVERSION_BAD == log_parse_version(fail_13c, fail_c))
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
}

/**
 * Helper method to log_parse_uri_UT()
 * populating of field logfields_ary[LOG_URI
 * Wrapper to bool log_parse_uri() 
 * - retrun true / false
 */
bool CSVLineBase::log_parse_uri(const string& ut)
{
   // erase elements from previous test
   for (int i = 0; i < URI_ENUM_SIZE; i++)
      urifields_ary[i] = string();
   logfields_ary[LOG_URI] = ut;
   return log_parse_uri();
}

/**
 * log_parse_uri_UT()
 * - Wrapper to bool log_parse_uri(const string& ut);
 *  Test JS on
 *  - PASS when correct fields and sperators are present
 *  - FAIL when a field is missing
 *  Test JS off
 *  - PASS when correcct fields and sperators are present
 *  - FAIL when a field is missing
*/

void CSVLineBase::log_parse_uri_UT()
{
   string pass_js_on  = "GET /V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode]/[EncType]/ Http"; //JS TEST
   string fail_js_on  = "GET /V13a[URL]*[FixedURL]*[DocumentTitle]*[Referrer]*[JavaSupport]/[ScreenSize]/[ColorDepth]/[AccountName]/[CountryCode] Http"; //JS TEST
   string pass_js_off_referrer = "GET /V13a*[FixedURL]*[DocumentTitle]*[ReferrerURL]*[AccountName]/[CountryCode]/[EncType]/[TmSec]/ Http"; //NO JS TEST
   string pass_js_off_no_referrer = "GET /V13a*[FixedURL]*[DocumentTitle]**[AccountName]/[CountryCode]/[EncType]/[TmSec]/ Http"; //NO JS TEST
   //string pass_js_off = "/V13a***http://www.yandex.ru/yandsearch?rpt=rad&text=%E3%E0%F1%F2%F0%EE%EB%E8+%F2%E5%E0%F2%F0%E0+%CB%E5%ED%EA%EE%EC*timeout_ru/ru/UTF-8/tmsec=timeout_total/"; 

   string fail_js_off = "GET /V13a*[FixedURL]*[DocumentTitle]*[ReferrerURL]*[AccountName]/[CountryCode]/[EncType]/ Http"; //NO JS TEST
   string pass_js_off_rloc = "/V13a*R>[FixedURL_ORIDE_LOC]*[DocumentTitle]*[ReferrerURL]*[AccountName]/[CountryCode]/[EncType]/[TmSec]/";
   string pass_js_off_rref = "/V13a*[FixedURL]*[DocumentTitle]*R>[ReferrerURL_ORIDE_Ref]*[AccountName]/[CountryCode]/[EncType]/[TmSec]/";
   // JS ON: Test matching field count
   if ( log_parse_uri(pass_js_on) && !log_parse_uri(fail_js_on) )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
   // JS OFF: Test matching field count
   if ( log_parse_uri(pass_js_off_referrer) && !log_parse_uri(fail_js_off) )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
   // JS ON: Test matching fields 
   log_parse_uri(pass_js_on);
   if ( "[URL]" 	 	== urifields_ary[URI_LOC] 	&&
        "[FixedURL]"	 	== urifields_ary[URI_FURL]	&&
        "[DocumentTitle]"	== urifields_ary[URI_DOCTITLE] 	&&
  	"[Referrer]" 		== urifields_ary[URI_REFERRER] 	&&
   	"[JavaSupport]"		== urifields_ary[URI_JAVA] 	&&
 	"[ScreenSize]"		== urifields_ary[URI_SCREEN] 	&&
 	"[ColorDepth]"		== urifields_ary[URI_COLOR] 	&&
	"[AccountName]"		== urifields_ary[URI_TMAC] 	&&
	"[CountryCode]"		== urifields_ary[URI_CC] 	&&
	"[EncType]"		== urifields_ary[URI_ENCODEING] &&
	"true"			== urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);

   // JS OFF: Test matching fields 
   logfields_ary[LOG_HTTP_REF] = "[HTTP_REFERRER]";
   //log_parse_uri(pass_js_off);
   log_parse_uri(pass_js_off_referrer);
   if ( "[HTTP_REFERRER]?[TmSec]" == urifields_ary[URI_LOC]       &&
        "[FixedURL]"            == urifields_ary[URI_FURL]      &&
        "[DocumentTitle]"       == urifields_ary[URI_DOCTITLE]  &&
        "[ReferrerURL]"         == urifields_ary[URI_REFERRER]  &&
        urifields_ary[URI_JAVA].empty()                         &&
        urifields_ary[URI_SCREEN].empty()                       &&
        urifields_ary[URI_COLOR].empty()                        &&
        "[AccountName]"         == urifields_ary[URI_TMAC]      &&
        "[CountryCode]"         == urifields_ary[URI_CC]        &&
        "[EncType]"             == urifields_ary[URI_ENCODEING] &&
        "false"                 == urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);
   logfields_ary[LOG_HTTP_REF].clear();

   log_parse_uri(pass_js_off_referrer);
   if ( "http://BLANK/?[TmSec]" == urifields_ary[URI_LOC]       &&
        "[FixedURL]"            == urifields_ary[URI_FURL]      &&
        "[DocumentTitle]"       == urifields_ary[URI_DOCTITLE]  &&
        "[ReferrerURL]"         == urifields_ary[URI_REFERRER]  &&
        urifields_ary[URI_JAVA].empty()                         &&
        urifields_ary[URI_SCREEN].empty()                       &&
        urifields_ary[URI_COLOR].empty()                        &&
        "[AccountName]"         == urifields_ary[URI_TMAC]      &&
        "[CountryCode]"         == urifields_ary[URI_CC]        &&
        "[EncType]"             == urifields_ary[URI_ENCODEING] &&
        "false"                 == urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);
   log_parse_uri(pass_js_off_no_referrer);
   if ( "http://BLANK/?[TmSec]" == urifields_ary[URI_LOC]       &&
        "[FixedURL]"            == urifields_ary[URI_FURL]      &&
        "[DocumentTitle]"       == urifields_ary[URI_DOCTITLE]  &&
        "NOJSCRIPT"             == urifields_ary[URI_REFERRER]  &&
        urifields_ary[URI_JAVA].empty()                         &&
        urifields_ary[URI_SCREEN].empty()                       &&
        urifields_ary[URI_COLOR].empty()                        &&
        "[AccountName]"         == urifields_ary[URI_TMAC]      &&
        "[CountryCode]"         == urifields_ary[URI_CC]        &&
        "[EncType]"             == urifields_ary[URI_ENCODEING] &&
        "false"                 == urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);

   log_parse_uri(pass_js_off_rloc);
   if ( "[FixedURL_ORIDE_LOC]?[TmSec]" == urifields_ary[URI_LOC]&&
        "[FixedURL_ORIDE_LOC]" == urifields_ary[URI_FURL]       &&
        "[DocumentTitle]"       == urifields_ary[URI_DOCTITLE]  &&
        "[ReferrerURL]"             == urifields_ary[URI_REFERRER]  &&
        urifields_ary[URI_JAVA].empty()                         &&
        urifields_ary[URI_SCREEN].empty()                       &&
        urifields_ary[URI_COLOR].empty()                        &&
        "[AccountName]"         == urifields_ary[URI_TMAC]      &&
        "[CountryCode]"         == urifields_ary[URI_CC]        &&
        "[EncType]"             == urifields_ary[URI_ENCODEING] &&
        "false"                 == urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);
   
   log_parse_uri(pass_js_off_rref);
   if ( "http://BLANK/?[TmSec]" 	== urifields_ary[URI_LOC]&&
        "[FixedURL]" 		== urifields_ary[URI_FURL]      &&
        "[DocumentTitle]"       == urifields_ary[URI_DOCTITLE]  &&
        "[ReferrerURL_ORIDE_Ref]"== urifields_ary[URI_REFERRER] &&
        urifields_ary[URI_JAVA].empty()                         &&
        urifields_ary[URI_SCREEN].empty()                       &&
        urifields_ary[URI_COLOR].empty()                        &&
        "[AccountName]"         == urifields_ary[URI_TMAC]      &&
        "[CountryCode]"         == urifields_ary[URI_CC]        &&
        "[EncType]"             == urifields_ary[URI_ENCODEING] &&
        "false"                 == urifields_ary[URI_JSCRIPT] )
   {
      print_UT(__PRETTY_FUNCTION__, P);
   }
   else
      print_UT(__PRETTY_FUNCTION__, F);
/*
   cout << urifields_ary[URI_LOC] <<endl;
   cout << urifields_ary[URI_FURL] <<endl;
   cout << urifields_ary[URI_DOCTITLE] <<endl;
   cout << urifields_ary[URI_REFERRER] <<endl;
   cout << urifields_ary[URI_JAVA] <<endl;
   cout << urifields_ary[URI_SCREEN] <<endl;
   cout << urifields_ary[URI_COLOR] <<endl;
   cout << urifields_ary[URI_TMAC] <<endl;
   cout << urifields_ary[URI_CC] <<endl;
   cout << urifields_ary[URI_ENCODEING] <<endl;
   cout << urifields_ary[URI_JSCRIPT] <<endl;
*/
}

/**
 * truncate_field_UT()
 * Test the truncation of fields longer than #define MAX_FIELD_LENGTH
 * Wrapper to string truncate_field(string& s)
 *  - PASS when correct truncation occurs
 *  - FAIL when no trucation occurs.
 *  Test shrt string has no truncation 
 *  - PASS when no change 
 *  - FAIL when a change occurs
 *
 */
void CSVLineBase::truncate_field_UT()
{
   // Generate an exceeded length string
   string pass_trunc = "http://www.a.com/";
   for (int i = 0; i <300; i++)
      pass_trunc += "b";
   pass_trunc += ".html";
   
   if ( MAX_FIELD_LENGTH == truncate_field(pass_trunc).length() )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
   // Generate a short string
   pass_trunc = "http://www.a.com/";
   if (pass_trunc.length() == truncate_field(pass_trunc).length() )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
}

/**
 * csv_put_cookie_UT()
 * Wrapper to helper csv_put_cookie(bool sid_b , bool cid_b) and act on return
 *  - PASS when all tests pass
 *  - FAIL on error
 */
void CSVLineBase::csv_put_cookie_UT()
{
   if ( csv_put_cookie(true, true) && csv_put_cookie(true, false) && csv_put_cookie(false, true) && csv_put_cookie(false, false) )
      print_UT(__PRETTY_FUNCTION__, P);
   else
      print_UT(__PRETTY_FUNCTION__, F);
}

/**
 * csv_put_cookie(bool, bool)
 * Helper method to csv_put_cookie_UT()
 * Wrapper to void csv_put_cookie()
 * Set the cookies based on the args passed in
 * Test the associated csvfields_ary[CSVFIELD_COOKIE] 
 *  - PASS when correct value 'pass' exists
 *  - FAIL on error
 */
bool CSVLineBase::csv_put_cookie(bool sid_b , bool cid_b)
{
   string sid_s = "0123456789ABCDEF0123456789abcdef";
   string cid_s = "0123456789abcdef0123456789ABCDEF";
   logfields_ary[LOG_SID] = string();
   logfields_ary[LOG_CID] = string();
   // Session & User
   if (sid_b && cid_b)
   { 
      string pass = "true:true";
      logfields_ary[LOG_SID] = sid_s;
      logfields_ary[LOG_CID] = cid_s;
      csv_put_cookie();
      if (pass == csvfields_ary[CSVFIELD_COOKIE])
         return true;
      else
         return false;
   }
   // Session only
   else if (sid_b)
   {
      string pass = "true:false";
      logfields_ary[LOG_SID] = sid_s;
      csv_put_cookie();
      if (pass == csvfields_ary[CSVFIELD_COOKIE])
         return true;
      else
         return false;
   }
   // User only
   else if (cid_b)
   {
      string pass = "false:true";
      logfields_ary[LOG_CID] = cid_s;
      csv_put_cookie();
      if (pass == csvfields_ary[CSVFIELD_COOKIE])
         return true;
      else
         return false;
   }
   // No Cookies
   else
      return true;
   return false;
}

/**
 * csv_parse_cookie_UT()
 * Test the cookie manipulation
 * Ensure handling of 16 & 32 char strings
 * Test the associated csvfields_ary[CSVFIELD_COOKIE] 
 *  - PASS when correct value 'pass' exists
 *  - FAIL on error
 */
void CSVLineBase::csv_parse_cookies_UT()
{
    string user16 = "000089A2F92C6702:1159361840";
    string sess16 = "000089A2F92C6702:timeifpresent";
    //if (csv_parse_cookies(user_32_xor, sess_32_xor))
    if ( csv_parse_cookies(user16, sess16) )
       print_UT(__PRETTY_FUNCTION__, P);
    else
       print_UT(__PRETTY_FUNCTION__, F);

    string user_32_xor = "03000000451A7530000089A2F92C6702:1159361840";
    string sess_32_xor = "03000000457FD9AB000089A274E71E02";
    if (csv_parse_cookies(user_32_xor, sess_32_xor))
       print_UT(__PRETTY_FUNCTION__, P);
    else
       print_UT(__PRETTY_FUNCTION__, F);
}

/**
 * csv_parse_cookies()
 * Test the cookie manipulation
 * Ensure handling of 16 & 32 char strings
 * Test the associated csvfields_ary[CSVFIELD_COOKIE] 
 *  - PASS when correct value 'pass' exists
 *  - FAIL on error
 */
bool CSVLineBase::csv_parse_cookies(string& utUser, string utSession)
{
  csvfields_ary[CSVFIELD_USERSTRING]    = utUser;
  csvfields_ary[CSVFIELD_SESSIONSTRING] = utSession;
  csv_parse_cookies(); 
  if ( utUser.length() < 32 && utSession.length() < 32)
  {
     if ( csvfields_ary[CSVFIELD_USERSTRING] == utUser.substr(0,16) && csvfields_ary[CSVFIELD_SESSIONSTRING] == utSession.substr(0,16) )
        return true;
     else
        return false;
  }
  else
  {
     if ( "3000982acb632123" == csvfields_ary[CSVFIELD_USERSTRING] && "3000982a13897c9a" == csvfields_ary[CSVFIELD_SESSIONSTRING] ) 
        return true; 
     else
        return false;
   }
}
