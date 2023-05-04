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
 * Provides functionality for creating composites.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-01-19
 */
#include "composite.h"
#include "polarvolume.h"
#include "raveobject_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include "projection_pipeline.h"
#include <string.h>
#include "rave_field.h"
#include <float.h>
#include <stdio.h>
#include <math.h>


#define MAX_NO_OF_SURROUNDING_POSITIONS 8 // pow(2, NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS)

/**
 * Represents the cartesian product.
 */
struct _Composite_t {
  RAVE_OBJECT_HEAD /** Always on top */
  Rave_ProductType ptype; /**< the product type, default PCAPPI */
  CompositeSelectionMethod_t method; /**< selection method, default CompositeSelectionMethod_NEAREST */
  CompositeInterpolationMethod_t interpolationMethod; /**< interpolation method, default CompositeInterpolationMethod_NEAREST */
  double height; /**< the height when generating pcapppi, cappi, pmax default 1000 */
  double elangle; /**< the elevation angle when generating ppi, default 0.0 */
  double range;  /*< the range when generating pmax, default = 500000 meters */
  RaveList_t* parameters; /**< the parameters to generate */
  RaveDateTime_t* datetime;  /**< the date and time */
  RaveList_t* objectList;
  CompositeAlgorithm_t* algorithm; /**< the specific algorithm */
  char* qiFieldName; /**< the Quality Indicator field name to use when determining the radar usage */
};

typedef struct CompositeRadarItem {
  RaveCoreObject* object;
  int radarIndexValue;
} CompositeRadarItem_t;

/**
 * Structure for keeping track on parameters that should be composited.
 */
typedef struct CompositingParameter_t {
  char* name;      /**< quantity */
  double gain;     /**< gain to be used in composite data*/
  double offset;   /**< offset to be used in composite data*/
  double minvalue; /**< minimum value that can be expressed for this quantity in the composite. Used for interpolation. */
} CompositingParameter_t;

/**
 * Structure for holding information regarding a specific position and values 
 * connected with it.
 */
typedef struct CompositeValuePosition_t {
  PolarNavigationInfo navinfo;
  double value;       /**< value */
  double qivalue;     /**< quality index value */
  RaveValueType type; /**< value type */
  int valid;          /**< 1 if position valid, otherwise 0 */ 
} CompositeValuePosition_t;

/**
 * Structure for keeping track on values / parameter
 */
typedef struct CompositeValues_t {
  RaveValueType vtype; /**< value type */
  double value;       /**< value */
  double mindist;     /**< min distance */
  double radardist;   /**< distance to radar */
  int radarindex;     /**< radar index in list of radars */
  const char* name;   /**< name of quantity */
  CartesianParam_t* parameter; /**< the cartesian parameter */
  double qivalue;     /**< quality index value */
  int noOfValuePositions; /**< the number of valid positions in valuePositions-array below */
  CompositeValuePosition_t valuePositions[MAX_NO_OF_SURROUNDING_POSITIONS]; /**< value positions array */
} CompositeValues_t;

/**
 * Direction in which to perform interpolation.
 * NOTE: order is of importance here. The order controls in which order
 * interpolation is done, if done in several dimensions. Since height interpolation
 * must the be done last, it should be last in this enum.
 */
typedef enum CompositeInterpolationDimension_t {
  CompositeInterpolationDimension_AZIMUTH = 0,
  CompositeInterpolationDimension_RANGE,
  CompositeInterpolationDimension_HEIGHT,
  NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS
} CompositeInterpolationDimension_t;

/** 
 * Function pointer definition for functions where value positions are prepared with values prior to interpolation. 
 * This type of function shall be provided to CompositeInternal_getInterpolatedValue() 
 * */
typedef int (*FUNC_PREPARE_VALUEPOS)(Composite_t*, RaveCoreObject*, const char*, const char*, CompositeValuePosition_t[], int, int[]);

/** The resolution to use for scaling the distance from pixel to used radar. */
/** By multiplying the values in the distance field by 2000, we get the value in unit meters. */
#define DISTANCE_TO_RADAR_RESOLUTION 2000.0

/** Same for height, scaled to 100 m resolution up to 25.5 km */
#define HEIGHT_RESOLUTION 100.0

/** The name of the task for specifying distance to radar */
#define DISTANCE_TO_RADAR_HOW_TASK "se.smhi.composite.distance.radar"

/** The name of the task for specifying height above sea level */
#define HEIGHT_ABOVE_SEA_HOW_TASK "se.smhi.composite.height.radar"

/** The name of the task for indexing the radars used */
#define RADAR_INDEX_HOW_TASK "se.smhi.composite.index.radar"

/*@{ Private functions */
/**
 * Creates a parameter that should be composited
 * @param[in] name - quantity
 * @param[in] gain - gain
 * @param[in] offset - offset
 * @param[in] minvalue - minimum value
 * @return the parameter or NULL on failure
 */
static CompositingParameter_t* CompositeInternal_createParameter(const char* name, double gain, double offset, double minvalue)
{
  CompositingParameter_t* result = NULL;
  if (name != NULL) {
    result = RAVE_MALLOC(sizeof(CompositingParameter_t));
    if (result != NULL) {
      result->name = RAVE_STRDUP(name);
      result->gain = gain;
      result->offset = offset;
      if (result->name == NULL) {
        RAVE_FREE(result);
        result = NULL;
      }
      result->minvalue = minvalue;
    }
  }
  return result;
}

/**
 * Frees the parameter including its members
 * @param[in] p - the parameter to release
 */
static void CompositeInternal_freeParameter(CompositingParameter_t* p)
{
  if (p != NULL) {
    RAVE_FREE(p->name);
    RAVE_FREE(p);
  }
}

/**
 * Releases the complete parameter list including its items
 * @param[in] p - a pointer to the parameter list
 */
static void CompositeInternal_freeParameterList(RaveList_t** p)
{
  if (p != NULL && *p != NULL) {
    CompositingParameter_t* cp = RaveList_removeLast(*p);
    while (cp != NULL) {
      CompositeInternal_freeParameter(cp);
      cp = RaveList_removeLast(*p);
    }
    RAVE_OBJECT_RELEASE(*p);
  }
}

/**
 * Frees the radar item
 * @param[in] p - the radar item to release
 */
static void CompositeInternal_freeRadarItem(CompositeRadarItem_t* p)
{
  if (p != NULL) {
    RAVE_OBJECT_RELEASE(p->object);
    RAVE_FREE(p);
  }
}

/**
 * Frees the list of radar items
 * @param[in] p - the list to be released
 */
static void CompositeInternal_freeObjectList(RaveList_t** p)
{
  if (p != NULL && *p != NULL) {
    CompositeRadarItem_t* ri = RaveList_removeLast(*p);
    while (ri != NULL) {
      CompositeInternal_freeRadarItem(ri);
      ri = RaveList_removeLast(*p);
    }
    RAVE_OBJECT_RELEASE(*p);
  }
}


/**
 * Clones a radar item
 * @param[in] p - item to clone
 * @return the clone or NULL on failure
 */
static CompositeRadarItem_t* CompositeInternal_cloneRadarItem(CompositeRadarItem_t* p)
{
  CompositeRadarItem_t* result = NULL;
  if (p != NULL) {
    result = RAVE_MALLOC(sizeof(CompositeRadarItem_t));
    if (result != NULL) {
      result->object = RAVE_OBJECT_CLONE(p->object);
      result->radarIndexValue = p->radarIndexValue;
      if (result->object == NULL) {
        RAVE_FREE(result);
        result = NULL;
      }
    }
  }
  return result;
}

/**
 * Clones a parameter list
 * @param[in] p - the parameter list to clone
 * @return the clone or NULL on failure
 */
static RaveList_t* CompositeInternal_cloneRadarItemList(RaveList_t* p)
{
  int len = 0, i = 0;
  RaveList_t *result = NULL, *clone = NULL;;
  if (p != NULL) {
    clone = RAVE_OBJECT_NEW(&RaveList_TYPE);
    if (clone != NULL) {
      len = RaveList_size(p);
      for (i = 0; i < len; i++) {
        CompositeRadarItem_t* cp = RaveList_get(p, i);
        CompositeRadarItem_t* cpclone = CompositeInternal_cloneRadarItem(cp);
        if (cpclone == NULL || !RaveList_add(clone, cpclone)) {
          if (cpclone != NULL) {
            RAVE_OBJECT_RELEASE(cpclone->object);
            RAVE_FREE(cpclone);
          }
          goto done;
        }
      }
    }
  }

  result = RAVE_OBJECT_COPY(clone);
done:
  RAVE_OBJECT_RELEASE(clone);
  return result;
}

/**
 * Clones a parameter
 * @param[in] p - parameter to clone
 * @return the clone or NULL on failure
 */
static CompositingParameter_t* CompositeInternal_cloneParameter(CompositingParameter_t* p)
{
  CompositingParameter_t* result = NULL;
  if (p != NULL) {
    result = RAVE_MALLOC(sizeof(CompositingParameter_t));
    if (result != NULL) {
      result->name = RAVE_STRDUP(p->name);
      result->gain = p->gain;
      result->offset = p->offset;
      if (result->name == NULL) {
        RAVE_FREE(result);
        result = NULL;
      }
      result->minvalue = p->minvalue;
    }
  }
  return result;
}

/**
 * Verifies that the radar index mapping contains a mapping between string - long rave atttribute
 * @param[in] src - the mapping
 * @return 1 if ok, otherwise 0
 */
