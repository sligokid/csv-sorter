/**
 * The Config class holds basic configuration data used throughout the program.
 *
 * @class Config
 * @bug date stuff here is not DST aware
 * @todo Create a DST Date class for all date related stuff.
 * @todo Load default config from DB
 * @todo Overriding config file parse object seems very restricitve.
 */

/**
 * Implementation of the Config class.
 * @file Config.cc
 */

#include <iostream>
#include <string>
#include "headers/Config.h"
#include <errno.h>
#include <sysexits.h>
#include "headers/util.h"
#include "headers/std.h"
#include "headers/Date.h"
#include <confuse.h>
#include <libgen.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/case_conv.hpp>

extern bool running;

/**
 * Contructor
 * Initialise _private members  
 */
Config::Config() : 
   _name(""), _processDate(""), _logLocation(""), _TZ(""),_encodeingFallback(""), _csv_path("csv/"), _optExit(false), _optShowDebug(false)
{
   timezone_map["dk"] = "Europe/Copenhagen";
   timezone_map["ee"] = "Europe/Tallinn";
   timezone_map["fi"] = "Europe/Helsinki";
   timezone_map["ie"] = "Europe/Dublin";
   timezone_map["il"] = "Asia/Jerusalem";
   timezone_map["jp"] = "Asia/Tokyo";
   timezone_map["lt"] = "Europe/Vilnius";
   timezone_map["lv"] = "Europe/Riga";
   timezone_map["no"] = "Europe/Oslo";
   timezone_map["ru"] = "Europe/Moscow";
   timezone_map["se"] = "Europe/Stockholm";
}

/**
 * Destructor
 */
Config::~Config()
{
   cfg_free(this->cfg);
}

/**
 * Initialise Default Config Seettings.
 * Overriding Config File Settings must have default here.
 * @todo These defaults Will be loaded from DB.
 */
void  Config::initConfig(char* const * const argv) 
{
   cfg_opt_t opts[] = 
   {
      CFG_STR_LIST("csv_locations",  "{}", CFGF_NONE),
      CFG_STR_LIST("csv_names",      "{}", CFGF_NONE),
      CFG_INT("csv_wait",            NULL, CFGF_NONE),
      CFG_INT("sort_sleep",            NULL, CFGF_NONE),
      CFG_STR("timezone",            "", CFGF_NONE),
      CFG_STR("name",                "", CFGF_NONE),
      CFG_STR("encoding_fallback",   "", CFGF_NONE),
      CFG_STR("log_location",        "", CFGF_NONE),
      CFG_STR("base_workdir",        "..", CFGF_NONE), // Relative paths are relative from the config file's location.
      CFG_STR("csv_path",            "csv/", CFGF_NONE),
      CFG_END()
   };
   this->cfg = cfg_init(opts, CFGF_NONE);
   const string configFile = argv[optind];

   // cd to conffile directory (so that include paths can be relative)
   char curdir[PATH_MAX];
   char *confdir;
   getcwd(curdir, PATH_MAX);
   confdir = strdup(configFile.c_str());
   chdir(dirname(confdir));
   free(confdir);

   // load and parse the config file
   char *filename = strdup(configFile.c_str());
   const int retval = cfg_parse(cfg, basename(filename));
   if (retval == CFG_FILE_ERROR)
   {
      throw runtime_error_pf("Error opening config file: %s: %s", configFile.c_str(), strerror(errno));
   }
   else if (retval == CFG_PARSE_ERROR)
   {
      /// @bug generally useless error, use better error reporting from confuse.
      throw runtime_error_pf("Error Parsing config file: %s: %s", configFile.c_str(), strerror(errno));
   }
   free(filename);

   // Cd to base workdir
   chdir(cfg_getstr(this->cfg, "base_workdir"));   

   // Set object _private members
   this->set_name(cfg_getstr(this->cfg, "name"));
   this->set_logLocation(cfg_getstr(this->cfg, "log_location"));
   this->set_encodeingFallback(cfg_getstr(this->cfg, "encoding_fallback"));
   this->set_csv_path(cfg_getstr(this->cfg, "csv_path"));
   this->set_csv_wait(cfg_getint(this->cfg, "csv_wait"));
   this->set_sort_sleep(cfg_getint(this->cfg, "sort_sleep"));
}

/**
 * Return a vector of CSV file names.
 */
#include <fstream>
#include <iostream>
vector<string> Config::get_csvFiles()
{
   if (this->_csvFiles.empty())
   {
      const int csvLocationCount = cfg_size(this->cfg, "csv_locations");
      const int csvNamesCount    = cfg_size(this->cfg, "csv_names");
      if (csvLocationCount == 0)
      {
         throw runtime_error_pf("No csv Locations Defined");
      }
      if (csvNamesCount == 0)
      {
         throw runtime_error_pf("No csv Files Defined");
      }
      for (int b = 0; b < csvNamesCount; ++b)
      {
         for (int a = 0; a < csvLocationCount; ++a)
         {
            int offset = a + 1;
            string fname = string(this->get_cssFileNamePrefix()) + string("-") + boost::lexical_cast<string>(offset) + string(".LOG");;
            string fpath = string(cfg_getnstr(this->cfg, "csv_locations", a )) + fname;
            std::ifstream filecheck;
            filecheck.open(fpath.c_str(), std::ifstream::in);
            // check if file is available
            if ( filecheck.is_open() )
            {
               LOGF("FILE: Loading %s for this timeblock", fpath.c_str());
               this->_csvFiles.push_back(fpath);
               filecheck.close();
            }
            // TODO If its on today & LIVE(wihin config delay) sleep bofore a retry
            else if(_on_today) 
            {
               filecheck.open(fpath.c_str(), std::fstream::in);
               if ( filecheck.is_open() )
               {
                  this->_csvFiles.push_back(fpath);
               }
               else 
               {
                  LOGF("WAIT: Ignoring %s from this timeblock", fpath.c_str());
               }
            }
            // date in past if file not here ignore
            else
            {
               LOGF("ERROR: Ignoring %s from this timeblock", fpath.c_str());   
            }
         }
      }
   }
   if (_on_today && this->_csvFiles.empty())
   {
      LOGF("SLEEP: No logs sleeping %d secs before retry", _sort_sleep); 
      int sleeptime = 0;
      while (running && (sleeptime < _sort_sleep))
      { 
         sleep(5);
         sleeptime+=5;
      }
      if (!running)
      {
         exit(1);
      }
      get_csvFiles();
   }
   return this->_csvFiles;
}
    
