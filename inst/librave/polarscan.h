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
 * Defines the functions available when working with polar scans.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-15
 */
#ifndef POLARSCAN_H
#define POLARSCAN_H
#include "polarnav.h"
#include "projection.h"
#include "polarscanparam.h"
#include "rave_object.h"
#include "rave_types.h"
#include "rave_list.h"
#include "raveobject_list.h"
#include "rave_field.h"

/**
 * Defines a Polar Scan
 */
typedef struct _PolarScan_t PolarScan_t;

/**
 * Enum defining how a an integer value should be selected from a 
 * float type-value.
 */
typedef enum PolarScanSelectionMethod_t {
  PolarScanSelectionMethod_ROUND = 0,
  PolarScanSelectionMethod_FLOOR,
  PolarScanSelectionMethod_CEIL
} PolarScanSelectionMethod_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType PolarScan_TYPE;

/**
 * Sets a navigator for the polar scan, this is preferrable to use
 * when this scan is included in a volume since the settings will
 * be identical for all scans included in the volume. Otherwise,
 * if the scan is managed separately, use longitude/latitude and height
 * instead.
 * @param[in] scan - the scan
 * @param[in] navigator - the polar navigator (MAY NOT BE NULL)
 */
void PolarScan_setNavigator(PolarScan_t* scan, PolarNavigator_t* navigator);

/**
 * Returns the navigator that is used for this scan.
 * @param[in] scan - the scan
 * @returns the polar navigator
 */
PolarNavigator_t* PolarScan_getNavigator(PolarScan_t* scan);

/**
 * Sets the projection to be used for this scan.
 * @param[in] scan - the scan
 * @param[in] projection - the projection (MAY NOT BE NULL)
 */
void PolarScan_setProjection(PolarScan_t* scan, Projection_t* projection);

/**
 * Returns the current projection for this scan.
 * @param[in] scan - the scan
 * @returns the projection used within this scan.
 */
Projection_t* PolarScan_getProjection(PolarScan_t* scan);

/**
 * Sets the nominal time.
 * @param[in] scan - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setTime(PolarScan_t* scan, const char* value);

/**
 * Returns the nominal time.
 * @param[in] scan - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* PolarScan_getTime(PolarScan_t* scan);

/**
 * Sets the start time.
 * @param[in] scan - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setStartTime(PolarScan_t* scan, const char* value);

/**
 * Returns the start time.
 * @param[in] scan - self
 * @returns the start time (or NULL if there is none)
 */
const char* PolarScan_getStartTime(PolarScan_t* scan);

/**
 * Sets the end time.
 * @param[in] scan - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setEndTime(PolarScan_t* scan, const char* value);

/**
 * Returns the end time.
 * @param[in] scan - self
 * @returns the end time (or NULL if there is none)
 */
const char* PolarScan_getEndTime(PolarScan_t* scan);

/**
 * Sets the nominal date.
 * @param[in] scan - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setDate(PolarScan_t* scan, const char* value);

/**
 * Returns the nominal date.
 * @param[in] scan - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* PolarScan_getDate(PolarScan_t* scan);

/**
 * Sets the start date.
 * @param[in] scan - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setStartDate(PolarScan_t* scan, const char* value);

/**
 * Returns the start date.
 * @param[in] scan - self
 * @returns the start date (or NULL if there is none)
 */
const char* PolarScan_getStartDate(PolarScan_t* scan);

/**
 * Sets the end date.
 * @param[in] scan - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setEndDate(PolarScan_t* scan, const char* value);

/**
 * Returns the end date.
 * @param[in] scan - self
 * @returns the end date (or NULL if there is none)
 */
const char* PolarScan_getEndDate(PolarScan_t* scan);

/**
 * Sets the source.
 * @param[in] scan - self
 * @param[in] value - the source
 * @returns 1 on success, otherwise 0
 */
int PolarScan_setSource(PolarScan_t* scan, const char* value);

/**
 * Returns the source.
 * @param[in] scan - self
 * @returns the source or NULL if there is none
 */
const char* PolarScan_getSource(PolarScan_t* scan);

/**
 * Sets the longitude
 * @param[in] scan - self
 * @param[in] lon - the longitude
 */
void PolarScan_setLongitude(PolarScan_t* scan, double lon);

/**
 * Returns the longitude
 * @param[in] scan - self
 * @returns the longitude
 */
double PolarScan_getLongitude(PolarScan_t* scan);

/**
 * Sets the latitude
 * @param[in] scan - self
 * @param[in] lat - the latitude
 */