static int CompositeInternal_verifyRadarIndexMapping(RaveObjectHashTable_t* src)
{
  RaveList_t* keys = NULL;
  RaveAttribute_t* attr = NULL;
  int result = 0;

  if (src == NULL) {
    goto done;
  }

  keys = RaveObjectHashTable_keys(src);
  if (keys != NULL) {
    int i;
    int nattrs = RaveList_size(keys);
    for (i = 0; i < nattrs; i++) {
      const char* key = (const char*)RaveList_get(keys, i);
      attr = (RaveAttribute_t*)RaveObjectHashTable_get(src, key);
      if (attr == NULL || !RAVE_OBJECT_CHECK_TYPE(attr, &RaveAttribute_TYPE) || RaveAttribute_getFormat(attr) != RaveAttribute_Format_Long) {
        RAVE_ERROR0("Could not handle radar index mapping, must be mapping between key - long attribute");
        goto done;
      }
      RAVE_OBJECT_RELEASE(attr);
    }
  }

  result = 1;
done:

  RaveList_freeAndDestroy(&keys);
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

/**
 * Clones a parameter list
 * @param[in] p - the parameter list to clone
 * @return the clone or NULL on failure
 */
static RaveList_t* CompositeInternal_cloneParameterList(RaveList_t* p)
{
  int len = 0, i = 0;
  RaveList_t *result = NULL, *clone = NULL;;
  if (p != NULL) {
    clone = RAVE_OBJECT_NEW(&RaveList_TYPE);
    if (clone != NULL) {
      len = RaveList_size(p);
      for (i = 0; i < len; i++) {
        CompositingParameter_t* cp = RaveList_get(p, i);
        CompositingParameter_t* cpclone = CompositeInternal_cloneParameter(cp);
        if (cpclone == NULL || !RaveList_add(clone, cpclone)) {
          CompositeInternal_freeParameter(cpclone);
          goto done;
        }
      }
    }
  }

  result = RAVE_OBJECT_COPY(clone);
done:
  RAVE_OBJECT_RELEASE(clone);
  return result;
}

/**
 * Returns a pointer to the internall stored parameter in the composite.
 * @param[in] composite - composite
 * @param[in] quantity - the parameter
 * @return the found parameter or NULL if not found
 */
static CompositingParameter_t* CompositeInternal_getParameterByName(Composite_t* composite, const char* quantity)
{
  int len = 0, i = 0;
  if (quantity != NULL) {
    len = RaveList_size(composite->parameters);
    for (i = 0; i < len; i++) {
      CompositingParameter_t* cp = RaveList_get(composite->parameters, i);
      if (strcmp(cp->name, quantity) == 0) {
        return cp;
      }
    }
  }
  return NULL;
}

static int CompositeInternal_addGainAndOffsetToField(RaveField_t* field, double gain, double offset) {
  RaveAttribute_t* gainattribute = NULL;
  int result = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");

  gainattribute = RaveAttributeHelp_createDouble("what/gain", gain);
  if (gainattribute == NULL ||
      !RaveField_addAttribute(field, gainattribute)) {
    RAVE_ERROR0("Failed to create gain attribute for quality field");
    goto done;
  }
  RAVE_OBJECT_RELEASE(gainattribute);

  gainattribute = RaveAttributeHelp_createDouble("what/offset", offset);
  if (gainattribute == NULL ||
      !RaveField_addAttribute(field, gainattribute)) {
    RAVE_ERROR0("Failed to create offset attribute for quality field");
    goto done;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(gainattribute);
  return result;

}

static RaveField_t* CompositeInternal_createQualityField(char* howtaskstr, int xsize, int ysize, double gain, double offset) {
  RaveField_t* qfield = RAVE_OBJECT_NEW(&RaveField_TYPE);
  RaveAttribute_t* howtaskattribute = NULL;

  if (qfield == NULL) {
    RAVE_ERROR0("Failed to create quality field");
    goto error;
  }

  howtaskattribute = RaveAttributeHelp_createString("how/task", howtaskstr);
  if (howtaskattribute == NULL) {
    RAVE_ERROR0("Failed to create quality field (how/task attribute could not be created)");
    goto error;
  }

  if (!RaveField_addAttribute(qfield, howtaskattribute)) {
    RAVE_ERROR0("Failed to create how/task attribute for distance quality field");
    goto error;
  }
  RAVE_OBJECT_RELEASE(howtaskattribute);

  if (!CompositeInternal_addGainAndOffsetToField(qfield, gain, offset)) {
    RAVE_ERROR0("Failed to add gain and offset attribute to quality field");
    goto error;
  }

  if(!RaveField_createData(qfield, xsize, ysize, RaveDataType_UCHAR)) {
    RAVE_ERROR0("Failed to create quality field");
    goto error;
  }

  return qfield;
error:
  RAVE_OBJECT_RELEASE(qfield);
  RAVE_OBJECT_RELEASE(howtaskattribute);
  return NULL;

}

/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int Composite_constructor(RaveCoreObject* obj)
{
  Composite_t* this = (Composite_t*)obj;
  this->ptype = Rave_ProductType_PCAPPI;
  this->method = CompositeSelectionMethod_NEAREST;
  this->interpolationMethod = CompositeInterpolationMethod_NEAREST;
  this->height = 1000.0;
  this->elangle = 0.0;
  this->range = 500000.0;
  this->parameters = NULL;
  this->algorithm = NULL;
  this->objectList = RAVE_OBJECT_NEW(&RaveList_TYPE);
  this->datetime = RAVE_OBJECT_NEW(&RaveDateTime_TYPE);
  this->parameters = RAVE_OBJECT_NEW(&RaveList_TYPE);
  this->qiFieldName = NULL;

  if (this->objectList == NULL || this->parameters == NULL || this->datetime == NULL) {
    goto error;
  }
  return 1;
error:
  CompositeInternal_freeParameterList(&this->parameters);
  CompositeInternal_freeObjectList(&this->objectList);
  RAVE_OBJECT_RELEASE(this->datetime);
  return 0;
}

/**
 * Copy constructor.
 * @param[in] obj - the created object
 * @param[in] srcobj - the source (that is copied)
 */
static int Composite_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  Composite_t* this = (Composite_t*)obj;
  Composite_t* src = (Composite_t*)srcobj;
  this->ptype = src->ptype;
  this->method = src->method;
  this->height = src->height;
  this->elangle = src->elangle;
  this->range = src->range;
  this->algorithm = NULL;
  this->parameters = CompositeInternal_cloneParameterList(src->parameters);
  this->objectList = CompositeInternal_cloneRadarItemList(src->objectList);
  this->datetime = RAVE_OBJECT_CLONE(src->datetime);
  this->qiFieldName = NULL;

  if (this->objectList == NULL || this->datetime == NULL || this->parameters == NULL) {
    goto error;
  }

  if (!Composite_setQualityIndicatorFieldName(this, src->qiFieldName)) {
    goto error;
  }

  if (src->algorithm != NULL) {
    this->algorithm = RAVE_OBJECT_CLONE(src->algorithm);
    if (this->algorithm == NULL) {
      goto error;
    }
  }

  return 1;
error:
  CompositeInternal_freeParameterList(&this->parameters);
  CompositeInternal_freeObjectList(&this->objectList);
  RAVE_OBJECT_RELEASE(this->datetime);
  RAVE_OBJECT_RELEASE(this->algorithm);
  RAVE_FREE(this->qiFieldName);
  return 0;
}

/**
 * Destructor
 * @param[in] obj - the object to destroy
 */
static void Composite_destructor(RaveCoreObject* obj)
{
  Composite_t* this = (Composite_t*)obj;
  CompositeInternal_freeObjectList(&this->objectList);
  RAVE_OBJECT_RELEASE(this->datetime);
  CompositeInternal_freeParameterList(&this->parameters);
  RAVE_OBJECT_RELEASE(this->algorithm);
  RAVE_FREE(this->qiFieldName);
}

/**
 * Returns the lowest height found among a group of value positions.
 * @param[in] valuePositions - array with the value positions to check
 * @param[in] noOfValuePositions - the length of the value positions array
 * @returns the lowest height found among the value positions
 */
static double CompositeInternal_getValuePositionsLowestHeight(
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions)
{
  int i;
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  double lowestHeight = DBL_MAX;

  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos = &valuePositions[i];
    if (valuePos->navinfo.actual_height < lowestHeight) {
      lowestHeight = valuePos->navinfo.actual_height;
    }
  }

  return lowestHeight;
}

/**
 * Sets up an array of booleans (0 or 1), indicating in which dimensions interpolation shall be 
 * performed, based on the interpolation method set in the composite. Indices in the array match 
 * CompositeInterpolationDimension_t enum values.
 * @param[in] composite - self
 * @param[in,out] interpolationDimArray - the array of booleans indicating in which dimensions
 *                                        interpolation shall be performed
 */
static void CompositeInternal_setInterpolationDimensionsArray(
    Composite_t* composite, 
    int interpolationDimArray[]) 
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");

  int i = 0;
  for (i = 0; i < NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS; i++) {
    RAVE_ASSERT((interpolationDimArray[i] == 0), 
                "All positions in interpolation dimensions array not initiated to 0");
  }

  if (composite->interpolationMethod == CompositeInterpolationMethod_LINEAR_HEIGHT ||
      composite->interpolationMethod == CompositeInterpolationMethod_QUADRATIC_HEIGHT) {
    interpolationDimArray[CompositeInterpolationDimension_HEIGHT] = 1;
  } else if (composite->interpolationMethod == CompositeInterpolationMethod_LINEAR_RANGE) {
    interpolationDimArray[CompositeInterpolationDimension_RANGE] = 1;
  } else if (composite->interpolationMethod == CompositeInterpolationMethod_LINEAR_AZIMUTH) {
    interpolationDimArray[CompositeInterpolationDimension_AZIMUTH] = 1;
  } else if (composite->interpolationMethod == CompositeInterpolationMethod_LINEAR_RANGE_AND_AZIMUTH) {
    interpolationDimArray[CompositeInterpolationDimension_RANGE] = 1;
    interpolationDimArray[CompositeInterpolationDimension_AZIMUTH] = 1;
  } else if (composite->interpolationMethod == CompositeInterpolationMethod_LINEAR_3D ||
             composite->interpolationMethod == CompositeInterpolationMethod_QUADRATIC_3D) {
    interpolationDimArray[CompositeInterpolationDimension_HEIGHT] = 1;
    interpolationDimArray[CompositeInterpolationDimension_RANGE] = 1;
    interpolationDimArray[CompositeInterpolationDimension_AZIMUTH] = 1;
  }
}

/**
 * Creates an array of CompositeValues_t with length nparam.
 * @param[in] nparam - the number of items in the array
 * @returns the array on success or NULL on failure
 */
static CompositeValues_t* CompositeInternal_createCompositeValues(int nparam)
{
  CompositeValues_t* result = NULL;
  if (nparam > 0) {
    result = RAVE_MALLOC(sizeof(CompositeValues_t) * nparam);
    if (result == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for composite values");
    } else {
      memset(result, 0, sizeof(CompositeValues_t) * nparam);
    }
  }
  return result;
}

/**
 * Resets the array of composite values except the CartesianParam parameter
 * and the dToRadar field.
 * @param[in] nparam - number of parameters
 * @param[in] p - pointer at the array
 */
static void CompositeInternal_resetCompositeValues(Composite_t* composite, int nparam, CompositeValues_t* p)
{
  int i = 0;
  for (i = 0; i < nparam; i++) {
    p[i].mindist = 1e10;
    p[i].radarindex = -1;
    p[i].noOfValuePositions = 0;
    p[i].vtype = RaveValueType_NODATA;
    p[i].name = (const char*)((CompositingParameter_t*)RaveList_get(composite->parameters, i))->name;
    p[i].qivalue = 0.0;
  }
}

/**
 * Returns 1 if the interpolation method of the composite indicates that weights should be
 * raised to the power of 2 before applying them in value interpolation, otherwise 0.
 *
 * @param[in] composite - self
 * @return 1 if interpolation method is quadratic, otherwise 0
 */
static int CompositeInternal_isQuadraticInterpolation(Composite_t* composite) {
  int isQuadratic = 0;
  if (composite->interpolationMethod == CompositeInterpolationMethod_QUADRATIC_HEIGHT ||
      composite->interpolationMethod == CompositeInterpolationMethod_QUADRATIC_3D) {
    isQuadratic = 1;;
  }
  return isQuadratic;
}

/**
 * Tries to find the next available integer that is not filtered by indexes.
 * @param[in] indexes - filter of already used integers
 * @param[in] n_objs - number of indexes
 * @param[in] available - the integer that should be tested for availability
 * @return 1 if already exists, otherwise 0
 */
static int CompositeInternal_containsRadarIndex(int* indexes, int n_objs, int available)
{
  int i = 0;
  for (i = 0; i < n_objs; i++) {
    if (indexes[i] == available) {
      return 1;
    }
  }
  return 0;
}

static char* CompositeInternal_getTypeAndIdFromSource(const char* source, const char* id)
{
  char* result = NULL;
  if (source != NULL && id != NULL) {
    char* p = strstr(source, id);
    if (p != NULL) {
      int len = 0;
      char* pbrk = NULL;
      len = strlen(p);
      pbrk = strpbrk((const char*)p, ",");

      if (pbrk != NULL) {
        len = pbrk - p;
      }
      result = RAVE_MALLOC(sizeof(char) * (len + 1));
      if (result != NULL) {
        strncpy(result, p, len);
        result[len] = '\0';
      }
    }
  }
  return result;
}

static char* CompositeInternal_getIdFromSource(const char* source, const char* id)
{
  char* result = NULL;
  if (source != NULL && id != NULL) {
    char* p = strstr(source, id);
    if (p != NULL) {
      int len = 0;
      char* pbrk = NULL;
      p += strlen(id);
      len = strlen(p);
      pbrk = strpbrk((const char*)p, ",");

      if (pbrk != NULL) {
        len = pbrk - p;
      }

      result = RAVE_MALLOC(sizeof(char) * (len + 1));
      if (result != NULL) {
        strncpy(result, p, len);
        result[len] = '\0';
      }
    }
  }
  return result;
}

static char* CompositeInternal_getAnyIdFromSource(const char* source)
{
  char* result = NULL;
  result = CompositeInternal_getIdFromSource(source, "WMO:");
  if (result == NULL) {
    result = CompositeInternal_getIdFromSource(source, "RAD:");
  }
  if (result == NULL) {
    result = CompositeInternal_getIdFromSource(source, "NOD:");
  }
  if (result == NULL) {
    result = CompositeInternal_getIdFromSource(source, "CMT:");
  }
  return result;
}

static int CompositeInternal_concateStr(char** ids, int* len, const char* str)
{
  int result = 0;
  char* p = *ids;
  int n = *len;
  int currStrLen = strlen(p);
  if (currStrLen + strlen(str) + 1 > n) {
    int newsize = n + strlen(str) + 1;
    char* newp = RAVE_REALLOC(p, newsize * sizeof(char));
    if (newp != NULL) {
      p = newp;
      n = newsize;
    } else {
      goto done;
    }
  }
  strcat(p, str);

  *ids = p;
  *len = n;
  result = 1;
done:
  return result;
}

static int CompositeInternal_concateInt(char** ids, int* len, int value)
{
  char buff[16];
  memset(buff, 0, sizeof(char)*16);
  snprintf(buff, 16, "%d", value);
  return CompositeInternal_concateStr(ids, len, buff);
}
/**
 * Returns the next available integer with a filter of already aquired indexes.
 * We assume that we always want to index radars from 1-gt;N. Which means that the first time
 * you call this function you should specify lastIndex = 0, then the subsequent calls the
 * index returned from this function should be passed in the next iteration.
 *
 * I.e.
 * lastIndex = 0
 * lastIndex = CompositeInternal_nextAvailableRadarIndexValue(indexes, n_objs, lastIndex);
 * lastIndex = CompositeInternal_nextAvailableRadarIndexValue(indexes, n_objs, lastIndex);
 * and so on.
 *
 */
static int CompositeInternal_nextAvailableRadarIndexValue(int* indexes, int n_objs, int lastIndex)
{
  int ctr = lastIndex + 1;
  while(CompositeInternal_containsRadarIndex(indexes, n_objs, ctr)) {
    ctr++;
  }
  return ctr;
}

