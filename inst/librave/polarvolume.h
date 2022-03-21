/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of RAVE.

RAVE is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RAVE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with RAVE.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

/**
 * Defines the functions available when working with polar volumes.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-14
 */
#ifndef POLARVOLUME_H
#define POLARVOLUME_H
#include "polarscan.h"
#include "projection.h"
#include "rave_object.h"
#include "raveobject_list.h"

/**
 * Defines a Polar Volume
 */
typedef struct _PolarVolume_t PolarVolume_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PolarVolume_TYPE;

/**
 * Sets the nominal time.
 * @param[in] pvol - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int PolarVolume_setTime(PolarVolume_t* pvol, const char* value);

/**
 * Returns the nominal time.
 * @param[in] pvol - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* PolarVolume_getTime(PolarVolume_t* pvol);

/**
 * Sets the nominal date.
 * @param[in] pvol - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int PolarVolume_setDate(PolarVolume_t* pvol, const char* value);

/**
 * Returns the nominal date.
 * @param[in] pvol - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* PolarVolume_getDate(PolarVolume_t* pvol);

/**
 * Sets the source.
 * @param[in] pvol - self
 * @param[in] value - the source
 * @returns 1 on success, otherwise 0
 */
int PolarVolume_setSource(PolarVolume_t* pvol, const char* value);

/**
 * Returns the source.
 * @param[in] pvol - self
 * @returns the source or NULL if there is none
 */
const char* PolarVolume_getSource(PolarVolume_t* pvol);

/**
 * Sets the longitude for the volume
 * @param[in] pvol - the polar volume
 * @param[in] lon - the longitude
 */
void PolarVolume_setLongitude(PolarVolume_t* pvol, double lon);

/**
 * Returns the longitude for the volume
 * @param[in] pvol - the polar volume
 * @returns the longitude
 */
double PolarVolume_getLongitude(PolarVolume_t* pvol);

/**
 * Sets the latitude for the volume
 * @param[in] pvol - the polar volume
 * @param[in] lat - the latitude
 */
void PolarVolume_setLatitude(PolarVolume_t* pvol, double lat);

/**
 * Returns the latitude for the volume
 * @param[in] pvol - the polar volume
 * @returns the latitude
 */
double PolarVolume_getLatitude(PolarVolume_t* pvol);

/**
 * Sets the height for the volume
 * @param[in] pvol - the polar volume
 * @param[in] height - the height
 */
void PolarVolume_setHeight(PolarVolume_t* pvol, double height);

/**
 * Returns the height for the volume
 * @param[in] pvol - the polar volume
 * @returns the height
 */
double PolarVolume_getHeight(PolarVolume_t* pvol);

/**
 * Sets the beamwidth for the volume. All scans will get the specified
 * beamwidth. If you only want to make the beamwidth affect an individual
 * scan, modify the scan directly.
 * @param[in] pvol - self
 * @param[in] bw - beam width in radians
 */
void PolarVolume_setBeamwidth(PolarVolume_t* pvol, double bw);

/**
 * Returns the beam width for the volume.
 * @param[in] pvol - self
 * @returns the beam width in radians
 */
double PolarVolume_getBeamwidth(PolarVolume_t* pvol);

/**
 * Sets the horizontal beamwidth. Default is 1.0 * M_PI/360.0
 * @param[in] self - self
 * @param[in] beamwidth - the beam width in radians
 */
void PolarVolume_setBeamwH(PolarVolume_t* self, double beamwidth);

/**
 * Returns the horizontal beamwidth. Default is 1.0 * M_PI/360.0.
 * @param[in] self - self
 * @return the beam width om radians
 */
double PolarVolume_getBeamwH(PolarVolume_t* self);

/**
 * Sets the vertical beamwidth. Default is 1.0 * M_PI/360.0
 * @param[in] self - self
 * @param[in] beamwidth - the beam width in radians
 */
void PolarVolume_setBeamwV(PolarVolume_t* self, double beamwidth);

/**
 * Returns the vertical beamwidth. Default is 1.0 * M_PI/360.0.
 * @param[in] self - self
 * @return the beam width om radians
 */
