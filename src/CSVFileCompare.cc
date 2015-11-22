/**
 * The CSVFileCompare class is used by CSVBuffer to sort its set<CSVFile> by who's got the oldest line.
 * @class CSVFileCompare
 */

/**
 * Implemetation of CSVFileCompare class.
 * @file CSVFileCompare.cc
 */

#include "headers/CSVFileCompare.h"

bool CSVFileCompare::operator()(CSVFile* const & csvfile1, CSVFile* const & csvfile2) const
{
   //return (csvfile1->csvline->csvfields[CSVLineBase::CSVFIELD_TIMESTAMP] < csvfile2->csvline->csvfields[CSVLineBase::CSVFIELD_TIMESTAMP]);
   if (csvfile1->csvline->time == csvfile2->csvline->time)
   {
      if (csvfile1->csvline->microtime != csvfile2->csvline->microtime)
         return (csvfile1->csvline->microtime < csvfile2->csvline->microtime);
      else
         return (csvfile1 < csvfile2);
   }
   else
   {
      return (csvfile1->csvline->time < csvfile2->csvline->time);
   }
}