/**
 * Makes sure that all objects used in the composite gets a unique value.
 */
static int CompositeInternal_updateRadarIndexes(Composite_t* composite, RaveObjectHashTable_t* mapping)
{
  int n_objs = 0, i = 0;
  int result = 0;
  int* indexes = NULL;
  int lastIndex = 0;

  if (!CompositeInternal_verifyRadarIndexMapping(mapping)) {
    goto done;
  }

  /* First reset indexes */
  n_objs = RaveList_size(composite->objectList);
  indexes = RAVE_MALLOC(sizeof(int) * n_objs);
  if (indexes == NULL) {
    goto done;
  }
  memset(indexes, 0, sizeof(int)*n_objs);

  for (i = 0; i < n_objs; i++) {
    CompositeRadarItem_t* ri = (CompositeRadarItem_t*)RaveList_get(composite->objectList, i);
    char* src = NULL;
    ri->radarIndexValue = 0;

    if (RAVE_OBJECT_CHECK_TYPE(ri->object, &PolarVolume_TYPE)) {
      src = (char*)PolarVolume_getSource((PolarVolume_t*)ri->object);
    } else if (RAVE_OBJECT_CHECK_TYPE(ri->object, &PolarScan_TYPE)) {
      src = (char*)PolarScan_getSource((PolarScan_t*)ri->object);
    }

    if (src != NULL) {
      char *str = NULL;
      str = CompositeInternal_getTypeAndIdFromSource(src, "WMO:");
      if (str == NULL || !RaveObjectHashTable_exists(mapping, str)) {
        RAVE_FREE(str);
        str = CompositeInternal_getTypeAndIdFromSource(src, "RAD:");
      }
      if (str == NULL || !RaveObjectHashTable_exists(mapping, str)) {
        RAVE_FREE(str);
        str = CompositeInternal_getTypeAndIdFromSource(src, "NOD:");
      }

      if (str != NULL && RaveObjectHashTable_exists(mapping, str)) {
        RaveAttribute_t* attr = (RaveAttribute_t*)RaveObjectHashTable_get(mapping, str);
        long v = 0;
        if (RaveAttribute_getLong(attr, &v)) {
          ri->radarIndexValue = (int)v;
          indexes[i] = (int)v;
        }
        RAVE_OBJECT_RELEASE(attr);
      } else if (RaveObjectHashTable_exists(mapping, src)) {
        RaveAttribute_t* attr = (RaveAttribute_t*)RaveObjectHashTable_get(mapping, src);
        long v = 0;
        if (RaveAttribute_getLong(attr, &v)) {
          ri->radarIndexValue = (int)v;
          indexes[i] = (int)v;
        }
        RAVE_OBJECT_RELEASE(attr);
      }

      RAVE_FREE(str);
    }
  }

  /* Any radarIndexValue = 0, needs to get a suitable value, take first available one */
  for (i = 0; i < n_objs; i++) {
    CompositeRadarItem_t* ri = (CompositeRadarItem_t*)RaveList_get(composite->objectList, i);
    if (ri->radarIndexValue == 0) {
      ri->radarIndexValue = lastIndex = CompositeInternal_nextAvailableRadarIndexValue(indexes, n_objs, lastIndex);
    }
  }

  result = 1;
done:
  RAVE_FREE(indexes);
  return result;
}

/**
 * Returns a list of the closest positions surrounding the specified lon/lat according to
 * the composites attributes like type/elevation/height/etc.
 * @param[in] composite - self
 * @param[in] object - the data object
 * @param[in] plon - the longitude
 * @param[in] plat - the latitude
 * @param[in] surroundingScans - indicates whether surrounding or nearest positions in the
 *                               height dimension should be collected. 0 indicates that 
 *                               only the nearest scan will be used, while 1 indicates that 
 *                               positions both on the closest scan above and the closest scan 
 *                               below will be returned. 
 * @param[in] surroundingRangeBins - indicates whether surrounding or nearest positions in the
 *                               range dimension should be collected. 0 indicates that 
 *                               only the nearest range bin will be used, while 1 indicates that 
 *                               positions for both the range bin above and the range bin  
 *                               below the target range will be returned.
 * @param[in] surroundingRays - indicates whether surrounding or nearest positions in the
 *                               azimuth dimension should be collected. 0 indicates that 
 *                               only the positions on the nearest ray will be used, while 1 
 *                               indicates that positions both on the ray to the 'left' and the 
 *                               'right' of the target azimuth will be returned.
 * @param[out] navinfos - array of navigation information structs for the surrounding 
 *                        positions
 * @return 1 if successful or 0 if not
 */
static int CompositeInternal_surroundingPositions(
  Composite_t* composite,
  RaveCoreObject* object,
  double plon,
  double plat,
  int surroundingScans,
  int surroundingRangeBins,
  int surroundingRays,
  PolarNavigationInfo navinfos[])
{
  int result = 0;

  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((navinfos != NULL), "navinfos == NULL");
  
  if (object != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
      if (composite->ptype == Rave_ProductType_PPI ||
          composite->ptype == Rave_ProductType_PCAPPI ||
          composite->ptype == Rave_ProductType_PMAX) {
        result = PolarScan_getSurroundingNavigationInfos((PolarScan_t*)object,
                                                         plon,
                                                         plat,
                                                         surroundingRangeBins,
                                                         surroundingRays,
                                                         navinfos);
      }
    } else if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      if (composite->ptype == Rave_ProductType_PCAPPI ||
          composite->ptype == Rave_ProductType_CAPPI ||
          composite->ptype == Rave_ProductType_PMAX) {

        int insidee = (composite->ptype == Rave_ProductType_PCAPPI || composite->ptype == Rave_ProductType_PMAX)?0:1;
        result = PolarVolume_getSurroundingNavigationInfos((PolarVolume_t*)object,
                                                           plon,
                                                           plat,
                                                           Composite_getHeight(composite),
                                                           insidee,
                                                           surroundingScans,
                                                           surroundingRangeBins,
                                                           surroundingRays,
                                                           navinfos);

      } else if (composite->ptype == Rave_ProductType_PPI) {
        PolarScan_t* scan = PolarVolume_getScanClosestToElevation((PolarVolume_t*)object,
                                                                  Composite_getElevationAngle(composite),
                                                                  0);
        if (scan == NULL) {
          RAVE_ERROR1("Failed to fetch scan nearest to elevation %g",
                      Composite_getElevationAngle(composite));
          goto done;
        }
        result = PolarScan_getSurroundingNavigationInfos(scan,
                                                         plon,
                                                         plat,
                                                         surroundingRangeBins,
                                                         surroundingRays,
                                                         navinfos);

        PolarVolume_addEiForNavInfos((PolarVolume_t*)object, scan, navinfos, result, 0);

        RAVE_OBJECT_RELEASE(scan);
      }
    }
  }

done:
  return result;
}

/**
 * Returns the position that is closest to the specified lon/lat according to
 * the composites attributes like type/elevation/height/etc.
 * @param[in] composite - self
 * @param[in] object - the data object
 * @param[in] plon - the longitude
 * @param[in] plat - the latitude
 * @param[out] nav - the navigation information
 * @return 1 if hit or 0 if outside
 */
static int CompositeInternal_nearestPosition(
  Composite_t* composite,
  RaveCoreObject* object,
  double plon,
  double plat,
  PolarNavigationInfo* nav)
{
  int result = 0;

  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((nav != NULL), "nav == NULL");
  
  if (object != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
      if (composite->ptype == Rave_ProductType_PPI ||
          composite->ptype == Rave_ProductType_PCAPPI ||
          composite->ptype == Rave_ProductType_PMAX) {
        result = PolarScan_getNearestNavigationInfo((PolarScan_t*)object, plon, plat, nav);
      }
    } else if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      if (composite->ptype == Rave_ProductType_PCAPPI ||
          composite->ptype == Rave_ProductType_CAPPI ||
          composite->ptype == Rave_ProductType_PMAX) {
        int insidee = (composite->ptype == Rave_ProductType_PCAPPI || composite->ptype == Rave_ProductType_PMAX)?0:1;
        result = PolarVolume_getNearestNavigationInfo((PolarVolume_t*)object,
                                                      plon,
                                                      plat,
                                                      Composite_getHeight(composite),
                                                      insidee,
                                                      nav);
      } else if (composite->ptype == Rave_ProductType_PPI) {
        PolarScan_t* scan = PolarVolume_getScanClosestToElevation((PolarVolume_t*)object,
                                                                  Composite_getElevationAngle(composite),
                                                                  0);
        if (scan == NULL) {
          RAVE_ERROR1("Failed to fetch scan nearest to elevation %g",
                      Composite_getElevationAngle(composite));
          goto done;
        }
        result = PolarScan_getNearestNavigationInfo((PolarScan_t*)scan, plon, plat, nav);
        nav->ei = PolarVolume_indexOf((PolarVolume_t*)object, scan);
        RAVE_OBJECT_RELEASE(scan);
      }
    }
  }

done:
  return result;
}

/**
 * Returns a value position (CompositeValuePosition_t) that is closest to
 * the specified lon/lat according to the composites attributes like
 * type/elevation/height/etc.
 * @param[in] composite - self
 * @param[in] object - the data object
 * @param[in] plon - the longitude
 * @param[in] plat - the latitude
 * @param[out] valuePositions - array of value positions. Only position
 *                              0 used for this function
 * @return 1 if hit or 0 if outside
 */
static int CompositeInternal_getValuePositions_nearest(
  Composite_t* composite,
  RaveCoreObject* object,
  double plon,
  double plat,
  CompositeValuePosition_t valuePositions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");

  int result = -1;

  CompositeValuePosition_t* valuePosition = &valuePositions[0];
  if (CompositeInternal_nearestPosition(composite, object, plon, plat, &valuePosition->navinfo)) {
    valuePosition->valid = 1;
    result = 1;
  }

  return result;
}

/**
 * Returns a list of value position (CompositeValuePosition_t) that are
 * closest to the specified lon/lat according to the composites attributes like
 * type/elevation/height/etc. How many and which positions that are returned is
 * depending on the interpolation dimensions provided.
 * @param[in] composite - self
 * @param[in] object - the data object
 * @param[in] plon - the longitude
 * @param[in] plat - the latitude
 * @param[in] interpolationDimensions - array indicating in which dimensions
 *                                      interpolation shall be performed. This
 *                                      affects the number of value positions that
 *                                      are returned.
 * @param[out] valuePositions - array of value positions.
 * @return the number of value positions contained in the valuePositions array
 */
static int CompositeInternal_getValuePositions_interpolated(
  Composite_t* composite,
  RaveCoreObject* object,
  double plon,
  double plat,
  int interpolationDimensions[],
  CompositeValuePosition_t valuePositions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");

  PolarNavigationInfo navinfos[MAX_NO_OF_SURROUNDING_POSITIONS];

  int noofNavinfos = CompositeInternal_surroundingPositions(composite,
                                                            object,
                                                            plon,
                                                            plat,
                                                            interpolationDimensions[CompositeInterpolationDimension_HEIGHT],
                                                            interpolationDimensions[CompositeInterpolationDimension_RANGE],
                                                            interpolationDimensions[CompositeInterpolationDimension_AZIMUTH],
                                                            navinfos);

  int i;
  for (i = 0; i < noofNavinfos; i++) {
    valuePositions[i].navinfo = navinfos[i];
    valuePositions[i].valid = 1;
    valuePositions[i].value = 0;
  }

  return noofNavinfos;
}

/**
 * Returns a list of value position (CompositeValuePosition_t) that are
 * closest to the specified lon/lat according to the composites attributes like
 * type/elevation/height/etc. How many and which positions that are returned is
 * depending on the interpolation dimensions provided. If interpolation method
 * 'nearest' is used, the interpolation dimensions are ignored and only one
 * nearest value position is returned.
 * @param[in] composite - self
 * @param[in] object - the data object
 * @param[in] plon - the longitude
 * @param[in] plat - the latitude
 * @param[in] interpolationDimensions - array indicating in which dimensions
 *                                      interpolation shall be performed. This
 *                                      affects the number of value positions that
 *                                      are returned.
 * @param[out] valuePositions - array of value positions.
 * @return the number of value positions contained in the valuePositions array
 */
static int CompositeInternal_getValuePositions(
  Composite_t* composite,
  RaveCoreObject* object,
  double plon,
  double plat,
  int interpolationDimensions[],
  CompositeValuePosition_t valuePositions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");

  int noOfValuePositions = -1;

  if (composite->interpolationMethod == CompositeInterpolationMethod_NEAREST) {
    noOfValuePositions = CompositeInternal_getValuePositions_nearest(composite, object, plon, plat, valuePositions);
  } else {
    noOfValuePositions = CompositeInternal_getValuePositions_interpolated(composite, object, plon, plat, interpolationDimensions, valuePositions);
  }

  return noOfValuePositions;
}

