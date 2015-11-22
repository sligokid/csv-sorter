/**
 * Declaration of class CSVLine.
 * @file CSVLine.h
 */

#ifndef CSVLINE_H
#define CSVLINE_H

#include "CSVLineBase.h"

class CSVLine : public CSVLineBase
{
   //friend class CSVFile;
   //friend class CSVFileCompare;
   friend class Filterable;
  public:
   CSVLine();
   CSVLine(const string& line, unsigned int lineCount, const string& filename);
};

#endif // CSVLINE_H