double PolarVolume_getBeamwV(PolarVolume_t* self);

/**
 * Returns the distance from the radar to the specified lon/lat coordinate pair.
 * @param[in] pvol - self
 * @param[in] lon - the longitude
 * @param[in] lat - the latitude
 * @returns the distance in meters.
 */
double PolarVolume_getDistance(PolarVolume_t* pvol, double lon, double lat);

/**
 * Returns the maximum distance (at ground level) that this volume will cover.
 * @param[in] pvol - self
 * @return the maximum distance in meters
 */
double PolarVolume_getMaxDistance(PolarVolume_t* pvol);

/**
 * Returns the scan with the highest distance coverage (at ground level).
 * @param[in] pvol - self
 * @return the scan with the highest distance coverage
 */
PolarScan_t* PolarVolume_getScanWithMaxDistance(PolarVolume_t* pvol);

/**
 * Sets the projection for this polar volume. All scans belonging to this volume
 * will also get this projection assigned.
 * @param[in] pvol - the polar volume
 * @param[in] projection - the projection
 */
void PolarVolume_setProjection(PolarVolume_t* pvol, Projection_t* projection);

/**
 * Returns a copy of the projection that is used for this polar volume.
 * I.e. remember to release it.
 * @param[in] pvol - the polar volume
 * @returns a projection (or NULL if none is set)
 */
Projection_t* PolarVolume_getProjection(PolarVolume_t* pvol);

/**
 * Adds a scan to the volume. The scan will automatically be assigned the
 * volumes navigator and projection.
 * @param[in] pvol - the volume
 * @param[in] scan - the scan
 * ®return 0 on failure, otherwise success
 */
int PolarVolume_addScan(PolarVolume_t* pvol, PolarScan_t* scan);

/**
 * Returns the scan at given index.
 * @param[in] pvol - the volume
 * @param[in] index - the index
 * @returns the scan at the specified index or NULL on failure.
 */
PolarScan_t* PolarVolume_getScan(PolarVolume_t* pvol, int index);

/**
 * Returns the number of scans.
 * @param[in] pvol - the volume
 * @returns -1 on failure, otherwise a value >= 0
 */
int PolarVolume_getNumberOfScans(PolarVolume_t* pvol);

/**
 * Removes the scan at specified index
 * @param[in] pvol - the volume
 * @param[in] index - the index of the scan
 * @returns 1 on sucess otherwise 0
 */
int PolarVolume_removeScan(PolarVolume_t* pvol, int index);

/**
 * Returns the scan with elevation closest to the specified elevation. This function
 * requires that the scans are ordered in ascending order, otherwise the behaviour
 * will be undefined.
 * @param[in] pvol - the polar volume
 * @param[in] e - the elevation (in radians)
 * @param[in] inside - if the elevation must be within the min-max elevation or not. Any value != 0 means that elevation must be within range.
 * @returns the scan or NULL if nothing is found
 */
PolarScan_t* PolarVolume_getScanClosestToElevation(PolarVolume_t* pvol, double e, int inside);

/**
 * Returns the index in the list of scans of the specified scan.
 * @param[in] pvol - self
 * @param[in] scan - the scan that is searched for
 * @return -1 if not found, otherwise the index of the scan
 */
int PolarVolume_indexOf(PolarVolume_t* pvol, PolarScan_t* scan);

/**
 * Returns the navigation information that is the result from finding the lon/lat-coordinate
 * at the specified height for this volume
 * @param[in] scan - self
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[in] height - the altitude (in meters)
 * @param[in,out] info - the navigation information
 */
void PolarVolume_getLonLatNavigationInfo(PolarVolume_t* pvol, double lon, double lat, double height, PolarNavigationInfo* info);

/**
 * Fetches the value nearest to the specified position.
 * @param[in] pvol - the polar volume (MAY NOT BE NULL)
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude (in radians)
 * @param[in] height - the height
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[out] v - the value (MAY NOT BE NULL)
 * @return what type of value that has been set in v.
 */