/**
 * Gets the quality value at the specified position for the specified quantity and quality field.
 * @param[in] composite - self
 * @param[in] obj - the object
 * @param[in] quantity - the quantity
 * @param[in] qualityField - the quality field
 * @param[in] nav - the navigation information
 * @param[out] value - the value
 * @return 1 on success or 0 if value not could be retrieved
 */
static int CompositeInternal_getQualityValueAtPosition(
  Composite_t* composite,
  RaveCoreObject* obj,
  const char* quantity,
  const char* qualityField,
  PolarNavigationInfo* nav,
  double* value)
{
  int result = 0;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((nav != NULL), "nav == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  RAVE_ASSERT((qualityField != NULL), "qualityField == NULL");
  *value = 0.0;

  if (obj != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
      if (!PolarScan_getQualityValueAt((PolarScan_t*)obj, quantity, nav->ri, nav->ai, qualityField, 1, value)) {
        *value = 0.0;
      }
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
      if (!PolarVolume_getQualityValueAt((PolarVolume_t*)obj, quantity, nav->ei, nav->ri, nav->ai, qualityField, 1, value)) {
        *value = 0.0;
      }
    } else {
      RAVE_WARNING0("Unsupported object type");
      goto done;
    }
  }

  result = 1;
done:
  return result;
}

/**
 * Gets the value at the specified position for the specified quantity.
 * @param[in] composite - self
 * @param[in] obj - the object
 * @param[in] quantity - the quantity
 * @param[in] nav - the navigation information
 * @param[out] type - the value type
 * @param[out] value - the value
 * @return 1 on success or 0 if value not could be retrieved
 */
static int CompositeInternal_getValueAtPosition(
  Composite_t* composite,
  RaveCoreObject* obj,
  const char* quantity,
  PolarNavigationInfo* nav,
  RaveValueType* type,
  double* value,
  double* qiv)
{
  int result = 0;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((nav != NULL), "nav == NULL");
  RAVE_ASSERT((type != NULL), "type == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  *qiv = 0.0;

  if (obj != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
      *type = PolarScan_getConvertedParameterValue((PolarScan_t*)obj, quantity, nav->ri, nav->ai, value);
      if (composite->qiFieldName != NULL) {
        if (!PolarScan_getQualityValueAt((PolarScan_t*)obj, quantity, nav->ri, nav->ai, (const char*)composite->qiFieldName, 0, qiv)) {
          *qiv = 0.0;
        }
      }
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
      *type = PolarVolume_getConvertedParameterValueAt((PolarVolume_t*)obj, quantity, nav->ei, nav->ri, nav->ai, value);
      if (composite->qiFieldName != NULL) {
        if (!PolarVolume_getQualityValueAt((PolarVolume_t*)obj, quantity, nav->ei, nav->ri, nav->ai, (const char*)composite->qiFieldName, 0, qiv)) {
          *qiv = 0.0;
        }
      }
    } else {
      RAVE_WARNING0("Unsupported object type");
      goto done;
    }
  }

  result = 1;
done:
  return result;
}

/**
 * Searches an array of value positions for a position that can be used in interpolation 
 * together with an input position in a specific dimension. For a position to be considered 
 * matching, its position attrubtes must be matching in all dimensions except the one in
 * which interpolation is to be performed. An index is returned if a match is found, 
 * otherwise -1. 
 * 
 * @param[in] valuePos - the source value position for which a match will be searched
 * @param[in] valuePositions - the array of value positions to search
 * @param[in] noOfValuePositions - the length of the value position array
 * @param[in] dimension - the dimension in which interpolation is intended to be performed.
 *                        which value positions is considered matching is based on the 
 *                        dimension.
 * @param[in] startPos - position in the value position array (valuePositions) where the 
 *                       search will be started. Indices below this will not be checked.
 * @return an index on success or -1 if no match was found
 */
static int CompositeInternal_getMatchingValuePosIndex(
    CompositeValuePosition_t* valuePos,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    CompositeInterpolationDimension_t dimension,
    int startPos)
{
	RAVE_ASSERT((valuePos != NULL), "valuePos == NULL");
	RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");

  PolarNavigationInfo* navinfo = &valuePos->navinfo;

  int foundIndex = -1;

  int i = 0;
  for (i = startPos; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* compareValuePos = &valuePositions[i];
    PolarNavigationInfo* compareNavinfo = &compareValuePos->navinfo;

    int matchFound = 0;
    if (dimension == CompositeInterpolationDimension_HEIGHT) {
      matchFound = (compareNavinfo->actual_azimuth == navinfo->actual_azimuth) &&
                   (compareNavinfo->actual_range   == navinfo->actual_range);
    } else if (dimension == CompositeInterpolationDimension_RANGE) {
      matchFound = (compareNavinfo->actual_azimuth == navinfo->actual_azimuth) &&
                   (compareNavinfo->elevation      == navinfo->elevation);
    } else if (dimension == CompositeInterpolationDimension_AZIMUTH) {
      matchFound = (compareNavinfo->elevation      == navinfo->elevation) &&
                   (compareNavinfo->actual_range   == navinfo->actual_range);
    } else {
      RAVE_ERROR1("Invalid interpolation dimension: %i", dimension);
    }

    if (matchFound) {
      foundIndex = i;
      break;
    }
  }

  return foundIndex;
}

/**
 * Calculates and returns the the absolute difference between actual and targeted position 
 * of a navigation info structure (PolarNavigationInfo) in one dimension. The unit depends 
 * on the dimension.
 * 
 * @param[in] navinfo - the navigation information
 * @param[in] dimension - the dimension in which to collect the position difference
 * 
 * @return the absolute difference between target position and actual position
 */
static double CompositeInternal_getDimensionAbsoluteDiff(
    PolarNavigationInfo* navinfo,
    CompositeInterpolationDimension_t dimension)
{
  RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");

  double dimensionDiff = 0;
  if (dimension == CompositeInterpolationDimension_HEIGHT) {
    dimensionDiff = fabs(navinfo->actual_height - navinfo->height);
  } else if (dimension == CompositeInterpolationDimension_RANGE) {
    dimensionDiff = fabs(navinfo->actual_range - navinfo->range);
  } else if (dimension == CompositeInterpolationDimension_AZIMUTH) {
    dimensionDiff = fabs(navinfo->actual_azimuth - navinfo->azimuth);
    while (dimensionDiff > (M_PI / 2)) {
      dimensionDiff = fabs(dimensionDiff - M_PI);
    }
  } else {
    RAVE_ERROR1("Invalid interpolation dimension: %i", dimension);
  }

  return dimensionDiff;
}

/**
 * Alters a navigation info structure (PolarNavigationInfo) so that actual 
 * position is set to the target position in one dimension. This is something 
 * that can be done after an interpolation, with the interpolated navigation
 * info, since the position can be seen as placed on the target.
 * 
 * @param[in] navinfo - the navigation information
 * @param[in] dimension - the dimension in which to set interpolated position
 * 
 * @return 1 if succesfully updated navinfo, 0 if update failed
 */
static int CompositeInternal_setInterpolatedPosition(
    PolarNavigationInfo* navinfo,
    CompositeInterpolationDimension_t dimension)
{
	RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");

  if (dimension == CompositeInterpolationDimension_HEIGHT) {
    navinfo->actual_height = navinfo->height;
  } else if (dimension == CompositeInterpolationDimension_RANGE) {
    navinfo->actual_range = navinfo->range;
  } else if (dimension == CompositeInterpolationDimension_AZIMUTH) {
    navinfo->actual_azimuth = navinfo->azimuth;
  } else {
    RAVE_ERROR1("Invalid interpolation dimension: %i", dimension);
    return 0;
  }

  return 1;
}

/**
 * Combines value positions by performing interpolation in one dimension. For each 
 * value position in the provided array, the rest of the array is searched for value
 * positions that are located equally, except in the interpolation dimension. When 
 * such a matching value position is found, the two value positions are combined by 
 * performing an interpolation. The two value positions are replaced by one result 
 * value position, with an interpolated value and a position equal to the original 
 * value positions, except in the interpolation dimension where the location is now
 * at the target position (between the interpolated positions).
 * 
 * @param[in] composite - self
 * @param[in] dimension - the dimension in which to combine value positions
 * @param[in] valuePositions - value positions to combine
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * 
 * @return the number of resulting value positions
 */
static int CompositeInternal_combineValuePosInDimension(
    Composite_t* composite,
    CompositeInterpolationDimension_t dimension,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");

  int noOfResultPositions = 0;

  int i = 0;
  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos1 = &valuePositions[i];
    if (valuePos1->valid == 1) {
      int valuePos2Index = CompositeInternal_getMatchingValuePosIndex(valuePos1, valuePositions, noOfValuePositions, dimension, i+1);

      if (valuePos2Index != -1) {
        CompositeValuePosition_t* valuePos2 = &valuePositions[valuePos2Index];
        valuePos2->valid = 0;

        double dimensionDiff1 = CompositeInternal_getDimensionAbsoluteDiff(&valuePos1->navinfo, dimension);
        double dimensionDiff2 = CompositeInternal_getDimensionAbsoluteDiff(&valuePos2->navinfo, dimension);
        double weight1 = dimensionDiff1;
        double weight2 = dimensionDiff2;
        if (CompositeInternal_isQuadraticInterpolation(composite)) {
          weight1 = pow(weight1, 2);
          weight2 = pow(weight2, 2);
        }
        double totalWeight = weight1 + weight2;
        weight1 = 1.0 - (weight1 / totalWeight); // inverted weight
        weight2 = 1.0 - (weight2 / totalWeight); // inverted weight

        // update value pos 1
        valuePos1->value = (valuePos2->value * weight2) + (valuePos1->value * weight1);
        valuePos1->qivalue = (valuePos2->qivalue * weight2) + (valuePos1->qivalue * weight1);

        if (valuePos2->type == RaveValueType_DATA || valuePos1->type == RaveValueType_DATA) {
          // if any of the positions has data, set the overall type to DATA
          valuePos1->type = RaveValueType_DATA;
        } else if (valuePos2->type == RaveValueType_UNDETECT || valuePos1->type == RaveValueType_UNDETECT) {
          valuePos1->type = RaveValueType_UNDETECT;
        } else {
          valuePos1->type = RaveValueType_NODATA;
        }
      }

      CompositeInternal_setInterpolatedPosition(&valuePos1->navinfo, dimension);
      valuePositions[noOfResultPositions] = *valuePos1;
      noOfResultPositions++;
    }
  }

  return noOfResultPositions;
}

/**
 * Performs interpolation between a number of input value positions in the dimensions
 * defined by input parameter 'interpolationDimensions'. 
 * 
 * @param[in] composite - self
 * @param[in] interpolationDimensions - the dimensions in which to perform interpolation
 * @param[in] valuePositions - value positions to interpolate
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[out] type - the resulting value type
 * @param[out] value - the resulting value
 * @param[out] qiv - the resulting quality indicator value
 * 
 * @return 1 if interpolation was successful, 0 otherwise
 */
static int CompositeInternal_interpolateValuePositions(
    Composite_t* composite,
    int interpolationDimensions[],
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    RaveValueType* type,
    double* value,
    double* qiv)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((type != NULL), "type == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  RAVE_ASSERT((qiv != NULL), "qiv == NULL");

  int result = 0;
  CompositeValuePosition_t interpolatedPositions[MAX_NO_OF_SURROUNDING_POSITIONS];

  // copy value positions to array to not effect originals
  memcpy(interpolatedPositions, valuePositions, sizeof(CompositeValuePosition_t)*MAX_NO_OF_SURROUNDING_POSITIONS);

  int i;
  for (i = 0; i < NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS; i++) {
    if (interpolationDimensions[i]) {
      noOfValuePositions = CompositeInternal_combineValuePosInDimension(composite, i, interpolatedPositions, noOfValuePositions);
      if (noOfValuePositions == 1) {
        break;
      }
    }
  }

  if (noOfValuePositions != 1) {
    RAVE_ERROR1("Only one value position should remain after interpolation. Remaining value positions: %i", noOfValuePositions);
    goto done;
  }

  CompositeValuePosition_t* valuePos = &interpolatedPositions[0];
  *value = valuePos->value;
  *qiv = valuePos->qivalue;
  *type = valuePos->type;

  result = 1;
done:
  return result;
}

/**
 * Prepares a navigation info structure (PolarNavigationInfo) so that it can be
 * used in interpolation. For the dimensions where no interpolation will be
 * performed, the actual position is set to the target dimension. This is done
 * to allow the interpolation method to correctly recognise which positions to
 * interpolate between.
 *
 * @param[in] navinfo - the navigation information
 * @param[in] dimension - the dimension in which to set interpolated position
 *
 * @return 1 if succesfully updated navinfo, 0 if update failed
 */
