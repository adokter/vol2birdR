/* --------------------------------------------------------------------
Copyright (C) 2010 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Cartesian ODIM decorator
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-09-09
 */
#include "cartesian_odim_io.h"
#include "rave_hlhdf_utilities.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "raveobject_hashtable.h"
#include "odim_io_utilities.h"
#include <math.h>
#include <string.h>
#include "projection_pipeline.h"
#include <stdlib.h>

typedef struct CartesianOdimArg {
  LazyNodeListReader_t* lazyReader; /**< the lazy node list reader */
  HL_NodeList* nodelist;
  RaveCoreObject* object;
  RaveObjectHashTable_t* attrs;
} CartesianOdimArg;

/**
 * Represents the adaptor
 */
struct _CartesianOdimIO_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveIO_ODIM_Version version;
  int strict; /**< if writing should be validated strictly or not */
  char error_message[1024];                /**< if an error occurs during writing an error message might give you the reason */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int CartesianOdimIO_constructor(RaveCoreObject* obj)
{
  ((CartesianOdimIO_t*)obj)->version = RaveIO_ODIM_Version_2_4;
  ((CartesianOdimIO_t*)obj)->strict = 1;
  strcpy(((CartesianOdimIO_t*)obj)->error_message, "");

  return 1;
}

/**
 * Copy constructor
 */
static int CartesianOdimIO_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  ((CartesianOdimIO_t*)obj)->version = ((CartesianOdimIO_t*)srcobj)->version;
  ((CartesianOdimIO_t*)obj)->strict = ((CartesianOdimIO_t*)srcobj)->strict;
  strcpy(((CartesianOdimIO_t*)obj)->error_message, ((CartesianOdimIO_t*)srcobj)->error_message);
  return 1;
}

/**
 * Destroys the object
 * @param[in] obj - the instance
 */
static void CartesianOdimIO_destructor(RaveCoreObject* obj)
{
}


/**
 * Checks if an environment variable has been set indicating that the cartesian legacy extent calculation
 * should be performed. Environment variable RAVE_USE_CARTESIAN_LEGACY_EXTENT. Value = yes or true indicates
 * that it should be used, otherwise it will not be used.
 * return 1 if legacy extent should be used otherwise 0
 */
static int CartesianOdimIOInternal_useCartesianLegacyExtent(void) {
  char* useCartesianLegacyExtent = getenv("RAVE_USE_CARTESIAN_LEGACY_EXTENT");
  if (RaveUtilities_isLegacyProjEnabled() && useCartesianLegacyExtent != NULL) {
    if (strcasecmp("yes", useCartesianLegacyExtent)==0 || strcasecmp("true", useCartesianLegacyExtent)==0) {
      return 1;
    }
  }
  return 0;
}


/**
 * Gets the nodename/attribute value if nodename/data<index>/attribute doesn't exist.
 * @param[in] nodelist - the node list
 * @param[in] nodename - the group name, e.g. dataset1
 * @param[in] index - the data index
 * @param[in] attribute - the attribute, e.g. what/gain
 * @param[out] v - the found value if any
 * @return 1 if value was found in nodename/attribute and not in nodename/data<index>/attribute. Otherwise 0
 */
static int CartesianOdimIOInternal_getDatasetDoubleValueIfMissing(HL_NodeList* nodelist, const char* nodename, int index, const char* attribute, double* v)
{
  int result = 0;
  if (nodelist == NULL || nodename == NULL || attribute == NULL || v == NULL) {
    return 0;
  }

  if (!RaveHL_hasNodeByName(nodelist, "%s/data%d/%s", nodename, index, attribute)) {
    RaveAttribute_t* attr = RaveHL_getAttribute(nodelist, "%s/%s", nodename, attribute);
    if (attr != NULL) {
      double value = 0;
      if (RaveAttribute_getDouble(attr, &value)) {
        *v = value;
        result = 1;
      }
    }
    RAVE_OBJECT_RELEASE(attr);
  }

  return result;
}

/**
 * Atempts to create a projection instance from a projdef.
 * @param[in] projdef - the projection definition
 * @returns the projection or NULL
 */
static Projection_t* CartesianOdimIO_createProjection(const char* projdef)
{
  Projection_t* projection = NULL;
  Projection_t* result = NULL;
  if (projdef != NULL) {
    projection = RAVE_OBJECT_NEW(&Projection_TYPE);
    if (projection == NULL) {
      RAVE_ERROR0("Could not create projection");
      goto done;
    }
    if (!Projection_init(projection, "raveio-projection", "autoloaded projection", projdef)) {
      RAVE_ERROR0("Could not initialize projection");
      goto done;
    }
  }
  result = RAVE_OBJECT_COPY(projection);
done:
  RAVE_OBJECT_RELEASE(projection);
  return result;
}

