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
 * Type definitions for RAVE
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-15
 */
#ifndef RAVE_TYPES_H
#define RAVE_TYPES_H

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
 * The /Conventions version in a ODIM HDF5 file.
 */
typedef enum RaveIO_ODIM_Version {
  RaveIO_ODIM_Version_UNDEFINED = -1, /**< Undefined */
  RaveIO_ODIM_Version_2_0 = 0,        /**< ODIM 2.0 */
  RaveIO_ODIM_Version_2_1 = 1,        /**< ODIM 2.1 */
  RaveIO_ODIM_Version_2_2 = 2,        /**< ODIM 2.2 */
  RaveIO_ODIM_Version_2_3 = 3,        /**< ODIM 2.3 */
  RaveIO_ODIM_Version_2_4 = 4,        /**< ODIM 2.4, The default version */
} RaveIO_ODIM_Version;

#ifndef RAVEIO_API_ODIM_VERSION
#define RAVEIO_API_ODIM_VERSION RaveIO_ODIM_Version_2_3
#endif

/**
 * The /what/version in a ODIM HDF5 file
 */
typedef enum RaveIO_ODIM_H5rad_Version {
  RaveIO_ODIM_H5rad_Version_UNDEFINED = -1, /**< undefined */
  RaveIO_ODIM_H5rad_Version_2_0 = 0,  /**< ODIM 2.0 */
  RaveIO_ODIM_H5rad_Version_2_1 = 1,  /**< ODIM 2.1 */
  RaveIO_ODIM_H5rad_Version_2_2 = 2,  /**< ODIM 2.2 */
  RaveIO_ODIM_H5rad_Version_2_3 = 3,  /**< ODIM 2.3 */
  RaveIO_ODIM_H5rad_Version_2_4 = 4,  /**< ODIM 2.4, The default version */
} RaveIO_ODIM_H5rad_Version;

/**
 * Different value types. When initializing a data field it is wise
 * to always initiallize to nodata instead of undetect.
 */
typedef enum RaveValueType {
  RaveValueType_UNDEFINED = -1,     /**< undefined, i.e. non-existing, etc. */
  RaveValueType_UNDETECT = 0,       /**< undetect */
  RaveValueType_NODATA = 1,         /**< nodata */
  RaveValueType_DATA = 2            /**< data */
} RaveValueType;

/**
 * Object types that defines the /what/object in the ODIM format.
 */
typedef enum Rave_ObjectType {
  Rave_ObjectType_UNDEFINED = -1,
  Rave_ObjectType_PVOL = 0,       /**< Polar volume */
  Rave_ObjectType_CVOL = 1,       /**< Cartesian volume */
  Rave_ObjectType_SCAN,           /**< Polar scan */
  Rave_ObjectType_RAY,            /**< Single polar ray */
  Rave_ObjectType_AZIM,           /**< Azimuthal object */
  Rave_ObjectType_ELEV,           /**< Elevational object */
  Rave_ObjectType_IMAGE,          /**< 2-D cartesian image */
  Rave_ObjectType_COMP,           /**< Cartesian composite image(s) */
  Rave_ObjectType_XSEC,           /**< 2-D vertical cross section(s) */
  Rave_ObjectType_VP,             /**< 1-D vertical profile */
  Rave_ObjectType_PIC,            /**< Embedded graphical image */
  Rave_ObjectType_ENDOFTYPES      /**< Last entry */
} Rave_ObjectType;

/**
 * Product types that defines the &lt;datasetX&gt;/what/product in the ODIM format.
 */
typedef enum Rave_ProductType {
  Rave_ProductType_UNDEFINED = -1, /**< Undefined */
  Rave_ProductType_SCAN = 0, /**< A scan of polar data */
  Rave_ProductType_PPI,      /**< Plan position indicator */
  Rave_ProductType_CAPPI,    /**< Constant altitude PPI */
  Rave_ProductType_PCAPPI,   /**< Pseudo-CAPPI */
  Rave_ProductType_ETOP,     /**< Echo top */
  Rave_ProductType_MAX,      /**< Maximum */
  Rave_ProductType_RR,       /**< Accumulation */
  Rave_ProductType_VIL,      /**< Vertically integrated liquid water */
  Rave_ProductType_COMP,     /**< Composite */
  Rave_ProductType_VP,       /**< Vertical profile */
  Rave_ProductType_RHI,      /**< Range height indicator */
  Rave_ProductType_XSEC,     /**< Arbitrary vertical slice */
  Rave_ProductType_VSP,      /**< Vertical side panel */
  Rave_ProductType_HSP,      /**< Horizontal side panel */
  Rave_ProductType_RAY,      /**< Ray */
  Rave_ProductType_AZIM,     /**< Azimuthal type product */
  Rave_ProductType_QUAL,     /**< Quality metric */
  Rave_ProductType_PMAX,     /**< Pseudo-MAX */
  Rave_ProductType_SURF,     /**< Surface type */
  Rave_ProductType_EBASE,    /**< Echo base */
  Rave_ProductType_ENDOFTYPES /**< Last entry */
} Rave_ProductType;


