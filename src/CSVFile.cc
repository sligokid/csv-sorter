/**
 * The CSVFile class is a wrapper around ifstream to read lines from a CSV file.
 * Each instance represents one CSV file to read from.
 *
 * @class CSVFile
 */

/**
 * Implementation of the CSVFile class.
 * @file CSVFile.cc
 */

#include <sysexits.h>
#include "headers/CSVFile.h"

extern bool running;

CSVFile::CSVFile(int* linestats, const string& fname) :
  filename(fname), csvline(shared_ptr<CSVLine>((CSVLine*)NULL)), lineCount(0), line_stats(linestats)
{
   DEBUG("Constructing CSVFile '%s'", this->filename.c_str());
   this->open(this->filename.c_str());
   if (!this->is_open())
      throw runtime_error_pf("couldn't open csvfile \"%s\"", this->filename.c_str()); ///< @todo show system error
   
   this->exceptions(std::ifstream::badbit|std::ifstream::eofbit|std::ifstream::failbit);
}

CSVFile::~CSVFile()
{
   DEBUG("Destructing CSVFile '%s' (%d lines read)", this->filename.c_str(), this->lineCount);
   this->close();
}

/**
 * Read a line from the file.
 * Will retry until a valid line comes in, or EOF is reached.
 * Note that a lot of methods are inherited from its parent class 'ifstream'.
 * Note that we assume lines are writen atomically to the file, so that we cannot get EOF in the middle of a line.
 *
 * @return false if EOF is reached without having parsed a correct line (parsed line can be found in this->csvline and this->line)
 * @todo Check the code's behaviour under read-error conditions (NFS down, etc)
 * @todo why do we only clear() for eof-type error ?
 */
bool CSVFile::getline()
{
   bool tryagain = true;

   // Keep trying until we get a correct line
   while (tryagain && running)
   {
      try
      {
         this->line.clear();
         std::getline(*this, this->line, '\n');
         this->lineCount++;
         this->csvline = shared_ptr<CSVLine>(new CSVLine(this->line, this->lineCount, this->filename));
         ++this->line_stats[this->csvline->validity];
         tryagain = (this->csvline->validity != VALID);
      }
      catch (std::ifstream::failure& e)
      {
         if (std::ifstream::bad())
         {
            // see also nfs configuration
            ERROR("Bad read from csvfile \"%s\": %s", this->filename.c_str(), e.what());
            sleep(SLEEP_TIME);
         }
         else if (std::ifstream::eof())
         {
            if (line.empty())
            {
               clear();
               return false;
            }
         }
         else
         {
            // see also nfs configuration
            ERROR("Failed read from csvfile \"%s\": %s", this->filename.c_str(), e.what());
            sleep(SLEEP_TIME);
         }
      }
   }      
   return true;
}