/**
 * Cartesian root attributes.
 * @param[in] object - the CartesianOdimArg pointing to a polar scan
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_loadRootAttribute(void* object, RaveAttribute_t* attribute)
{
  Cartesian_t* cartesian = (Cartesian_t*)((CartesianOdimArg*)object)->object;
  RaveObjectHashTable_t* attrs = (RaveObjectHashTable_t*)((CartesianOdimArg*)object)->attrs;
  Projection_t* projection = NULL;

  const char* name;
  int result = 0;

  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (strcasecmp("what/date", name)==0 ||
      strcasecmp("what/time", name)==0 ||
      strcasecmp("what/source", name)==0 ||
      strcasecmp("where/projdef", name)==0 ||
      strcasecmp("what/object", name)==0 ||
      strcasecmp("what/version", name)==0) {
    // Strings
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR1("Failed to extract %s as a string", name);
      goto done;
    }
    if (strcasecmp("what/date", name)==0) {
      result = Cartesian_setDate(cartesian, value);
    } else if (strcasecmp("what/time", name)==0) {
      result = Cartesian_setTime(cartesian, value);
    } else if (strcasecmp("what/source", name)==0) {
      result = Cartesian_setSource(cartesian, value);
    } else if (strcasecmp("where/projdef", name)==0) {
      projection = CartesianOdimIO_createProjection(value);
      if (projection == NULL) {
        RAVE_ERROR1("Failed to generate projection definition from '%s'", (value == NULL)?"NULL":value);
        goto done;
      }
      Cartesian_setProjection(cartesian, projection);
      result = 1;
    } else if (strcasecmp("what/object", name)==0) {
      if (RaveTypes_getObjectTypeFromString(value) == Rave_ObjectType_IMAGE) {
        result = 1;
      } else {
        RAVE_ERROR1("what/object = '%s' but should be IMAGE",value);
      }
    } else {
      // Allowed but not relevant
      result = 1;
    }
  } else if (strcasecmp("where/xscale", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/xscale not a double");
      goto done;
    }
    Cartesian_setXScale(cartesian, value);
  } else if (strcasecmp("where/yscale", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/yscale not a double");
      goto done;
    }
    Cartesian_setYScale(cartesian, value);
  } else if (strcasecmp("where/xsize", name)==0 ||
             strcasecmp("where/ysize", name)==0 ) {
    result = 1;
  } else if (strcasecmp("where/LL_lon", name)==0 ||
             strcasecmp("where/LL_lat", name)==0 ||
             strcasecmp("where/UL_lon", name)==0 ||
             strcasecmp("where/UL_lat", name)==0 ||
             strcasecmp("where/UR_lon", name)==0 ||
             strcasecmp("where/UR_lat", name)==0 ||
             strcasecmp("where/LR_lon", name)==0 ||
             strcasecmp("where/LR_lat", name)==0) {
    if (!RaveObjectHashTable_put(attrs, name, (RaveCoreObject*)attribute)) {
      RAVE_ERROR1("Failed to add %s to internal table", name);
      goto done;
    }
    result = 1;
  } else {
    if (!Cartesian_addAttribute(cartesian, attribute)) {
      RAVE_WARNING1("Ignored attribute %s", name);
    }
    result = 1;
  }

done:
  RAVE_OBJECT_RELEASE(projection);
  return result;
}

/**
 * Cartesian volume root attributes.
 * @param[in] object - the CartesianOdimArg pointing to a polar scan
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_loadVolumeRootAttribute(void* object, RaveAttribute_t* attribute)
{
  CartesianVolume_t* volume = (CartesianVolume_t*)((CartesianOdimArg*)object)->object;
  RaveObjectHashTable_t* attrs = (RaveObjectHashTable_t*)((CartesianOdimArg*)object)->attrs;
  Projection_t* projection = NULL;

  const char* name;
  int result = 0;

  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (strcasecmp("what/date", name)==0 ||
      strcasecmp("what/time", name)==0 ||
      strcasecmp("what/source", name)==0 ||
      strcasecmp("where/projdef", name)==0 ||
      strcasecmp("what/object", name)==0 ||
      strcasecmp("what/version", name)==0) {
    // Strings
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR1("Failed to extract %s as a string", name);
      goto done;
    }
    if (strcasecmp("what/date", name)==0) {
      result = CartesianVolume_setDate(volume, value);
    } else if (strcasecmp("what/time", name)==0) {
      result = CartesianVolume_setTime(volume, value);
    } else if (strcasecmp("what/source", name)==0) {
      result = CartesianVolume_setSource(volume, value);
    } else if (strcasecmp("where/projdef", name)==0) {
      projection = CartesianOdimIO_createProjection(value);
      if (projection == NULL) {
        RAVE_ERROR1("Failed to generate projection definition from '%s'", (value == NULL)?"NULL":value);
        goto done;
      }
      CartesianVolume_setProjection(volume, projection);
      result = 1;
    } else if (strcasecmp("what/object", name)==0) {
      Rave_ObjectType otype = RaveTypes_getObjectTypeFromString(value);
      if (otype == Rave_ObjectType_COMP ||
          otype == Rave_ObjectType_CVOL) {
        CartesianVolume_setObjectType(volume, otype);
        result = 1;
      } else {
        RAVE_ERROR1("what/object = '%s' but should be COMP or CVOL",value);
      }
    } else {
      // Allowed but not relevant
      result = 1;
    }
  } else if (strcasecmp("where/xscale", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/xscale not a double");
      goto done;
    }
    CartesianVolume_setXScale(volume, value);
  } else if (strcasecmp("where/yscale", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/yscale not a double");
      goto done;
    }
    CartesianVolume_setYScale(volume, value);
  } else if (strcasecmp("where/zscale", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/zscale not a double");
      goto done;
    }
    CartesianVolume_setZScale(volume, value);
  } else if (strcasecmp("where/zstart", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("where/zstart not a double");
      goto done;
    }
    CartesianVolume_setZStart(volume, value);
  } else if (strcasecmp("where/xsize", name)==0 ||
             strcasecmp("where/ysize", name)==0 ||
             strcasecmp("where/zsize", name)==0) {
    result = 1;
  } else if (strcasecmp("where/LL_lon", name)==0 ||
             strcasecmp("where/LL_lat", name)==0 ||
             strcasecmp("where/UL_lon", name)==0 ||
             strcasecmp("where/UL_lat", name)==0 ||
             strcasecmp("where/UR_lon", name)==0 ||
             strcasecmp("where/UR_lat", name)==0 ||
             strcasecmp("where/LR_lon", name)==0 ||
             strcasecmp("where/LR_lat", name)==0) {
    if (!RaveObjectHashTable_put(attrs, name, (RaveCoreObject*)attribute)) {
      RAVE_ERROR1("Failed to add %s to internal table", name);
      goto done;
    }
    result = 1;
  } else {
    if (!CartesianVolume_addAttribute(volume, attribute)) {
      RAVE_WARNING1("Ignored attribute %s", name);
    }
    result = 1;
  }

done:
  RAVE_OBJECT_RELEASE(projection);
  return result;
}

static int CartesianOdimIOInternal_createExtent(RaveObjectHashTable_t* attrs, Projection_t* projection, double* llX, double* llY, double* urX, double* urY)
{
  int result = 0;

  ProjectionPipeline_t* pipeline = NULL;

  RAVE_ASSERT((attrs != NULL), "attrs == NULL");
  RAVE_ASSERT((projection != NULL), "projection == NULL");
  RAVE_ASSERT((llX != NULL), "llX == NULL");
  RAVE_ASSERT((llY != NULL), "llY == NULL");
  RAVE_ASSERT((urX != NULL), "urX == NULL");
  RAVE_ASSERT((urY != NULL), "urY == NULL");

  *llX = 0.0;
  *llY = 0.0;
  *urX = 0.0;
  *urY = 0.0;

  if (CartesianOdimIOInternal_useCartesianLegacyExtent()) {
    /* Legacy variant excludes the +datum=WGS84 when calculating the extent when using pj_fwd and pj_inv */
    pipeline = ProjectionPipeline_createPipelineFromDef("+proj=longlat +ellps=WGS84", Projection_getDefinition(projection));
  } else {
    pipeline = ProjectionPipeline_createDefaultLonLatPipeline(projection);
  }
  if (pipeline == NULL) {
    RAVE_ERROR0("Could not create default lon/lat pipeline");
    goto done;
  }

  if (projection != NULL &&
      RaveObjectHashTable_exists(attrs, "where/LL_lon") &&
      RaveObjectHashTable_exists(attrs, "where/LL_lat") &&
      RaveObjectHashTable_exists(attrs, "where/UR_lon") &&
      RaveObjectHashTable_exists(attrs, "where/UR_lat")) {
    double LL_lon = 0.0, LL_lat = 0.0, UR_lon = 0.0, UR_lat = 0.0;
    if (RaveUtilities_getRaveAttributeDoubleFromHash(attrs, "where/LL_lon", &LL_lon) &&
        RaveUtilities_getRaveAttributeDoubleFromHash(attrs, "where/LL_lat", &LL_lat) &&
        RaveUtilities_getRaveAttributeDoubleFromHash(attrs, "where/UR_lon", &UR_lon) &&
        RaveUtilities_getRaveAttributeDoubleFromHash(attrs, "where/UR_lat", &UR_lat)) {
      if (!ProjectionPipeline_fwd(pipeline, LL_lon * M_PI/180.0, LL_lat * M_PI/180.0, llX, llY)) {
        RAVE_ERROR0("Could not generate XY pair for LL");
        goto done;
      }

      if (!ProjectionPipeline_fwd(pipeline, UR_lon * M_PI/180.0, UR_lat * M_PI/180.0, urX, urY)) {
        RAVE_ERROR0("Could not generate XY pair for UR");
        goto done;
      }
      result = 1;
    }
  }
