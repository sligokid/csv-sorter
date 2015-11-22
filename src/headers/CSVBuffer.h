/**
 * Declaration of CSVBuffer class.
 * @file CSVBuffer.h
 */

#ifndef CSVBUFFER_H
#define CSVBUFFER_H

class CSVBuffer;

#include "std.h"
#include "CSVFile.h"
#include "CSVFileCompare.h"
#include "CSVLine.h"
#include "Queue.h"

#include <list>

class CSVBuffer 
{
   friend void* csvbuffer_run(void* arg);

public:
   CSVBuffer(vector<string>& csv_filenames);
   ~CSVBuffer();
   void                  PrintStats();
   shared_ptr<CSVLine>   getline();

   /// Stores line count by CSVValidity
   int line_stats[VALIDITY_ENUM_SIZE];
   Queue<shared_ptr<CSVLine> > newlines;
   /// when csv_files are being read "live" (on the same day that they are being created),
   /// sleep this many seconds after the end of the file has been reached in one of the csv_files.
   static const unsigned int SLEEP_TIME = 300;
   pthread_t thread;

private:
   shared_ptr<CSVLine>   prepline();

   set<CSVFile*, CSVFileCompare>   csv_files; // Sort fuction fo this set is CSVFileCompare
   list<CSVFile*>                  csv_files_slow;
   /// The number of seconds after a day is actually over to wait before csv_files are closed.
   static const time_t DAY_PADDING = 60;
   /// The end-of-day timestamp
   time_t csv_close_time;
   /// char* names for CSVBuffer::line_stats
   char* STATUS_NAMES[VALIDITY_ENUM_SIZE];
   static const size_t QUEUE_SIZE = 5000;
};

void* csvbuffer_run(void* arg);

#endif // CSVBUFFER_H