RaveValueType PolarVolume_getNearest(PolarVolume_t* pvol, double lon, double lat, double height, int insidee, double* v);

/**
 * Fetches the nearest parameter value for the specified position.
 * @param[in] pvol - self (MAY NOT BE NULL)
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude (in radians)
 * @param[in] height - the height
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[out] v - the value (MAY NOT BE NULL)
 * @return what type of value that has been set in v. If the parameter does not exist in the found scan, RaveValueType_UNDEFINED will be returned.
 */
RaveValueType PolarVolume_getNearestParameterValue(PolarVolume_t* pvol, const char* quantity, double lon, double lat, double height, int insidee, double* v);

/**
 * Returns the vertical max value at the specified lon/lat coordinate.
 * @param[in] self - self
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[out] v - the value (MAY NOT BE NULL)
 * @param[out] navinfo - will be updated with navigation information
 * @returns the type of the value that has been set in v.
 */
RaveValueType PolarVolume_getConvertedVerticalMaxValue(PolarVolume_t* self, const char* quantity, double lon, double lat, double* v, PolarNavigationInfo* navinfo);

/**
 * Returns the converted parameter value at the specified location
 * @param[in] pvol - self
 * @param[in] quantity - the quantity
 * @param[in] ei - elevation index
 * @param[in] ri - bin index
 * @param[in] ai - azimuthal index
 * @return the type of the value
 */
RaveValueType PolarVolume_getConvertedParameterValueAt(PolarVolume_t* pvol, const char* quantity, int ei, int ri, int ai, double* v);

/**
 * Returns the navigation nearest to specified lon/lat/height.
 * @param[in] pvol - self (MAY NOT BE NULL)
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude (in radians)
 * @param[in] height - the height
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[in,out] navinfo - the navigation information (may be NULL).
 * @return 1 if ei, ri and ai are inside boundaries otherwise 0
 */
int PolarVolume_getNearestNavigationInfo(PolarVolume_t* pvol, double lon, double lat, double height, int insidee, PolarNavigationInfo* navinfo);

/**
 * Returns the navigation info structures representing positions closest surrounding the specfied lon/lat/height.
 * With the input parameters it is possible to state whether surrounding or just nearest position should be returned
 * in each dimension. The navinfos-array must be allocated by calling function to accommodate the number of expected
 * navigation infos to be returned.
 *
 * @param[in] pvol - self (MAY NOT BE NULL)
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude (in radians)
 * @param[in] height - the height
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[in] surroundingScans - 1 if surrounding positions in the height dimension shall be collected. 0 if
 *                               only nearest position shall be collected
 * @param[in] surroundingRangeBins - 1 if surrounding positions in the range dimension shall be collected. 0 if
 *                                   only nearest position shall be collected
 * @param[in] surroundingRays - 1 if surrounding positions in the azimuth dimension shall be collected. 0 if
 *                              only nearest position shall be collected
 * @param[out] navinfos - array of returned navigation informations.
 * @return the number of returned navigation infos
 */
int PolarVolume_getSurroundingNavigationInfos(
    PolarVolume_t* pvol,
    double lon,
    double lat,
    double height,
    int insidee,
    int surroundingScans,
    int surroundingRangeBins,
    int surroundingRays,
    PolarNavigationInfo navinfos[]);

/**
 * Adds elevation angle index information to a list of navigation info structures,
 * based on an input scan.
 *
 * @param[in] pvol - self (MAY NOT BE NULL)
 * @param[in] scan - the target scan within the the pvol. this is the scan for which the set ei
 *                   will be valid.
 * @param[in,out] navinfos - array of navigation informations to update with ei information.
 * @param[in] noofNavinfos - the length of the navinfos array
 * @param[in] startNavInfoIndex - position in the navinfos array at which to start the update.
 *                                positions located before this index will not be updated.
 */
void PolarVolume_addEiForNavInfos(
    PolarVolume_t* pvol,
    PolarScan_t* scan,
    PolarNavigationInfo navinfos[],
    int noofNavinfos,
    int startNavInfoIndex);

