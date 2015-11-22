/**
 * Declaration of CSVFile and CSVFileCompare classes.
 * @file CSVFile.h
 */

#ifndef CSVFILE_H
#define CSVFILE_H

#include "std.h"
#include "CSVLine.h"

#include <fstream>

class CSVFile : public std::ifstream
{
  public:
   CSVFile(int* linestats, const string& filename);
   ~CSVFile();
   bool getline();

   const string filename;
   shared_ptr<CSVLine> csvline;
   string line;
   unsigned int lineCount;

  private:
   /// Will sleep that many seconds before retrying a failed read
   static const unsigned int SLEEP_TIME = 60;
   /// Pointer to CSVBuffer::line_stats
   int* line_stats;
};

#endif // CSVFILE_H
