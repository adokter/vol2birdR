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
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-10-15
 */
#include "rave_attribute.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include "rave_object.h"
#include "rave_datetime.h"
#include "rave_transform.h"
#include "raveobject_hashtable.h"
#include "rave_data2d.h"
#include <errno.h>

/**
 * This is the default parameter value that should be used when working
 * with scans.
 */
#define DEFAULT_PARAMETER_NAME "DBZH"

/**
 * Represents one scan in a volume.
 */
struct _RaveAttribute_t {
  RAVE_OBJECT_HEAD /** Always on top */
  char* name;    /**< the source string */
  RaveAttribute_Format format;  /**< the attribute format */
  char* sdata;    /**< the string value */
  long ldata;       /**< the long value */
  double ddata;     /**< the double value */
  long* ldataarray; /**< the long array */
  double* ddataarray; /**< the double array */
  int arraylen;     /**< length of arrays */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int RaveAttribute_constructor(RaveCoreObject* obj)
{
  RaveAttribute_t* attr = (RaveAttribute_t*)obj;
  attr->name = NULL;
  attr->format = RaveAttribute_Format_Undefined;
  attr->sdata = NULL;
  attr->ldata = 0;
  attr->ddata = 0.0;
  attr->ldataarray = NULL;
  attr->ddataarray = NULL;
  attr->arraylen = 0;
  return 1;
}

static int RaveAttribute_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveAttribute_t* this = (RaveAttribute_t*)obj;
  RaveAttribute_t* src = (RaveAttribute_t*)srcobj;
  this->name = NULL;
  this->sdata = NULL;
  this->ldata = 0;
  this->ddata = 0.0;
  this->format = RaveAttribute_Format_Undefined;
  this->ldataarray = NULL;
  this->ddataarray = NULL;
  this->arraylen = 0;

  if (!RaveAttribute_setName(this, RaveAttribute_getName(src))) {
    goto error;
  }
  this->format = src->format;
  if (this->format == RaveAttribute_Format_Long) {
    this->ldata = src->ldata;
  } else if (this->format == RaveAttribute_Format_Double) {
    this->ddata = src->ddata;
  } else if (this->format == RaveAttribute_Format_String) {
    if (!RaveAttribute_setString(this, src->sdata)) {
      goto error;
    }
  } else if (this->format == RaveAttribute_Format_LongArray) {
    if (!RaveAttribute_setLongArray(this, src->ldataarray, src->arraylen)) {
      goto error;
    }
  } else if (this->format == RaveAttribute_Format_DoubleArray) {
    if (!RaveAttribute_setDoubleArray(this, src->ddataarray, src->arraylen)) {
      goto error;
    }
  }

  return 1;
error:
  RAVE_FREE(this->name);
  RAVE_FREE(this->sdata);
  return 0;
}

/**
 * Destructor.
 */
static void RaveAttribute_destructor(RaveCoreObject* obj)
{
  RaveAttribute_t* attr = (RaveAttribute_t*)obj;
  RAVE_FREE(attr->name);
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);
}

/**
 * Returns the suggested data type for an array from the rave data type.
 * E.g. char, uchar, short, etc will result in long and float, double will
 * result in double.
 * @param[in] type - the data type
 * @returns the suggested data type or undefined if not possible
 */
static RaveDataType RaveAttributeInternal_getArrayType(RaveDataType type)
{
  RaveDataType result = RaveDataType_UNDEFINED;
  switch(type) {
  case RaveDataType_CHAR:
  case RaveDataType_UCHAR:
  case RaveDataType_SHORT:
  case RaveDataType_INT:
  case RaveDataType_LONG:
    result = RaveDataType_LONG;
    break;
  case RaveDataType_FLOAT:
  case RaveDataType_DOUBLE:
    result = RaveDataType_DOUBLE;
    break;
  default:
    break;
  }
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
int RaveAttribute_setName(RaveAttribute_t* attr, const char* name)
{
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_FREE(attr->name);
  if (name != NULL) {
    attr->name = RAVE_STRDUP(name);
    if (attr->name == NULL) {
      RAVE_CRITICAL0("Failure when copying name");
      return 0;
    }
  }
  return 1;
}

const char* RaveAttribute_getName(RaveAttribute_t* attr)
{
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  return (const char*)attr->name;
}

RaveAttribute_Format RaveAttribute_getFormat(RaveAttribute_t* attr)
{
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  return attr->format;
}

void RaveAttribute_setLong(RaveAttribute_t* attr, long value)
{
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);
  attr->ldata = value;
  attr->format = RaveAttribute_Format_Long;
}

