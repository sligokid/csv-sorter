/**
 * Initialisation
 * Signal handling
 *
 * @file init.cc
 * @bug basic Logging Streams are not thread safe.
 * @todo void initConfigLoad(const int argc, char* const * const argv);
 * @todo void initCentralDb();
 * @todo cleanup error handling
 * @todo Implement Thread Safe Logging
 */

#include <signal.h>
#include <sysexits.h>
#include <libgen.h>
#include <errno.h>
#include <boost/lexical_cast.hpp>
#include <confuse.h>
#include "headers/Config.h"
#include "headers/CSVLine.h"
#include "headers/CharsetConv.h"
#include "headers/FilterableBuffer.h"
#include "headers/util.h"
#include "headers/std.h"

// Declare Global Obects
Config  conf_obj;
FilterableBuffer* fb;
FILE *logF_p, *errF_p, *dbgF_p, *logerrF_p, *logfailF_p, *repF_p, *pid_file_p;

// Import Global objects
extern bool running;
extern bool do_printStatus;
extern CharsetConv gCharsetConv;

extern const int V_MAJOR;
extern const int V_MINOR;
extern const int V_BUILD;

void init_UT()
{
   CSVLine* UT = new CSVLine();
   UT->split_UT();
   UT->log_parse_version_UT();
   UT->truncate_field_UT();
   UT->log_parse_uri_UT();
   UT->csv_put_cookie_UT();
   UT->csv_parse_cookies_UT();
   delete UT;
}

/**
 * Signal handler; process.cc will pick it up to call printMemory()
 */
void sig_printStatus(const int signal __attribute__ ((unused)))
{
   do_printStatus = true;
}

/**
 * Signal handler; various parts of the program will pick it up to exit their loop.
 * Note we use cerr instead of log() here, as it is too complicated for signal handling.
 */
void sig_terminate(const int signal)
{
   cerr << "Received signal: " << signal << ", cleaning up\n";
   running = false;
}

void print_help(char *progname) 
{
   cout << "NAME:" << endl;
   cout << "\t" << progname << endl << endl;
   cout << "SYNOPSIS:" << endl;
   cout << "\t" << progname << " [-h|-v|-p|-?] config/sort.conf <date> <time>"<< endl << endl;
   cout << "OPTIONS:" << endl;
   cout << "\t-h show this help menu" << endl;
   cout << "\t-v show version info\n";
   cout << "\t-p process flag for a date in past" << endl;
   cout << "\t-? alias for -h" << endl << endl;
   cout << "ARGUMENTS:" << endl;
   cout << "\tconfig/sort.conf: program configuration file" << endl;
   cout << "\tdate: Process date in YYYYMMDD format" << endl;
   cout << "\ttime: Process time to start as seed in HH[00|15|30|45] format" << endl << endl;
}

void print_version()
{
   cout << "CSS Sort Version: " << V_MAJOR << "." << V_MINOR << "." << V_BUILD << endl;
#ifdef V2_COMPATIBILITY_MODE
   cout << "   V2 compatibility mode   enabled" <<endl;
#else
   cout << "   V2 compatibility mode   disabled" <<endl;
#endif
#ifdef USE_DEFINITION_CACHE
   cout << "   Definition cache        enabled" <<endl;
#else
   cout << "   Definition cache        disabled" <<endl;
#endif
#ifdef DO_DEBUG
   cout << "   Debug statements        enabled" <<endl;
#else
   cout << "   Debug statements        disabled" <<endl;
#endif
#ifdef DO_ASSERT
   cout << "   Extra runtime checks    enabled" <<endl;
#else
   cout << "   Extra runtime checks    disabled" <<endl;
#endif
}

/**
 * Inintialize config booleans from command line
 * These will be picked up via accessods through conf_obj
 * @param argc count of arguements passed on command line
 * @param argv pointer to an array of passed aguments
 */
void init_opts(const int argc, char* const * const argv)
{
   char optChar;
   opterr = 0;
   while ((optChar = getopt(argc, argv, "hpvx?DU")) != -1)
   {
      switch (optChar)
      {
         case 'h':
            print_help(argv[0]);
            // Fall through, print version and exit
         case 'v':
            print_version();
	    exit(0);

         case '?':
            print_help(argv[0]);
	    exit(0);

         case 'p':
            conf_obj.set_optRerun(true);
            break;
   
         case 'x':
            conf_obj.set_optExit(true);
            break;

         case 'D':
            conf_obj.set_optShowDebug(true);
            break;

         case 'U':
            init_UT();
	    exit(0);
            break;

         default:
           // Bo default opreation 
           break;
      }
   }
   if (argc - optind != 3)
   {
      print_help(argv[0]);
      exit(1);
   }
}

/**
 * Setup the signal handler and add listeners
 */
void init_signals()
{
   struct sigaction sa1, sa2, sa3;
   sigemptyset(&sa1.sa_mask);
   sigemptyset(&sa2.sa_mask);
   sigemptyset(&sa3.sa_mask);
   sa1.sa_flags = sa2.sa_flags = sa3.sa_flags = 0;
   sa1.sa_handler = sig_terminate;
   sa2.sa_handler = sig_printStatus;
   sa3.sa_handler = SIG_IGN;
   sigaction(SIGINT, &sa1, NULL);
   sigaction(SIGTERM, &sa1, NULL);
   sigaction(SIGUSR1, &sa2, NULL);
   sigaction(SIGHUP, &sa3, NULL);
}

/**
 * Remove client csvs from the pile if we are (re-)starting the day
 * @bug it prints an error to stderr on failure
 */