void PolarScan_setLatitude(PolarScan_t* scan, double lat);

/**
 * Returns the latitude
 * @param[in] scan - self
 * @returns the latitude
 */
double PolarScan_getLatitude(PolarScan_t* scan);

/**
 * Sets the height
 * @param[in] scan - self
 * @param[in] height - the height
 */
void PolarScan_setHeight(PolarScan_t* scan, double height);

/**
 * Returns the height
 * @param[in] scan - self
 * @returns the height
 */
double PolarScan_getHeight(PolarScan_t* scan);

/**
 * Returns the distance from the radar to the specified lon/lat coordinate pair.
 * @param[in] scan - self
 * @param[in] lon - the longitude
 * @param[in] lat - the latitude
 * @returns the distance in meters.
 */
double PolarScan_getDistance(PolarScan_t* scan, double lon, double lat);

/**
 * Returns the maximum distance (at ground level) that this scan will cover.
 * @param[in] scan
 * @return the maximum distance in meters
 */
double PolarScan_getMaxDistance(PolarScan_t* scan);

/**
 * Sets the elevation angle for the scan
 * @param[in] scan - self
 * @param[in] elangle - the elevation angle
 */
void PolarScan_setElangle(PolarScan_t* scan, double elangle);

/**
 * Returns the elevation angle for the scan
 * @param[in] scan - the scan
 * @return the elevation angle
 */
double PolarScan_getElangle(PolarScan_t* scan);

/**
 * Returns the number of bins
 * @param[in] scan - the scan
 * @return the number of bins
 */
long PolarScan_getNbins(PolarScan_t* scan);

/**
 * Sets the range scale for the scan
 * @param[in] scan - the scan
 * @param[in] rscale - the scale of the range bin
 */
void PolarScan_setRscale(PolarScan_t* scan, double rscale);

/**
 * Returns the range bin scale for the scan
 * @param[in] scan - the scan
 * @return the scale of the range bin
 */
double PolarScan_getRscale(PolarScan_t* scan);

/**
 * Returns the number of rays/scan
 * @param[in] scan - the scan
 * @return the number of rays
 */
long PolarScan_getNrays(PolarScan_t* scan);

/**
 * Sets the ray start for the scan
 * @param[in] scan - the scan
 * @param[in] rstart - the start position of the ray
 */
void PolarScan_setRstart(PolarScan_t* scan, double rstart);

/**
 * Returns the ray start for the scan
 * @param[in] scan - the scan
 * @return the ray start position
 */
double PolarScan_getRstart(PolarScan_t* scan);

/**
 * Returns the data type
 * @param[in] scan - the scan
 * @return the data type
 */
RaveDataType PolarScan_getDataType(PolarScan_t* scan);

/**
 * Sets the a1gate
 * @param[in] scan - the scan
 * @param[in] a1gate - a1gate
 */
void PolarScan_setA1gate(PolarScan_t* scan, long a1gate);

/**
 * Returns the a1gate
 * @param[in] scan - the scan
 * @return the a1gate
 */
long PolarScan_getA1gate(PolarScan_t* scan);

/**
 * Sets the horizontal beamwidth (same as PolarScan_setBeamwH). Default is 1.0 * M_PI/360.0
 * @param[in] scan - the polar scan
 * @param[in] beamwidth - the beam width in radians
 */
void PolarScan_setBeamwidth(PolarScan_t* scan, double beamwidth);

/**
 * Returns the horizontal beamwidth (same as PolarScan_getBeamwH). Default is 1.0 * M_PI/360.0.
 * @param[in] scan - the polar scan
 * @return the beam width om radians
 */
double PolarScan_getBeamwidth(PolarScan_t* scan);

/**
 * Sets the horizontal beamwidth. Default is 1.0 * M_PI/360.0
 * @param[in] scan - the polar scan
 * @param[in] beamwidth - the beam width in radians
 */
void PolarScan_setBeamwH(PolarScan_t* scan, double beamwidth);

/**
 * Returns the horizontal beamwidth. Default is 1.0 * M_PI/360.0.
 * @param[in] scan - the polar scan
 * @return the beam width om radians
 */
double PolarScan_getBeamwH(PolarScan_t* scan);

/**
 * Sets the vertical beamwidth. Default is 1.0 * M_PI/360.0
 * @param[in] scan - the polar scan
 * @param[in] beamwidth - the beam width in radians
 */
void PolarScan_setBeamwV(PolarScan_t* scan, double beamwidth);

