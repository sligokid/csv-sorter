/**
 * Wrapper around CSVBuffer.
 * @class FilterableBuffer
 */

/**
 * Implementation of FilterableBuffer class.
 * @file FilterableBuffer.cc
 */

#include "headers/FilterableBuffer.h"


FilterableBuffer::FilterableBuffer(vector<string> csv_filenames) 
  : csvbuffer(new CSVBuffer(csv_filenames))
{
}

FilterableBuffer::~FilterableBuffer()
{
   DEBUG("Destructing FilterableBuffer");
   delete this->csvbuffer;
}

shared_ptr<Filterable> FilterableBuffer::get()
{
   if (filterables.empty())
   {
      shared_ptr<CSVLine> csvline(csvbuffer->getline());
      if (csvline) 
      {
         filterables = vector<shared_ptr<Filterable> >(1, shared_ptr<Filterable>(new Filterable(csvline)));
         return get();
      }
      else 
      {
         return shared_ptr<Filterable>((Filterable*)NULL);
      }
   }
   else
   {
      shared_ptr<Filterable> f(filterables.back());
      filterables.pop_back();
      return f;
   }
}