done:
  RAVE_OBJECT_RELEASE(pipeline);
  return result;
}

/**
 * Cartesian dataset attributes.
 * @param[in] object - the CartesianOdimArg pointing to a polar scan
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_loadDatasetAttribute(void* object, RaveAttribute_t* attribute)
{
  Cartesian_t* cartesian = (Cartesian_t*)((CartesianOdimArg*)object)->object;

  const char* name;
  int result = 0;

  RAVE_ASSERT((attribute != NULL), "attribute == NULL");
  /*
  /dataset1/what/prodpar Attribute, Table 15
  */
  name = RaveAttribute_getName(attribute);
  if (strcasecmp("what/product", name)==0 ||
      strcasecmp("what/prodname", name)==0 ||
      strcasecmp("what/startdate", name)==0 ||
      strcasecmp("what/starttime", name)==0 ||
      strcasecmp("what/enddate", name)==0 ||
      strcasecmp("what/endtime", name)==0) {
    /* Strings */
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR1("Failed to extract %s as a string", name);
      goto done;
    }
    if (strcasecmp("what/product", name)==0) {
      result = Cartesian_setProduct(cartesian, RaveTypes_getProductTypeFromString(value));
    } else if (strcasecmp("what/prodname", name)==0) {
      result = Cartesian_setProdname(cartesian, value);
    } else if (strcasecmp("what/startdate", name)==0) {
      result = Cartesian_setStartDate(cartesian, value);
    } else if (strcasecmp("what/starttime", name)==0) {
      result = Cartesian_setStartTime(cartesian, value);
    } else if (strcasecmp("what/enddate", name)==0) {
      result = Cartesian_setEndDate(cartesian, value);
    } else if (strcasecmp("what/endtime", name)==0) {
      result = Cartesian_setEndTime(cartesian, value);
    } else {
      /* Allowed but not relevant */
      result = 1;
    }
  } else if (strcasecmp("what/quantity", name)==0 ||
             strcasecmp("what/gain", name) == 0 ||
             strcasecmp("what/offset", name) == 0 ||
             strcasecmp("what/nodata", name) == 0 ||
             strcasecmp("what/undetect", name) == 0) {
    /* Do nothing but make sure they aren't added to cartesian object */
    result = 1;
  } else {
    Cartesian_addAttribute(cartesian, attribute);
    result = 1;
  }

done:
  return result;
}

/**
 * Cartesian data attributes.
 * @param[in] object - the CartesianOdimArg pointing to a cartesian parameter
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_loadDatasetDataAttribute(void* object, RaveAttribute_t* attribute)
{
  CartesianParam_t* param = (CartesianParam_t*)((CartesianOdimArg*)object)->object;

  const char* name;
  int result = 0;

  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (strcasecmp("what/quantity", name)==0) {
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR1("Failed to extract %s as a string", name);
      goto done;
    }
    if (!CartesianParam_setQuantity(param, RaveHL_convertQuantity(value))) {
      goto done;
    }
    result = 1;
  } else if (strcasecmp("what/gain", name)==0 ||
             strcasecmp("what/offset", name)==0 ||
             strcasecmp("what/nodata", name)==0 ||
             strcasecmp("what/undetect", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR1("%s not a double", name);
      goto done;
    }
    if (strcasecmp("what/gain", name)==0) {
      CartesianParam_setGain(param, value);
    } else if (strcasecmp("what/offset", name)==0) {
      CartesianParam_setOffset(param, value);
    } else if (strcasecmp("what/nodata", name)==0) {
      CartesianParam_setNodata(param, value);
    } else if (strcasecmp("what/undetect", name)==0) {
      CartesianParam_setUndetect(param, value);
    }
  } else {
    if (!CartesianParam_addAttribute(param, attribute)) {
      RAVE_INFO1("Ignoring %s", name);
    }
    result = 1;
  }

done:
  return result;
}

/**
 * Called when an dataset belonging to a cartesian parameter
 * is found.
 * @param[in] object - the CartesianOdimArg pointing to a cartesian parameter
 * @param[in] nbins - the number of bins
 * @param[in] nrays - the number of rays
 * @param[in] data  - the data
 * @param[in] dtype - the type of the data.
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_loadDatasetDataDataset(void* object, hsize_t xsize, hsize_t ysize, void* data, RaveDataType dtype, const char* nodeName)
{
  int result = 0;
  CartesianParam_t* param = (CartesianParam_t*)((CartesianOdimArg*)object)->object;
  if (data == NULL && ((CartesianOdimArg*)object)->lazyReader != NULL) {
    LazyDataset_t* datasetReader = RAVE_OBJECT_NEW(&LazyDataset_TYPE);
    if (datasetReader != NULL) {
      result = LazyDataset_init(datasetReader, ((CartesianOdimArg*)object)->lazyReader, nodeName);
    }
    if (result) {
      result = CartesianParam_setLazyDataset(param, datasetReader);
    }
    RAVE_OBJECT_RELEASE(datasetReader);
  } else {
    result = CartesianParam_setData(param, xsize, ysize, data, dtype);
  }
  return result;
}

/**
 * Loads a cartesian parameter.
 * @param[in] nodelist - the hlhdf node list
 * @param[in] fmt - the varargs format
 * @param[in] ... - the varargs
 * @return the parameter on success otherwise NULL
 */