/**
 * Returns the vertical beamwidth. Default is 1.0 * M_PI/360.0.
 * @param[in] scan - the polar scan
 * @return the beam width om radians
 */
double PolarScan_getBeamwV(PolarScan_t* scan);

/**
 * Sets the default parameter for this scan. I.e. all operations
 * that retrieves/sets values that does not contain a parameter name
 * as well will use the default parameter. Note, there is nothing
 * verifying if the parameter actually exists so if you are uncertain
 * use \ref #hasParameter first.
 * @param[in] scan - self
 * @param[in] quantity - the parameter
 * @returns 1 on success otherwise 0
 */
int PolarScan_setDefaultParameter(PolarScan_t* scan, const char* quantity);

/**
 * Returns the currently specified default parameter name.
 * @param[in] scan - self
 * @returns the default parameter name
 */
const char* PolarScan_getDefaultParameter(PolarScan_t* scan);

/**
 * Adds a parameter to the polar scan. Note, if there already exists
 * a parameter with the same quantity, that parameter will be replaced
 * by this. Also, several consistency checks will be performed to ensure
 * that dimensions and similar are the same for all parameters that
 * are added.
 * @param[in] scan - self
 * @param[in] parameter - the parameter
 * @returns 1 on success, otherwise 0
 */
int PolarScan_addParameter(PolarScan_t* scan, PolarScanParam_t* parameter);

/**
 * Removes (and returns) the parameter that is specified by the quantity.
 * Note, since the parameter returned is inc-refed, remember to release it.
 * @param[in] scan - self
 * @param[in] key - the quantity name
 * @returns NULL if nothing found or the parameter if it exists.
 */
PolarScanParam_t* PolarScan_removeParameter(PolarScan_t* scan, const char* quantity);

/**
 * Removes all parameters that are in the scan as well as dereferencing
 * the default parameter.
 * @param[in] scan - self
 * @returns 1 on success or 0 on failure (if default name can not be reset)
 */
int PolarScan_removeAllParameters(PolarScan_t* scan);

/**
 * Returns the parameter that is specified by the quantity.
 * Note, since the parameter returned is inc-refed, remember to release it.
 * @param[in] scan - self
 * @param[in] key - the quantity name
 * @returns NULL if nothing found or the parameter if it exists.
 */
PolarScanParam_t* PolarScan_getParameter(PolarScan_t* scan, const char* quantity);

/**
 * Returns all parameters belonging to this scan.
 * @param[in] scan - self
 * @returns a list of 0 or more parameters on success otherwise NULL.
 */
RaveObjectList_t* PolarScan_getParameters(PolarScan_t* scan);

/**
 * Returns if the scan contains the specified parameter or not.
 * @param[in] scan - self
 * @param[in] quantity - the quantity name
 * @returns 1 if the parameter exists, otherwise 0
 */
int PolarScan_hasParameter(PolarScan_t* scan, const char* quantity);

/**
 * Returns this scans parameter names.
 * @param[in] scan - self
 * @returns this scans contained parameters. NULL on failure. Use \ref #RaveList_freeAndDestroy to destroy
 */
RaveList_t* PolarScan_getParameterNames(PolarScan_t* scan);

/**
 * Adds a quality field to this scan.
 * @param[in] scan - self
 * @param[in] field - the field to add
 * @returns 1 on success otherwise 0
 */
int PolarScan_addQualityField(PolarScan_t* scan, RaveField_t* field);

/**
 * Adds or replaces a quality field. The field is replaced if the field contains a how/task attribute
 * and it already exists another quality field with the same how/task name.
 * @param[in] scan - self
 * @param[in] field - the field to add or replace
 * @returns 1 on success otherwise 0
 */
int PolarScan_addOrReplaceQualityField(PolarScan_t* scan, RaveField_t* field);

/**
 * Returns the quality field at the specified location.
 * @param[in] scan - self
 * @param[in] index - the index
 * @returns the quality field if found, otherwise NULL
 */
RaveField_t* PolarScan_getQualityField(PolarScan_t* scan, int index);

/**
 * Returns the number of quality fields
 * @param[in] scan - self
 * @returns the number of quality fields
 */
int PolarScan_getNumberOfQualityFields(PolarScan_t* scan);

/**
 * Removes the quality field at the specified location
 * @param[in] scan - self
 * @param[in] index - the index
 */
void PolarScan_removeQualityField(PolarScan_t* scan, int index);

