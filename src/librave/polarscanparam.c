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
 * Defines the functions available when working with polar scans
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI),,
 * @date 2009-10-15
 */
#include "polarscanparam.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include "rave_object.h"
#include "rave_datetime.h"
#include "rave_transform.h"
#include "rave_data2d.h"
#include "raveobject_hashtable.h"
#include "rave_utilities.h"
#include <float.h>

/**
 * Represents one param in a scan
 */
struct _PolarScanParam_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveData2D_t* data; /**< data ptr */
  LazyDataset_t* lazyDataset; /**< the lazy dataset */
  char* quantity;    /**< what does this data represent */
  double gain;       /**< gain when scaling */
  double offset;     /**< offset when scaling */
  double nodata;     /**< nodata */
  double undetect;   /**< undetect */
  RaveObjectHashTable_t* attrs; /**< attributes */
  RaveObjectList_t* qualityfields; /**< quality fields */
};

/*@{ Private functions */

/**
 * Constructor.
 */
static int PolarScanParam_constructor(RaveCoreObject* obj)
{
  PolarScanParam_t* this = (PolarScanParam_t*)obj;
  this->data = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
  this->attrs = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  this->qualityfields = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  this->quantity = NULL;
  this->gain = 0.0L;
  this->offset = 0.0L;
  this->nodata = 0.0L;
  this->undetect = 0.0L;
  this->lazyDataset = NULL;
  if (this->data == NULL || this->attrs == NULL || this->qualityfields == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);

  return 0;
}

/**
 * Ensures that we have got data in the data-table set
 */
static RaveData2D_t* PolarScanParamInternal_ensureData2D(PolarScanParam_t* scanparam)
{
  if (scanparam->lazyDataset != NULL) {
    RaveData2D_t* loaded = LazyDataset_get(scanparam->lazyDataset);
    if (loaded != NULL) {
      RAVE_DEBUG0("PolarScanParamInternal_ensureData2D: LazyDataset fetched");
      RAVE_OBJECT_RELEASE(scanparam->data);
      scanparam->data = RAVE_OBJECT_COPY(loaded);
      RAVE_OBJECT_RELEASE(scanparam->lazyDataset);
    } else {
      RAVE_ERROR0("Failed to load dataset");
    }
    RAVE_OBJECT_RELEASE(loaded);
  }
  return scanparam->data;
}

static int PolarScanParam_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PolarScanParam_t* this = (PolarScanParam_t*)obj;
  PolarScanParam_t* src = (PolarScanParam_t*)srcobj;
  this->data = RAVE_OBJECT_CLONE(PolarScanParamInternal_ensureData2D(src));
  this->attrs = RAVE_OBJECT_CLONE(src->attrs);
  this->qualityfields = RAVE_OBJECT_CLONE(src->qualityfields);
  this->quantity = NULL;
  this->lazyDataset = NULL;
  if (this->data == NULL || this->attrs == NULL || this->qualityfields == NULL) {
    RAVE_ERROR0("data, attrs or qualityfields NULL");
    goto error;
  }
  if (!PolarScanParam_setQuantity(this, PolarScanParam_getQuantity(src))) {
    RAVE_ERROR0("Failed to duplicate quantity");
    goto error;
  }

  this->gain = src->gain;
  this->offset = src->offset;
  this->nodata = src->nodata;
  this->undetect = src->undetect;
  return 1;
error:
  RAVE_ERROR0("Failed to clone polar scan parameter");
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_FREE(this->quantity);
  return 0;
}

/**
 * Destructor.
 */
static void PolarScanParam_destructor(RaveCoreObject* obj)
{
  PolarScanParam_t* this = (PolarScanParam_t*)obj;
  RAVE_OBJECT_RELEASE(this->data);
  RAVE_OBJECT_RELEASE(this->attrs);
  RAVE_OBJECT_RELEASE(this->qualityfields);
  RAVE_OBJECT_RELEASE(this->lazyDataset);
  RAVE_FREE(this->quantity);
}

/*@} End of Private functions */

/*@{ Interface functions */
int PolarScanParam_setQuantity(PolarScanParam_t* scanparam, const char* quantity)
{
  int result = 0;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (quantity != NULL) {
    char* tmp = RAVE_STRDUP(quantity);
    if (tmp != NULL) {
      RAVE_FREE(scanparam->quantity);
      scanparam->quantity = tmp;
      result = 1;
    }
  } else {
    RAVE_FREE(scanparam->quantity);
    result = 1;
  }
  return result;
}