static void CompositeInternal_prepareNavinfoForInterpolation(
    PolarNavigationInfo* navinfo,
    int interpolationDimensions[])
{
  RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");

  if (!interpolationDimensions[CompositeInterpolationDimension_RANGE]) {
    // no range interpolation shall be done
    if (!CompositeInternal_setInterpolatedPosition(navinfo, CompositeInterpolationDimension_RANGE)) {
      return;
    }
  }

  if (!interpolationDimensions[CompositeInterpolationDimension_AZIMUTH]) {
    // no azimuth interpolation shall be done
    if (!CompositeInternal_setInterpolatedPosition(navinfo, CompositeInterpolationDimension_AZIMUTH)) {
      return;
    }
  }
}

/**
 * Gets a quality value at a specific x-, y-coordinate for a quality 
 * algorithm that supports filling quality information. The value will be
 * interpolated, based on the input value position array anf the 
 * interpolation dimensions.
 * 
 * @param[in] composite - self
 * @param[in] obj - the rave core object instance
 * @param[in] field - the quality field
 * @param[in] name - the quality field name
 * @param[in] quantity - the quantity
 * @param[in] valuePositions - value positions to interpolate
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[in] x - x-coordinate
 * @param[in] y - y-coordinate
 * @param[in] interpolationDimensions - the dimensions in which to interpolate
 * 
 * @return 1 if successful, 0 otherwise
 */
static int CompositeInternal_getInterpolatedAlgorithmQualityValue(
    Composite_t* composite,
    RaveCoreObject* obj,
    RaveField_t* field,
    const char* name,
    const char* quantity,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    int x,
    int y,
    int interpolationDimensions[],
    double* resultValue)
{
	RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((field != NULL), "field == NULL");
  RAVE_ASSERT((name != NULL), "name == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  RAVE_ASSERT((resultValue != NULL), "resultValue == NULL");
  
  int result = 0;

  int i = 0;
  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos = &valuePositions[i];

    double value = 0;
    if (CompositeAlgorithm_fillQualityInformation(composite->algorithm, obj, name, quantity, field, x, y, &valuePos->navinfo,
                                                  COMPOSITE_QUALITY_FIELDS_GAIN, COMPOSITE_QUALITY_FIELDS_OFFSET)) {
      RaveField_getValue(field, x, y, &value);
    }

    valuePos->value = value;
    valuePos->type = RaveValueType_DATA; // assume all positions has data

    CompositeInternal_prepareNavinfoForInterpolation(&valuePos->navinfo, interpolationDimensions);
  }

  RaveValueType type;
  double qiv;
  if (!CompositeInternal_interpolateValuePositions(composite, interpolationDimensions, valuePositions, noOfValuePositions,
                                                   &type, resultValue, &qiv)) {
    goto done;
  }

  result = 1;
done:
  return result;
}

/**
 * Prepares value positions for interpolation by setting height values as 'value' in 
 * them, based on the position information.
 * 
 * @param[in] composite - self
 * @param[in] obj - the rave core object instance
 * @param[in] quantity - unused in this function. need to be present to comply 
 *                       with function pointer pattern
 * @param[in] qualityField - unused in this function. need to be present to comply 
 *                           with function pointer pattern
 * @param[in] valuePositions - value positions to prepare
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[in] interpolationDimensions - the dimensions in which to interpolate
 * 
 * @return 1 if successful, 0 otherwise
 */
static int CompositeInternal_setHeightValuesInValuePos(
    Composite_t* composite,
    RaveCoreObject* obj,
    const char* quantity,
    const char* qualityField,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    int interpolationDimensions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  
  int i = 0;
  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos = &valuePositions[i];

    valuePos->value = valuePos->navinfo.actual_height/HEIGHT_RESOLUTION;

    CompositeInternal_prepareNavinfoForInterpolation(&valuePos->navinfo, interpolationDimensions);
  }

  return 1;
}

/**
 * Prepares value positions for interpolation by setting quality values as 'value' in 
 * them, based on the position information, quantity and quality field.
 * 
 * @param[in] composite - self
 * @param[in] obj - the rave core object instance
 * @param[in] quantity - the quantity
 * @param[in] qualityField - the quality field to collect value for
 * @param[in] valuePositions - value positions to prepare
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[in] interpolationDimensions - the dimensions in which to interpolate
 * 
 * @return 1 if successful, 0 otherwise
 */
static int CompositeInternal_setQualityValuesInValuePos(
    Composite_t* composite,
    RaveCoreObject* obj,
    const char* quantity,
    const char* qualityField,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    int interpolationDimensions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((qualityField != NULL), "qualityField == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  
  int i = 0;
  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos = &valuePositions[i];

    if (!CompositeInternal_getQualityValueAtPosition(composite, obj, quantity, qualityField, &valuePos->navinfo, &valuePos->value)) {
      return 0;
    }

    valuePos->type = RaveValueType_DATA;

    CompositeInternal_prepareNavinfoForInterpolation(&valuePos->navinfo, interpolationDimensions);
  }

  return 1;
}

/**
 * Prepares value positions for interpolation by quantity values as 'value' in 
 * them, based on the position information.
 * 
 * @param[in] composite - self
 * @param[in] obj - the rave core object instance
 * @param[in] quantity - the quantity
 * @param[in] qualityField - unused in this function. need to be present to comply 
 *                           with function pointer pattern
 * @param[in] valuePositions - value positions to prepare
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[in] interpolationDimensions - the dimensions in which to interpolate
 * 
 * @return 1 if successful, 0 otherwise
 */
static int CompositeInternal_setValuesInValuePos(
    Composite_t* composite,
    RaveCoreObject* obj,
    const char* quantity,
    const char* qualityField,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    int interpolationDimensions[])
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((quantity != NULL), "quantity == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  
  int i = 0;
  for (i = 0; i < noOfValuePositions; i++) {
    CompositeValuePosition_t* valuePos = &valuePositions[i];

    if (!CompositeInternal_getValueAtPosition(composite, obj, quantity, &valuePos->navinfo, &valuePos->type, &valuePos->value, &valuePos->qivalue)) {
      return 0;
    }

    if (valuePos->value == RaveValueType_UNDETECT) {
      CompositingParameter_t* param = CompositeInternal_getParameterByName(composite, quantity);
      valuePos->value = param->minvalue;
    }

    CompositeInternal_prepareNavinfoForInterpolation(&valuePos->navinfo, interpolationDimensions);
  }

  return 1;
}

/**
 * 
 * 
 * @param[in] composite - self
 * @param[in] obj - the rave core object instance
 * @param[in] interpolationDimensions - the dimensions in which to interpolate
 * @param[in] quantity - the quantity
 * @param[in] qualityField - the quality field. only used in combination with 
 *                           some prepare-value-functions
 * @param[in] valuePositions - value positions to prepare
 * @param[in] noOfValuePositions - the length of the value positions array (valuePositions)
 * @param[in] prepareValuePosFunc - function pointer to function that will be used to
 *                                  prepare the value positions with correct values, prior to
 *                                  interpolation
 * @param[out] type - the resulting value type
 * @param[out] value - the resulting value
 * @param[out] qiv - the resulting quality indicator value
 * 
 * @return 1 if successful, 0 otherwise
 */
static int CompositeInternal_getInterpolatedValue(
    Composite_t* composite,
    RaveCoreObject* obj,
    int interpolationDimensions[],
    const char* quantity,
    const char* qualityField,
    CompositeValuePosition_t valuePositions[],
    int noOfValuePositions,
    FUNC_PREPARE_VALUEPOS prepareValuePosFunc,
    RaveValueType* type,
    double* value,
    double* qiv)
{
  int result = 0;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");
  RAVE_ASSERT((valuePositions != NULL), "valuePositions == NULL");
  RAVE_ASSERT((type != NULL), "type == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  RAVE_ASSERT((qiv != NULL), "qiv == NULL");

  *type = RaveValueType_NODATA;

  if (noOfValuePositions == 0) {
    goto done;
  }

  if (!prepareValuePosFunc(composite, obj, quantity, qualityField, valuePositions, noOfValuePositions, interpolationDimensions)) {
    goto done;
  }

  if (!CompositeInternal_interpolateValuePositions(composite,
                                                   interpolationDimensions,
                                                   valuePositions,
                                                   noOfValuePositions,
                                                   type,
                                                   value,
                                                   qiv)) {
    goto done;
  }

  result = 1;
done:
  return result;
}

/**
 * Returns the vertical max value for the specified quantity at the provided lon/lat position.
 * If no suitable value is found, vtype and vvalue will be left as is.
 *
 * @param[in] self - self
 * @param[in] radarindex - the index of the radar object in the composite list
 * @param[in] quantity - the parameter
 * @param[in] lon - longitude in radians
 * @param[in] lat - latitude in radians
 * @param[out] vtype - the value type (MUST NOT BE NULL)
 * @param[out] vvalue - the value (MUST NOT BE NULL)
 * @param[out] navinfo - the navigation information (MAY BE NULL)
 * @return 1 on success or 0 on failure.
 */
static int CompositeInternal_getVerticalMaxValue(
  Composite_t* self,
  int radarindex,
  const char* quantity,
  double lon,
  double lat,
  RaveValueType* vtype,
  double* vvalue,
  PolarNavigationInfo* navinfo,
  double* qiv)
{
  int result = 0;
  RaveCoreObject* obj = NULL;
  PolarNavigationInfo info;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((vtype != NULL), "vtype == NULL");
  RAVE_ASSERT((vvalue != NULL), "vvalue == NULL");

  obj = Composite_get(self, radarindex);
  if (obj == NULL) {
    goto done;
  }

  if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
    *vtype = PolarScan_getNearestConvertedParameterValue((PolarScan_t*)obj, quantity, lon, lat, vvalue, &info);
    if (self->qiFieldName != NULL && (qiv != NULL)) {
      if (!PolarScan_getQualityValueAt((PolarScan_t*)obj, quantity, info.ri, info.ai, (const char*)self->qiFieldName, 0, qiv)) {
        *qiv = 0.0;
      }
    }
  } else {
    *vtype = PolarVolume_getConvertedVerticalMaxValue((PolarVolume_t*)obj, quantity, lon, lat, vvalue, &info);
    if (self->qiFieldName != NULL && (qiv != NULL)) {
      if (!PolarVolume_getQualityValueAt((PolarVolume_t*)obj, quantity, info.ei, info.ri, info.ai, (const char*)self->qiFieldName, 0, qiv)) {
        *qiv = 0.0;
      }
    }
  }

  if (navinfo != NULL) {
    *navinfo = info;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(obj);
  return result;
}


static int CompositeInternal_addNodeIdsToFieldHowTaskArgs(Composite_t* self, RaveField_t* field)
{
  int i = 0, n = 0;
  char* ids = NULL;
  int idsLength = 0;
  int result = 0;
  RaveAttribute_t* howTaskArgs = NULL;
  char* srcid = NULL;
  RaveCoreObject* obj = NULL;
  n = Composite_getNumberOfObjects(self);
  /* We assume that length of ids is <nr radars> * 10 (WMO-number and a ':', followed by a 3-digit number and finally a ',') */
  idsLength = n * 10 + 1;
  ids = RAVE_MALLOC(sizeof(char) * idsLength);
  if (ids == NULL) {
    return 0;
  }
  memset(ids, 0, sizeof(char)*idsLength);

  for (i = 0; i < n; i++) {
    obj = Composite_get(self, i);
    srcid = NULL;
    if (obj != NULL) {
      if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
        const char* source = PolarScan_getSource((PolarScan_t*)obj);
        srcid = CompositeInternal_getAnyIdFromSource(source);
      } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
        const char* source = PolarVolume_getSource((PolarVolume_t*)obj);
        srcid = CompositeInternal_getAnyIdFromSource(source);
      }

      if (srcid != NULL) {
        if (!CompositeInternal_concateStr(&ids, &idsLength, srcid)) {
          goto done;
        }
      } else {
        if (!CompositeInternal_concateStr(&ids, &idsLength, "Unknown")) {
          goto done;
        }
      }
      if (!CompositeInternal_concateStr(&ids, &idsLength, ":")) {
        goto done;
      }
      if (!CompositeInternal_concateInt(&ids, &idsLength, Composite_getRadarIndexValue(self, i))) {
        goto done;
      }
      if (i < n-1) {
        if (!CompositeInternal_concateStr(&ids, &idsLength,",")) {
          goto done;
        }
      }
    }
    RAVE_FREE(srcid);
    RAVE_OBJECT_RELEASE(obj);
  }

  howTaskArgs = RaveAttributeHelp_createString("how/task_args", ids);
  if (howTaskArgs == NULL) {
    goto done;
  }
  if (!RaveField_addAttribute(field, howTaskArgs)) {
    goto done;
  }

  result = 1;
done:
  RAVE_FREE(srcid);
  RAVE_OBJECT_RELEASE(obj);
  RAVE_FREE(ids);
  RAVE_OBJECT_RELEASE(howTaskArgs);
  return result;
}