/**
 * Returns all quality fields belonging to this scan. The returned
 * object is only a reference so do not modify it.
 * @param[in] scan - self
 * @returns a list of 0 or more quality fields or NULL on error.
 */
RaveObjectList_t* PolarScan_getQualityFields(PolarScan_t* scan);

/**
 * Returns a quality field based on the value of how/task that should be a
 * string.
 * @param[in] scan - self
 * @param[in] value - the value of the how/task attribute
 * @return the field if found otherwise NULL
 */
RaveField_t* PolarScan_getQualityFieldByHowTask(PolarScan_t* scan, const char* value);

/**
 * Atempts to locate a quality field with how/task = value. First it will
 * check in the default parameter, then it will check the scan it self.
 * @param[in] scan - self
 * @param[in] value - the how/task value
 * @param[in] quantity - the parameter to search in. If NULL, then default parameter will be searched.
 * @return the field if found, otherwise NULL
 */
RaveField_t* PolarScan_findQualityFieldByHowTask(PolarScan_t* scan, const char* value, const char* quantity);

/**
 * Basically the same as \ref #PolarScan_findQualityFieldByHowTask with the exception that
 * all quantities are searched until first occurance is found.
 * @param[in] scan - self
 * @param[in] value - the how/task value
 * @return the field if found, otherwise NULL
 */
RaveField_t* PolarScan_findAnyQualityFieldByHowTask(PolarScan_t* scan, const char* value);

/**
 * Returns the range index for the specified range (in meters).
 * @param[in] scan - the scan
 * @param[in] r - the range
 * @param[in] selectionMethod - defines how range index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @param[in] rangeMidpoint - defines whether range indices should be calculated from range midpoints or from
 *                            their starting points. 0 for using starting points and other values for midpoints.
 * @return -1 on failure, otherwise a index between 0 and nbins
 */
int PolarScan_getRangeIndex(PolarScan_t* scan, double r, PolarScanSelectionMethod_t selectionMethod, int rangeMidpoint);

/**
 * Returns the range for the specified range index.
 * Same as rscale * ri. If something goes wrong a negative
 * value will be returned.
 * @param[in] scan - self
 * @param[in] ri - range index
 * @param[in] rangeMidpoint - defines whether range should be calculated from range midpoints or from
 *                            their starting points. 0 for using starting points and other values for midpoints.
 * @return the range in meters or a negative value upon bad input
 */
double PolarScan_getRange(PolarScan_t* scan, int ri, int rangeMidpoint);

/**
 * Sets if the azimuthal nav information (astart / startazA / stopazA) should
 * be used when calculating azimuthal index.
 * @param[in] self - self
 * @param[in] v - 1 if nav information should be used, otherwise 0
 */
void PolarScan_setUseAzimuthalNavInformation(PolarScan_t* self, int v);

/**
 * Returns if the azimuthal nav information should be used or not
 * @param[in] self - self
 * @return 1 if nav information should be used, otherwise 0.
 */
int PolarScan_useAzimuthalNavInformation(PolarScan_t* self);

/**
 * Returns the nothmost index by checking the occurance of startazA/stopazA and the index of the angle closest to north.
 * If startazA/stopazA doesn't exist. This method will always return 0.
 * @param[in] self - self
 * @return the index of the northmost ray
 */
int PolarScan_getNorthmostIndex(PolarScan_t* self);

/**
 * Returns the rotation needed to to get the first ray in the scan to be the north most ray. If a negative  value is returned, the shift should
 * be performed in a negative circular direction, if 0, no rotation required, if greater than 0, a positive circular shift is required.
 * If startazA/stopazA doesn't exist. This method will always return 0.
 * @param[in] self - self
 * @return the rotation required to shift the scan so that first ray is the north most. If no rotation required, 0 is returned.
 */
int PolarScan_getRotationRequiredToNorthmost(PolarScan_t* self);

/**
 * Returns the azimuth index for the specified azimuth.
 * @param[in] scan - the scan
 * @param[in] a - the azimuth (in radians)
 * @param[in] selectionMethod - defines how azimuth index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @return -1 on failure, otherwise a index between 0 and nrays.
 */
int PolarScan_getAzimuthIndex(PolarScan_t* scan, double a, PolarScanSelectionMethod_t selectionMethod);

/**
 * Returns the azimuth for the specified azimuth index.
 * If something goes wrong a negative value will be returned.
 * @param[in] scan - self
 * @param[in] ai - azimuth index
 * @return the azimuth or a negative value upon bad input
 */
