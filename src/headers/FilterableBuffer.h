/**
 * Declaration of FilterableBuffer class.
 * @file FilterableBuffer.h
 */

#ifndef FILTERABLEBUFFER_H
#define FILTERABLEBUFFER_H

#include "CSVBuffer.h"
#include "Filterable.h"


class FilterableBuffer 
{
public:
   FilterableBuffer(vector<string> csv_filenames);
   ~FilterableBuffer();
   shared_ptr<Filterable> get();
   CSVBuffer* csvbuffer;

private:
   vector<shared_ptr<Filterable> > filterables;
};

#endif // FILTERABLEBUFFER_H