void RaveAttribute_setDouble(RaveAttribute_t* attr, double value)
{
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);

  attr->ddata = value;
  attr->format = RaveAttribute_Format_Double;
}

int RaveAttribute_setString(RaveAttribute_t* attr, const char* value)
{
  char* tdata = NULL;
  RAVE_ASSERT((attr != NULL), "attr == NULL");

  if (value != NULL) {
    tdata = RAVE_STRDUP(value);
    if (tdata == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for string");
      goto error;
    }
  }
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);
  if (tdata != NULL) {
    attr->sdata = tdata;
  }
  attr->format = RaveAttribute_Format_String;
  return 1;
error:
  return 0;
}

int RaveAttribute_setLongArray(RaveAttribute_t* attr, long* value, int len)
{
  long* ldata = NULL;
  RAVE_ASSERT((attr != NULL), "attr == NULL");

  if (value != NULL) {
    ldata = RAVE_MALLOC(sizeof(long) * len);
    if (ldata == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for long array");
      goto error;
    }
    memcpy(ldata, value, sizeof(long) * len);
  } else {
    attr->arraylen = 0;
  }
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);
  if (ldata != NULL) {
    attr->ldataarray = ldata;
  }
  attr->arraylen = len;
  attr->format = RaveAttribute_Format_LongArray;
  return 1;
error:
  return 0;
}

int RaveAttribute_setDoubleArray(RaveAttribute_t* attr, double* value, int len)
{
  double* ddata = NULL;
  RAVE_ASSERT((attr != NULL), "attr == NULL");

  if (value != NULL) {
    ddata = RAVE_MALLOC(sizeof(double) * len);
    if (ddata == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for double array");
      goto error;
    }
    memcpy(ddata, value, sizeof(double) * len);
  } else {
    attr->arraylen = 0;
  }
  RAVE_FREE(attr->sdata);
  RAVE_FREE(attr->ldataarray);
  RAVE_FREE(attr->ddataarray);
  if (ddata != NULL) {
    attr->ddataarray = ddata;
  }
  attr->arraylen = len;
  attr->format = RaveAttribute_Format_DoubleArray;
  return 1;
error:
  return 0;
}

int RaveAttribute_setArrayFromData(RaveAttribute_t* attr, void* value, int len, RaveDataType type)
{
  RaveData2D_t* data = NULL;
  RaveDataType dtype = RaveDataType_UNDEFINED;
  int result = 0;

  RAVE_ASSERT((attr != NULL), "attr == NULL");

  dtype = RaveAttributeInternal_getArrayType(type);

  if (dtype != RaveDataType_UNDEFINED) {
    data = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
    if (data != NULL && RaveData2D_setData(data, len, 1, value, type)) {
      if (dtype == RaveDataType_LONG) {
        long* ndata = RAVE_MALLOC(sizeof(long) * len);
        if (ndata != NULL) {
          int i = 0;
          for (i = 0; i < len; i++) {
            double v;
            RaveData2D_getValue(data, i, 0, &v);
            ndata[i] = (long)v;
          }
          result = RaveAttribute_setLongArray(attr, ndata, len);
        }
        RAVE_FREE(ndata);
      } else if (dtype == RaveDataType_DOUBLE) {
        double* ndata = RAVE_MALLOC(sizeof(double) * len);
        if (ndata != NULL) {
          int i = 0;
          for (i = 0; i < len; i++) {
            double v;
            RaveData2D_getValue(data, i, 0, &v);
            ndata[i] = v;
          }
          result = RaveAttribute_setDoubleArray(attr, ndata, len);
        }
        RAVE_FREE(ndata);
      }
    } else {
      RAVE_ERROR0("Memory error");
    }
  } else {
    RAVE_ERROR0("Unsupported data type for array");
  }
  RAVE_OBJECT_RELEASE(data);
  return result;

}