/**
 * Adds quality flags to the composite.
 * @apram[in] self - self
 * @param[in] image - the image to add quality flags to
 * @param[in] qualityflags - a list of strings identifying the how/task value in the quality fields
 * @return 1 on success otherwise 0
 */
static int CompositeInternal_addQualityFlags(Composite_t* self, Cartesian_t* image, RaveList_t* qualityflags)
{
  int result = 0;
  int nqualityflags = 0;
  RaveField_t* field = NULL;
  CartesianParam_t* param = NULL;
  RaveList_t* paramNames = NULL;

  int xsize = 0, ysize = 0;
  int i = 0, j = 0;
  int nparam = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((image != NULL), "image == NULL");

  xsize = Cartesian_getXSize(image);
  ysize = Cartesian_getYSize(image);

  paramNames = Cartesian_getParameterNames(image);
  if (paramNames == NULL) {
    goto done;
  }

  nparam = RaveList_size(paramNames);

  if (qualityflags != NULL) {
    nqualityflags = RaveList_size(qualityflags);
  }

  for (i = 0; i < nqualityflags; i++) {
    char* howtaskvaluestr = (char*)RaveList_get(qualityflags, i);
    double gain = 1.0, offset = 0.0;

    if (strcmp(DISTANCE_TO_RADAR_HOW_TASK, howtaskvaluestr) == 0) {
      gain = DISTANCE_TO_RADAR_RESOLUTION;
      offset = 0.0;
    } else if (strcmp(HEIGHT_ABOVE_SEA_HOW_TASK, howtaskvaluestr) == 0) {
      gain = HEIGHT_RESOLUTION;
      offset = 0.0;
    } else if (strcmp(RADAR_INDEX_HOW_TASK, howtaskvaluestr) == 0) {
      gain = 1.0;
      offset = 0.0;
    } else {
      // set the same, fixed gain and offset that is used for all quality fields (except distance) in the composite
      gain = COMPOSITE_QUALITY_FIELDS_GAIN;
      offset = COMPOSITE_QUALITY_FIELDS_OFFSET;
    }

    field = CompositeInternal_createQualityField(howtaskvaluestr, xsize, ysize, gain, offset);

    if (strcmp(RADAR_INDEX_HOW_TASK, howtaskvaluestr)==0) {
      CompositeInternal_addNodeIdsToFieldHowTaskArgs(self, field);
    }

    if (field != NULL) {
      for (j = 0; j < nparam; j++) {
        param = Cartesian_getParameter(image, (const char*)RaveList_get(paramNames, j));
        if (param != NULL) {
          RaveField_t* cfield = RAVE_OBJECT_CLONE(field);
          if (cfield == NULL ||
              !CartesianParam_addQualityField(param, cfield)) {
            RAVE_OBJECT_RELEASE(cfield);
            RAVE_ERROR0("Failed to add quality field");
            goto done;
          }
          RAVE_OBJECT_RELEASE(cfield);
        }
        RAVE_OBJECT_RELEASE(param);
      }
    } else {
      RAVE_WARNING1("Could not create quality field for: %s", howtaskvaluestr);
    }

    RAVE_OBJECT_RELEASE(field);
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(param);
  RaveList_freeAndDestroy(&paramNames);
  return result;
}

/**
 * Uses the navigation information of the value positions and fills all 
 * associated cartesian quality with the composite objects quality fields. If
 * there is more than one value position in the valuePositions-array, an 
 * an interpolation of the quality value will be performed, along the dimensions
 * defined in the interpolationDimensions-array.
 * 
 * @param[in] composite - self
 * @param[in] x - x coordinate
 * @param[in] y - y coordinate
 * @param[in] cvalues - the composite values
 * @param[in] interpolationDimensions - dimensions to perform interpolation in                      
 */
static void CompositeInternal_fillQualityInformation(
  Composite_t* composite,
  int x,
  int y,
  CompositeValues_t* cvalues,
  int interpolationDimensions[])
{
  int nfields = 0, i = 0;
  const char* quantity;
  CartesianParam_t* param = NULL;
  double radardist = 0;
  int radarindex = 0;
  CompositeValuePosition_t* valuePositions = NULL;
  int noOfValuePositions = 0;

  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((cvalues != NULL), "cvalues == NULL");
  RAVE_ASSERT((interpolationDimensions != NULL), "interpolationDimensions == NULL");

  param = cvalues->parameter;
  radardist = cvalues->radardist;
  radarindex = cvalues->radarindex;
  valuePositions = cvalues->valuePositions;
  noOfValuePositions = cvalues->noOfValuePositions;

  nfields = CartesianParam_getNumberOfQualityFields(param);
  quantity = CartesianParam_getQuantity(param);

  for (i = 0; i < nfields; i++) {
    RaveField_t* field = NULL;
    RaveAttribute_t* attribute = NULL;
    char* name = NULL;
    double value = 0.0;

    field = CartesianParam_getQualityField(param, i);
    if (field != NULL) {
      attribute = RaveField_getAttribute(field, "how/task");
    }
    if (attribute != NULL) {
      RaveAttribute_getString(attribute, &name);
    }

    if (name != NULL) {
      RaveCoreObject* obj = Composite_get(composite, radarindex);
      if (obj != NULL) {
        int setValue = 1;
        double dummyValue;
        RaveValueType type;
        if (strcmp(DISTANCE_TO_RADAR_HOW_TASK, name) == 0) {
          value = radardist/DISTANCE_TO_RADAR_RESOLUTION;
        } else if (strcmp(HEIGHT_ABOVE_SEA_HOW_TASK, name) == 0) {
          if (!CompositeInternal_getInterpolatedValue(composite, obj, interpolationDimensions,
                                                      quantity, name, valuePositions,
                                                      noOfValuePositions,
                                                      CompositeInternal_setHeightValuesInValuePos,
                                                      &type, &value, &dummyValue))
          {
            value = 0.0;
          }
        } else if (strcmp(RADAR_INDEX_HOW_TASK, name) == 0) {
          value = (double)Composite_getRadarIndexValue(composite, radarindex);
        } else if (composite->algorithm != NULL && CompositeAlgorithm_supportsFillQualityInformation(composite->algorithm, name)) {
          // If the algorithm indicates that it is able to support the provided how/task field, then do so
          if (noOfValuePositions == 1) {
            PolarNavigationInfo navinfo = valuePositions[0].navinfo;
            if (CompositeAlgorithm_fillQualityInformation(composite->algorithm, obj, name, quantity, field, x, y, &navinfo,
                                                          COMPOSITE_QUALITY_FIELDS_GAIN, COMPOSITE_QUALITY_FIELDS_OFFSET)) {
              setValue = 0;
            } else {
              value = 0.0;
            }
          } else {
            if (!CompositeInternal_getInterpolatedAlgorithmQualityValue(composite, obj, field, name, quantity,
                                                                        valuePositions, noOfValuePositions, x, y,
                                                                        interpolationDimensions, &value))
            {
              value = 0.0;
            }
          }
        } else {
          int valueInterpolated = CompositeInternal_getInterpolatedValue(composite, obj, interpolationDimensions,
                                                                         quantity, name, valuePositions,
                                                                         noOfValuePositions,
                                                                         CompositeInternal_setQualityValuesInValuePos,
                                                                         &type, &value, &dummyValue);
          if (valueInterpolated) {
            value = (value - COMPOSITE_QUALITY_FIELDS_OFFSET) / COMPOSITE_QUALITY_FIELDS_GAIN;
          }
        }

        if (setValue) {
          RaveField_setValue(field, x, y, value);
        }
      }
      RAVE_OBJECT_RELEASE(obj);
    }

    RAVE_OBJECT_RELEASE(field);
    RAVE_OBJECT_RELEASE(attribute);
  }
}

/**
 * Creates the resulting composite image.
 * @param[in] self - self
 * @param[in] area - the area the composite image(s) should have
 * @returns the cartesian on success otherwise NULL
 */
static Cartesian_t* CompositeInternal_createCompositeImage(Composite_t* self, Area_t* area)
{
  Cartesian_t *result = NULL, *cartesian = NULL;
  RaveAttribute_t* prodpar = NULL;
  int nparam = 0, i = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  cartesian = RAVE_OBJECT_NEW(&Cartesian_TYPE);
  if (cartesian == NULL) {
    goto done;
  }
  Cartesian_init(cartesian, area);

  nparam = Composite_getParameterCount(self);
  if (nparam <= 0) {
    RAVE_ERROR0("You can not generate a composite without specifying at least one parameter");
    goto done;
  }

  if (self->ptype == Rave_ProductType_CAPPI ||
      self->ptype == Rave_ProductType_PCAPPI) {
    prodpar = RaveAttributeHelp_createDouble("what/prodpar", self->height);
  } else if (self->ptype == Rave_ProductType_PMAX) {
    char s[256];
    snprintf(s, 256, "%f,%f",self->height,self->range);
    prodpar = RaveAttributeHelp_createString("what/prodpar", s);
  } else {
    prodpar = RaveAttributeHelp_createDouble("what/prodpar", self->elangle * 180.0/M_PI);
  }
  if (prodpar == NULL) {
    goto done;
  }

  Cartesian_setObjectType(cartesian, Rave_ObjectType_COMP);
  Cartesian_setProduct(cartesian, self->ptype);
  if (!Cartesian_addAttribute(cartesian, prodpar)) {
    goto done;
  }
  if (Composite_getTime(self) != NULL) {
    if (!Cartesian_setTime(cartesian, Composite_getTime(self))) {
      goto done;
    }
  }
  if (Composite_getDate(self) != NULL) {
    if (!Cartesian_setDate(cartesian, Composite_getDate(self))) {
      goto done;
    }
  }
  if (!Cartesian_setSource(cartesian, Area_getID(area))) {
    goto done;
  }

  for (i = 0; i < nparam; i++) {
    double gain = 0.0, offset = 0.0;
    const char* name = Composite_getParameter(self, i, &gain, &offset);
    CartesianParam_t* cp = Cartesian_createParameter(cartesian, name, RaveDataType_UCHAR, 0);
    if (cp == NULL) {
      goto done;
    }
    CartesianParam_setNodata(cp, 255.0);
    CartesianParam_setUndetect(cp, 0.0);
    CartesianParam_setGain(cp, gain);
    CartesianParam_setOffset(cp, offset);
    RAVE_OBJECT_RELEASE(cp);
  }

  result = RAVE_OBJECT_COPY(cartesian);
done:
  RAVE_OBJECT_RELEASE(cartesian);
  RAVE_OBJECT_RELEASE(prodpar);
  return result;
}

/**
 * Returns the projection object that belongs to this obj.
 * @param[in] obj - the rave core object instance
 * @return the projection or NULL if there is no projection instance
 */
static Projection_t* CompositeInternal_getProjection(RaveCoreObject* obj)
{
  Projection_t* result = NULL;
  if (obj != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
      result = PolarVolume_getProjection((PolarVolume_t*)obj);
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
      result = PolarScan_getProjection((PolarScan_t*)obj);
    }
  }
  return result;
}

static int CompositeInternal_getDistances(RaveCoreObject* obj, double lon, double lat, double* distance, double* maxdistance)
{
  int result = 0;
  RAVE_ASSERT((distance != NULL), "distance == NULL");
  RAVE_ASSERT((maxdistance != NULL), "maxdistance == NULL");
  if (obj != NULL) {
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
      *distance = PolarVolume_getDistance((PolarVolume_t*)obj, lon, lat);
      *maxdistance = PolarVolume_getMaxDistance((PolarVolume_t*)obj);
      result = 1;
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
      *distance = PolarScan_getDistance((PolarScan_t*)obj, lon, lat);
      *maxdistance = PolarScan_getMaxDistance((PolarScan_t*)obj);
      result = 1;
    }
  }
  return result;
}

/**
 * Pure max is a quite difference composite generator that does not care about proximity to ground
 * or radar or anything else. It only cares about maximum value at the specific position so we handle
 * this as a separate scheme instead of trying to mix into _nearest.
 *
 * This processing scheme does not support algorithm.process but it support algorithm.fillQualityInformation.
 *
 * @param[in] composite - self
 * @param[in] area - the area we are working with
 * @param[in] qualityflags - the quality flags we want to have set
 * @return the cartesian product
 */