double PolarScan_getAzimuth(PolarScan_t* scan, int ai);

/**
 * Sets the value at the specified position
 * @param[in] scan - self
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[in] v - the value
 * @returns 1 on success otherwise 0
 */
int PolarScan_setValue(PolarScan_t* scan, int bin, int ray, double v);

/**
 * Sets the parameter value at the specified position
 * @param[in] scan - self
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[in] v - the value
 * @returns 1 on success otherwise 0
 */
int PolarScan_setParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double v);

/**
 * Returns the value at the specified index.
 * @param[in] scan - the scan
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType PolarScan_getValue(PolarScan_t* scan, int bin, int ray, double* v);

/**
 * Returns the parameter value at the specified index.
 * @param[in] scan - self
 * @param[in] quantity - the parameter (MAY NOT be NULL)
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the found value
 */
RaveValueType PolarScan_getParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double* v);

/**
 * Returns the linear converted value at the specified index. That is,
 * offset + gain * value;
 * @param[in] scan - the scan
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType PolarScan_getConvertedValue(PolarScan_t* scan, int bin, int ray, double* v);

/**
 * Returns the linear converted parameter value at the specified index. That is,
 * offset + gain * value;
 * @param[in] scan - the scan
 * @param[in] quantity - the parameter (MAY NOT BE NULL)
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] v - the data at the specified index
 * @return the type of data
 */
RaveValueType PolarScan_getConvertedParameterValue(PolarScan_t* scan, const char* quantity, int bin, int ray, double* v);

/**
 * Returns the bin and ray index from a specified azimuth and range.
 * @param[in] scan - self (MAY NOT BE NULL)
 * @param[in] a - the azimuth (in radians)
 * @param[in] r - the range (in meters)
 * @param[in] azimuthSelectionMethod - defines how azimuth index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @param[in] rangeSelectionMethod - defines how range index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @param[in] rangeMidpoint - defines whether range should be calculated from range midpoints or from
 *                            their starting points. 0 for using starting points and other values for midpoints.
 * @param[out] ray - the ray index (MAY NOT BE NULL)
 * @param[out] bin - the bin index (MAY NOT BE NULL)
 * @returns 1 on success, otherwise 0 and in that case, bin and ray can not be relied on.
 */
int PolarScan_getIndexFromAzimuthAndRange(
    PolarScan_t* scan,
    double a,
    double r,
    PolarScanSelectionMethod_t azimuthSelectionMethod,
    PolarScanSelectionMethod_t rangeSelectionMethod,
    int rangeMidpoint,
    int* ray,
    int* bin);

/**
 * Calculates the azimuth and range from bin and ray index.
 * @param[in] scan - self
 * @param[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] a - azimuth
 * @param[out] r - range
 * @returns 1 on success otherwise 0
 */
int PolarScan_getAzimuthAndRangeFromIndex(PolarScan_t* scan, int bin, int ray, double* a, double* r);

/**
 * Gets the value at the provided azimuth and range.
 * @param[in] scan - the scan
 * @param[in] a - the azimuth (in radians)
 * @param[in] r - the range (in meters)
 * @param[in] convert - indicates if value should be converted with gain and offset or not. 0 for false, true otherwise
 * @param[out] v - the value
 * @return a rave value type
 */
RaveValueType PolarScan_getValueAtAzimuthAndRange(PolarScan_t* scan, double a, double r, int convert, double* v);

/**
 * Returns the parameter value at the specified azimuth and range
 * @param[in] scan - self
 * @param[in] quantity - the parameter name
 * @param[in] a - the azimuth (in radians)
 * @param[in] r - the range (in meters)
 * @param[out] v - the value
 * @returns a rave value type (if scan does not contain specified parameter, RaveValueType_UNDEFINED will be returned).
 */
RaveValueType PolarScan_getParameterValueAtAzimuthAndRange(PolarScan_t* scan, const char* quantity, double a, double r, double* v);

/**
 * Returns the converted value at the specified azimuth and range
 * @param[in] scan - self
 * @param[in] quantity - the parameter name
 * @param[in] a - the azimuth (in radians)
 * @param[in] r - the range (in meters)
 * @param[out] v - the value
 * @returns a rave value type (if scan does not contain specified parameter, RaveValueType_UNDEFINED will be returned).
 */
RaveValueType PolarScan_getConvertedParameterValueAtAzimuthAndRange(PolarScan_t* scan, const char* quantity, double a, double r, double* v);