const char* PolarScanParam_getQuantity(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return (const char*)scanparam->quantity;
}

void PolarScanParam_setGain(PolarScanParam_t* scanparam, double gain)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  scanparam->gain = gain;
}

double PolarScanParam_getGain(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return scanparam->gain;
}

void PolarScanParam_setOffset(PolarScanParam_t* scanparam, double offset)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  scanparam->offset = offset;
}

double PolarScanParam_getOffset(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return scanparam->offset;
}

void PolarScanParam_setNodata(PolarScanParam_t* scanparam, double nodata)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  scanparam->nodata = nodata;
}

double PolarScanParam_getNodata(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return scanparam->nodata;
}

void PolarScanParam_setUndetect(PolarScanParam_t* scanparam, double undetect)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  scanparam->undetect = undetect;
}

double PolarScanParam_getUndetect(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return scanparam->undetect;
}

int PolarScanParam_setData(PolarScanParam_t* scanparam, long nbins, long nrays, void* data, RaveDataType type)
{
  int result = 0;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  result = RaveData2D_setData(scanparam->data, nbins, nrays, data, type);
  if (result) {
    RAVE_OBJECT_RELEASE(scanparam->lazyDataset);
  }
  return result;
}

int PolarScanParam_setLazyDataset(PolarScanParam_t* scanparam, LazyDataset_t* lazyDataset)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (RaveData2D_getData(scanparam->data) == NULL) {
    scanparam->lazyDataset = RAVE_OBJECT_COPY(lazyDataset);
    return 1;
  } else {
    RAVE_ERROR0("Trying to set lazy dataset loader when data exists");
    return 0;
  }
}

int PolarScanParam_setData2D(PolarScanParam_t* scanparam, RaveData2D_t* data2d)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (data2d != NULL) {
    RaveData2D_t* tmp = RAVE_OBJECT_CLONE(data2d);
    if (tmp != NULL) {
      RAVE_OBJECT_RELEASE(scanparam->data);
      scanparam->data = tmp;
      scanparam->nodata = RaveData2D_getNodata(scanparam->data);
      scanparam->gain = 1.0;
      scanparam->offset = 0.0;
      RAVE_OBJECT_RELEASE(scanparam->lazyDataset);
      return 1;
    }
  }
  return 0;
}

int PolarScanParam_createData(PolarScanParam_t* scanparam, long nbins, long nrays, RaveDataType type)
{
  int result = 0;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  result = RaveData2D_createData(scanparam->data, nbins, nrays, type, 0);
  if (result) {
    RAVE_OBJECT_RELEASE(scanparam->lazyDataset);
  }
  return result;
}

void* PolarScanParam_getData(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return RaveData2D_getData(PolarScanParamInternal_ensureData2D(scanparam));
}

RaveData2D_t* PolarScanParam_getData2D(PolarScanParam_t* scanparam)
{
  RaveData2D_t* result = NULL;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  result = RAVE_OBJECT_CLONE(PolarScanParamInternal_ensureData2D(scanparam));
  if (result != NULL) {
    RaveData2D_setNodata(result, scanparam->nodata);
    RaveData2D_useNodata(result, 1);
  }
  return result;
}

long PolarScanParam_getNbins(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (scanparam->lazyDataset != NULL) {
    return LazyDataset_getXsize(scanparam->lazyDataset);
  }
  return RaveData2D_getXsize(scanparam->data);
}

long PolarScanParam_getNrays(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (scanparam->lazyDataset != NULL) {
    return LazyDataset_getYsize(scanparam->lazyDataset);
  }
  return RaveData2D_getYsize(scanparam->data);
}

RaveDataType PolarScanParam_getDataType(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (scanparam->lazyDataset != NULL) {
    return LazyDataset_getDataType(scanparam->lazyDataset);
  }
  return RaveData2D_getType(scanparam->data);
}

RaveValueType PolarScanParam_getValue(PolarScanParam_t* scanparam, int bin, int ray, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  double value = 0.0;

  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");

  value = scanparam->nodata;

  if (RaveData2D_getValue(PolarScanParamInternal_ensureData2D(scanparam), bin, ray, &value)) {
    result = RaveValueType_DATA;
    if (value == scanparam->nodata) {
      result = RaveValueType_NODATA;
    } else if (value == scanparam->undetect) {
      result = RaveValueType_UNDETECT;
    }
  }

  if (v != NULL) {
    *v = value;
  }

  return result;
}

