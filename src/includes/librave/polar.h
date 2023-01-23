/**
 * Navigation routines for calculating distances and heights.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 1998-
 */
#ifndef POLAR_H
#define POLAR_H

#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#endif

/**
 * Position object used for navigation.
 */
typedef struct {
  double alt0; /**< alt0 */
  double lat0; /**< lat0 */
  double lon0; /**< lon0 */
  double alt; /**< altitude */
  double lat; /**< latitude */
  double lon; /**< longitude */
  double azimuth; /**< azimuth */
  double distance; /**< distance */
  double dndh; /**< dndh */
  double range; /**< range */
  double elevation; /**< elevation */
  double momelev; /**< momelev */
} Position;

/**
 * Resets the position struct to have all values set to 0
 * @param[in] pos the position struct
 */
void resetPosStruct(Position* pos);

/**
 * Copies a position struct and returns an allocated
 * instance with the same values.
 * @param[in] src the struct to be copied
 * @return the position struct or NULL on failure
 */
Position* copyPosStruct(Position* src);

/**
 * Returns the earth radius at the specified latitude
 * @param[in] lat0 the latitude in radians
 * @return the earth radius in meters
 */
double getEarthRadius(double lat0);

/**
 * Latitude/Longitude to Distance/Azimuth
 * @param[in] src the position object, lon0, lat0, lon and lat should be specified
 * @param[in,out] tgt the resulting position object, distanze and azimuth will be set.
 */
void llToDa(Position* src, Position* tgt);

/**
 * Distance/Azimuth to Latitude/Longitude.
 * @param[in] src the position object, lon0, lat0, distance and azimuth should be specified
 * @param[in,out] tgt the resulting position object, lon and lat will be set
 */
void daToLl(Position* src, Position* tgt);

/**
 * Distance/Altitude to Range/Elevation.
 * @param[in] src the position object, lat0, dndh, alt, alt0, distance, elevation should be specified
 * @param[in,out] tgt the resulting position object, range, elevation and momelev will be set
 */
void dhToRe(Position* src, Position* tgt);

/**
 * Distance/Elevation to Range/Altitude
 * @param[in] src the position object, lat0, dndh, alt0, alt, distance and elevation should be specified
 * @param[in,out] tgt the resulting position object, range, alt and momelev will be set
 */
void deToRh(Position* src, Position* tgt);

/**
 * Range/Elevation to Distance/Altitude
 * @param[in] src the position object, lat0, dndh, alt0, range and elevation should be specified
 * @param[in,out] tgt the resulting position object, alt, distance and momelev will be set
 */
void reToDh(Position* src, Position* tgt);

/**
 * Elevation/Height to Range/Distance
 * @param[in] src the position object, lat0, dndh, alt, alt0 and elevation should be specified
 * @param[in,out] tgt the resulting position object, range, distance and momelev will be set
 */
void ehToRd(Position* src, Position* tgt);

#endif