/**
G
 * Initialize all date-related items.
 * Note that after calling this function, local time is set according to config file.
 * @todo Remove all deprecated date objects
 */
void Config::initDates(char* const * const argv)
{
   // Local timezone
   if (setenv("TZ", cfg_getstr(this->cfg, "timezone"), true) != 0)
      runtime_error_pf("Couldn't set timezone to '%s' (%s)", cfg_getstr(this->cfg, "timezone"), strerror(errno));
   tzset();
   this->set_TZ(cfg_getstr(this->cfg, "timezone"));
   // Processing day/week/month
   try
   {
      this->processday.setDate(argv[optind+1]);
   }
   catch (...)
   {
      throw runtime_error_pf("Invalid processing date '%s': should be in YYYYMMDD format.", argv[optind+1]);
   }
   if (time(NULL) < this->processday.epoch_daybegin())
   {
      throw runtime_error_pf("Invalid processing date '%s': date is in the future.", argv[optind+1]);
   }
   Date::setSmjdOffset(argv[optind+1]);
   tm date_tm;
   time_t date_time = this->processday.epoch_daybegin();
   localtime_r(&date_time, &date_tm);
   this->processYesterday.setDate((this->processday.jd() - 1), "jd");
   this->processweek.setDate(this->processday.jd() - (date_tm.tm_wday ? date_tm.tm_wday - 1 : 6), "jd");
   this->processmonth.setDate(date_tm.tm_year + 1900, date_tm.tm_mon + 1, 1);

   // Various dates derived from processing day (all deprecated)
   this->processdate = days_since_epoch(&date_tm);
   this->set_processDate(argv[optind+1]);
  
   this->_on_today = (time(NULL) <= processday.epoch_dayend() && time(NULL) >= processday.epoch_daybegin()) ? true : false;

   this->date_str = this->processday.str();
   this->epoch = mktime(&date_tm);
   this->start_of_week_var = start_of_week(&date_tm);
   this->start_of_month_var = start_of_month(&date_tm);
   char buffer [9];
   time_t timeperiod_st = start_of_week_var * 86400;
   tm * timeperiod_tm = localtime(&timeperiod_st);
   sprintf(buffer, "%04d%02d%02d",timeperiod_tm->tm_year+1900, timeperiod_tm->tm_mon+1, timeperiod_tm->tm_mday );
   this->week_str = buffer;
   timeperiod_st = start_of_month_var * 86400;
   timeperiod_tm = localtime(&timeperiod_st);
   sprintf(buffer, "%04d%02d%02d",timeperiod_tm->tm_year+1900, timeperiod_tm->tm_mon+1, timeperiod_tm->tm_mday );
   this->month_str = buffer;
}

void Config::initTimeblocks(char* const * const argv)
{
   string seed = argv[optind + 2];
   set_seed(seed);
   LOGF("INIT: Seeding daemon for day %s with seed %s", _processDate.c_str(), seed.c_str());
   for (int h = 0; h < 24; h++)
   {
      for (int min = 0; min < 60; min < 60 ? min +=15 : min = 0)
      {
         std::stringstream tmp;
         if ( h < 10)
         {
            tmp << "0";
         }
//Fixme use strtol and compare int rather than string
         tmp << h ;
         if ( min == 0)
         {
            tmp << "0";
         }
         tmp << min;
         if (seed <= tmp.str())
         {
            this->timeblocks.push_back(tmp.str());
            string tmp_str = tmp.str();
            DEBUG("Loading timeblock %s", tmp_str.c_str());
         }
      }
   }
}

__attribute__ ((deprecated)) // Use the dst-aware date object
short Config::start_of_week(tm* data_date) 
{
   int days ;
   if (data_date->tm_wday == 0)
      days = 6;
   else 
      days = data_date->tm_wday - 1;
   short days_to_week_start = (short)(days_since_epoch(data_date) - days) ; 
   return days_to_week_start;
}

__attribute__ ((deprecated)) // Use the dst-aware date object
short Config::start_of_month(tm* data_date) 
{
   short days_to_month_start = (short)(days_since_epoch(data_date) - data_date->tm_mday + 1) ; 
   return days_to_month_start;
}

__attribute__ ((deprecated)) // Use the dst-aware date object
short Config::days_since_epoch(tm* data_date)
{
   time_t time = mktime(data_date);
   short shtime = (short)(time/86400);
   //@todo NEED TO ADD ONE...DAY IS ROUNDED DOWN. WE PREFER ROUNDED UP
   ++shtime;///< @bug not if division is exact...
   return shtime;
}
