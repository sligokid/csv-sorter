/**
 * Declaration of Date class.
 * @file Date.h
 */

#ifndef DATE_H
#define DATE_H

#include "std.h"
#include <time.h>

/**
 * Julian day is the julian day minus an offset, so that it fits into one byte.
 * The offset is computed at program start, so that the prosessing date == SMJD_REFERRENCEDATE in smjd.
 */
typedef unsigned char smjd_t;

/**
 * The value of 'processing date' in smjd_t.
 * 215 allows us to see a bit more than 7 months in the past and a bit more than 1 month in the future.
 */
#define SMJD_REFERRENCEDATE      215

class Date
{
   public:
      Date() {};
      Date(string d);
      Date(int d, string mode = "epoch");
      void setDate(string d);
      void setDate(int d, string mode = "epoch");
      void setDate(int y, int m, int d);
      static void setSmjdOffset(string date);

      string str();
      time_t epoch_daybegin();
      time_t epoch_dayend();
      int jd();
      smjd_t smjd();

      static int SmjdToJd(smjd_t smjd);
      static smjd_t JdToSmjd(int jd);
      static int GregorianToJD(int y, int m, int d);
      static int GregorianToJD(tm date);
      static int GregorianToJD(string date);
      static void JDToGregorian(int date, int* year, int* month, int* day);
      static time_t JDToGregorian_epoch_daybegin(int date);
      static time_t JDToGregorian_epoch_dayend(int date);
      static void JDToGregorian_tm(int date, tm* res);
      static string JDToGregorian_str(int date);

   private:
      string str_;
      time_t epoch_daybegin_;
      time_t epoch_dayend_;
      int jd_;
};

#endif //DATE_H