RaveValueType PolarScanParam_getConvertedValue(PolarScanParam_t* scanparam, int bin, int ray, double* v)
{
  RaveValueType result = RaveValueType_NODATA;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (v != NULL) {
    result =  PolarScanParam_getValue(scanparam, bin, ray, v);
    if (result == RaveValueType_DATA) {
      *v = scanparam->offset + (*v) * scanparam->gain;
    }
  }
  return result;
}

int PolarScanParam_setValue(PolarScanParam_t* scanparam, int bin, int ray, double v)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return RaveData2D_setValue(PolarScanParamInternal_ensureData2D(scanparam), bin, ray, v);
}

int PolarScanParam_addAttribute(PolarScanParam_t* scanparam,
  RaveAttribute_t* attribute)
{
  const char* name = NULL;
  char* aname = NULL;
  char* gname = NULL;
  int result = 0;
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);
  if (name != NULL) {
    if (!RaveAttributeHelp_extractGroupAndName(name, &gname, &aname)) {
      RAVE_ERROR1("Failed to extract group and name from %s", name);
      goto done;
    }
    if ((strcasecmp("how", gname)==0 && RaveAttributeHelp_validateHowGroupAttributeName(gname, aname)) ||
        ((strcasecmp("what", gname)==0 || strcasecmp("where", gname)==0) && strchr(aname, '/') == NULL)) {
      result = RaveObjectHashTable_put(scanparam->attrs, name, (RaveCoreObject*)attribute);
    }
  }

done:
  RAVE_FREE(aname);
  RAVE_FREE(gname);
  return result;
}

RaveAttribute_t* PolarScanParam_getAttribute(PolarScanParam_t* scanparam,
  const char* name)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  if (name == NULL) {
    RAVE_ERROR0("Trying to get an attribute with NULL name");
    return NULL;
  }
  return (RaveAttribute_t*)RaveObjectHashTable_get(scanparam->attrs, name);
}

int PolarScanParam_hasAttribute(PolarScanParam_t* scanparam, const char* name)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return RaveObjectHashTable_exists(scanparam->attrs, name);
}

RaveList_t* PolarScanParam_getAttributeNames(PolarScanParam_t* scanparam)
{
  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  return RaveObjectHashTable_keys(scanparam->attrs);
}

RaveObjectList_t* PolarScanParam_getAttributeValues(PolarScanParam_t* scanparam)
{
  RaveObjectList_t* result = NULL;
  RaveObjectList_t* tableattrs = NULL;

  RAVE_ASSERT((scanparam != NULL), "scanparam == NULL");
  tableattrs = RaveObjectHashTable_values(scanparam->attrs);
  if (tableattrs == NULL) {
    goto error;
  }
  result = RAVE_OBJECT_CLONE(tableattrs);
  if (result == NULL) {
    goto error;
  }

  RAVE_OBJECT_RELEASE(tableattrs);
  return result;
error:
  RAVE_OBJECT_RELEASE(result);
  RAVE_OBJECT_RELEASE(tableattrs);
  return NULL;
}

int PolarScanParam_addQualityField(PolarScanParam_t* param, RaveField_t* field)
{
  RAVE_ASSERT((param != NULL), "param == NULL");
  return RaveObjectList_add(param->qualityfields, (RaveCoreObject*)field);
}

RaveField_t* PolarScanParam_getQualityField(PolarScanParam_t* param, int index)
{
  RAVE_ASSERT((param != NULL), "param == NULL");
  return (RaveField_t*)RaveObjectList_get(param->qualityfields, index);
}

int PolarScanParam_getNumberOfQualityFields(PolarScanParam_t* param)
{
  RAVE_ASSERT((param != NULL), "param == NULL");
  return RaveObjectList_size(param->qualityfields);
}

void PolarScanParam_removeQualityField(PolarScanParam_t* param, int index)
{
  RaveField_t* field = NULL;
  RAVE_ASSERT((param != NULL), "param == NULL");
  field = (RaveField_t*)RaveObjectList_remove(param->qualityfields, index);
  RAVE_OBJECT_RELEASE(field);
}

RaveObjectList_t* PolarScanParam_getQualityFields(PolarScanParam_t* param)
{
  RAVE_ASSERT((param != NULL), "param == NULL");
  return (RaveObjectList_t*)RAVE_OBJECT_COPY(param->qualityfields);
}

