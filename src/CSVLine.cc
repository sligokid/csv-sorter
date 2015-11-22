/**
 * Implemetation of CSVLine class
 * CSV lines are the basic unit of input data.
 * @class CSVLine
 * @todo not much use for this class, it should be merged with CSVLineBase
 * @file CSVLine.cc
 */

#include "headers/CSVLine.h"
#include "headers/CSVLineBase.h"
void log_fail_line(const string& line);
/**
 * Construct a CSV line from a string
 * Does some basic parsing and sanity checking
 * @param line, ref to a line from the csvbuffer
 * @param linecount, LOG line number
 * @param filename logfile name the line belongs to 
 * populates the logfields_ary[] array - data from the raw LOG
 * populates the urifields_ary[] array - data form the request URI
 * populates the csvfields_ary[] array - data parsed from both arrays above to be written to CSV
 */
CSVLine::CSVLine() {}

CSVLine::CSVLine(const string& line, unsigned int lineCount, const string& filename) : CSVLineBase()
{
   size_t expectedFieldCount, fieldCount, splitReserve;
   //DEBUG("new line %s", line.c_str());
   // Check version and split string
   validity = VALID;
   //string LOGV = "";
   string(LOGV);
   // populate the logfields[] array
   if ( CSVVERSION_BAD == log_parse_version(line, LOGV) ) 
   {
      validity = INVALID_VERSION;
      ERROR("invalid version '%s' at %s:%d", line.substr(0, 6).c_str(), filename.c_str(), lineCount);
      return;
   }
   switch (version)
   {
      case CSVVERSION_SITE13:
         expectedFieldCount = 9;
         splitReserve       = 9; // Must not be more than we need
         fieldCount = split(logfields_ary, line, "\t", 0, splitReserve);
         //cout << fieldCount <<endl;
         break;

      default:
         validity = INVALID_UNWANTED;
         return;
   }

   if ( fieldCount != expectedFieldCount )
   {
      validity = INVALID_FIELDCOUNT;
      ERROR("invalid field count (%zu instead of %d) at %s:%d", fieldCount, expectedFieldCount, filename.c_str(), lineCount);
      return;
   }
   //TODO: ensure the time is within the TIMEBLOCK
   if (! ensure_timestamp(logfields_ary[LOG_TIME]) )
   {
      validity = INVALID_TIMESTAMP;
      ERROR("invalid timestamp '%s' at %s:%d", logfields_ary[LOG_TIME].c_str(), filename.c_str(), lineCount);
      return;
   }
   //TODO: Verify this UA is not a konwn bot. Add UT for this
   //if (! ensure_useragent() )

   // populates the urifields_ary[]
   if (! log_parse_uri() )
   {
      validity = INVALID_LOCATION_URL;
      //cout << line << endl;
      ERROR("invalid uri '%s' at %s:%d", logfields_ary[LOG_URI].c_str(), filename.c_str(), lineCount);
//    Turn this off to save /usr
//      log_fail_line(line);
      return;
   }
   // populate the csvfields_ary[]
   csv_put_version(LOGV);
   csv_put_timestamp();	//validated from ensure_timestamp()
   csv_put_uri();	//validated form log_parse_uri()
   csv_put_ip();	//TODO validation
   csv_put_user_agent();//TODO validate form ensure_useragent()
   csv_put_lang();
   csv_put_proxy();
   csv_put_cookie();
   csv_encode();
   //array_dump(logfields_ary, LOG_ENUM_SIZE);
   //array_dump(urifields_ary, URI_ENUM_SIZE);
   //array_dump(csvfields_ary, CSVFIELD_DATA_END);
   csv_parse_cookies();
   //array_dump(csvfields_ary, CSVFIELD_ENUM_SIZE);
   csv_write_stream();
}
