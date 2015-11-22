/**
 * Provides a Queue of parsed CSVLine, kept full and ordered.
 * Runs in it own thread.
 * @class CSVBuffer
 */

/**
 * Implementation of CSVBuffer class
 * @file CSVBuffer.cc
 */

#include <sysexits.h>
#include <iostream>
#include "headers/CSVBuffer.h"
#include "headers/CSVLineBase.h"
#include "headers/util.h"
#include "headers/Config.h"

extern bool running;
extern Config conf_obj;

CSVBuffer::CSVBuffer(vector<string>& csv_filenames)
{
   // Various inits
   for (int i = 0; i < VALIDITY_ENUM_SIZE; ++i)
      this->line_stats[i] = 0;
   this->STATUS_NAMES[VALID] = "VALID";
   this->STATUS_NAMES[INVALID_VERSION] = "INVALID_VERSION";
   this->STATUS_NAMES[INVALID_UNWANTED] = "INVALID_UNWANTED";
   this->STATUS_NAMES[INVALID_FIELDCOUNT] = "INVALID_FIELDCOUNT";
   this->STATUS_NAMES[INVALID_TIMESTAMP] = "INVALID_TIMESTAMP";
   this->STATUS_NAMES[INVALID_LOCATION_URL] = "INVALID_LOCATION_URL";
   this->STATUS_NAMES[INVALID_IP] = "INVALID_IP";

   // Open every file and read their first line
   for(vector<string>::iterator i = csv_filenames.begin(); i != csv_filenames.end(); ++i)
   {
      CSVFile* newCSVFile = new CSVFile(this->line_stats, *i);
      if (newCSVFile->getline())
         this->csv_files.insert(newCSVFile);
      else
         this->csv_files_slow.push_back(newCSVFile);
   }

   // Set end of day timestamp
   this->csv_close_time = conf_obj.processday.epoch_dayend() + CSVBuffer::DAY_PADDING;
   DEBUG("%s", ((time(NULL) <= conf_obj.processday.epoch_dayend() && time(NULL) >= conf_obj.processday.epoch_daybegin()) ? "Running live" : "Not running live"));
}

CSVBuffer::~CSVBuffer()
{
   // Close queue
   newlines.close();

   // Close remaining CSVFiles
   if (!this->csv_files.empty() || !this->csv_files_slow.empty())
   {
      DEBUG("Closing remainging CSVFiles.");
      while (!this->csv_files.empty())
      {
         delete *(this->csv_files.begin());
         this->csv_files.erase(this->csv_files.begin());
      }
      while (!this->csv_files_slow.empty())
      {
         delete *(this->csv_files_slow.begin());
         this->csv_files_slow.erase(this->csv_files_slow.begin());
      }
   }
}

shared_ptr<CSVLine> CSVBuffer::getline()
{
   return newlines.get();
}

/**
 * Print some CSV statistics
 *
 * @todo return string instead of printing
 */
void CSVBuffer::PrintStats()
{
   int invalid = 0;
   for (int i = 1; i < VALIDITY_ENUM_SIZE; ++i)
   {
      invalid += this->line_stats[i];
      DEBUG("%-20s%8i", this->STATUS_NAMES[i], this->line_stats[i]);
   }
   int valid = this->line_stats[VALID];
   DEBUG("%-20s%8i", "INVALID", invalid);
   DEBUG("%-20s%8i", "VALID", valid);
   DEBUG("%-20s%8i", "READ", invalid + valid);
}

/**
 * Read lines from the csvfile objects, in a sorted order.
 * - csv_files holds all CSVfile that currently have data (sorted via CSVFileCompare)
 * - csv_files_slow holds files that have no data for now but that we must keep reading
 * @return pointer to next CSVLine (null if all CSVFile have been completely read)
 */
