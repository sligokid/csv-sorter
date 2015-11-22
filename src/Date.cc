/**
 * The Date class handles dates as string, unix timestamp, julian day.
 * All calendar dates are local time (as set by tzset()). Epoch dates are utc by nature.
 * @class Date
 * @todo better invalid dates cheking
 */

/**
 * Implementation of Date class.
 * @file Date.cc
 */

#include "headers/Date.h"
#include "headers/Config.h"
#include "headers/util.h"

extern Config conf_obj;

static int SmjdOffset = -1;

////////////////////////////////////////////////////////////////////////////////
// Initialisation functions
////////////////////////////////////////////////////////////////////////////////

Date::Date(string d)
{
   setDate(d);
}

Date::Date(int d, string mode)
{
   setDate(d, mode);
}

/**
 * Set the date from a YYYYMMDD string.
 */
void Date::setDate(string d)
{
   setDate(GregorianToJD(d), "jd");
}

/**
 * Set the date from year, month, day ints.
 */
void Date::setDate(int y, int m, int d)
{
   setDate(GregorianToJD(y, m, d), "jd");
}

/**
 * Set the date from an int.
 * Note that "epoch" dates are truncated to the start of the day.
 * @param d input date
 * @param mode interpret \e d as \e epoch, \e jd, or \e smjd
 */
void Date::setDate(int d, string mode)
{
   if ("epoch" == mode)
   {
      tm t1;
      time_t t2 = d;
      localtime_r(&t2, &t1);
      this->jd_ = Date::GregorianToJD(t1);
   }
   else if ("jd" == mode)
   {
      this->jd_ = d;
   }
   else if ("smjd" == mode)
   {
      this->jd_ = Date::SmjdToJd(d);
   }
   else
   {
      throw runtime_error_pf("Unknown date mode \"%s\"", mode.c_str());
   }
   this->epoch_daybegin_ = Date::JDToGregorian_epoch_daybegin(this->jd_);
   this->epoch_dayend_ = Date::JDToGregorian_epoch_dayend(this->jd_);
   this->str_ = Date::JDToGregorian_str(this->jd_);
}

/**
 * Initialise Date class's smjd-related functions.
 * You should call this once at the beginning of the program, with a
 * date close enough to dates handled by the program (e.g. the processing date).
 */
void Date::setSmjdOffset(string date)
{
   if (SmjdOffset != -1)
      throw runtime_error("Date::setSmjdOffset() called more than once");
   SmjdOffset = GregorianToJD(date) - SMJD_REFERRENCEDATE;
}

////////////////////////////////////////////////////////////////////////////////
// retrieval functions
////////////////////////////////////////////////////////////////////////////////

string Date::str()
{
   return this->str_;
}

time_t Date::epoch_daybegin()
{
   return this->epoch_daybegin_;
}

time_t Date::epoch_dayend()
{
   return this->epoch_dayend_;
}

int Date::jd()
{
   return this->jd_;
}

smjd_t Date::smjd()
{
   return Date::JdToSmjd(this->jd_);
}

/**
 * Convert Julian Day to SiteMetrix Julian Day.
 */
smjd_t Date::JdToSmjd(int jd)
{
   if (SmjdOffset == -1)
      throw runtime_error("Need to call Date::setSmjdOffset() first");
   if (jd - SmjdOffset < 0 || jd - SmjdOffset > 255)
      throw runtime_error_pf("Julian day '%d' out of range for smjd", jd);
   return jd - SmjdOffset;
}

/**
 * Convert SiteMetrix Julian Day to Julian Day.
 */
int Date::SmjdToJd(smjd_t smjd)
{
   if (SmjdOffset == -1)
      throw runtime_error("Need to call Date::setSmjdOffset() first");
   return smjd + SmjdOffset;
}

/**
 * Return the Julian Day from a Gregorian date (supplied as ints).
 * Formula taken from http://hermetic.nofadz.com/cal_stud/jdn.htm
 */