/**
 * Returns the navigation information that is the result from finding the lon/lat-coordinate
 * for this scan.
 * @param[in] scan - self
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[in,out] info - the navigation information
 */
void PolarScan_getLonLatNavigationInfo(PolarScan_t* scan, double lon, double lat, PolarNavigationInfo* info);

/**
 * Calculates range and elevation index from the azimuth and range
 * in the info object.
 * @param[in] scan - self
 * @param[in] azimuthSelectionMethod - defines how azimuth index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @param[in] rangeSelectionMethod - defines how range index shall be selected. 'ceiled', 'floored' or 'rounded'
 * @param[in] rangeMidpoint - defines whether range should be calculated from range midpoints or from
 *                            their starting points. 0 for using starting points and other values for midpoints.
 * @param[in,out] info - Will use info.azimuth and info.range to calculate info.ai and info.ri
 * @return 1 if indexes are in range, otherwise 0
 */
int PolarScan_fillNavigationIndexFromAzimuthAndRange(
  PolarScan_t* scan,
  PolarScanSelectionMethod_t azimuthSelectionMethod,
  PolarScanSelectionMethod_t rangeSelectionMethod,
  int rangeMidpoint,
  PolarNavigationInfo* info);

/**
 * Returns the nearest value to the specified longitude, latitude.
 * @param[in] scan - the scan
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude  (in radians)
 * @param[in] convert - indicates if value should be converted with gain and offset or not. 0 for false, true otherwise
 * @param[out] v - the found value
 * @returns a rave value type
 */
RaveValueType PolarScan_getNearest(PolarScan_t* scan, double lon, double lat, int convert, double* v);

/**
 * Returns the nearest parameter value to the specified longitude, latitude.
 * @param[in] scan - the scan
 * @param[in] quantity - the quantity
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude  (in radians)
 * @param[out] v - the found value
 * @returns a rave value type
 */
RaveValueType PolarScan_getNearestParameterValue(PolarScan_t* scan, const char* quantity, double lon, double lat, double* v);

/**
 * Appends navigation info structs to array, based on a target navigation info struct and
 * input parameters. The added navigation infos will represent positions surrounding the position
 * of the target navigation info in range and azimuth dimensions. The parameters surroundingRangeBins
 * and surroundingRays controls whether positions on both sides of the target shall be added, or 
 * only the closest.
 * @param[in] scan - the scan
 * @param[in] targetNavInfo - the target navigation info
 * @param[in] surroundingRangeBins - boolean indicating whether surrounding or only closest range bin 
 *                                   shall be added. 0 for closest, surrounding otherwise.
 * @param[in] surroundingRays - boolean indicating whether surrounding or only closest rays/azimuths 
 *                              shall be added. 0 for closest, surrounding otherwise.
 * @param[in] noofNavinfos - no of valid elements in the navinfos array at function call
 * @param[in,out] navinfos - array of navigation infos. will be updated with surrounding navigation 
 *                           infos
 * @returns no of valid elements in navinfos array after additions
 */
int PolarScan_addSurroundingNavigationInfosForTarget(
    PolarScan_t* scan,
    PolarNavigationInfo* targetNavInfo,
    int surroundingRangeBins,
    int surroundingRays,
    int noofNavinfos,
    PolarNavigationInfo navinfos[]);

/**
 * Returns an array of surrounding navigation info structs for the specified lon/lat. The returned 
 * navigation infos will represent positions surrounding the lon/lat in range and azimuth dimensions. 
 * The parameters surroundingRangeBins and surroundingRays controls whether positions on both sides 
 * of the lon/lat target shall be added, or only the closest.
 * @param[in] scan - the scan
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude  (in radians)
 * @param[in] surroundingRangeBins - boolean indicating whether surrounding or only closest range bin 
 *                                   shall be added. 0 for closest, surrounding otherwise.
 * @param[in] surroundingRays - boolean indicating whether surrounding or only closest rays/azimuths 
 *                              shall be added. 0 for closest, surrounding otherwise.
 * @param[out] navinfos - array of navigation infos. is assumed to be empty at function call. must be 
 *                        allocated by calling function to admit expected no of surrounding navigation 
 *                        infos
 * @returns no of valid elements in navinfos array after additions
 */
int PolarScan_getSurroundingNavigationInfos(
    PolarScan_t* scan,
    double lon,
    double lat,
    int surroundingRangeBins,
    int surroundingRays,
    PolarNavigationInfo navinfos[]);