static CartesianParam_t* CartesianOdimIOInternal_loadCartesianParameter(LazyNodeListReader_t* lazyReader, const char* fmt, ...)
{
  char nodeName[1024];
  CartesianParam_t* param = NULL;
  CartesianParam_t* result = NULL;
  RaveField_t* field = NULL;
  CartesianOdimArg arg;
  int pindex = 0;
  va_list ap;
  int n = 0;

  RAVE_ASSERT((lazyReader != NULL), "nodelist == NULL");

  arg.attrs = NULL;

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR1("Failed to create image name from fmt=%s", fmt);
    goto done;
  }

  param = RAVE_OBJECT_NEW(&CartesianParam_TYPE);
  if (param == NULL) {
    goto done;
  }

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)param;
  arg.attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (arg.attrs == NULL) {
    goto done;
  }

  if (!RaveHL_loadAttributesAndData(arg.nodelist, &arg,
                                    CartesianOdimIOInternal_loadDatasetDataAttribute,
                                    CartesianOdimIOInternal_loadDatasetDataDataset,
                                    nodeName)) {
    RAVE_ERROR1("Failed to load data and attributes for %s", nodeName);
  }

  pindex = 1;
  while (RaveHL_hasNodeByName(arg.nodelist, "%s/quality%d", nodeName, pindex)) {
    field = OdimIoUtilities_loadField(lazyReader, "%s/quality%d", nodeName, pindex);
    if (field == NULL ||
        !CartesianParam_addQualityField(param, field)) {
      RAVE_ERROR0("Failed to load quality field for parameter");
      goto done;
    }
    pindex++;
    RAVE_OBJECT_RELEASE(field);
  }

  result = RAVE_OBJECT_COPY(param);
done:
  RAVE_OBJECT_RELEASE(arg.attrs);
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(field);
  return result;
}

/**
 * Adds a cartesian parameter to the nodelist to be written
 * @param[in] param - the parameter to write
 * @param[in] nodelist - the hlhdf node list to be updated
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs list
 * @return 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_addParameterToNodeList(CartesianParam_t* param, HL_NodeList* nodelist, const char* fmt, ...)
{
  int result = 0;
  char nodeName[1024];
  RaveObjectList_t* attributes = NULL;
  RaveObjectList_t* qualityfields = NULL;
  va_list ap;
  int n = 0;

  RAVE_ASSERT((param != NULL), "param == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR1("Failed to create image name from fmt=%s", fmt);
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, nodeName)) {
    if (!RaveHL_createGroup(nodelist, nodeName)) {
      goto done;
    }
  }

  attributes = CartesianParam_getAttributeValues(param);
  if (attributes == NULL) {
    goto done;
  }

  if (!RaveUtilities_addDoubleAttributeToList(attributes, "what/gain", CartesianParam_getGain(param)) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "what/nodata", CartesianParam_getNodata(param)) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "what/offset", CartesianParam_getOffset(param)) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "what/undetect", CartesianParam_getUndetect(param)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/quantity", CartesianParam_getQuantity(param))) {
    goto done;
  }

  if (!RaveHL_addAttributes(nodelist, attributes, nodeName)) {
    goto done;
  }

  if (!RaveHL_addData(nodelist,
                      CartesianParam_getData(param),
                      CartesianParam_getXSize(param),
                      CartesianParam_getYSize(param),
                      CartesianParam_getDataType(param),
                      nodeName)) {
    goto done;
  }

  if ((qualityfields = CartesianParam_getQualityFields(param)) == NULL) {
    goto done;
  }

  result = OdimIoUtilities_addQualityFields(qualityfields, nodelist, nodeName);

  result = 1;
done:
  RAVE_OBJECT_RELEASE(qualityfields);
  RAVE_OBJECT_RELEASE(attributes);
  return result;
}

/**
 * Adds a cartesian image (belonging to a volume) to a node list.
 * @param[in] cvol - the cartesian image to be added to a node list
 * @param[in] nodelist - the nodelist the nodes should be added to
 * @returns 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_addCartesianImageToNodeList(Cartesian_t* cartesian, RaveIO_ODIM_Version version, HL_NodeList* nodelist, const char* fmt, ...)
{
  int result = 0;
  char nodeName[1024];
  RaveObjectList_t* attributes = NULL;
  RaveObjectList_t* qualityfields = NULL;
  RaveList_t* params = NULL;
  int i = 0;
  va_list ap;
  int n = 0;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR1("Failed to create image name from fmt=%s", fmt);
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, nodeName)) {
    if (!RaveHL_createGroup(nodelist, nodeName)) {
      goto done;
    }
  }

  attributes = Cartesian_getAttributeValues(cartesian);
  if (attributes == NULL) {
    goto done;
  }

  if (!RaveUtilities_replaceStringAttributeInList(attributes, "what/product",
                                                    RaveTypes_getStringFromProductType(Cartesian_getProduct(cartesian))) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/startdate", Cartesian_getStartDate(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/starttime", Cartesian_getStartTime(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/enddate", Cartesian_getEndDate(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/endtime", Cartesian_getEndTime(cartesian))) {
    goto done;
  }

  if (version >= RaveIO_ODIM_Version_2_3) {
    if (Cartesian_getProdname(cartesian) == NULL) {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", "BALTRAD cartesian")) {
        goto done;
      }
    } else {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", Cartesian_getProdname(cartesian))) {
        goto done;
      }
    }
  }

  if (!RaveHL_addAttributes(nodelist, attributes, nodeName)) {
    goto done;
  }

  params = Cartesian_getParameterNames(cartesian);
  if (params == NULL) {
    goto done;
  }
  n = RaveList_size(params);

  for (i = 0; i < n; i++) {
    const char* quantity = RaveList_get(params, i);
    CartesianParam_t* param = Cartesian_getParameter(cartesian, quantity);
    if (!CartesianOdimIOInternal_addParameterToNodeList(param, nodelist, "%s/data%d", nodeName, i+1)) {
      RAVE_OBJECT_RELEASE(param);
      goto done;
    }
    RAVE_OBJECT_RELEASE(param);
  }

  if ((qualityfields = Cartesian_getQualityFields(cartesian)) == NULL) {
    goto done;
  }

  result = OdimIoUtilities_addQualityFields(qualityfields, nodelist, nodeName);
done:
  RaveList_freeAndDestroy(&params);
  RAVE_OBJECT_RELEASE(attributes);
  RAVE_OBJECT_RELEASE(qualityfields);
  return result;
}

/**
 * Builds a cartesian dataset from the hlhdf nodelist
 * @param[in] nodelist - the hlhdf node list
 * @param[in] image - the cartesian image to fill
 * @param[in] fmt - the varargs formatter string
 * @param[in] ... - the varargs list
 * @returns 1 on success otherwise 0
 */