RaveField_t* PolarScanParam_getQualityFieldByHowTask(PolarScanParam_t* param, const char* value)
{
  int nfields = 0, i = 0;
  RaveField_t* result = NULL;

  RAVE_ASSERT((param != NULL), "param == NULL");
  if (value == NULL) {
    RAVE_WARNING0("Trying to use PolarScanParam_getQualityFieldByHowTask without a how/task value");
    return NULL;
  }
  nfields = RaveObjectList_size(param->qualityfields);
  for (i = 0; result == NULL && i < nfields; i++) {
    RaveField_t* field = (RaveField_t*)RaveObjectList_get(param->qualityfields, i);
    if (field != NULL && RaveField_hasAttributeStringValue(field, "how/task", value)) {
      result = RAVE_OBJECT_COPY(field);
    }
    RAVE_OBJECT_RELEASE(field);
  }
  return result;
}

RaveField_t* PolarScanParam_toField(PolarScanParam_t* param)
{
  RaveField_t* result = NULL;
  RaveField_t* field = NULL;
  RaveData2D_t* datafield = NULL;
  RaveObjectList_t* attrlist = NULL;
  RaveAttribute_t* attr = NULL;
  RaveAttribute_t* cloneattr = NULL;

  int nrattrs = 0, i = 0;

  RAVE_ASSERT((param != NULL), "param == NULL");

  field = RAVE_OBJECT_NEW(&RaveField_TYPE);
  if (field == NULL) {
    goto done;
  }

  datafield = RAVE_OBJECT_CLONE(PolarScanParamInternal_ensureData2D(param));
  if (datafield == NULL) {
    goto done;
  }

  if (!RaveField_setDatafield(field, datafield)) {
    RAVE_ERROR0("Failed to set data field");
    goto done;
  }

  attrlist = RaveObjectHashTable_values(param->attrs);
  if (attrlist == NULL) {
    RAVE_ERROR0("Could not get attribute values");
    goto done;
  }

  nrattrs = RaveObjectList_size(attrlist);
  for (i = 0; i < nrattrs; i++) {
    attr = (RaveAttribute_t*)RaveObjectList_get(attrlist, i);
    if (attr != NULL) {
      cloneattr = RAVE_OBJECT_CLONE(attr);
      if (cloneattr == NULL || !RaveField_addAttribute(field, cloneattr)) {
        RAVE_ERROR0("Could not clone attribute");
        goto done;
      }
    }
    RAVE_OBJECT_RELEASE(attr);
    RAVE_OBJECT_RELEASE(cloneattr);
  }

  // Copy specific parameter attributes
  attr = RaveAttributeHelp_createString("what/quantity", param->quantity);
  if (attr == NULL || !RaveField_addAttribute(field, attr)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(attr);
  attr = RaveAttributeHelp_createDouble("what/gain", param->gain);
  if (attr == NULL || !RaveField_addAttribute(field, attr)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(attr);
  attr = RaveAttributeHelp_createDouble("what/offset", param->offset);
  if (attr == NULL || !RaveField_addAttribute(field, attr)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(attr);
  attr = RaveAttributeHelp_createDouble("what/nodata", param->nodata);
  if (attr == NULL || !RaveField_addAttribute(field, attr)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(attr);
  attr = RaveAttributeHelp_createDouble("what/undetect", param->undetect);
  if (attr == NULL || !RaveField_addAttribute(field, attr)) {
    goto done;
  }
  RAVE_OBJECT_RELEASE(attr);

  result = RAVE_OBJECT_COPY(field);
done:
  RAVE_OBJECT_RELEASE(field);
  RAVE_OBJECT_RELEASE(datafield);
  RAVE_OBJECT_RELEASE(attrlist);
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(cloneattr);
  return result;
}

PolarScanParam_t* PolarScanParam_fromField(RaveField_t* field)
{
  PolarScanParam_t* param = NULL;
  PolarScanParam_t* result = NULL;
  RaveObjectList_t* attributes = NULL;
  RaveData2D_t* datafield = NULL;
  RaveAttribute_t* attr = NULL;
  RaveAttribute_t* cloneattr = NULL;
  double nodata = 255.0;
  double undetect = 0.0;
  double gain = 1.0;
  double offset = 0.0;
  char* quantity = NULL;
  int nrattrs = 0, i = 0;
  if (field == NULL) {
    RAVE_ERROR0("Trying to create a parameter from a NULL field");
    return NULL;
  }
  datafield = RaveField_getDatafield(field);
  attributes = RaveField_getAttributeValues(field);
  param = RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
  if (datafield == NULL || attributes == NULL || param == NULL) {
    goto done;
  }
  nrattrs = RaveObjectList_size(attributes);
  for (i = 0; i < nrattrs; i++) {
    attr = (RaveAttribute_t*)RaveObjectList_get(attributes, i);
    if (attr != NULL) {
      const char* name = RaveAttribute_getName(attr);
      if (strcmp("what/gain", name) == 0) {
        RaveAttribute_getDouble(attr, &gain);
      } else if (strcmp("what/offset", name) == 0) {
        RaveAttribute_getDouble(attr, &offset);
      } else if (strcmp("what/nodata", name) == 0) {
        RaveAttribute_getDouble(attr, &nodata);
      } else if (strcmp("what/undetect", name) == 0) {
        RaveAttribute_getDouble(attr, &undetect);
      } else if (strcmp("what/quantity", name) == 0) {
        RaveAttribute_getString(attr, &quantity);
      } else {
        cloneattr = RAVE_OBJECT_CLONE(attr);
        if (cloneattr == NULL || !PolarScanParam_addAttribute(param, cloneattr)) {
          RAVE_ERROR0("Failed to add attribute to parameter");
          goto done;
        }
        RAVE_OBJECT_RELEASE(cloneattr);
      }
    }
    RAVE_OBJECT_RELEASE(attr);
  }
  if (quantity != NULL) {
    PolarScanParam_setQuantity(param, quantity);
  }
  PolarScanParam_setGain(param, gain);
  PolarScanParam_setOffset(param, offset);
  PolarScanParam_setNodata(param, nodata);
  PolarScanParam_setUndetect(param, undetect);
  RAVE_OBJECT_RELEASE(param->data);
  param->data = RAVE_OBJECT_COPY(datafield);

  result = RAVE_OBJECT_COPY(param);
done:
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(attributes);
  RAVE_OBJECT_RELEASE(datafield);
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(cloneattr);
  return result;
}

int PolarScanParam_convertDataDoubleToUchar(PolarScanParam_t* param) {
  RaveField_t* field=NULL;
  void* data=NULL;
  double iv, ov, gain, offset, nodata, undetect;
  int r, b, nrays, nbins;
  int retval=0;
  RaveDataType rdt;

  rdt = PolarScanParam_getDataType(param);
  if (rdt != RaveDataType_DOUBLE) {
    RAVE_ERROR0("Trying to convert a non-double dataset");
    return retval;
  }

  gain = PolarScanParam_getGain(param);
  offset = PolarScanParam_getOffset(param);
  nodata = PolarScanParam_getNodata(param);
  undetect = PolarScanParam_getUndetect(param);
  nrays = (int)PolarScanParam_getNrays(param);
  nbins = (int)PolarScanParam_getNbins(param);

  field = RAVE_OBJECT_NEW(&RaveField_TYPE);
  RaveField_createData(field, (long)nbins, (long)nrays, RaveDataType_UCHAR);

  for (r = 0; r < nrays; r++) {
    for (b = 0; b < nbins; b++) {
      PolarScanParam_getValue(param, b, r, &iv);

      if ( (iv > -DBL_MAX) && (iv < DBL_MAX) ) {
        ov = (iv - offset) / gain;
        if (ov < undetect)
          ov = undetect;
        else if (ov > nodata)
          ov = nodata;
      } else if (iv == -DBL_MAX) {
        ov = undetect;
      } else {
        ov = nodata;
      }

      retval = RaveField_setValue(field, b, r, ov);
    }
  }
  data = RaveField_getData(field);
  PolarScanParam_setData(param, (long)nbins, (long)nrays, data, RaveDataType_UCHAR);
  RAVE_OBJECT_RELEASE(field);

  return retval;
}

int PolarScanParam_shiftData(PolarScanParam_t* param, int nrays)
{
  int result = 0;
  RAVE_ASSERT((param != NULL), "param == NULL");
  result = RaveData2D_circshiftData(PolarScanParamInternal_ensureData2D(param), 0, nrays);
  if (result) {
    int i = 0;
    int nrqualityfields = PolarScanParam_getNumberOfQualityFields(param);
    for (i = 0; result && i < nrqualityfields; i++) {
      RaveField_t* field = PolarScanParam_getQualityField(param, i);
      result = RaveField_circshiftData(field, 0, nrays);
      RAVE_OBJECT_RELEASE(field);
    }
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType PolarScanParam_TYPE = {
    "PolarScanParam",
    sizeof(PolarScanParam_t),
    PolarScanParam_constructor,
    PolarScanParam_destructor,
    PolarScanParam_copyconstructor
};
