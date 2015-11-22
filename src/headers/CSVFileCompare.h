/**
 * Declaration of CSVFileCompare class.
 * @file CSVFileCompare.h
 */

#ifndef CSVFILECOMPARE_H
#define CSVFILECOMPARE_H

#include <boost/shared_ptr.hpp>

class CSVFile;

struct CSVFileCompare 
{
   bool operator()(CSVFile* const & csvfile1, CSVFile* const & csvfile2) const;
};

#include "CSVFile.h"

#endif // CSVFILECOMPARE_H