static Cartesian_t* Composite_nearest_max(Composite_t* composite, Area_t* area, RaveList_t* qualityflags)
{
  Cartesian_t* result = NULL;
  Projection_t* projection = NULL;
  RaveObjectList_t* pipelines = NULL;
  PolarNavigationInfo navinfo;
  CompositeValues_t* cvalues = NULL;
  int x = 0, y = 0, i = 0, xsize = 0, ysize = 0, nradars = 0;
  int nqualityflags = 0;
  int nparam = 0;
  int interpolationDimensions[NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS] = {0};

  RAVE_ASSERT((composite != NULL), "composite == NULL");
  if (area == NULL) {
    RAVE_ERROR0("Trying to generate composite with NULL area");
    goto fail;
  }

  nparam = Composite_getParameterCount(composite);
  if (nparam <= 0) {
    RAVE_ERROR0("You can not generate a composite without specifying at least one parameter");
    goto fail;
  }

  CompositeInternal_setInterpolationDimensionsArray(composite, interpolationDimensions);

  result = CompositeInternal_createCompositeImage(composite, area);
  if (result == NULL) {
    goto fail;
  }

  if ((cvalues = CompositeInternal_createCompositeValues(nparam)) == NULL) {
    goto fail;
  }

  for (i = 0; i < nparam; i++) {
    const char* name = Composite_getParameter(composite, i, NULL, NULL);
    cvalues[i].parameter = Cartesian_getParameter(result, name); // Keep track on parameters
    if (cvalues[i].parameter == NULL) {
      RAVE_ERROR0("Failure in parameter handling\n");
      goto fail;
    }
  }

  xsize = Cartesian_getXSize(result);
  ysize = Cartesian_getYSize(result);
  projection = Cartesian_getProjection(result);
  nradars = Composite_getNumberOfObjects(composite);

  if (qualityflags != NULL) {
    nqualityflags = RaveList_size(qualityflags);
    if (!CompositeInternal_addQualityFlags(composite, result, qualityflags)) {
      goto fail;
    }
  }

  if (composite->algorithm != NULL) {
    if (!CompositeAlgorithm_initialize(composite->algorithm, composite)) {
      goto fail;
    }
  }

  pipelines = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (pipelines == NULL) {
    goto fail;
  }
  for (i = 0; i < nradars; i++) {
    RaveCoreObject* obj = Composite_get(composite, i);
    if (obj != NULL) {
      Projection_t* objproj = CompositeInternal_getProjection(obj);
      ProjectionPipeline_t* pipeline = NULL;
      if (objproj == NULL) {
        RAVE_OBJECT_RELEASE(obj);
        RAVE_ERROR0("No projection for object");
        goto fail;
      }
      pipeline = ProjectionPipeline_createPipeline(projection, objproj);
      RAVE_OBJECT_RELEASE(objproj);
      RAVE_OBJECT_RELEASE(obj);
      if (pipeline == NULL || !RaveObjectList_add(pipelines, (RaveCoreObject*)pipeline)) {
        RAVE_ERROR0("Failed to create pipeline");
        RAVE_OBJECT_RELEASE(pipeline);
        goto fail;
      }
      RAVE_OBJECT_RELEASE(pipeline);
    }
  }

  for (y = 0; y < ysize; y++) {
    double herey = Cartesian_getLocationY(result, y);
    for (x = 0; x < xsize; x++) {
      int cindex = 0;
      double herex = Cartesian_getLocationX(result, x);
      double olon = 0.0, olat = 0.0;

      CompositeInternal_resetCompositeValues(composite, nparam, cvalues);
      if (composite->algorithm != NULL) {
        CompositeAlgorithm_reset(composite->algorithm, x, y);
      }

      for (i = 0; i < nradars; i++) {
        RaveCoreObject* obj = NULL;
        ProjectionPipeline_t* pipeline = NULL;
        obj = Composite_get(composite, i);
        if (obj != NULL) {
          pipeline = (ProjectionPipeline_t*)RaveObjectList_get(pipelines, i);
        }

        if (pipeline != NULL) {
          /* We will go from surface coords into the lonlat projection assuming that a polar volume uses a lonlat projection*/
          if (!ProjectionPipeline_fwd(pipeline, herex, herey, &olon, &olat)) {
            RAVE_WARNING0("Failed to transform from composite into polar coordinates");
          } else {
            double dist = 0.0;
            double maxdist = 0.0;

            // We only use distance & max distance to speed up processing but it isn't used for anything else
            // in the pure vertical max implementation.
            if (CompositeInternal_getDistances(obj, olon, olat, &dist, &maxdist) && dist <= maxdist) {
              for (cindex = 0; cindex < nparam; cindex++) {
                RaveValueType otype = RaveValueType_NODATA;
                double ovalue = 0.0, qivalue = 0.0;
                CompositeInternal_getVerticalMaxValue(composite, i, cvalues[cindex].name, olon, olat, &otype, &ovalue, &navinfo, &qivalue);
                if (otype == RaveValueType_DATA || otype == RaveValueType_UNDETECT) {
                  if ((cvalues[cindex].vtype != RaveValueType_DATA && cvalues[cindex].vtype != RaveValueType_UNDETECT) ||
                      (cvalues[cindex].vtype == RaveValueType_UNDETECT && otype == RaveValueType_DATA) ||
                      (cvalues[cindex].vtype == RaveValueType_DATA && otype == RaveValueType_DATA && composite->qiFieldName == NULL && ovalue > cvalues[cindex].value) ||
                      (composite->qiFieldName != NULL && (qivalue > cvalues[cindex].qivalue))) {
                    cvalues[cindex].vtype = otype;
                    cvalues[cindex].value = ovalue;
                    cvalues[cindex].mindist = dist;
                    cvalues[cindex].radardist = dist;
                    cvalues[cindex].radarindex = i;
                    cvalues[cindex].qivalue = qivalue;
                    cvalues[cindex].noOfValuePositions = 1;
                    cvalues[cindex].valuePositions[0].navinfo = navinfo;
                    cvalues[cindex].valuePositions[0].valid = 1;
                  }
                }
              }
            }
          }
        }
        RAVE_OBJECT_RELEASE(obj);
        RAVE_OBJECT_RELEASE(pipeline);
      }

      for (cindex = 0; cindex < nparam; cindex++) {
        double vvalue = cvalues[cindex].value;
        double vtype = cvalues[cindex].vtype;
        CartesianParam_setConvertedValue(cvalues[cindex].parameter, x, y, vvalue, vtype);

        if ((vtype == RaveValueType_DATA || vtype == RaveValueType_UNDETECT) &&
            cvalues[cindex].radarindex >= 0 && nqualityflags > 0) {
          CompositeInternal_fillQualityInformation(composite, x, y, &cvalues[cindex], interpolationDimensions);
        }
      }
    }
  }

  for (i = 0; cvalues != NULL && i < nparam; i++) {
    RAVE_OBJECT_RELEASE(cvalues[i].parameter);
  }
  RAVE_FREE(cvalues);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_OBJECT_RELEASE(pipelines);
  return result;
fail:
  for (i = 0; cvalues != NULL && i < nparam; i++) {
    RAVE_OBJECT_RELEASE(cvalues[i].parameter);
  }
  RAVE_FREE(cvalues);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_OBJECT_RELEASE(pipelines);
  RAVE_OBJECT_RELEASE(result);
  return result;

}


/*@} End of Private functions */

/*@{ Interface functions */
int Composite_add(Composite_t* composite, RaveCoreObject* object)
{
  CompositeRadarItem_t* item = NULL;
  int result = 0;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");

  if (!RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE) &&
      !RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
    RAVE_ERROR0("Providing an object that not is a PolarVolume nor a PolarScan during composite generation");
    return 0;
  }
  item = RAVE_MALLOC(sizeof(CompositeRadarItem_t));
  if (item != NULL) {
    item->object = RAVE_OBJECT_COPY(object);
    result = RaveList_add(composite->objectList, item);
    if (result == 0) {
      RAVE_OBJECT_RELEASE(item->object);
      RAVE_FREE(item);
    }
    item->radarIndexValue = RaveList_size(composite->objectList);
  }

  return result;
}

int Composite_getNumberOfObjects(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveList_size(composite->objectList);
}

RaveCoreObject* Composite_get(Composite_t* composite, int index)
{
  CompositeRadarItem_t* item = NULL;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  item = (CompositeRadarItem_t*)RaveList_get(composite->objectList, index);
  if (item != NULL) {
    return RAVE_OBJECT_COPY(item->object);
  }
  return NULL;
}

int Composite_getRadarIndexValue(Composite_t* composite, int index)
{
  CompositeRadarItem_t* item = NULL;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  item = (CompositeRadarItem_t*)RaveList_get(composite->objectList, index);
  if (item != NULL) {
    return item->radarIndexValue;
  }
  return 0;

}

void Composite_setProduct(Composite_t* composite, Rave_ProductType type)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  if (type == Rave_ProductType_PCAPPI ||
      type == Rave_ProductType_CAPPI ||
      type == Rave_ProductType_PPI ||
      type == Rave_ProductType_PMAX ||
      type == Rave_ProductType_MAX) {
    composite->ptype = type;
  } else {
    RAVE_ERROR0("Only supported algorithms are PPI, CAPPI, PCAPPI, PMAX and MAX");
  }
}

Rave_ProductType Composite_getProduct(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return composite->ptype;
}

int Composite_setSelectionMethod(Composite_t* self, CompositeSelectionMethod_t method)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (method >= CompositeSelectionMethod_NEAREST && method <= CompositeSelectionMethod_HEIGHT) {
    self->method = method;
    result = 1;
  }
  return result;
}

CompositeSelectionMethod_t Composite_getSelectionMethod(Composite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->method;
}

int Composite_setInterpolationMethod(Composite_t* self, CompositeInterpolationMethod_t interpolationMethod)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (interpolationMethod >= CompositeInterpolationMethod_NEAREST && interpolationMethod <= CompositeInterpolationMethod_QUADRATIC_3D) {
    self->interpolationMethod = interpolationMethod;
    result = 1;
  }
  return result;
}

CompositeInterpolationMethod_t Composite_getInterpolationMethod(Composite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->interpolationMethod;
}

void Composite_setHeight(Composite_t* composite, double height)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  composite->height = height;
}

double Composite_getHeight(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return composite->height;
}

void Composite_setElevationAngle(Composite_t* composite, double angle)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  composite->elangle = angle;
}

double Composite_getElevationAngle(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return composite->elangle;
}

void Composite_setRange(Composite_t* self, double range)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->range = range;
}

double Composite_getRange(Composite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->range;
}

int Composite_setQualityIndicatorFieldName(Composite_t* self, const char* qiFieldName)
{
  char* tmp = NULL;
  int result = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");

  if (qiFieldName != NULL) {
    tmp = RAVE_STRDUP(qiFieldName);
    if (tmp != NULL) {
      RAVE_FREE(self->qiFieldName);
      self->qiFieldName = tmp;
      tmp = NULL;
      result = 1;
    }
  } else {
    RAVE_FREE(self->qiFieldName);
    result = 1;
  }
  return result;
}

const char* Composite_getQualityIndicatorFieldName(Composite_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->qiFieldName;
}

int Composite_addParameter(Composite_t* composite, const char* quantity, double gain, double offset, double minvalue)
{
  int result = 0;
  CompositingParameter_t* param = NULL;

  RAVE_ASSERT((composite != NULL), "composite == NULL");

  param = CompositeInternal_getParameterByName(composite, quantity);
  if (param != NULL) {
    param->gain = gain;
    param->offset = offset;
    param->minvalue = minvalue;
    result = 1;
  } else {
    param = CompositeInternal_createParameter(quantity, gain, offset, minvalue);
    if (param != NULL) {
      result = RaveList_add(composite->parameters, param);
      if (!result) {
        CompositeInternal_freeParameter(param);
      }
    }
  }
  return result;
}

int Composite_hasParameter(Composite_t* composite, const char* quantity)
{
  int result = 0;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  if (quantity != NULL) {
    int i = 0;
    int len = RaveList_size(composite->parameters);
    for (i = 0; result == 0 && i < len ; i++) {
      CompositingParameter_t* s = RaveList_get(composite->parameters, i);
      if (s != NULL && s->name != NULL && strcmp(quantity, s->name) == 0) {
        result = 1;
      }
    }
  }
  return result;
}

int Composite_getParameterCount(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveList_size(composite->parameters);
}

const char* Composite_getParameter(Composite_t* composite, int index, double* gain, double* offset)
{
  CompositingParameter_t* param = NULL;
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  param = RaveList_get(composite->parameters, index);
  if (param != NULL) {
    if (gain != NULL) {
      *gain = param->gain;
    }
    if (offset != NULL) {
      *offset = param->offset;
    }
    return (const char*)param->name;
  }
  return NULL;
}

int Composite_setTime(Composite_t* composite, const char* value)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveDateTime_setTime(composite->datetime, value);
}

const char* Composite_getTime(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveDateTime_getTime(composite->datetime);
}

int Composite_setDate(Composite_t* composite, const char* value)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveDateTime_setDate(composite->datetime, value);
}

const char* Composite_getDate(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RaveDateTime_getDate(composite->datetime);
}

int Composite_applyRadarIndexMapping(Composite_t* composite, RaveObjectHashTable_t* mapping)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");

  return CompositeInternal_updateRadarIndexes(composite, mapping);
}