void csv_cleanup()
{
   if ( conf_obj.is_on_today() || conf_obj.is_optRerun() )
   {
/* NOT TZ aware, Cant do this in GMT
      if ( "0000" == conf_obj.get_seed() )
      {
         LOGF("INIT: CSV Cleanup");
         string rmcmd("rm ");
         format syscmd(rmcmd + conf_obj.get_csv_path() + "*" + conf_obj.get_processDate() + ".csv > /dev/null");
         try
         {
            int rc = system(syscmd.str().c_str());
            if ( rc != 0 )
            {
               throw runtime_error_pf("Warning Csv cleanup failed. %s", syscmd.str().c_str());
            }
         }
         catch (const exception &e)
         {
            ERROR("%s", e.what());
         }
      }
*/
   }
   else
   {
   // date in past
      cout << "ERROR: \tdate is in past" << endl;
      cout << "\tUse -p to override, or see -h for details" << endl <<endl;
      exit(1);
   }
}

/**
 * Temporary do loop, should be removed when filterable buffer is removed
 */
void do_main()
{
   LOGF("MAIN:");
   fb = new FilterableBuffer(conf_obj.get_csvFiles());
}

/**
 * Initalize a log file
 * @param suffix file extension
 * @param file_p pointer to a File handle
 */
void init_log(string suffix, FILE*& file_p)
{
   //string FileName = conf_obj.get_logLocation() + conf_obj.get_name() + "_" + conf_obj.get_cssFileNamePrefix() + suffix;
   string FileName = conf_obj.get_logLocation() + conf_obj.get_name() + "_" + conf_obj.get_processDate() + suffix;
   file_p = fopen(FileName.c_str(), "w");
   if (file_p == NULL)
      throw runtime_error_pf("Cannot write to %s : %s", FileName.c_str(), strerror(errno));
}

/**
 * Setup the logs 
 * Generate the Access log filename from the command line date and time
 * Various debug extras
 * @param argv pointer to an array of passed aguments
 */
void init_files(char* const * const argv)
{
   string pre_1 = string(argv[optind+1]);
   string pre_2 = string(argv[optind+2]);
   string fname = pre_1 + "-" + pre_2;
   conf_obj.set_cssFileNamePrefix(fname);
   // Open various files for log purposes
   init_log(".log", logF_p);       // Basic program flow and information
   init_log(".err", errF_p);       // Errors sysadmins/devs should be aware of
   init_log(".logerr", logerrF_p);    // Harmless Csv parsing errors
   init_log(".logfail", logfailF_p);    // Harmless Csv parsing errors
#ifdef DO_DEBUG
   init_log(".debug", dbgF_p);     // Verbose debug
   init_log(".replay", repF_p);    // Special-purpose, for stub functions
#endif
   // Log a "top" of ouselves at regular intervals
   string topfile = conf_obj.get_logLocation() + conf_obj.get_name() + "_" + conf_obj.processday.str() + ".top";
#ifdef __FreeBSD__
   string topcmd("top|grep '^ *%d '");
#else
   string topcmd("top -b -n1 -c|grep '^ *%d '");
#endif
   string datecmd("echo -n $(date '+%%Y-%%m-%%d %%H:%%M:%%S:')' '");
   format syscmd(datecmd+" > "+topfile+";while "+topcmd+" >> "+topfile+";do sleep 60;"+datecmd+" >> "+topfile+";done &");
   syscmd % getpid();
   system(syscmd.str().c_str());
}

/**
 * PID file control locking
 * Ensure no two instances or rerun is possible.
 * Create a file and write pid
 */
void set_pid(const string& timeblock)
{
   string FileName = conf_obj.get_name() + "_" + conf_obj.get_processDate() + ".pid";
   pid_file_p = fopen(FileName.c_str(), "r");
   if (pid_file_p != NULL)
   {
      cerr << "Error: PID file exists, Another instance running? :"<< FileName.c_str() << endl;
      throw runtime_error_pf("Error: PID file exists, Another instance running?: %s", FileName.c_str());
   }
   else
   {
      pid_file_p = fopen(FileName.c_str(), "w");
      fprintf(pid_file_p, "TIMEBLOCK: %s, PID: %d \n", timeblock.c_str(), getpid());
      fclose(pid_file_p);
   }
}

/**
 * PID file control unlocking
 * Remove pid file created by set_pid()
 */
void rm_pid()
{
   string FileName = conf_obj.get_name() + "_" + conf_obj.get_processDate() + ".pid";
   string rmcmd("rm ");
   format syscmd(rmcmd + FileName);
   int rc = system(syscmd.str().c_str());
   if ( rc != 0 )
   {
      cerr << "Error PID file creation failed. " << syscmd.str().c_str() << strerror(errno) <<endl;
      throw runtime_error_pf("Error PID file creation failed. %s", syscmd.str().c_str());
   }
}

/**
 * Main Setup
 * Initialise dates, timeblocks, via config object
 * @param argc count of arguements passed on command line
 * @param argv pointer to an array of passed aguments
 */
void init(int argc, char* const * const argv)
{
   try
   {
      logF_p = errF_p = dbgF_p = logerrF_p = logfailF_p = repF_p = stderr; // making sure those are usable early on
      init_opts(argc, argv);
      conf_obj.initConfig(argv);
      conf_obj.initDates(argv);
      init_files(argv);
      conf_obj.initTimeblocks(argv);
      init_signals();
      csv_cleanup();
   }
   catch (const exception &e)
   {
      ERROR("%s", e.what());
      ERROR("Program init failed");
      exit(1);
   }
}