static int CartesianOdimIOInternal_fillCartesianDataset(LazyNodeListReader_t* lazyReader, Cartesian_t* image, const char* fmt, ...)
{
  int result = 0;
  char nodeName[1024];
  CartesianOdimArg arg;
  va_list ap;
  int n = 0;
  int pindex = 0;

  RAVE_ASSERT((lazyReader != NULL), "lazyReader == NULL");
  RAVE_ASSERT((image != NULL), "image == NULL");

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)image;

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR1("Failed to create name from fmt=%s", fmt);
    goto done;
  }

  if (!RaveHL_loadAttributesAndData(arg.nodelist, &arg,
                                    CartesianOdimIOInternal_loadDatasetAttribute,
                                    NULL,
                                    nodeName)) {
    RAVE_ERROR1("Failed to load attributes for cartesian at dataset level %s", nodeName);
    goto done;
  }

  pindex = 1;
  while (RaveHL_hasNodeByName(arg.nodelist, "%s/data%d", nodeName, pindex)) {
    double v = 0.0;
    CartesianParam_t* param = CartesianOdimIOInternal_loadCartesianParameter(lazyReader, "%s/data%d", nodeName, pindex);
    if (param == NULL) {
      RAVE_ERROR2("Failed to load cartesian parameter %s/data%d", nodeName, pindex);
      goto done;
    }

    /* Fix so that parameter contains all necessary stuff */
    if (CartesianOdimIOInternal_getDatasetDoubleValueIfMissing(arg.nodelist, nodeName, pindex, "what/gain", &v)) {
      CartesianParam_setGain(param, v);
    }
    if (CartesianOdimIOInternal_getDatasetDoubleValueIfMissing(arg.nodelist, nodeName, pindex, "what/offset", &v)) {
      CartesianParam_setOffset(param, v);
    }
    if (CartesianOdimIOInternal_getDatasetDoubleValueIfMissing(arg.nodelist, nodeName, pindex, "what/nodata", &v)) {
      CartesianParam_setNodata(param, v);
    }
    if (CartesianOdimIOInternal_getDatasetDoubleValueIfMissing(arg.nodelist, nodeName, pindex, "what/undetect", &v)) {
      CartesianParam_setUndetect(param, v);
    }
    if (CartesianParam_getQuantity(param) == NULL) {
      RaveAttribute_t* attr = RaveHL_getAttribute(arg.nodelist, "%s/what/quantity", nodeName);
      char* value = NULL;
      if (attr == NULL) {
        RAVE_ERROR0("Could not find any quantity for cartesian parameter");
        goto done;
      }
      if (!RaveAttribute_getString(attr, &value)) {
        RAVE_ERROR0("Quantity not a string valuefor cartesian parameter");
        goto done;
      }
      if (!CartesianParam_setQuantity(param, RaveHL_convertQuantity(value))) {
        RAVE_ERROR0("Could not set quantity in parameter");
        goto done;
      }
      RAVE_OBJECT_RELEASE(attr);
    }

    if (!Cartesian_addParameter(image, param)) {
      RAVE_ERROR2("Failed to add parameter to cartesian %s/data%d", nodeName, pindex);
      RAVE_OBJECT_RELEASE(param);
      goto done;
    }
    RAVE_OBJECT_RELEASE(param);
    pindex++;
  }

  result = 1;
  pindex = 1;
  while (result == 1 && RaveHL_hasNodeByName(arg.nodelist, "%s/quality%d", nodeName, pindex)) {
    RaveField_t* field = OdimIoUtilities_loadField(lazyReader, "%s/quality%d", nodeName, pindex);
    if (field != NULL) {
      result = Cartesian_addQualityField(image, field);
    } else {
      result = 0;
    }
    pindex++;
    RAVE_OBJECT_RELEASE(field);
  }
  Cartesian_setObjectType(image, Rave_ObjectType_IMAGE);

done:
  return result;
}

/**
 * Adds the lon lat corner extent to the attribute list. If llX, llY, urX and urY are all 0.0, then
 * nothing will be added to the attribute list.
 * @param[in] list - the list to add the attributes to
 * @param[in] projection - the projection to use for converting to corner coordinates
 * @param[in] llX - the lower left X coordinate
 * @param[in] llY - the lower left Y coordinate
 * @param[in] urX - the upper right X coordinate
 * @param[in] urY - the upper right Y coordinate
 * @returns 1 on success otherwise 0
 */