int RaveAttribute_getLong(RaveAttribute_t* attr, long* value)
{
  int result = 0;
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  if (attr->format == RaveAttribute_Format_Long) {
    *value = attr->ldata;
    result = 1;
  } else if (attr->format == RaveAttribute_Format_LongArray && attr->arraylen == 1) {
    *value = attr->ldataarray[0];
    result = 1;
  }
  return result;
}

int RaveAttribute_getDouble(RaveAttribute_t* attr, double* value)
{
  int result = 0;
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  if (attr->format == RaveAttribute_Format_Double) {
    *value = attr->ddata;
    result = 1;
  } else if (attr->format == RaveAttribute_Format_DoubleArray && attr->arraylen == 1) {
    *value = attr->ddataarray[0];
    result = 1;
  }
  return result;
}

int RaveAttribute_getString(RaveAttribute_t* attr, char** value)
{
  int result = 0;
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  if (attr->format == RaveAttribute_Format_String) {
    *value = attr->sdata;
    result = 1;
  }
  return result;
}

int RaveAttribute_getLongArray(RaveAttribute_t* attr, long** value, int* len)
{
  int result = 0;
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  RAVE_ASSERT((len != NULL), "len == NULL");

  if (attr->format == RaveAttribute_Format_LongArray) {
    *value = attr->ldataarray;
    *len = attr->arraylen;
    result = 1;
  }
  return result;
}

int RaveAttribute_getDoubleArray(RaveAttribute_t* attr, double** value, int* len)
{
  int result = 0;
  RAVE_ASSERT((attr != NULL), "attr == NULL");
  RAVE_ASSERT((value != NULL), "value == NULL");
  RAVE_ASSERT((len != NULL), "len == NULL");

  if (attr->format == RaveAttribute_Format_DoubleArray) {
    *value = attr->ddataarray;
    *len = attr->arraylen;
    result = 1;
  }
  return result;
}

int RaveAttribute_shiftArray(RaveAttribute_t* attr, int nx)
{
  int result = 0;
  int i = 0;
  if (attr->format == RaveAttribute_Format_DoubleArray) {
    double* tmp = RAVE_MALLOC(sizeof(double)*attr->arraylen);
    if (tmp != NULL) {
      for (i = 0; i < attr->arraylen; i++) {
        int shiftindex = i + nx;
        if (shiftindex < 0) {
          shiftindex += attr->arraylen;
        }
        if (shiftindex >= attr->arraylen) {
          shiftindex -= attr->arraylen;
        }
        tmp[shiftindex] = attr->ddataarray[i];
      }
      memcpy(attr->ddataarray, tmp, sizeof(double)*attr->arraylen);
      RAVE_FREE(tmp);
      result = 1;
    } else {
      RAVE_ERROR0("Failed to allocate memory during array shift");
    }
  } else if (attr->format == RaveAttribute_Format_LongArray) {
    long* tmp = RAVE_MALLOC(sizeof(long)*attr->arraylen);
    if (tmp != NULL) {
      for (i = 0; i < attr->arraylen; i++) {
        int shiftindex = i + nx;
        if (shiftindex < 0) {
          shiftindex += attr->arraylen;
        }
        if (shiftindex >= attr->arraylen) {
          shiftindex -= attr->arraylen;
        }
        tmp[shiftindex] = attr->ldataarray[i];
      }
      memcpy(attr->ldataarray, tmp, sizeof(long)*attr->arraylen);
      RAVE_FREE(tmp);
      result = 1;
    } else {
      RAVE_ERROR0("Failed to allocate memory during array shift");
    }
  }
  return result;
}