/**
 * Fetches the nearest converted parameter value for the specified position.
 * @param[in] pvol - self (MAY NOT BE NULL)
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude (in radians)
 * @param[in] height - the height
 * @param[in] insidee - if the estimated elevation must be within the min-max elevation or not to be valid
 * @param[out] v - the value (MAY NOT BE NULL)
 * @param[in,out] navinfo - the navigation information (may be NULL).
 * @return what type of value that has been set in v. If the parameter does not exist in the found scan, RaveValueType_UNDEFINED will be returned.
 */
RaveValueType PolarVolume_getNearestConvertedParameterValue(PolarVolume_t* pvol, const char* quantity, double lon, double lat, double height, int insidee, double* v, PolarNavigationInfo* navinfo);

/**
 * Returns the quality value for the quality field that has a name matching the how/task attribute
 * for the specified scan.
 *
 * @param[in] pvol - self
 * @param[in] quantity - the parameter quantity
 * @param[in] ri - the range index (bin)
 * @param[in] ai - the azimuth index (ray)
 * @param[in] name - the value of the how/task attribute
 * @param[in] convert - indicates if value should be converted with gain and offset or not. 0 for false, true otherwise
 * @param[out] v - the found value
 * @return 1 if value found otherwise 0
 */
int PolarVolume_getQualityValueAt(PolarVolume_t* pvol, const char* quantity, int ei, int ri, int ai, const char* name, int convert, double* v);

/**
 * Sets the default parameter that should be used when operating on this
 * volume unless the parameter (quantity) explicitly has been specified in the
 * function. All scans that currently are held by this volume and eventual new
 * ones will all get the same default parameter.
 * @param[in] scan - self
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @returns 1 on success otherwise 0
 */
int PolarVolume_setDefaultParameter(PolarVolume_t* pvol, const char* quantity);

/**
 * Returns the currently specified default parameter name.
 * @param[in] scan - self
 * @returns the default parameter name
 */
const char* PolarVolume_getDefaultParameter(PolarVolume_t* pvol);

/**
 * Arranges the scans in either ascending or descending elevation.
 * @param[in] pvol - the volume
 * @param[in] ascending - if 1, ascending sort will be done, otherwise descending
 */
void PolarVolume_sortByElevations(PolarVolume_t* pvol, int ascending);

/**
 * Verifies that the scans are sorted in ascending order.
 * @param[in] pvol - the volume
 * @returns 1 if the scans are sorted in ascending order otherwise 0.
 */
int PolarVolume_isAscendingScans(PolarVolume_t* pvol);

/**
 * Verifies that all preconditions are met in order to perform
 * a transformation.
 * @param[in] pvol - the polar volume
 * @returns 1 if the polar volume is ready, otherwise 0.
 */
int PolarVolume_isTransformable(PolarVolume_t* pvol);

/**
 * Adds a rave attribute to the volume.
 * what/date, what/time, what/source, where/lon, where/lat and where/height
 * are handled specially and will be added to respective member instead
 * of stored as attributes.
 * what/date, what/time and what/source must be string.
 * where/lon and where/lat must be double in degrees.
 * where/height must be double in meters.
 * @param[in] pvol - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int PolarVolume_addAttribute(PolarVolume_t* pvol,
  RaveAttribute_t* attribute);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] pvol - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* PolarVolume_getAttribute(PolarVolume_t* pvol,
  const char* name);

/**
 * Removes a rave attribute from the volume.
 * @param[in] pvol - self
 * @param[in] attrname - the name of the attribute to remove
 */
void PolarVolume_removeAttribute(PolarVolume_t* pvol, const char* attrname);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] pvol - self
 * @returns a list of attribute names
 */
RaveList_t* PolarVolume_getAttributeNames(PolarVolume_t* pvol);

/**
 * Returns a list of attribute values that should be stored for this volume.
 * @param[in] pvol - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* PolarVolume_getAttributeValues(PolarVolume_t* pvol);

/**
 * Returns if the volume has the specified attribute or not
 * @param[in] pvol - self
 * @param[in] name - the name of the attribute
 * @returns 1 if it exists otherwise 0
 */