int CartesianOdimIOInternal_addLonLatExtentToAttributeList(RaveObjectList_t* attrs, Projection_t* projection, double llX, double llY, double urX, double urY)
{
  int result = 0;
  double LL_lat = 0.0, LL_lon = 0.0, LR_lat = 0.0, LR_lon = 0.0;
  double UL_lat = 0.0, UL_lon = 0.0, UR_lat = 0.0, UR_lon = 0.0;
  ProjectionPipeline_t* pipeline = NULL;

  if (CartesianOdimIOInternal_useCartesianLegacyExtent()) {
    /* Legacy variant excludes the +datum=WGS84 when calculating the extent when using pj_fwd and pj_inv */
    pipeline = ProjectionPipeline_createPipelineFromDef("+proj=longlat +ellps=WGS84", Projection_getDefinition(projection));
  } else {
    pipeline = ProjectionPipeline_createDefaultLonLatPipeline(projection);
  }

  if (pipeline == NULL) {
    RAVE_ERROR0("Could not create default lon/lat pipeline");
    goto done;
  }
  RAVE_ASSERT((attrs != NULL), "attrs == NULL");
  RAVE_ASSERT((projection != NULL), "projection == NULL");

  // Generate the correct corner coordinates.

  if (!ProjectionPipeline_inv(pipeline, llX, llY, &LL_lon, &LL_lat) ||
      !ProjectionPipeline_inv(pipeline, llX, urY, &UL_lon, &UL_lat) ||
      !ProjectionPipeline_inv(pipeline, urX, urY, &UR_lon, &UR_lat) ||
      !ProjectionPipeline_inv(pipeline, urX, llY, &LR_lon, &LR_lat)) {
    RAVE_ERROR0("Failed to translate surface extent into lon/lat corner pairs\n");
    goto done;
  }

  if (!RaveUtilities_replaceDoubleAttributeInList(attrs, "where/LL_lat", LL_lat * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/LL_lon", LL_lon * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/LR_lat", LR_lat * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/LR_lon", LR_lon * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/UL_lat", UL_lat * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/UL_lon", UL_lon * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/UR_lat", UR_lat * 180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attrs, "where/UR_lon", UR_lon * 180.0/M_PI)) {
    goto done;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(pipeline);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
void CartesianOdimIO_setVersion(CartesianOdimIO_t* self, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->version = version;
}

RaveIO_ODIM_Version CartesianOdimIO_getVersion(CartesianOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->version;
}

void CartesianOdimIO_setStrict(CartesianOdimIO_t* self, int strict)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->strict = strict;
}

int CartesianOdimIO_isStrict(CartesianOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->strict;
}

const char* CartesianOdimIO_getErrorMessage(CartesianOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->error_message;
}

int CartesianOdimIO_readCartesian(CartesianOdimIO_t* self, LazyNodeListReader_t* lazyReader, Cartesian_t* cartesian)
{
  int result = 0;
  CartesianOdimArg arg;
  Projection_t* proj = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((lazyReader != NULL), "nodelist == NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)cartesian;
  arg.attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (arg.attrs == NULL) {
    RAVE_ERROR0("Failed to allocate memory");
    goto done;
  }

  if (!RaveHL_loadAttributesAndData(arg.nodelist, &arg,
                                    CartesianOdimIOInternal_loadRootAttribute,
                                    NULL,
                                    "")) {
    RAVE_ERROR0("Failed to load attributes for cartesian at root level");
    goto done;
  }

  if ((proj = Cartesian_getProjection(cartesian)) != NULL) {
    double llX = 0.0, llY = 0.0, urX = 0.0, urY = 0.0;

    if (!CartesianOdimIOInternal_createExtent(arg.attrs, proj, &llX, &llY, &urX, &urY)) {
      goto done;
    }
    Cartesian_setAreaExtent(cartesian, llX, llY, urX, urY);
  }

  RAVE_OBJECT_RELEASE(arg.attrs);

  if (!CartesianOdimIOInternal_fillCartesianDataset(lazyReader, cartesian, "/dataset1")) {
    goto done;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(arg.attrs);
  RAVE_OBJECT_RELEASE(proj);

  return result;
}

int CartesianOdimIO_readVolume(CartesianOdimIO_t* self, LazyNodeListReader_t* lazyReader, CartesianVolume_t* volume)
{
  int result = 0;
  CartesianOdimArg arg;
  int index = 1;
  Projection_t* proj = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((lazyReader != NULL), "lazyReader == NULL");
  RAVE_ASSERT((volume != NULL), "volume == NULL");

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)volume;
  arg.attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (arg.attrs == NULL) {
    RAVE_ERROR0("Failed to allocate memory");
    goto done;
  }
  if (!RaveHL_loadAttributesAndData(arg.nodelist, &arg,
                                    CartesianOdimIOInternal_loadVolumeRootAttribute,
                                    NULL,
                                    "")) {
    RAVE_ERROR0("Failed to load attributes for volume at root level");
    goto done;
  }
  if ((proj = CartesianVolume_getProjection(volume)) != NULL) {
    double llX = 0.0, llY = 0.0, urX = 0.0, urY = 0.0;

    if (!CartesianOdimIOInternal_createExtent(arg.attrs, proj, &llX, &llY, &urX, &urY)) {
      goto done;
    }
    CartesianVolume_setAreaExtent(volume, llX, llY, urX, urY);
  }

  result = 1;
  index = 1;
  while (result == 1 && RaveHL_hasNodeByName(arg.nodelist, "/dataset%d", index)) {
    Cartesian_t* image = RAVE_OBJECT_NEW(&Cartesian_TYPE);
    if (image != NULL) {
      result = CartesianOdimIOInternal_fillCartesianDataset(lazyReader, image, "/dataset%d", index);
      if (result == 1) {
        result = CartesianVolume_addImage(volume, image);
      }
    } else {
      result = 0;
    }
    index++;
    RAVE_OBJECT_RELEASE(image);
  }

done:
  RAVE_OBJECT_RELEASE(arg.attrs);
  RAVE_OBJECT_RELEASE(proj);
  return result;
}

int CartesianOdimIO_fillImage(CartesianOdimIO_t* self, HL_NodeList* nodelist, Cartesian_t* cartesian)
{
  int result = 0, n = 0, i = 0;
  RaveObjectList_t* attributes = NULL;
  Rave_ObjectType otype = Rave_ObjectType_UNDEFINED;
  Projection_t* projection = NULL;
  RaveObjectList_t* qualityfields = NULL;
  RaveList_t* params = NULL;
  char* source = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  strcpy(self->error_message, "");
  if (!CartesianOdimIO_isValidImageAddMsg(cartesian, self->error_message, 1024) &&
      !CartesianOdimIO_isValidVolumeImageAddMsg(cartesian, self->error_message, 1024)) {
    goto done;
  }

  if (!CartesianOdimIO_validateCartesianHowAttributes(self, cartesian)) {
    RAVE_ERROR0("Could not validate cartesian how-attributes");
    goto done;
  }

  otype = Cartesian_getObjectType(cartesian);
  projection = Cartesian_getProjection(cartesian);

  if (otype != Rave_ObjectType_COMP && otype != Rave_ObjectType_IMAGE) {
    RAVE_WARNING1("CartesianOdimIO_fillImage does not support objectType = %d\n", otype);
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, "/Conventions")) {
    if (!RaveHL_createStringValue(nodelist, RaveHL_getOdimVersionString(self->version), "/Conventions")) {
      goto done;
    }
  }

  attributes = Cartesian_getAttributeValues(cartesian);
  if (attributes != NULL) {
    const char* objectType = RaveTypes_getStringFromObjectType(Cartesian_getObjectType(cartesian));
    if (!RaveUtilities_addStringAttributeToList(attributes, "what/object", objectType) ||
        !RaveUtilities_replaceStringAttributeInList(attributes, "what/version", RaveHL_getH5RadVersionStringFromOdimVersion(self->version))) {
      RAVE_ERROR0("Failed to add what/object or what/version to attributes");
      goto done;
    }
  } else {
    RAVE_ERROR0("Failed to aquire attributes for cartesian product");
    goto done;
  }

  if (projection != NULL) {
    double llX = 0.0, llY = 0.0, urX = 0.0, urY = 0.0;

    if (!RaveUtilities_addStringAttributeToList(attributes, "where/projdef", Projection_getDefinition(projection))) {
      goto done;
    }
    Cartesian_getAreaExtent(cartesian, &llX, &llY, &urX, &urY);
    if (!CartesianOdimIOInternal_addLonLatExtentToAttributeList(attributes, projection, llX, llY, urX, urY)) {
      goto done;
    }
  }

  source = RaveUtilities_handleSourceVersion(Cartesian_getSource(cartesian), self->version);
  if (self->strict && !RaveUtilities_isSourceValid(source, self->version)) {
    strcpy(self->error_message, "what/source is not valid, missing ORG or NOD?");
    goto done;
  }

  if (!RaveUtilities_addStringAttributeToList(attributes, "what/date", Cartesian_getDate(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/time", Cartesian_getTime(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/source", source) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "where/xscale", Cartesian_getXScale(cartesian)) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "where/yscale", Cartesian_getYScale(cartesian)) ||
      !RaveUtilities_replaceLongAttributeInList(attributes, "where/xsize", Cartesian_getXSize(cartesian)) ||
      !RaveUtilities_replaceLongAttributeInList(attributes, "where/ysize", Cartesian_getYSize(cartesian))) {
    goto done;
  }

  // prodpar is dataset specific.. so it should only be there for images in volumes.
  RaveUtilities_removeAttributeFromList(attributes, "what/prodpar");

  if (attributes == NULL || !RaveHL_addAttributes(nodelist, attributes, "")) {
    goto done;
  }

  RAVE_OBJECT_RELEASE(attributes);

  attributes = Cartesian_getAttributeValues(cartesian);

  if (!RaveUtilities_addStringAttributeToList(attributes, "what/startdate", Cartesian_getStartDate(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/starttime", Cartesian_getStartTime(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/enddate", Cartesian_getEndDate(cartesian)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/endtime", Cartesian_getEndTime(cartesian)) ||
      !RaveUtilities_replaceStringAttributeInList(attributes, "what/product",
                                                  RaveTypes_getStringFromProductType(Cartesian_getProduct(cartesian)))) {
    goto done;
  }

  if (self->version >= RaveIO_ODIM_Version_2_3) {
    if (Cartesian_getProdname(cartesian) == NULL) {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", "BALTRAD cartesian")) {
        goto done;
      }
    } else {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", Cartesian_getProdname(cartesian))) {
        goto done;
      }
    }
  }

  if (!RaveHL_createGroup(nodelist,"/dataset1")) {
    goto done;
  }

  if (attributes == NULL || !RaveHL_addAttributes(nodelist, attributes, "/dataset1")) {
    goto done;
  }

  params = Cartesian_getParameterNames(cartesian);
  if (params == NULL) {
    goto done;
  }
  n = RaveList_size(params);

  for (i = 0; i < n; i++) {
    const char* quantity = RaveList_get(params, i);
    CartesianParam_t* param = Cartesian_getParameter(cartesian, quantity);
    if (!CartesianOdimIOInternal_addParameterToNodeList(param, nodelist, "/dataset1/data%d", i+1)) {
      RAVE_OBJECT_RELEASE(param);
      goto done;
    }
    RAVE_OBJECT_RELEASE(param);
  }

  if ((qualityfields = Cartesian_getQualityFields(cartesian)) == NULL) {
    goto done;
  }

  result = OdimIoUtilities_addQualityFields(qualityfields, nodelist, "/dataset1");
done:
  RaveList_freeAndDestroy(&params);
  RAVE_OBJECT_RELEASE(attributes);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_OBJECT_RELEASE(qualityfields);
  RAVE_FREE(source);
  return result;
}