/**
 * Returns the navigation information for the specified lon/lat.
 * @param[in] scan - self
 * @param[in] lon - longitude (in radians)
 * @param[in] lat - latitude (in radians)
 * @param[in,out] navinfo - the navigation information (MAY NOT BE NULL)
 * @returns 1 if navigation info is in range of scan otherwise 0
 */
int PolarScan_getNearestNavigationInfo(PolarScan_t* scan, double lon, double lat, PolarNavigationInfo* navinfo);

/**
 * Returns the nearest converted parameter value to the specified longitude, latitude.
 * @param[in] scan - the scan
 * @param[in] quantity - the quantity
 * @param[in] lon  - the longitude (in radians)
 * @param[in] lat  - the latitude  (in radians)
 * @param[out] v - the found value
 * @param[in,out] navinfo - the navigation information (may be NULL)
 * @returns a rave value type
 */
RaveValueType PolarScan_getNearestConvertedParameterValue(PolarScan_t* scan, const char* quantity, double lon, double lat, double* v, PolarNavigationInfo* navinfo);

/**
 * Returns the nearest index to the specified long/lat pair.
 * @param[in] scan - self
 * @param[in] lon - the longitude (in radians)
 * @param[in] lat - the latitude (in radians)
 * @param[out] bin - the bin index (MAY NOT BE NULL)
 * @param[out] ray - the ray index (MAY NOT BE NULL)
 * @returns 0 if either bin and/or ray is outside boundaries, otherwise 1
 */
int PolarScan_getNearestIndex(PolarScan_t* scan, double lon, double lat, int* bin, int* ray);

/**
 * Calculates the lon / lat from the index with the adjusted elevation angle.
 * Beamwidth will also be taken into account. Which means that the maximum position
 * will be on the edges of the beam.
 * @param[in] scan - self
 * @þaram[in] bin - the bin index
 * @param[in] ray - the ray index
 * @param[out] lon - the longitude in radians
 * @param[out] lat - the latitude in radians
 * @returns 1 on success otherwise 0
 */
int PolarScan_getLonLatFromIndex(PolarScan_t* scan, int bin, int ray, double* lon, double* lat);

/**
 * Returns the quality value for the quality field that has a name matching the how/task attribute
 * in the list of fields. It will first search the parameter for the quality field, then it
 * will search the scan for the quality field if no field is found it will return 0.
 *
 * @param[in] scan - self
 * @param[in] quantity - the parameter quantity
 * @param[in] ri - the range index (bin)
 * @param[in] ai - the azimuth index (ray)
 * @param[in] name - the value of the how/task attribute
 * @param[in] convert - indicates if value should be converted with gain and offset or not. 0 for false, true otherwise
 * @param[out] v - the found value
 * @return 1 if value found otherwise 0
 */
int PolarScan_getQualityValueAt(PolarScan_t* scan, const char* quantity, int ri, int ai, const char* name, int convert, double* v);

/**
 * Verifies that all preconditions are met in order to perform
 * a transformation.
 * @param[in] scan - the polar scan
 * @returns 1 if the polar scan is ready, otherwise 0.
 */
int PolarScan_isTransformable(PolarScan_t* scan);

/**
 * Adds a rave attribute to the scan.
 * @param[in] scan - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */
int PolarScan_addAttribute(PolarScan_t* scan, RaveAttribute_t* attribute);

/**
 * Removes a rave attribute from the scan.
 * @param[in] scan - self
 * @param[in] attrname - the name of the attribute to remove
 */
void PolarScan_removeAttribute(PolarScan_t* scan, const char* attrname);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] scan - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* PolarScan_getAttribute(PolarScan_t* scan, const char* name);

/**
 * Returns if the specified attribute exists.
 * @param[in] scan - self
 * @param[in] name - the name of the attribute
 * @returns 1 if attribute exists, otherwise 0
 */
int PolarScan_hasAttribute(PolarScan_t* scan, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] scan - self
 * @returns a list of attribute names
 */
RaveList_t* PolarScan_getAttributeNames(PolarScan_t* scan);

/**
 * Returns a list of attribute values belonging to this scan.
 * @param[in] scan - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* PolarScan_getAttributeValues(PolarScan_t* scan);

/**
 * Performs a circular shift of an array attribute. if nx < 0, then shift is performed counter clockwise, if nx > 0, shift is performed clock wise, if 0, no shift is performed.
 * @param[in] scan - self
 * @param[in] name - attribute to shift
 * @param[in] nx - number of positions to shift
 * return 1 if successful, 0 if trying to shift an attribute that isn't an array or an error occurs during shift.
 */
