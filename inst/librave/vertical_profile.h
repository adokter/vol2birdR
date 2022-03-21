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
 * Defines the functions available when working with vertical profiles.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-08-24
 *
 * @author Ulf E. Nordh (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2017-02-23 Added functionality to yield an extended set of fields for 
 * vertical profiles e.g. HGHT, n (sample size), UWND and VWND
 */
 
#ifndef VERTICALPROFILE_H
#define VERTICALPROFILE_H
#include "rave_object.h"
#include "rave_types.h"
#include "rave_field.h"
#include "rave_attribute.h"

/**
 * Defines a Vertical Profile
 */
typedef struct _VerticalProfile_t VerticalProfile_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType VerticalProfile_TYPE;

/**
 * Sets the nominal time.
 * @param[in] self - self
 * @param[in] value - the time in the format HHmmss
 * @returns 1 on success, otherwise 0
 */
int VerticalProfile_setTime(VerticalProfile_t* self, const char* value);

/**
 * Returns the nominal time.
 * @param[in] self - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* VerticalProfile_getTime(VerticalProfile_t* self);

/**
 * Sets the nominal date.
 * @param[in] self - self
 * @param[in] value - the date in the format YYYYMMDD
 * @returns 1 on success, otherwise 0
 */
int VerticalProfile_setDate(VerticalProfile_t* self, const char* value);

/**
 * Returns the nominal date.
 * @param[in] scan - self
 * @returns the nominal time (or NULL if there is none)
 */
const char* VerticalProfile_getDate(VerticalProfile_t* self);

/**
 * Sets the source.
 * @param[in] self - self
 * @param[in] value - the source
 * @returns 1 on success, otherwise 0
 */
int VerticalProfile_setSource(VerticalProfile_t* self, const char* value);

/**
 * Returns the source.
 * @param[in] self - self
 * @returns the source or NULL if there is none
 */
const char* VerticalProfile_getSource(VerticalProfile_t* self);

/**
 * Sets the product name.
 * @param[in] self - self
 * @param[in] value - the product name
 * @returns 1 on success, otherwise 0
 */
int VerticalProfile_setProdname(VerticalProfile_t* self, const char* value);

/**
 * Returns the product name.
 * @param[in] self - self
 * @returns the product name or NULL if there is none
 */
const char* VerticalProfile_getProdname(VerticalProfile_t* self);

/**
 * Sets the longitude
 * @param[in] self - self
 * @param[in] lon - the longitude (in radians)
 */
void VerticalProfile_setLongitude(VerticalProfile_t* self, double lon);

/**
 * Returns the longitude
 * @param[in] self - self
 * @return the longitude in radians
 */
double VerticalProfile_getLongitude(VerticalProfile_t* self);

/**
 * Sets the latitude
 * @param[in] self - self
 * @param[in] lat - the latitude (in radians)
 */
void VerticalProfile_setLatitude(VerticalProfile_t* self, double lat);

/**
 * Returns the latitude
 * @param[in] self - self
 * @return the latitude in radians
 */
double VerticalProfile_getLatitude(VerticalProfile_t* self);

/**
 * Sets the height of the centre of the antenna
 * @param[in] self - self
 * @param[in] height - the height (in meters)
 */
void VerticalProfile_setHeight(VerticalProfile_t* self, double h);

/**
 * Returns the height of the centre of the antenna
 * @param[in] self - self
 * @return the height
 */
double VerticalProfile_getHeight(VerticalProfile_t* self);

/**
 * Sets the number of levels in the profile
 * @param[in] self - self
 * @param[in] levels - the number of levels
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setLevels(VerticalProfile_t* self, long l);

/**
 * Returns the number of levels in the profile
 * @param[in] self - self
 * @return the number of levels
 */
long VerticalProfile_getLevels(VerticalProfile_t* self);

/**
 * Sets the starttime of the lowest accepted scan
 * @param[in] self - self
 * @param[in] starttime - the starttime
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setStartTime(VerticalProfile_t* self, const char* s);

/**
 * Returns the starttime for the lowest accepted scan
 * @param[in] self - self
 * @return the starttime
 */
const char* VerticalProfile_getStartTime(VerticalProfile_t* self);

/**
 * Sets the endtime of the highest scan
 * @param[in] self - self
 * @param[in] endtime - the endtime
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setEndTime(VerticalProfile_t* self, const char* s);

/**
 * Returns the endtime for the highest scan
 * @param[in] self - self
 * @return the endtime
 */
const char* VerticalProfile_getEndTime(VerticalProfile_t* self);

/**
 * Sets the startdate for the VP
 * @param[in] self - self
 * @param[in] startdate - the startdate
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setStartDate(VerticalProfile_t* self, const char* s);

/**
 * Returns the startdate for the VP
 * @param[in] self - self
 * @return the startdate
 */
const char* VerticalProfile_getStartDate(VerticalProfile_t* self);

/**
 * Sets the enddate of the VP
 * @param[in] self - self
 * @param[in] enddate - the enddate
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setEndDate(VerticalProfile_t* self, const char* s);

/**
 * Returns the enddate for the VP
 * @param[in] self - self
 * @return the enddate
 */
const char* VerticalProfile_getEndDate(VerticalProfile_t* self);

/**
 * Sets the product for the VP
 * @param[in] self - self
 * @param[in] product - the product
 * @return 1 on success or 0 on failure
 */
int VerticalProfile_setProduct(VerticalProfile_t* self, const char* s);

/**
 * Returns the product for the VP
 * @param[in] self - self
 * @return the starttime
 */
const char* VerticalProfile_getProduct(VerticalProfile_t* self);

/**
 * Sets the vertical distance (m) between height intervals, or 0.0 if variable
 * @param[in] self - self
 * @param[in] i - the interval (in meters)
 */
void VerticalProfile_setInterval(VerticalProfile_t* self, double i);

/**
 * Returns the vertical distance (m) between height intervals, or 0.0 if variable
 * @param[in] self - self
 * @return the interval
 */
double VerticalProfile_getInterval(VerticalProfile_t* self);

/**
 * Sets the minimum height in meters above mean sea level
 * @param[in] self - self
 * @param[in] h - the height (in meters)
 */
void VerticalProfile_setMinheight(VerticalProfile_t* self, double h);

/**
 * Returns the minimum height in meters above mean sea level
 * @param[in] self - self
 * @return the interval
 */
double VerticalProfile_getMinheight(VerticalProfile_t* self);

/**
 * Sets the maximum height in meters above mean sea level
 * @param[in] self - self
 * @param[in] h - the height (in meters)
 */
void VerticalProfile_setMaxheight(VerticalProfile_t* self, double h);

/**
 * Returns the maximum height in meters above mean sea level
 * @param[in] self - self
 * @return the interval
 */
double VerticalProfile_getMaxheight(VerticalProfile_t* self);

/**
 * Adds a rave attribute to the vertical profile.
 * @param[in] self - self
 * @param[in] attribute - the attribute
 * @return 1 on success otherwise 0
 */ 
int VerticalProfile_addAttribute(VerticalProfile_t* self, RaveAttribute_t* attribute);

/**
 * Returns the rave attribute that is named accordingly.
 * @param[in] self - self
 * @param[in] name - the name of the attribute
 * @returns the attribute if found otherwise NULL
 */
RaveAttribute_t* VerticalProfile_getAttribute(VerticalProfile_t* self, const char* name);

/**
 * Returns if the specified attribute exists or not
 * @param[in] self - self
 * @param[in] name - the name of the attribute
 * @returns 1 if the attribute exists otherwise 0
 */
int VerticalProfile_hasAttribute(VerticalProfile_t* self, const char* name);

/**
 * Returns a list of attribute names. Release with \@ref #RaveList_freeAndDestroy.
 * @param[in] self - self
 * @returns a list of attribute names
 */
RaveList_t* VerticalProfile_getAttributeNames(VerticalProfile_t* self);

/**
 * Returns a list of attribute values belonging to this vertical profile.
 * @param[in] self - self
 * @returns a list of RaveAttributes.
 */
RaveObjectList_t* VerticalProfile_getAttributeValues(VerticalProfile_t* self);

/**
 * Returns the Mean horizontal wind velocity (m/s).
 * @param[in] self - self
 * @return the mean horizontal wind velocity
 */
RaveField_t* VerticalProfile_getFF(VerticalProfile_t* self);

/**
 * Sets the Mean horizontal wind velocity (m/s)
 * This function will modify ff and add the attribute what/quantity = ff.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setFF(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the horizontal wind velocity
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getFFDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the horizontal wind velocity
 * This function will modify ff and add the attribute what/quantity = ff_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setFFDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Mean vertical wind velocity (positive upwards)
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getW(VerticalProfile_t* self);

/**
 * Sets the Mean vertical wind velocity (positive upwards)
 * This function will modify ff and add the attribute what/quantity = w.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setW(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the vertical wind velocity
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getWDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the vertical wind velocity
 * This function will modify ff and add the attribute what/quantity = w_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setWDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Mean horizontal wind direction
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDD(VerticalProfile_t* self);

/**
 * Sets the Mean horizontal wind direction
 * This function will modify ff and add the attribute what/quantity = dd.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDD(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the horizontal wind direction
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDDDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the horizontal wind direction
 * This function will modify ff and add the attribute what/quantity = dd_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDDDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Divergence
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDiv(VerticalProfile_t* self);

/**
 * Sets the Divergence
 * This function will modify ff and add the attribute what/quantity = div.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDiv(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the  Standard deviation of the divergence
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDivDev(VerticalProfile_t* self);

/**
 * Sets the  Standard deviation of the divergence
 * This function will modify ff and add the attribute what/quantity = div_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDivDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Deformation
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDef(VerticalProfile_t* self);

/**
 * Sets the Deformation
 * This function will modify ff and add the attribute what/quantity = def.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDef(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the deformation
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDefDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the deformation
 * This function will modify ff and add the attribute what/quantity = def_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDefDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Axis of dilation (0-360)
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getAD(VerticalProfile_t* self);

/**
 * Sets the Axis of dilation (0-360)
 * This function will modify ff and add the attribute what/quantity = ad.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setAD(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the axis of dilation
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getADDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the axis of dilation
 * This function will modify ff and add the attribute what/quantity = ad_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setADDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Mean radar reflectivity factor
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDBZ(VerticalProfile_t* self);

/**
 * Sets the Mean radar reflectivity factor
 * This function will modify ff and add the attribute what/quantity = dbz.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDBZ(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the Standard deviation of the radar reflectivity factor
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getDBZDev(VerticalProfile_t* self);

/**
 * Sets the Standard deviation of the radar reflectivity factor
 * This function will modify ff and add the attribute what/quantity = dbz_dev.
 * @param[in] self - self
 * @param[in] ff - ff (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setDBZDev(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the number of sample points for mean horizontal wind velocity
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getNV(VerticalProfile_t* self);

/**
 * Sets the number of sampled points for horizontal wind
 * This function will add the attribute what/quantity = n.
 * @param[in] self - self
 * @param[in] n - n (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setNV(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the number of sample points for reflectivity
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getNZ(VerticalProfile_t* self);

/**
 * Sets the number of sampled points for reflectivity
 * This function will modify add the attribute what/quantity = nz.
 * @param[in] self - self
 * @param[in] nz - nz (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */

int VerticalProfile_setNZ(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the different height levels. Each level is the center of the height bin
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getHGHT(VerticalProfile_t* self);

/**
 * Sets the different height levels. Each level is the center of the height bin
 * This function will modify ff and add the attribute what/quantity = HGHT.
 * @param[in] self - self
 * @param[in] HGHT - HGHT (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setHGHT(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the vind field UWND i.e. the wind component in the x-direction.
 * This field is calculated using the fields ff and dd
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getUWND(VerticalProfile_t* self);

/**
 * Sets the vind field UWND i.e. the wind component in the x-direction.
 * This field is calculated using the fields ff and dd
 * This function will modify ff and add the attribute what/quantity = UWND.
 * @param[in] self - self
 * @param[in] UWND - UWND (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setUWND(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns the vind field VWND i.e. the wind component in the y-direction.
 * This field is calculated using the fields ff and dd
 * @param[in] self - self
 * @return the field
 */
RaveField_t* VerticalProfile_getVWND(VerticalProfile_t* self);

/**
 * Sets the vind field VWND i.e. the wind component in the y-direction.
 * This field is calculated using the fields ff and dd
 * This function will modify ff and add the attribute what/quantity = VWND.
 * @param[in] self - self
 * @param[in] VWND - VWND (must be a 1 dimensional field with same dim as the other members).
 * @return 1 on success otherwise 0
 */
int VerticalProfile_setVWND(VerticalProfile_t* self, RaveField_t* ff);

/**
 * Returns a list of all existing fields in the vertical profile. Each field will contain
 * a what/quantity attribute for identification purposes.
 * @param[in] self - self
 * @return a list of all existing (set) fields
 */
RaveObjectList_t* VerticalProfile_getFields(VerticalProfile_t* self);

/**
 * Adds a field to the vertical profile. The field must have what/quantity set in order to
 * identify the type. This basically means that if addField is called with a field having
 * what/quantity = ff, it would be the same as calling \ref VerticalProfile_setFF.
 * Allowed quantities are: ff, ff_dev, w, w_dev, dd, dd_dev
 *   div, div_dev, def, def_dev. ad, ad_dev, dbz, dbz_dev, n, HGHT, UWND and VWND
 * @param[in] self - self
 * @param[in] field - the field
 * @return 1 on success or 0 on failure, either inconsistency or missing/bad what/quantity.
 */
int VerticalProfile_addField(VerticalProfile_t* self, RaveField_t* field);

/**
 * Another variant of getting the field, but use the quantity instead.
 * @param[in] self - self
 * @param[in] quantity - the quantity
 * @return the field or NULL if not found or error
 */
RaveField_t* VerticalProfile_getField(VerticalProfile_t* self, const char* quantity);

#endif
