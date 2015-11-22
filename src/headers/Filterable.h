/**
 * Declaration of Filterable class
 * @file Filterable.h
 */

#ifndef FILTERABLE_H
#define FILTERABLE_H

#include <boost/shared_ptr.hpp>
#include "util.h"
#include "CSVLineBase.h"

class CSVLine;

class Filterable
{
public:
   Filterable();
   Filterable(const shared_ptr<CSVLine>& csvline);
   const CSVVersion version();
   const time_t time();
   const char   user_status(); 
   const string& referrer_scheme(); 
   const string& referrer_domain();
   const string& referrer_path();
   const string& referrer_file();
   const string& referrer_querystring();
   const string& referrer_fragment();
   const string& location_scheme();
   const string& location_domain();
   const string& location_file();
   const string& location_path();
   const string& location_querystring();
   const string& location_fragment();
   const string& fixed_url();
   const string& ip();
   const string& cc();
   const string& org();
   const string& rdns();
   const string& user_agent();

   const string& screen();
   const string& color();
   const string& proxy();
   const string& javascript();
   const string& cookie();
   const string& java();
   const string& language();

   const string& media_url();
   const string& media_agent();
   const string& media_author();
   const string& media_title();
   const string& media_desc();
   const string& media_campagin();
   const string& media_type();
   const string& media_duration();
   const string& media_event();
   const string& access_end();
   unsigned long long userint();
   unsigned long long sessionInt();
   string userint1();
   string sessionInt1();
  
   vector<int> filters_passed;
   vector<int> pages_passed;
   vector<int> querystrings_passed;


  const string& csv_time();

private:
   shared_ptr<CSVLine> csvline;
};

#include "CSVLine.h"

#endif // FILTERABLE_H