shared_ptr<CSVLine> CSVBuffer::prepline()
{
   // Return null pointer if all files are finished
   if (csv_files.empty() && csv_files_slow.empty())
   {
      DEBUG("No more files to read from.");
      return shared_ptr<CSVLine>((CSVLine*)NULL);
   }
   // Retry the slow files once if we have some
   if (!csv_files_slow.empty())
   {
      for (list<CSVFile*>::iterator i = csv_files_slow.begin(); i != csv_files_slow.end(); ++i)
         if ((*i)->getline())
         {
            DEBUG("%s has data again (got %d until now).", (*i)->filename.c_str(), (*i)->lineCount);
            CSVFile* csvfile = *i;
            csv_files.insert(csv_files.end(), csvfile);
            csv_files_slow.erase(i);
         }
   }
   // Retry the slow files indefinitely while no data is available, but return NULL if this is the end of the day
   while (csv_files.empty())
   {
      for (list<CSVFile*>::iterator i = csv_files_slow.begin(); i != csv_files_slow.end(); ++i)
         if ((*i)->getline())
         {
            DEBUG("%s has data again (got %d until now).", (*i)->filename.c_str(), (*i)->lineCount);
            CSVFile* csvfile = *i;
            csv_files.insert(csv_files.end(), csvfile);
            csv_files_slow.erase(i);
         }
      if (csv_files.empty())
      {
         if (time(NULL) > csv_close_time)
         {
            DEBUG("No more files to read from.");
            return shared_ptr<CSVLine>((CSVLine*)NULL);
         }
         else
         {
            //LOGF("Sleeping %d seconds to wait for CSV lines in any file.", SLEEP_TIME);
            return shared_ptr<CSVLine>((CSVLine*)NULL);
            //sleep(SLEEP_TIME);
         }
      }
   }
   
   // Get the first CSVFile of the set (which contains the oldest CSV line thanks to container sorting)
   const set<CSVFile*>::iterator csvfile_iter = csv_files.begin();
   CSVFile* csvfile = *csvfile_iter;

   // Grab a pointer to the CSVLine created by the chosen CSVFile
   const shared_ptr<CSVLine> newline(csvfile->csvline);
//FIXME remove this
   if (!csvfile->getline())
   {
      if (time(NULL) > csv_close_time)
      {
         // That was the last line from this CSVFile - remove CSVFile
         DEBUG("No more lines in %s", csvfile->filename.c_str());
         LOGF("Log: %s: Processed %d", csvfile->filename.c_str(), csvfile->lineCount);
         delete csvfile;
         csv_files.erase(csvfile_iter);
         // No lines right now - wait a bit
      }
      else
      {
         // No lines right now - wait a bit
         LOGF("Log: %s: Processed %d", csvfile->filename.c_str(), csvfile->lineCount);
         //sleep(SLEEP_TIME);
         if (!csvfile->getline())
         {
            // No lines coming but other files might have data - put the file in the "slow" list
            DEBUG("%s is too slow, puting it aside until it receives data", csvfile->filename.c_str());
            csv_files.erase(csvfile_iter);
            csv_files_slow.push_back(csvfile);
         }
      }
   }
   else
   {
      // More lines coming from this CSVFile - Reinsert the CSVFile in the set, so that it gets sorted again
      csv_files.erase(csvfile_iter);
      csv_files.insert(csv_files.end(), csvfile);
   }

   //DEBUG("got line %d from '%s'", csvfile->lineCount, csvfile->filename.c_str());
   return newline;
}

/**
 * Main() function for the CSV thread.
 *
 * It reads lines into the 'newline' Queue, which is then used by the main thread.
 * It'll stop when running is false or when prepline() returns NULL.
 */
void* csvbuffer_run(void* arg)
{
   CSVBuffer& csvbuffer = *(CSVBuffer*)arg;

   while (running)
   {
      // Grab a pointer to a CSVLine
      shared_ptr<CSVLine> line(csvbuffer.prepline()); // Using pointer copy constructor
      if (line)
      {
         // Got a CSVLine - enqueue it
         csvbuffer.newlines.put(line);
      }
      else
      {
         // No more CSVLines - Close queue and exit loop
         csvbuffer.newlines.close();
         break;
      }
   }
   return NULL;
}