int PolarScan_shiftAttribute(PolarScan_t* scan, const char* name, int nx);

/**
 * Validates the scan can be seen to be valid regarding storage.
 * @param[in] scan - self
 * @param[in] otype - the object type this scan should be accounted for
 * @returns 1 if valid, otherwise 0
 */
int PolarScan_isValid(PolarScan_t* scan, Rave_ObjectType otype);

/**
 * Creates a new scan with settings from the scan and data is defined
 * by the field. Since the field does not necessarily define the quantity
 * the default parameter will either be UNKNOWN or the how/quantity value
 * from the field.
 * @param[in] self - self
 * @param[in] field - the data field to use
 * @returns a polar scan on success otherwise NULL
 */
PolarScan_t* PolarScan_createFromScanAndField(PolarScan_t* self, RaveField_t* field);

/**
 * Gets the distance field for self. Will always be a 1-dimensional array with nbins length.
 * The distance represents the distance on ground to get to the same position as the bin location.
 * @param[in] self - self
 * @returns the distance field
 */
RaveField_t* PolarScan_getDistanceField(PolarScan_t* self);

/**
 * Gets the height field for self. Will always be a 1-dimensional array with nbins length.
 * The height represents the altitude for the bin
 * @param[in] self - self
 * @returns the height field
 */
RaveField_t* PolarScan_getHeightField(PolarScan_t* self);

/**
 * Performs a circular shift of the datasets that are associated with this scan. It can be negative for counter clock wise
 * and positive for clock wise rotation.
 * If for some reason a value != 1 is returned you should not use this scan since the internals might have been altered in such a way
 * that there are inconsistancies.
 * @param[in] self - self
 * @param[in] nrays - the number of rays to shift
 * @return 1 if successful otherwise 0
 */
int PolarScan_shiftData(PolarScan_t* self, int nrays);

/**
 * Performs a circular shift of the datsets that are associated with the scan and also all attributes that are associated with
 * the rays. Currently these attributes are:
 * - how/elangles
 * - how/startazA
 * - how/stopazA
 * - how/startazT
 * - how/stopazT
 * - how/startelA
 * - how/stopelA
 * - how/startelT
 * - how/stopelT
 * - how/TXpower
 * If for some reason a value != 1 is returned you should not use this scan since the internals might have been altered in such a way
 * that there are inconsistancies.
 * @param[in] self - self
 * @param[in] nrays - number of rays the data & array values should be shifted. If negative, counter clockwise, if positive clockwise.
 * @return 1 on success 0 on error
 */
int PolarScan_shiftDataAndAttributes(PolarScan_t* self, int nrays);

/**
 * Removes all parameters from a scan except the ones specified in parameters (list of strings)
 * @param[in] scan - self
 * @param[in] parameters - a list of character arrays
 * @return 1 on success otherwise 0
 */
int PolarScan_removeParametersExcept(PolarScan_t* scan, RaveList_t* parameters);

/**
 * Framework internal function for setting the beamwidth in a scan,
 * used to indicate that the beamwidth comes from the polar volume.
 * I.E. DO NOT USE UNLESS YOU KNOW WHY.
 * @param[in] scan - self
 * @param[in] bw - the beam width in radians
 */
void PolarScanInternal_setPolarVolumeBeamwH(PolarScan_t* scan, double bw);

/**
 * Framework internal function for setting the beamwidth in a scan,
 * used to indicate that the beamwidth comes from the polar volume.
 * I.E. DO NOT USE UNLESS YOU KNOW WHY.
 * @param[in] scan - self
 * @param[in] bw - the beam width in radians
 */
void PolarScanInternal_setPolarVolumeBeamwV(PolarScan_t* scan, double bw);

/**
 * Returns if the beamwidth comes from a volume or not.
 * I.E. DO NOT USE UNLESS YOU KNOW WHY.
 * @param[in] scan - self
 * @returns -1 if default setting, 1 if beamwidth comes from a polar volume otherwise 0
 */
int PolarScanInternal_isPolarVolumeBeamwH(PolarScan_t* scan);

/**
 * Returns if the beamwidth comes from a volume or not.
 * I.E. DO NOT USE UNLESS YOU KNOW WHY.
 * @param[in] scan - self
 * @returns -1 if default setting, 1 if beamwidth comes from a polar volume otherwise 0
 */
int PolarScanInternal_isPolarVolumeBeamwV(PolarScan_t* scan);

#endif