int RaveAttributeHelp_extractGroupAndName(
  const char* attrname, char** group, char** name)
{
  char *n1 = NULL, *n2 = NULL, *p = NULL;
  int slen = 0, n1len = 0, n2len = 0;
  int result = 0;
  RAVE_ASSERT((attrname != NULL), "attrname == NULL");

  slen = strlen(attrname);
  p = strchr(attrname, '/');
  if (p != NULL) {
    n1len = (p - attrname);
    n2len = (slen - (n1len+1));
    if (n1len <= 0 || n2len <= 0) {
      RAVE_ERROR0("attrname is not in format <group>/<name> where group is how,what or where");
      goto done;
    }
    n1 = RAVE_MALLOC((n1len + 1)*sizeof(char));
    n2 = RAVE_MALLOC((n2len + 1)*sizeof(char));
    if (n1 == NULL || n2 == NULL) {
      RAVE_CRITICAL0("Failed to allocate memory for n1 or n2");
      goto done;
    }
    strncpy(n1, attrname, n1len);
    n1[n1len] = '\0';
    strncpy(n2, attrname+(n1len+1), n2len);
    n2[n2len] = '\0';
    *group = n1;
    *name = n2;
    n1 = NULL; /* release responsibility for memory */
    n2 = NULL; /* release responsibility for memory */
  } else {
    RAVE_ERROR0("attrname is not in format <group>/<name> where group is how,what or where\n");
    goto done;
  }

  result = 1;
done:
  RAVE_FREE(n1);
  RAVE_FREE(n2);
  return result;
}

int RaveAttributeHelp_validateHowGroupAttributeName(const char* gname, const char* aname)
{
  int result = 0;
  char* tmpstr = NULL;
  const char* acceptedCharacters="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_/.-";
  if (gname != NULL && strcasecmp("how", gname) != 0) {
    /* Group is not how, this is not valid */
    goto done;
  }

  if (aname != NULL) {
    if (strlen(aname) > 0 && aname[strlen(aname)-1] == '/') {
      RAVE_INFO1("how attribute %s ends with /", aname);
      goto done;
    }
    result = (strspn(aname, acceptedCharacters) == strlen(aname)) ? 1 : 0;
  }

  result = 1;
done:
  RAVE_FREE(tmpstr);
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createNamedAttribute(const char* name)
{
  RaveAttribute_t* result = NULL;
  if (name != NULL) {
    result = RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
    if (result != NULL) {
      if (!RaveAttribute_setName(result, name)) {
        RAVE_OBJECT_RELEASE(result);
      }
    }
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createLong(const char* name, long value)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    RaveAttribute_setLong(result, value);
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createDouble(const char* name, double value)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    RaveAttribute_setDouble(result, value);
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createDoubleFromString(const char* name, const char* value)
{
  double d = 0.0;
  RaveAttribute_t* result = NULL;
  d = strtod(value, NULL);
  if (d == 0.0 && errno == ERANGE) {
    RAVE_WARNING1("Value %s could not be parsed into double\n", value);
    return NULL;
  }
  result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    RaveAttribute_setDouble(result, d);
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createString(const char* name, const char* value)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    if (!RaveAttribute_setString(result, value)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createLongArray(const char* name, long* value, int len)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    if (!RaveAttribute_setLongArray(result, value, len)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createDoubleArray(const char* name, double* value, int len)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    if (!RaveAttribute_setDoubleArray(result, value, len)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

RaveAttribute_t* RaveAttributeHelp_createArrayFromData(const char* name, void* value, int len, RaveDataType type)
{
  RaveAttribute_t* result = RaveAttributeHelp_createNamedAttribute(name);
  if (result != NULL) {
    if (!RaveAttribute_setArrayFromData(result, value, len, type)) {
      RAVE_OBJECT_RELEASE(result);
    }
  }
  return result;
}

/*@} End of Interface functions */

RaveCoreObjectType RaveAttribute_TYPE = {
  "RaveAttribute",
  sizeof(RaveAttribute_t),
  RaveAttribute_constructor,
  RaveAttribute_destructor,
  RaveAttribute_copyconstructor
};