Cartesian_t* Composite_generate(Composite_t* composite, Area_t* area, RaveList_t* qualityflags)
{
  Cartesian_t* result = NULL;
  Projection_t* projection = NULL;
  CompositeValuePosition_t valuePositions[MAX_NO_OF_SURROUNDING_POSITIONS];
  CompositeValues_t* cvalues = NULL;
  RaveObjectList_t* pipelines = NULL;
  int interpolationDimensions[NO_OF_COMPOSITE_INTERPOLATION_DIMENSIONS] = {0};
  int x = 0, y = 0, i = 0, xsize = 0, ysize = 0, nradars = 0;
  int nqualityflags = 0;
  int nparam = 0;

  RAVE_ASSERT((composite != NULL), "composite == NULL");
  
  CompositeInternal_setInterpolationDimensionsArray(composite, interpolationDimensions);

  if (composite->ptype == Rave_ProductType_MAX && composite->interpolationMethod != CompositeInterpolationMethod_NEAREST) {
    RAVE_ERROR0("Product type MAX can currently only be used with interpolation method 'nearest value'.");
  } else if (composite->ptype == Rave_ProductType_PMAX && composite->interpolationMethod != CompositeInterpolationMethod_NEAREST) {
    RAVE_ERROR0("Product type PMAX can currently only be used with interpolation method 'nearest value'.");
  }

  if (composite->ptype == Rave_ProductType_MAX) { // Special handling of the max algorithm.
    return Composite_nearest_max(composite, area, qualityflags);
  }

  if (area == NULL) {
    RAVE_ERROR0("Trying to generate composite with NULL area");
    goto fail;
  }

  nparam = Composite_getParameterCount(composite);
  if (nparam <= 0) {
    RAVE_ERROR0("You can not generate a composite without specifying at least one parameter");
    goto fail;
  }

  result = CompositeInternal_createCompositeImage(composite, area);
  if (result == NULL) {
    goto fail;
  }

  if ((cvalues = CompositeInternal_createCompositeValues(nparam)) == NULL) {
    goto fail;
  }

  for (i = 0; i < nparam; i++) {
    const char* name = Composite_getParameter(composite, i, NULL, NULL);
    cvalues[i].parameter = Cartesian_getParameter(result, name); // Keep track on parameters
    if (cvalues[i].parameter == NULL) {
      RAVE_ERROR0("Failure in parameter handling\n");
      goto fail;
    }
  }

  xsize = Cartesian_getXSize(result);
  ysize = Cartesian_getYSize(result);
  projection = Cartesian_getProjection(result);
  nradars = Composite_getNumberOfObjects(composite);

  if (qualityflags != NULL) {
    nqualityflags = RaveList_size(qualityflags);
    if (!CompositeInternal_addQualityFlags(composite, result, qualityflags)) {
      goto fail;
    }
  }

  if (composite->algorithm != NULL) {
    if (!CompositeAlgorithm_initialize(composite->algorithm, composite)) {
      goto fail;
    }
  }

  pipelines = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (pipelines == NULL) {
    goto fail;
  }
  for (i = 0; i < nradars; i++) {
    RaveCoreObject* obj = Composite_get(composite, i);
    if (obj != NULL) {
      Projection_t* objproj = CompositeInternal_getProjection(obj);
      ProjectionPipeline_t* pipeline = NULL;
      if (objproj == NULL) {
        RAVE_OBJECT_RELEASE(obj);
        RAVE_ERROR0("No projection for object");
        goto fail;
      }
      pipeline = ProjectionPipeline_createPipeline(projection, objproj);
      RAVE_OBJECT_RELEASE(objproj);
      RAVE_OBJECT_RELEASE(obj);
      if (pipeline == NULL || !RaveObjectList_add(pipelines, (RaveCoreObject*)pipeline)) {
        RAVE_ERROR0("Failed to create pipeline");
        RAVE_OBJECT_RELEASE(pipeline);
        goto fail;
      }
      RAVE_OBJECT_RELEASE(pipeline);
    }
  }

  for (y = 0; y < ysize; y++) {
    double herey = Cartesian_getLocationY(result, y);
    for (x = 0; x < xsize; x++) {
      int cindex = 0;
      double herex = Cartesian_getLocationX(result, x);
      double olon = 0.0, olat = 0.0;

      CompositeInternal_resetCompositeValues(composite, nparam, cvalues);
      if (composite->algorithm != NULL) {
        CompositeAlgorithm_reset(composite->algorithm, x, y);
      }

      for (i = 0; i < nradars; i++) {
        RaveCoreObject* obj = NULL;
        ProjectionPipeline_t* pipeline = NULL;
        obj = Composite_get(composite, i);
        if (obj != NULL) {
          pipeline = (ProjectionPipeline_t*)RaveObjectList_get(pipelines, i);
        }

        if (pipeline != NULL) {
          /* We will go from surface coords into the lonlat projection assuming that a polar volume uses a lonlat projection*/
          if (!ProjectionPipeline_fwd(pipeline, herex, herey, &olon, &olat)) {
            RAVE_WARNING0("Failed to transform from composite into polar coordinates");
          } else {
            double dist = 0.0;
            double maxdist = 0.0;
            double rdist = 0.0;
            if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
              dist = PolarVolume_getDistance((PolarVolume_t*)obj, olon, olat);
              maxdist = PolarVolume_getMaxDistance((PolarVolume_t*)obj);
              PolarVolume_sortByElevations((PolarVolume_t*)obj, 1);
            } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
              dist = PolarScan_getDistance((PolarScan_t*)obj, olon, olat);
              maxdist = PolarScan_getMaxDistance((PolarScan_t*)obj);
            }
            if (dist <= maxdist) {
              int noOfValuePositions = CompositeInternal_getValuePositions(composite, obj, olon, olat,
                                                                           interpolationDimensions,
                                                                           valuePositions);
              if (noOfValuePositions > 0) {
                rdist = dist; /* Remember distance to radar */

                if (composite->method == CompositeSelectionMethod_HEIGHT) {
                  dist = CompositeInternal_getValuePositionsLowestHeight(valuePositions, noOfValuePositions);
                }

                for (cindex = 0; cindex < nparam; cindex++) {
                  RaveValueType otype = RaveValueType_NODATA;
                  double ovalue = 0.0, qivalue = 0.0;

                  if (!CompositeInternal_getInterpolatedValue(composite, obj, interpolationDimensions,
                                                              cvalues[cindex].name, NULL, valuePositions,
                                                              noOfValuePositions,
                                                              CompositeInternal_setValuesInValuePos,
                                                              &otype, &ovalue, &qivalue)) {
                    RAVE_ERROR0("Interpolation failed.\n");
                    goto fail;
                  }

                  if (composite->algorithm != NULL && CompositeAlgorithm_supportsProcess(composite->algorithm)) {
                    // NOTE: The CompositeAlgorithm_process interface expects only one single navigation info. In the below call, we always provide
                    // the navigation info for the first position. This will at least work for the 'nearest' interpolation method. For other interpolation methods,
                    // where multiple positions have been collected, it depends on the composite algorithm how it handles the navigation info.
                    if (CompositeAlgorithm_process(composite->algorithm, obj, cvalues[cindex].name, olon, olat, rdist, &otype, &ovalue, &valuePositions[0].navinfo)) {
                      cvalues[cindex].vtype = otype;
                      cvalues[cindex].value = ovalue;
                      cvalues[cindex].mindist = dist;
                      cvalues[cindex].radardist = rdist;
                      cvalues[cindex].radarindex = i;
                      cvalues[cindex].qivalue = qivalue;
                      cvalues[cindex].noOfValuePositions = noOfValuePositions;
                      memcpy(cvalues[cindex].valuePositions, valuePositions, sizeof(CompositeValuePosition_t)*MAX_NO_OF_SURROUNDING_POSITIONS);
                    }
                  } else {
                    if (otype == RaveValueType_DATA || otype == RaveValueType_UNDETECT) {
                      if (cvalues[cindex].vtype != RaveValueType_DATA && cvalues[cindex].vtype != RaveValueType_UNDETECT) {
                        /* First time */
                        cvalues[cindex].vtype = otype;
                        cvalues[cindex].value = ovalue;
                        cvalues[cindex].mindist = dist;
                        cvalues[cindex].radardist = rdist;
                        cvalues[cindex].radarindex = i;
                        cvalues[cindex].qivalue = qivalue;
                        cvalues[cindex].noOfValuePositions = noOfValuePositions;
                        memcpy(cvalues[cindex].valuePositions, valuePositions, sizeof(CompositeValuePosition_t)*MAX_NO_OF_SURROUNDING_POSITIONS);
                      } else if (
                          composite->qiFieldName != NULL &&
                          ((qivalue > cvalues[cindex].qivalue) ||
                           (qivalue == cvalues[cindex].qivalue && dist < cvalues[cindex].mindist))) {
                        cvalues[cindex].vtype = otype;
                        cvalues[cindex].value = ovalue;
                        cvalues[cindex].mindist = dist;
                        cvalues[cindex].radardist = rdist;
                        cvalues[cindex].radarindex = i;
                        cvalues[cindex].qivalue = qivalue;
                        cvalues[cindex].noOfValuePositions = noOfValuePositions;
                        memcpy(cvalues[cindex].valuePositions, valuePositions, sizeof(CompositeValuePosition_t)*MAX_NO_OF_SURROUNDING_POSITIONS);
                      } else if (composite->qiFieldName == NULL && dist < cvalues[cindex].mindist) {
                        cvalues[cindex].vtype = otype;
                        cvalues[cindex].value = ovalue;
                        cvalues[cindex].mindist = dist;
                        cvalues[cindex].radardist = rdist;
                        cvalues[cindex].radarindex = i;
                        cvalues[cindex].qivalue = qivalue;
                        cvalues[cindex].noOfValuePositions = noOfValuePositions;
                        memcpy(cvalues[cindex].valuePositions, valuePositions, sizeof(CompositeValuePosition_t)*MAX_NO_OF_SURROUNDING_POSITIONS);
                      }
                    }
                  }
                }
              }
            }
          }
        }
        RAVE_OBJECT_RELEASE(obj);
        RAVE_OBJECT_RELEASE(pipeline);
      }

      for (cindex = 0; cindex < nparam; cindex++) {
        double vvalue = cvalues[cindex].value;
        double vtype = cvalues[cindex].vtype;

        if (vtype != RaveValueType_NODATA && composite->ptype == Rave_ProductType_PMAX && cvalues[cindex].radardist < composite->range) {
          // only support for nearest value interpolation with PMAX, meaning that we only have one value position
          PolarNavigationInfo info = cvalues[cindex].valuePositions[0].navinfo;

          RaveValueType ntype = RaveValueType_NODATA;
          double nvalue = 0.0;
          if (vtype == RaveValueType_UNDETECT) {
            /* Undetect should not affect navigation information */
            CompositeInternal_getVerticalMaxValue(composite, cvalues[cindex].radarindex, cvalues[cindex].name, olon, olat, &ntype, &nvalue, NULL, NULL);
          } else {
            CompositeInternal_getVerticalMaxValue(composite, cvalues[cindex].radarindex, cvalues[cindex].name, olon, olat, &ntype, &nvalue, &info, NULL);
          }
          if (ntype != RaveValueType_NODATA) {
            vtype = ntype;
            vvalue = nvalue;
            cvalues[cindex].valuePositions[0].navinfo = info;
          }          
        }

        CartesianParam_setConvertedValue(cvalues[cindex].parameter, x, y, vvalue, vtype);

        if ((vtype == RaveValueType_DATA || vtype == RaveValueType_UNDETECT) &&
            cvalues[cindex].radarindex >= 0 && nqualityflags > 0) {
          CompositeInternal_fillQualityInformation(composite, x, y, &cvalues[cindex], interpolationDimensions);
        }
      }
    }
  }

  for (i = 0; cvalues != NULL && i < nparam; i++) {
    RAVE_OBJECT_RELEASE(cvalues[i].parameter);
  }
  RAVE_FREE(cvalues);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_OBJECT_RELEASE(pipelines);
  return result;
fail:
  for (i = 0; cvalues != NULL && i < nparam; i++) {
    RAVE_OBJECT_RELEASE(cvalues[i].parameter);
  }
  RAVE_FREE(cvalues);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_OBJECT_RELEASE(pipelines);
  RAVE_OBJECT_RELEASE(result);
  return result;
}

void Composite_setAlgorithm(Composite_t* composite, CompositeAlgorithm_t* algorithm)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  RAVE_OBJECT_RELEASE(composite->algorithm);
  composite->algorithm = RAVE_OBJECT_COPY(algorithm);
}

CompositeAlgorithm_t* Composite_getAlgorithm(Composite_t* composite)
{
  RAVE_ASSERT((composite != NULL), "composite == NULL");
  return RAVE_OBJECT_COPY(composite->algorithm);
}

/*@} End of Interface functions */

RaveCoreObjectType Composite_TYPE = {
    "Composite",
    sizeof(Composite_t),
    Composite_constructor,
    Composite_destructor,
    Composite_copyconstructor
};