int CartesianOdimIO_fillVolume(CartesianOdimIO_t* self, HL_NodeList* nodelist, CartesianVolume_t* volume)
{
  int result = 0;
  int nrImages = 0;
  int i = 0;
  RaveObjectList_t* attributes = NULL;
  Projection_t* projection = NULL;
  char* source = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((volume != NULL), "volume == NULL");

  // First verify that no bogus data is entered into the system.
  if (!CartesianOdimIO_isValidVolumeAddMsg(volume, self->error_message, 1024)) {
    goto done;
  }

  if (!CartesianOdimIO_validateVolumeHowAttributes(self, volume)) {
    RAVE_ERROR0("Could not validate volume how-attributes");
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, "/Conventions")) {
    if (!RaveHL_createStringValue(nodelist, RaveHL_getOdimVersionString(self->version), "/Conventions")) {
      goto done;
    }
  }

  attributes = CartesianVolume_getAttributeValues(volume);
  if (attributes != NULL) {
    const char* objectType = RaveTypes_getStringFromObjectType(CartesianVolume_getObjectType(volume));
    if (!RaveUtilities_addStringAttributeToList(attributes, "what/object", objectType) ||
        !RaveUtilities_replaceStringAttributeInList(attributes, "what/version", RaveHL_getH5RadVersionStringFromOdimVersion(self->version))) {
      RAVE_ERROR0("Failed to add what/object or what/version to attributes");
      goto done;
    }
  } else {
    goto done;
  }

  source = RaveUtilities_handleSourceVersion(CartesianVolume_getSource(volume), self->version);
  if (self->strict && !RaveUtilities_isSourceValid(source, self->version)) {
    strcpy(self->error_message, "what/source is not valid, missing ORG or NOD?");
    goto done;
  }

  if (!RaveUtilities_addStringAttributeToList(attributes, "what/date", CartesianVolume_getDate(volume)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/time", CartesianVolume_getTime(volume)) ||
      !RaveUtilities_addStringAttributeToList(attributes, "what/source", source) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "where/xscale", CartesianVolume_getXScale(volume)) ||
      !RaveUtilities_addDoubleAttributeToList(attributes, "where/yscale", CartesianVolume_getYScale(volume)) ||
      !RaveUtilities_replaceLongAttributeInList(attributes, "where/xsize", CartesianVolume_getXSize(volume)) ||
      !RaveUtilities_replaceLongAttributeInList(attributes, "where/ysize", CartesianVolume_getYSize(volume))) {
    goto done;
  }

  if (self->version >= RaveIO_ODIM_Version_2_3) {
    if (!RaveUtilities_addDoubleAttributeToList(attributes, "where/zscale", CartesianVolume_getZScale(volume)) ||
        !RaveUtilities_addDoubleAttributeToList(attributes, "where/zstart", CartesianVolume_getZStart(volume)) ||
        !RaveUtilities_addLongAttributeToList(attributes, "where/zsize", CartesianVolume_getZSize(volume))) {
      goto done;
    }
  }

  // Add projection + extent if possible
  projection = CartesianVolume_getProjection(volume);
  if (projection != NULL) {
    double llX = 0.0, llY = 0.0, urX = 0.0, urY = 0.0;

    if (!RaveUtilities_addStringAttributeToList(attributes, "where/projdef", Projection_getDefinition(projection))) {
      goto done;
    }
    CartesianVolume_getAreaExtent(volume, &llX, &llY, &urX, &urY);
    if (!CartesianOdimIOInternal_addLonLatExtentToAttributeList(attributes, projection, llX, llY, urX, urY)) {
      goto done;
    }
  }

  if (attributes == NULL || !RaveHL_addAttributes(nodelist, attributes, "")) {
    goto done;
  }

  result = 1;

  nrImages = CartesianVolume_getNumberOfImages(volume);
  for (i = 0; result == 1 && i < nrImages; i++) {
    Cartesian_t* image = CartesianVolume_getImage(volume, i);
    result = CartesianOdimIOInternal_addCartesianImageToNodeList(image, self->version, nodelist, "/dataset%d", (i+1));
    RAVE_OBJECT_RELEASE(image);
  }
done:
  RAVE_OBJECT_RELEASE(attributes);
  RAVE_OBJECT_RELEASE(projection);
  RAVE_FREE(source);
  return result;
}

