/**
 * Utilities for translating between different units and other useful functions.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 1998-
 */
#include "raveutil.h"
#include <limits.h>
#include <math.h>

double RAVEMAX(double a, double b)
{
  if (a > b)
    return a;
  else
    return b;
}

double RAVEMIN(double a, double b)
{
  if (a < b)
    return a;
  else
    return b;
}

/*
 Converts dBZ to Z.
 */
double dBZ2Z(double dbz)
{
  return pow(10.0, (dbz / 10.0));
}

/*
 Converts dBZ to mm/h, given an input dBZ and Z-R coefficients A and b.
 */
double dBZ2R(double dbz, double A, double b)
{
  double Z;
  double pow_v = 1.0 / b;

  Z = dBZ2Z(dbz);
  Z /= A;
  return pow(Z, pow_v);
}

/*
 Converts Z to dBZ.
 */
double Z2dBZ(double Z)
{
  return 10 * log10(Z);
}

/*
 Converts mm/h to dBZ, given an input dBZ and Z-R coefficients A and b.
 */
double R2dBZ(double R, double A, double b)
{
  double Z;

  Z = A * pow(R, b); /* Always convert to Z with rain coefficients */
  return Z2dBZ(Z);
}

/*
 Converts raw to dBZ using linear gain and offset.
 */
double raw2dbz(double raw, double gain, double offset)
{
  return gain * raw + offset;
}

/*
 Converts dBZ to raw and makes sure it fits within an 8-bit word.
 */
double dbz2raw(double dbz, double gain, double offset)
{
  double raw;

  raw = (dbz - offset) / gain;

  if (raw < offset) {
    raw = 0.0;
  } else if (raw >= 255) {
    raw = 255 - 1.0;
  }
  return raw;
}

/*
 Converts raw to R (mm/h) using linear gain and offset, and Z-R
 coefficients A and b.
 */
double raw2R(double raw, double gain, double offset, double A, double b)
{
  double dbz;

  dbz = raw2dbz(raw, gain, offset);
  return dBZ2R(dbz, A, b);
}

/*
 Converts R (mm/h) to raw using linear gain and offset, and Z-R coefficients
 A and b.
 */
double R2raw(double R, double gain, double offset, double A, double b)
{
  double dbz;

  dbz = R2dBZ(R, A, b);
  return dbz2raw(dbz, gain, offset);
}

#if defined( CREATE_ITRUNC )
int itrunc(double d)
{
  int a = (int)d;
  return a;
}
#endif

int mytrunc(double d)
{
  int a;/* = (int)d;*/

  if (d > INT_MAX)
    a = INT_MAX;
  else if (d < INT_MIN)
    a = INT_MIN;
  else
    a = (int) d;

  return a;
}

int myround_int(double d, double minlimit, double maxlimit)
{
  double rounded = round(d);

  int result;
  if (rounded > maxlimit)
    result = maxlimit;
  else if (d < minlimit)
    result = minlimit;
  else
    result = (int)rounded;

  return result;
}
