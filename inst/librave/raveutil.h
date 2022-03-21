/**
 * Utilities for translating between different units and other useful functions.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 1998-
 */
#ifndef RAVE_UTIL_H
#define RAVE_UTIL_H
#endif

#include "polar.h"

/**
 * Returns the maximum value of a and b.
 * @param[in] a
 * @param[in] b
 * @return the maximum value of a and b
 */
double RAVEMAX(double a, double b);

/**
 * Returns the minimum value of a and b.
 * @param[in] a
 * @param[in] b
 * @return the mainimum value of a and b
 */
double RAVEMIN(double a, double b);

/**
 * Converts dBZ to Z.
 * @param[in] dbz the dbz value to be converted
 * @return the Z value
 */
double dBZ2Z(double dbz);

/**
 * Converts dBZ to mm/h, given an input dBZ and Z-R constants A and b.
 * @param[in] dbz the dbz value to be converted
 * @param[in] A the A value
 * @param[in] b the b value
 * @return mm/h
 */
double dBZ2R(double dbz, double A, double b);

/**
 * Converts Z to dBZ.
 * @param[in] Z
 * @return dBZ
 */
double Z2dBZ(double Z);

/**
 * Converts mm/h to dBZ, given an input dBZ and Z-R constants A and b.
 * @param[in] R mm/h
 * @param[in] A the A value
 * @param[in] b the b value
 * @return dBZ
 */
double R2dBZ(double R, double A, double b);

/**
 * Converts raw to dBZ using linear gain and offset.
 * @param[in] raw the raw value
 * @param[in] gain the gain
 * @param[in] offset the offset
 * @return dBZ (raw * gain + offset)
 */
double raw2dbz(double raw, double gain, double offset);

/**
 * Converts dBZ to raw and makes sure it fits within an 8-bit word
 * using gain and offset.
 * @param[in] dbz the dBZ value
 * @param[in] gain the gain value, May NOT be 0.0
 * @param[in] offset the offset
 * @return the 8 bit raw ((dbz - offset)/gain)
 */
double dbz2raw(double dbz, double gain, double offset);

/**
 * Converts raw to R (mm/h) using linear gain and offset, and Z-R
 * coefficients A and b.
 * @param[in] raw the raw value
 * @param[in] gain the gain
 * @param[in] offset the offset
 * @param[in] A the A coefficient
 * @param[in] b the b coefficient
 * @return mm/h
 */
double raw2R(double raw, double gain, double offset, double A, double b);

/**
 * Converts R (mm/h) to raw using linear gain and offset, and Z-R coefficients
 * A and b.
 * @param[in] R mm/h
 * @param[in] gain the gain
 * @param[in] offset the offset
 * @param[in] A the A coefficient
 * @param[in] b the b coefficient
 * @return the raw value
 */
double R2raw(double R, double gain, double offset, double A, double b);

/**
 * Truncates a double to an integer.
 * @param[in] d the double v
 * @return the truncated int value
 */
int mytrunc(double d);

/**
 * Rounds a double to an integer.
 * @param[in] d the double v
 * @param[in] minlimit the minumim allowed value. if the rounded value becomes lower than this, the minlimit is used.
 * @param[in] maxlimit the maximum allowed value. if the rounded value becomes higher than this, the maxlimit is used.
 * @return the rounded int value
 */
int myround_int(double d, double minlimit, double maxlimit);
