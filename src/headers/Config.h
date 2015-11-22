/**
 * Declaration (and som implementation) of the Config class.
 * @file Config.h
 * @todo if we dont intend to do anything special, these set/get/is methods are useless, we should rather make the member public.
 */

#ifndef Config_H
#define Config_H

#include "std.h"
#include "Date.h"
#include <confuse.h>

class Config
{
public:
   Config();
   ~Config();

   void initConfig(char* const * const argv);
   void initDates(char* const * const argv);
   void initTimeblocks(char* const * const argv);

   void set_name(const string s)		{ _name = s; }
   void set_processDate(const string s)       	{ _processDate = s; }
   void set_logLocation(const string s)       	{ _logLocation = s; }
   void set_TZ(const string s)                	{ _TZ = s; }
   void set_encodeingFallback (const string s)	{ _encodeingFallback = s; }
   void set_cssFileNamePrefix(const string& s) 	{ _cssFileNamePrefix = s ; }
   void set_seed(const string& s) 		{ _seed = s ; }
   void set_csv_path(const string& s)         	{ _csv_path = s ; }
   void set_csv_wait(const int& x)         	{ _csv_wait = x ; }
   void set_sort_sleep(const int& x)         	{ _sort_sleep = x ; }
   void set_optExit         (bool o) 		{ _optExit = o; }
   void set_optShowDebug    (bool o) 		{ _optShowDebug = o; }
   void set_optRerun    (bool o) 		{ _optRerun = o; }
   void clear_csvFiles() 			{ _csvFiles.clear(); }
  
   const string& get_name()                	{ return _name; }
   const string& get_processDate()        	{ return _processDate; }
   const string& get_logLocation()        	{ return _logLocation; }
   const string& get_TZ()                  	{ return _TZ; }
   const string& get_dbConnectionString()  	{ return _dbConnectionString; }
   const string& get_encodeingFallback()   	{ return _encodeingFallback; }
   const string& get_cssFileNamePrefix()       	{ return _cssFileNamePrefix; }
   const string& get_csv_path()            	{ return _csv_path; }
   const string& get_seed()	            	{ return _seed;}
   
   bool is_on_today()				{ return _on_today; }
   bool is_optRerun()				{ return _optRerun; }
   bool is_optExit()           { return _optExit; }
   bool is_optShowDebug()      { return _optShowDebug; }

   vector<string> get_csvFiles();

   /// @todo use processday, processweek, and processmonth instead of these.
   short __attribute__ ((deprecated)) processdate;
   short __attribute__ ((deprecated)) start_of_week_var;
   short __attribute__ ((deprecated)) start_of_month_var;
   int __attribute__ ((deprecated)) epoch;
   string __attribute__ ((deprecated)) date_str;
   string __attribute__ ((deprecated)) week_str;
   string __attribute__ ((deprecated)) month_str;
   Date processYesterday;
   Date processday;
   Date processweek;
   Date processmonth;

   cfg_t* cfg;
   map<string, string> timezone_map;
   vector<string> timeblocks;

private:
   short start_of_week(tm* data_date);
   short days_since_epoch(tm* data_date);
   short start_of_month(tm* data_date);
   
   string _name;
   string _processDate;
   string _logLocation;
   string _TZ;
   string _dbConnectionString;
   string _encodeingFallback;
   string _backupDataDir;
   string _cssFileNamePrefix;
   string _csv_path;
   string _seed;

   int _csv_wait;
   int _sort_sleep;
   int _persistFrequency;
   int _user_purge_days;
   int _detail_limit_days;
    
   bool _on_today;          
   bool _optInitClient;
   bool _optVacuum;
   bool _optVacuumFull;
   bool _optAnalyze;
   bool _optCluster;
   bool _optReIndex;
   bool _optWriteSession;
   bool _optExit;
   bool _optShowDebug;
   bool _optUnitTest;
   bool _optRerun;

   vector<string> _csvFiles;
};

#endif // Config_H