int Date::GregorianToJD(int y, int m, int d)
{
   if (m < 1 || m > 12 || d < 1 || d > 31 || y < 1900 || y > 2100)
      throw runtime_error_pf("Date %d/%d/%d is out of bounds in GregorianToJD().", y, m, d);
   return ( 1461 * ( y + 4800 + ( m - 14 ) / 12 ) ) / 4 +
      ( 367 * ( m - 2 - 12 * ( ( m - 14 ) / 12 ) ) ) / 12 -
      ( 3 * ( ( y + 4900 + ( m - 14 ) / 12 ) / 100 ) ) / 4 +
      d - 32075;
}

/**
 * Return the Julian Day from a Gregorian date (supplied as tm).
 */
int Date::GregorianToJD(tm date)
{
   return GregorianToJD(date.tm_year + 1900, date.tm_mon + 1, date.tm_mday);
}

static PCRE reDate("^(\\d\\d\\d\\d)-?(\\d\\d)-?(\\d\\d)$");
/**
 * Return the Julian Day from a Gregorian date (supplied as string).
 */
int Date::GregorianToJD(string date)
{
   vector<string> v;
   if (reDate.exec_retrieve(date, v) != 4)
      throw runtime_error_pf("Invalid date format for GregorianToJD(): '%s'", date.c_str());
   return GregorianToJD(atoi(v[1].c_str()), atoi(v[2].c_str()), atoi(v[3].c_str()));
}

/**
 * Return the Gregorian date (as three integers) from a Julian Day.
 * Formula taken from http://hermetic.nofadz.com/cal_stud/jdn.htm
 */
void Date::JDToGregorian(int date, int* year, int* month, int* day)
{
   int l, n, i, j;
   l = date + 68569;
   n = ( 4 * l ) / 146097;
   l = l - ( 146097 * n + 3 ) / 4;
   i = ( 4000 * ( l + 1 ) ) / 1461001;
   l = l - ( 1461 * i ) / 4 + 31;
   j = ( 80 * l ) / 2447;
   *day = l - ( 2447 * j ) / 80;
   l = j / 11;
   *month = j + 2 - ( 12 * l );
   *year = 100 * ( n - 49 ) + i + l;
}

/**
 * Return the Gregorian date (as a timestamp, first second of the day) from a Julian Day.
 */
time_t Date::JDToGregorian_epoch_daybegin(int date)
{
   tm date_tm;
   int Y, M, D;

   Date::JDToGregorian(date, &Y, &M, &D);
   date_tm.tm_sec = 0;
   date_tm.tm_min = 0;
   date_tm.tm_hour = 0;
   date_tm.tm_mday = D;
   date_tm.tm_mon = M - 1;
   date_tm.tm_year = Y - 1900;
   date_tm.tm_isdst = -1;
   return mktime(&date_tm);
}

/**
 * Return the Gregorian date (as a timestamp, last second of the day) from a Julian Day.
 */
time_t Date::JDToGregorian_epoch_dayend(int date)
{
   tm date_tm;
   int Y, M, D;

   Date::JDToGregorian(date, &Y, &M, &D);
   date_tm.tm_sec = 59;
   date_tm.tm_min = 59;
   date_tm.tm_hour = 23;
   date_tm.tm_mday = D;
   date_tm.tm_mon = M - 1;
   date_tm.tm_year = Y - 1900;
   date_tm.tm_isdst = -1;
   return mktime(&date_tm);
}

/**
 * Return the Gregorian date (as a struct tm) from a Julian Day.
 */
void Date::JDToGregorian_tm(int date, tm* res)
{
   time_t t = JDToGregorian_epoch_daybegin(date);
   localtime_r(&t, res);
}

/**
 * Return the Gregorian date (as a string) from a Julian Day.
 */
string Date::JDToGregorian_str(int date)
{
   char buffer [9];
   int Y, M, D;

   Date::JDToGregorian(date, &Y, &M, &D);
   sprintf(buffer, "%04d%02d%02d", Y, M, D);
   return buffer;
}