/**
 * Different data types that are supported during transformation
 */
typedef enum RaveDataType {
  RaveDataType_UNDEFINED = -1, /**< Undefined data type */
  RaveDataType_CHAR = 0, /**< char */
  RaveDataType_UCHAR,    /**< unsigned char */
  RaveDataType_SHORT,    /**< short */
  RaveDataType_USHORT,   /**< unsigned short */
  RaveDataType_INT,      /**< int */
  RaveDataType_UINT,     /**< unisgned int */
  RaveDataType_LONG,     /**< long */
  RaveDataType_ULONG,    /**< unsigned long */
  RaveDataType_FLOAT,    /**< float */
  RaveDataType_DOUBLE,   /**< double */
  RaveDataType_LAST      /**< Always has to be at end and is not a real datatype */
} RaveDataType;

/**
 * Transformation methods
 */
typedef enum RaveTransformationMethod {
  NEAREST = 1,   /**< Nearest */
  BILINEAR,      /**< Bilinear */
  CUBIC,         /**< Cubic */
  CRESSMAN,      /**< Cressman */
  UNIFORM,       /**< Uniform */
  INVERSE        /**< Inverse */
} RaveTransformationMethod;

/**
 * Provides user with navigation information.
 */
typedef struct PolarNavigationInfo {
  double lon;    /**< longitude */
  double lat;    /**< latitude */
  double height; /**< height above sea surface in meters*/
  double actual_height; /**< actual height above sea surface in meters */
  double distance; /**< surface distance in meters*/
  double range;    /**< rays range */
  double actual_range; /**< actual rays range */
  double azimuth; /**< azimutal offset in radians */
  double actual_azimuth; /**< actual azimutal offset in radians */
  double elevation; /**< elevation angle in radians */

  Rave_ObjectType otype; /**< specifies if the data is from a scan or volume (default Rave_ObjectType_UNDEFINED) */
  int ei;         /**< elevation index in case of volumes (-1 meaning no elevation index found/calculated) */
  int ri;         /**< range index (-1 meaning no range index found/calculated or out of bounds) */
  int ai;         /**< azimuth index (-1 meaning no azimuth index found/calculated or out of bounds) */
} PolarNavigationInfo;

typedef struct PolarObservation {
  RaveValueType vt; /**< the value type */
  double v; /**< the corrected value */
  double distance; /**< the distance along ground to the radar */
  double height; /**< height above ground (center position) */
  double range; /**< range the range along the ray until we come to this bin */
  double elangle; /**< the elevation angle */
} PolarObservation;

/**
 * Linked list version of the PolarObservation.
 */
typedef struct PolarObservationLinkedList {
  PolarObservation obs;
  struct PolarObservationLinkedList* next; /**< the next observation in this set */
} PolarObservationLinkedList;

/**
 * Returns the size of the datatype.
 * @param[in] type - the rave data type
 * @return the size or -1 if size not can be determined
 */
int get_ravetype_size(RaveDataType type);

/**
 * Translates the string representation of the product type into the
 * enum.
 * @param[in] name - the string representation of the product type
 * @returns a product type or UNDEFINED if not found.
 */
Rave_ProductType RaveTypes_getProductTypeFromString(const char* name);

/**
 * Returns the string representation of the product type.
 * @param[in] type - the product type
 * @returns the string representation or NULL if nothing could be found.
 */
const char* RaveTypes_getStringFromProductType(Rave_ProductType type);

/**
 * Translates the string representation of the object type into the
 * enum.
 * @param[in] name - the string representation of the object type
 * @returns a object type or UNDEFINED if not found.
 */
Rave_ObjectType RaveTypes_getObjectTypeFromString(const char* name);

/**
 * Returns the string representation of the object type.
 * @param[in] type - the object type
 * @returns the string representation or NULL if nothing could be found.
 */
const char* RaveTypes_getStringFromObjectType(Rave_ObjectType type);

/**
 * Deallocates this linked list and all its children.
 * @param[in] obs - the observation to release
 */
void RaveTypes_FreePolarObservationLinkedList(PolarObservationLinkedList* obs);

/**
 * Creates an array of PolarObservations from a PolarObservationLinkedList
 * @param[in] obs - the linked list to be transformed into an array
 * @param[out] nritems - the number of items in the array
 * @return the array
 */
PolarObservation* RaveTypes_PolarObservationLinkedListToArray(PolarObservationLinkedList* obs, int* nritems);

/**
 * Removes all items that are not data-values in the observation array.
 * @param[in] observations - the array to be filtered
 * @param[in] nobservations - number of observations
 * @param[out] ndataobservations - the number of returned observations containing data values
 * @return the data observations
 */
PolarObservation* RaveTypes_FilterPolarObservationDataValues(PolarObservation* observations, int nobservations, int* ndataobservations);

/**
 * Sorts the array of observations. All observations that are not data will be places furthest down in the array.
 * @param[in] observations - the observations to be sorted
 * @param[in] nobservations - number of observations to be sorted
 */
void RaveTypes_SortPolarObservations(PolarObservation* observations, int nobservations);

#endif /* RAVE_TYPES_H */