int PolarVolume_hasAttribute(PolarVolume_t* pvol, const char* name);

/**
 * Validates the volume regarding storage.
 * @param[in] pvol - self
 * @returns 1 if valid, otherwise 0
 */
int PolarVolume_isValid(PolarVolume_t* pvol);

/**
 * Locates a scan with a quality field that has a how/task value == howtaskvalue.
 * @param[in] pvol - self
 * @param[in] howtaskvalue - the how/task value
 * @param[in] quantity - the quantity
 * @return the scan if found, otherwise NULL
 */
PolarScan_t* PolarVolume_findScanWithQualityFieldByHowTask(PolarVolume_t* pvol, const char* howtaskvalue, const char* quantity);

/**
 * Basically the same as \ref #PolarVolume_findScanWithQualityFieldByHowTask with the exception that all
 * parameters are traversed until a how/task value field is found to be matching.
 * @param[in] pvol - self
 * @param[in] howtaskvalue - the how/task value
 * @return the scan if found otherwise NULL
 */
PolarScan_t* PolarVolume_findAnyScanWithQualityFieldByHowTask(PolarVolume_t* pvol, const char* howtaskvalue);

/**
 * Gets the distance field for self. Will always be a 2-dimensional array with n elevation angles * nbins.
 * nbins will be the same number of bins as the scan with most bins has. Scans with less bins than the maximum number of bins will
 * get a -99999.0 value at each location that not can be represented. ysize represents the number of elevations and distance
 * is represented by xsize. The order of the elevation angles will be the same as the current volume so the scans probably should
 * be sorted before calling this method.
 * @param[in] self - self
 * @returns the distance field
 */
RaveField_t* PolarVolume_getDistanceField(PolarVolume_t* self);

/**
 * Gets the height field for self. Will always be a 2-dimensional array with n elevation angles * nbins.
 * nbins will be the same number of bins as the scan with most bins has. Scans with less bins than the maximum number of bins will
 * get a -99999.0 value at each location that not can be represented. ysize represents the number of elevations and distance
 * is represented by xsize. The order of the elevation angles will be the same as the current volume so the scans probably should
 * be sorted before calling this method.
 * @param[in] self - self
 * @returns the height field
 */
RaveField_t* PolarVolume_getHeightField(PolarVolume_t* self);

/**
 * Returns all observations at the specified height inside the specified gap. I.e. height +/- gap/2.
 * @param[in] self - self
 * @param[in] height - the height
 * @param[in] gap - the gap, i.e. +/- gap/2
 * @param[out] nobservations - number of returned observations in the array
 * @returns the polar observations
 */
PolarObservation* PolarVolume_getCorrectedValuesAtHeight(PolarVolume_t* self, double height, double gap, int* nobservations);

/**
 * Utility function for setting all scans in this volume to use or not use azimuthal navigation. Note, this will only affect
 * currently added scans and will not affect scans added after call to this function.
 * @param[in] self - self
 * @param[in] v - 0 if azimuthal nav information shouldn't be used, otherwise 1
 */
void PolarVolume_setUseAzimuthalNavInformation(PolarVolume_t* self, int v);

/**
 * Returns if the azimuthal nav information should be used or not. Since this information actually resides in the scans the
 * returned value will be depending on currently contained scans. If all scans have got azimuthal nav information set to 0, then
 * 0 will be returned. Otherwise 1. This means that if at least one scan is using azimuthal nav info, we want to know this when querying the volume.
 * @param[in] self - self
 * @return 1 if nav information is used, otherwise 0.
 */
int PolarVolume_useAzimuthalNavInformation(PolarVolume_t* self);

/**
 * Removes all parameters from all scans belongning to this volume except the ones specified in parameters (list of strings)
 * @param[in] self - self
 * @param[in] parameters - a list of character arrays
 * @return 1 on success otherwise 0
 */
int PolarVolume_removeParametersExcept(PolarVolume_t* self, RaveList_t* parameters);


#endif
