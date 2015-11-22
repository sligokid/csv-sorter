/**
 * @file process.cc
 *
 * Main process file for program
 *
 * Startup
 * Initialise Global Objects
 * Start / Stop Process Threads
 * Shutdown
 *   
 * @bug We get a Seg fault when config file doesnt exist
 */

#include <signal.h> ///< @todo remove when the "kill" hack is not needed any more
#include <sysexits.h>
#include <stdlib.h>
#include <time.h>
#include <confuse.h>
#include <sys/resource.h>
#include "headers/std.h"
#include "headers/FilterableBuffer.h"
#include "headers/Queue.h"
#include "headers/util.h"
#include "headers/Config.h"
#include "headers/CharsetConv.h"

int V_MAJOR = 1;
int V_MINOR = 0;
int V_BUILD = 5;

// Declare Global Obects
CharsetConv      gCharsetConv;

// Inport Global Obects
extern Config  conf_obj;
extern FilterableBuffer *fb;

// Local vars
bool running        = false;
bool do_printStatus = false;

// Function Prototypes
void init(int argc, char* const * const argv);
void set_pid(const string& timeblock);
void rm_pid();
void do_main();

void set_done_file()
{
   string flag_file = conf_obj.get_csv_path() + "sort_done_" + conf_obj.get_processDate() + ".csv";
   if ( NULL == fopen(flag_file.c_str(), "w") )
   {
      throw runtime_error_pf("Error: Cannot set process done flag: %s", flag_file.c_str());
   }
   LOGF("DONE: Setting Done Flag: %s", flag_file.c_str());
}
/**
 * Main processing loop
 * Initialise system based on passed arguments
 * @param argc count of arguements passed on command line
 * @param argv pointer to an array of passed aguments
 */
int main(int const argc, char * const * const argv)
{
   try
   {
      // Initialise program and threads
      running = true; /// @bug move this just before threads creation. running shouldnt need to be true before that.
      init(argc, argv);
      int daily_count = 0;
      for (vector<string>::iterator loop = conf_obj.timeblocks.begin(); loop != conf_obj.timeblocks.end(); loop++)
      {
//if (running)
//{
         string timeblock = *loop;
         set_pid(timeblock);
         LOGF("--------------------------------------------------------------------------------------------------");
         // used for overwriting files on restart
         conf_obj.set_cssFileNamePrefix(string(conf_obj.get_processDate() + "-" + timeblock));
         LOGF("MAIN: BEGIN\t TIMEBLOCK %s", timeblock.c_str());
         do_main(); 
         DEBUG("Start processing loop");

// We dont need this just using for counting
         if (int rc = pthread_create(&fb->csvbuffer->thread, NULL, &csvbuffer_run, fb->csvbuffer))
         {
            throw runtime_error_pf("Can't create CSV thread (pthread_create() returned %d)", rc);
         }
         // Read and process data
         shared_ptr<Filterable> f(new Filterable);

         int count = 0;
         while ((f = fb->get()) != NULL && running)
         {
            //cout << f->csv_time() <<endl;
            ++count;
            ++daily_count;
         }
         LOGF("MAIN: Processed %d log lines, %d log lines in total%s", count, daily_count, (running ? "" : " (interrupted before end)"));
// We dont need this just using for counting
      
         // Close queues and wait for reports to persist and finish
         TIME("MAIN: Closing queues");
         pthread_join(fb->csvbuffer->thread, NULL);

         // Print some stats
         string queuestat;
         queuestat += string("\tCSV reader\t")+fb->csvbuffer->newlines.stats().c_str()+"\n";
         DEBUG("Queue stats:\n%s", queuestat.c_str());
         fb->csvbuffer->PrintStats();
         // Cleanup
         conf_obj.clear_csvFiles();
         TIME("MAIN: Freeing memory");
         delete fb;
         LOGF("MAIN: END\t TIMEBLOCK %s", timeblock.c_str());  
//}
         rm_pid();
      }
      if (running)
      {
         set_done_file();
      }
   }
   catch (exception& e)
   {
      ERROR(e.what());
   }
}