int CartesianOdimIO_isValidImageAddMsg(Cartesian_t* cartesian, char* msg, int maxlen)
{
  int result = 0;
  Projection_t* projection = NULL;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  if (Cartesian_getDate(cartesian) == NULL ||
      Cartesian_getTime(cartesian) == NULL ||
      Cartesian_getSource(cartesian) == NULL) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "Date, Time and Source must be set", maxlen);
    }
    RAVE_INFO0("Date, Time and Source must be set");
    goto done;
  }

  if ((projection = Cartesian_getProjection(cartesian)) == NULL) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "Projection must be defined for cartesian", maxlen);
    }
    RAVE_INFO0("Projection must be defined for cartesian");
    goto done;
  }

  if (Cartesian_getXSize(cartesian) == 0 ||
      Cartesian_getYSize(cartesian) == 0 ||
      Cartesian_getXScale(cartesian) == 0.0 ||
      Cartesian_getYScale(cartesian) == 0.0) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "x/y sizes and scales must be defined", maxlen);
    }
    RAVE_INFO0("x/y sizes and scales must be defined");
    goto done;
  }

  if (Cartesian_getProduct(cartesian) == Rave_ProductType_UNDEFINED) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "product type must be defined", maxlen);
    }
    RAVE_INFO0("product type must be defined");
    goto done;
  }

  if (Cartesian_getParameterCount(cartesian) <= 0) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "Must be at least on parameter in a cartesian product", maxlen);
    }
    RAVE_INFO0("Must be at least on parameter in a cartesian product");
    goto done;
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(projection);
  return result;

}

int CartesianOdimIO_isValidImage(Cartesian_t* cartesian)
{
  return CartesianOdimIO_isValidImageAddMsg(cartesian, NULL, 0);
}

int CartesianOdimIO_isValidVolumeImageAddMsg(Cartesian_t* cartesian, char* msg, int maxlen)
{
  int result = 0;

  RAVE_ASSERT((cartesian != NULL), "cartesian == NULL");

  if (Cartesian_getStartDate(cartesian) == NULL ||
      Cartesian_getStartTime(cartesian) == NULL ||
      Cartesian_getEndDate(cartesian) == NULL ||
      Cartesian_getEndTime(cartesian) == NULL) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "start and end date/time must be set", maxlen);
    }
    RAVE_INFO0("start and end date/time must be set");
    goto done;
  }

  if (Cartesian_getXSize(cartesian) == 0 ||
      Cartesian_getYSize(cartesian) == 0 ||
      Cartesian_getXScale(cartesian) == 0.0 ||
      Cartesian_getYScale(cartesian) == 0.0) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "x/y sizes and scales must be defined", maxlen);
    }
    RAVE_INFO0("x/y sizes and scales must be defined");
    goto done;
  }

  if (Cartesian_getProduct(cartesian) == Rave_ProductType_UNDEFINED) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "product type must be defined", maxlen);
    }
    RAVE_INFO0("product type must be defined");
    goto done;
  }
  if (Cartesian_getParameterCount(cartesian) <= 0) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "Must at least exist one parameter for a cartesian product", maxlen);
    }
    RAVE_INFO0("Must at least exist one parameter for a cartesian product");
    goto done;
  }

  result = 1;
done:
  return result;

}


int CartesianOdimIO_isValidVolumeImage(Cartesian_t* cartesian)
{
  return CartesianOdimIO_isValidVolumeImageAddMsg(cartesian, NULL, 0);
}

int CartesianOdimIO_isValidVolumeAddMsg(CartesianVolume_t* volume, char* msg, int maxlen)
{
  int result = 0;
  int ncartesians = 0;
  int i = 0;
  RAVE_ASSERT((volume != NULL), "volume == NULL");
  if (CartesianVolume_getDate(volume) == NULL ||
      CartesianVolume_getTime(volume) == NULL ||
      CartesianVolume_getSource(volume) == NULL) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "date, time and source MUST be defined", maxlen);
    }
    RAVE_INFO0("date, time and source MUST be defined");
    goto done;
  }

  ncartesians = CartesianVolume_getNumberOfImages(volume);
  if (ncartesians <= 0) {
    if (msg != NULL && maxlen > 0) {
      strncpy(msg, "a cartesian volume must at least contains one product", maxlen);
    }
    RAVE_INFO0("a cartesian volume must at least contains one product");
    goto done;
  }

  result = 1;
  for (i = 0; result == 1 && i < ncartesians; i++) {
    Cartesian_t* cartesian = CartesianVolume_getImage(volume, i);
    result = CartesianOdimIO_isValidVolumeImageAddMsg(cartesian, msg, maxlen);
    RAVE_OBJECT_RELEASE(cartesian);
  }

  result = 1;
done:
  return result;
}

int CartesianOdimIO_isValidVolume(CartesianVolume_t* volume)
{
  return CartesianOdimIO_isValidVolumeAddMsg(volume, NULL, 0);
}

int CartesianOdimIO_validateVolumeHowAttributes(CartesianOdimIO_t* self, CartesianVolume_t* volume)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");

  if (!self->strict) {
    return 1;
  }

  if (self->version >= RaveIO_ODIM_Version_2_4) {
    int gotSimulated = CartesianVolume_hasAttribute(volume, "how/simulated");
    if (!gotSimulated) {
      int i = 0, nrimages = CartesianVolume_getNumberOfImages(volume);
      gotSimulated = 1;
      for (i = 0; i < nrimages && gotSimulated; i++) {
        Cartesian_t* image = CartesianVolume_getImage(volume, i);
        gotSimulated = Cartesian_hasAttribute(image, "how/simulated");
        RAVE_OBJECT_RELEASE(image);
      }
    }
    if (!gotSimulated) {
      RAVE_ERROR0("Failed to validate how attributes for volume. Missing required attribute in either volume or image");
      strcpy(self->error_message, "Failed to validate how attributes for volume. Missing required attribute in either volume or image");
    }
    result = gotSimulated;
  }
  return result;
}

int CartesianOdimIO_validateCartesianHowAttributes(CartesianOdimIO_t* self, Cartesian_t* image)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (!self->strict) {
    return 1;
  }
  if (self->version >= RaveIO_ODIM_Version_2_4) {
    int gotSimulated = Cartesian_hasAttribute(image, "how/simulated");
    if (!gotSimulated) {
      RAVE_ERROR0("Failed to validate how attributes for cartesian image. Missing required attribute.");
      strcpy(self->error_message, "Failed to validate how attributes for volume. Missing required attribute.");
    }
    result = gotSimulated;
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType CartesianOdimIO_TYPE = {
    "CartesianOdimIO",
    sizeof(CartesianOdimIO_t),
    CartesianOdimIO_constructor,
    CartesianOdimIO_destructor,
    CartesianOdimIO_copyconstructor
};
