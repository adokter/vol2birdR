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
 * Utilities for working with H5 files
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-09-10
 */
#include "rave_hlhdf_utilities.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_utilities.h"
#include "string.h"
#include "stdarg.h"

/*@{ Constants */

/**
 * Mapping between hlhdf format and rave data type
 */
struct RaveToHlhdfTypeMap {
  HL_FormatSpecifier hlhdfFormat; /**< the hlhdf format */
  RaveDataType raveType;          /**< the rave data type */
};

/**
 * The mapping table
 */
static const struct RaveToHlhdfTypeMap RAVE_TO_HLHDF_MAP[] = {
  {HLHDF_UNDEFINED, RaveDataType_UNDEFINED},
  {HLHDF_CHAR, RaveDataType_CHAR},
  {HLHDF_SCHAR, RaveDataType_CHAR},
  {HLHDF_UCHAR, RaveDataType_UCHAR},
  {HLHDF_SHORT, RaveDataType_SHORT},
  {HLHDF_USHORT, RaveDataType_USHORT},
  {HLHDF_INT, RaveDataType_INT},
  {HLHDF_UINT, RaveDataType_UINT},
  {HLHDF_LONG, RaveDataType_LONG},
  {HLHDF_ULONG, RaveDataType_ULONG},
  {HLHDF_LLONG, RaveDataType_UNDEFINED},
  {HLHDF_ULLONG, RaveDataType_UNDEFINED},
  {HLHDF_FLOAT, RaveDataType_FLOAT},
  {HLHDF_DOUBLE, RaveDataType_DOUBLE},
  {HLHDF_LDOUBLE, RaveDataType_UNDEFINED},
  {HLHDF_HSIZE, RaveDataType_UNDEFINED},
  {HLHDF_HSSIZE, RaveDataType_UNDEFINED},
  {HLHDF_HERR, RaveDataType_UNDEFINED},
  {HLHDF_HBOOL, RaveDataType_UNDEFINED},
  {HLHDF_STRING, RaveDataType_UNDEFINED},
  {HLHDF_COMPOUND, RaveDataType_UNDEFINED},
  {HLHDF_ARRAY, RaveDataType_UNDEFINED},
  {HLHDF_END_OF_SPECIFIERS, RaveDataType_UNDEFINED}
};

/**
 * Type for keeping mapping
 */
struct OdimVersionToStrTypeMap {
  RaveIO_ODIM_Version version; /**< the odim version */
  const char* str;          /**< the odim version str */
};

/** Mapping between version and str */
static const struct OdimVersionToStrTypeMap ODIM_VERSION_STR_MAP[] = {
    {RaveIO_ODIM_Version_UNDEFINED, "UNDEFINED"},
    {RaveIO_ODIM_Version_2_0, RAVE_ODIM_VERSION_2_0_STR},
    {RaveIO_ODIM_Version_2_1, RAVE_ODIM_VERSION_2_1_STR},
    {RaveIO_ODIM_Version_2_2, RAVE_ODIM_VERSION_2_2_STR},
    {RaveIO_ODIM_Version_2_3, RAVE_ODIM_VERSION_2_3_STR},
    {RaveIO_ODIM_Version_2_4, RAVE_ODIM_VERSION_2_4_STR},
    {-2, NULL}
};

/** Mapping between version and str */
static const struct OdimVersionToStrTypeMap ODIM_VERSION_H5RAD_STR_MAP[] = {
    {RaveIO_ODIM_Version_UNDEFINED, "UNDEFINED"},
    {RaveIO_ODIM_Version_2_0, RAVE_ODIM_H5RAD_VERSION_2_0_STR},
    {RaveIO_ODIM_Version_2_1, RAVE_ODIM_H5RAD_VERSION_2_1_STR},
    {RaveIO_ODIM_Version_2_2, RAVE_ODIM_H5RAD_VERSION_2_2_STR},
    {RaveIO_ODIM_Version_2_3, RAVE_ODIM_H5RAD_VERSION_2_3_STR},
    {RaveIO_ODIM_Version_2_4, RAVE_ODIM_H5RAD_VERSION_2_4_STR},
    {-2, NULL}
};

/**
 * Attribute names that has to be translated when jumping from 2.0/2.1 up to 2.2
 */
const char* ATTRIBUTE_NAMES_20_to_22[][2] = {
    {"how/TXloss", "how/TXlossH"},
    {"how/injectloss", "how/injectlossH"},
    {"how/RXloss", "how/RXlossH"},
    {"how/radomeloss", "how/radomelossH"},
    {"how/antgain", "how/antgainH"},
    {"how/beamw", "how/beamwH"},
    {"how/beamwidth", "how/beamwH"},
    {"how/radconst", "how/radconstH"},
    {"how/NEZ", "how/NEZH"},
    {"how/zcal", "how/zcalH"},
    {"how/nsample", "how/nsampleH"},
    {NULL, NULL}
};

/**
 * Quantities that has to be translated when jumping from 2.0/2.1 to 2.2
 */
const char* QUANTITIES_20_to_22[][2] = {
    {"DBZ", "DBZH"},
    {"SQI", "SQIH"},
    {"SNR", "SNRH"},
    {"CCOR", "CCORH"},
    {"VRAD", "VRADH"},
    {"WRAD", "WRADH"},
    {NULL, NULL}
};

/*@} End of Constants */

/*@{ Defines */

/**
 * Quick access function for reading one atomic value from a
 * HLHDF node.
 *
 * @param[in] vt - the type for the read data
 * @param[in] nn - the node
 * @param[in] ss - the size of the data type
 * @param[in] ot - the type of the output value where assignment will be done.
 * @param[in] ov - the output value
 */
#define RAVEHL_GET_ATOMIC_NODEVALUE(vt, nn, ss, ot, ov) \
{ \
  vt v; \
  memcpy(&v, HLNode_getData(nn), ss); \
  ov = (ot)v; \
}

/**
 * Quick access function for reading a simple array from a
 * HLHDF node.
 *
 * @param[in] vt - the type for the read data
 * @param[in] nn - the node
 * @param[in] ss - the size of the data type
 * @param[in] ot - the type of the output value where assignment will be done.
 * @param[in] ov - the output value
 * @param[in] dd - the dimension
 */
#define RAVEHL_GET_ARRAY_NODEVALUE(vt, nn, ss, ot, ov, dd) \
{ \
  int rhlidx; \
  for (rhlidx=0; rhlidx<dd; rhlidx++) { \
    ov[rhlidx] = (ot)((vt*)HLNode_getData(nn))[rhlidx]; \
  } \
}

/*@} End of Defines */

/*@{ Interface functions */

const char* RaveHL_convertAttributeName(const char* name)
{
  int idx = 0;
  const char** ptr = ATTRIBUTE_NAMES_20_to_22[0];
  while (ptr[0] != NULL) {
    if (strcasecmp(ptr[0], name) == 0) {
      return ptr[1];
    }
    ptr = ATTRIBUTE_NAMES_20_to_22[++idx];
  }
  return name;
}

const char* RaveHL_convertQuantity(const char* name)
{
  int idx = 0;
  const char** ptr = QUANTITIES_20_to_22[0];
  while (ptr[0] != NULL) {
    if (strcasecmp(ptr[0], name) == 0) {
      return ptr[1];
    }
    ptr = QUANTITIES_20_to_22[++idx];
  }
  return name;
}

/**
 *
 */
const char* RaveHL_getOdimVersionString(RaveIO_ODIM_Version version)
{
  int idx = 0;
  while (ODIM_VERSION_STR_MAP[idx].str != NULL) {
    if (ODIM_VERSION_STR_MAP[idx].version == version) {
      return ODIM_VERSION_STR_MAP[idx].str;
    }
    idx++;
  }
  return ODIM_VERSION_STR_MAP[0].str; /*UNDEFINED*/
}


/**
 *
 */
const char* RaveHL_getH5RadVersionStringFromOdimVersion(RaveIO_ODIM_Version version)
{
  int idx = 0;
  while (ODIM_VERSION_H5RAD_STR_MAP[idx].str != NULL) {
    if (ODIM_VERSION_H5RAD_STR_MAP[idx].version == version) {
      return ODIM_VERSION_H5RAD_STR_MAP[idx].str;
    }
    idx++;
  }
  return ODIM_VERSION_H5RAD_STR_MAP[0].str; /*UNDEFINED*/
}

/**
 *
 */
RaveIO_ODIM_Version RaveHL_getOdimVersionFromString(const char* str)
{
  int idx = 0;
  while (ODIM_VERSION_STR_MAP[idx].str != NULL) {
    if (strcmp(ODIM_VERSION_STR_MAP[idx].str, str) == 0) {
      return ODIM_VERSION_STR_MAP[idx].version;
    }
  }
  return RaveIO_ODIM_Version_UNDEFINED;
}


/**
 * Creates a rave attribute from a HLHDF node value.
 * Node must contain data that can be translated to long, double or strings otherwise
 * NULL will be returned. Note, the name will not be set on the attribute and has to
 * be set after this function has been called.
 * @param[in] node - the HLHDF node
 * @returns the rave attribute on success, otherwise NULL.
 */
RaveAttribute_t* RaveHL_createAttribute(HL_Node* node)
{
  size_t sz = 0;
  HL_FormatSpecifier format = HLHDF_UNDEFINED;
  RaveAttribute_t* result = NULL;

  RAVE_ASSERT((node != NULL), "node == NULL");

  result = RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  if (result == NULL) {
    goto done;
  }

  format = HLNode_getFormat(node);
  sz = HLNode_getDataSize(node);
  if (format >= HLHDF_SCHAR && format <= HLHDF_ULLONG) {
    if (HLNode_getRank(node) == 0) {
      long value = 0;
      if (sz == sizeof(char)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(char, node, sz, long, value);
      } else if (sz == sizeof(short)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(short, node, sz, long, value);
      } else if (sz == sizeof(int)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(int, node, sz, long, value);
      } else if (sz == sizeof(long)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(long, node, sz, long, value);
      } else if (sz == sizeof(long long)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(long long, node, sz, long, value);
      }
      RaveAttribute_setLong(result, value);
    } else if (HLNode_getRank(node) == 1) {
      hsize_t dim = HLNode_getDimension(node, 0);
      long* value = RAVE_MALLOC(sizeof(long) * dim);
      if (value == NULL) {
        RAVE_ERROR0("Failed to allocate memory");
        RAVE_OBJECT_RELEASE(result);
        goto done;
      }
      if (sz == sizeof(char)) {
        RAVEHL_GET_ARRAY_NODEVALUE(char, node, sz, long, value, dim);
      } else if (sz == sizeof(short)) {
        RAVEHL_GET_ARRAY_NODEVALUE(short, node, sz, long, value, dim);
      } else if (sz == sizeof(int)) {
        RAVEHL_GET_ARRAY_NODEVALUE(int, node, sz, long, value, dim);
      } else if (sz == sizeof(long)) {
        RAVEHL_GET_ARRAY_NODEVALUE(long, node, sz, long, value, dim);
      } else if (sz == sizeof(long long)) {
        RAVEHL_GET_ARRAY_NODEVALUE(long long, node, sz, long, value, dim);
      }
      if (!RaveAttribute_setLongArray(result, value, dim)) {
        RAVE_ERROR1("Failed to set long array for %s", HLNode_getName(node));
        RAVE_OBJECT_RELEASE(result);
      }
      RAVE_FREE(value);
    } else {
      RAVE_WARNING1("Node %s contains a simple array of rank != 1", HLNode_getName(node));
      RAVE_OBJECT_RELEASE(result);
      goto done;
    }
  } else if (format >= HLHDF_FLOAT && format <= HLHDF_LDOUBLE) {
    double value = 0.0;
    if (HLNode_getRank(node) == 0) {
      if (sz == sizeof(float)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(float, node, sz, double, value);
      } else if (sz == sizeof(double)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(double, node, sz, double, value);
      } else if (sz == sizeof(long double)) {
        RAVEHL_GET_ATOMIC_NODEVALUE(long double, node, sz, double, value);
      }
      RaveAttribute_setDouble(result, value);
    } else if (HLNode_getRank(node) == 1) {
      hsize_t dim = HLNode_getDimension(node, 0);
      double* value = RAVE_MALLOC(sizeof(double) * dim);
      if (value == NULL) {
        RAVE_ERROR0("Failed to allocate memory");
        RAVE_OBJECT_RELEASE(result);
        goto done;
      }
      if (sz == sizeof(float)) {
        RAVEHL_GET_ARRAY_NODEVALUE(float, node, sz, double, value, dim);
      } else if (sz == sizeof(double)) {
        RAVEHL_GET_ARRAY_NODEVALUE(double, node, sz, double, value, dim);
      } else if (sz == sizeof(long double)) {
        RAVEHL_GET_ARRAY_NODEVALUE(long double, node, sz, double, value, dim);
      }
      if (!RaveAttribute_setDoubleArray(result, value, dim)) {
        RAVE_ERROR1("Failed to set double array for %s", HLNode_getName(node));
        RAVE_OBJECT_RELEASE(result);
      }
      RAVE_FREE(value);
    } else {
      RAVE_WARNING1("Node %s contains a simple array of rank != 1", HLNode_getName(node));
      RAVE_OBJECT_RELEASE(result);
      goto done;
    }
  } else if (format == HLHDF_STRING) {
    RaveAttribute_setString(result, (char*)HLNode_getData(node));
  } else {
    RAVE_WARNING0("Node does not contain value conformant to rave_attribute");
    RAVE_OBJECT_RELEASE(result);
  }
done:
  return result;
}

RaveAttribute_t* RaveHL_getAttribute(HL_NodeList* nodelist, const char* fmt, ...)
{
  RaveAttribute_t* result = NULL;
  RaveAttribute_t* attr = NULL;

  HL_Node* node = NULL;

  char nodeName[1024];
  va_list ap;
  int n = 0;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR0("Failed to generate name");
    goto done;
  }

  node = HLNodeList_getNodeByName(nodelist, nodeName);
  if (node != NULL) {
    attr = RaveHL_createAttribute(node);
    if (attr != NULL) {
      if (!RaveAttribute_setName(attr, nodeName)) {
        goto done;
      }
    }
  }
  result = RAVE_OBJECT_COPY(attr);

done:
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

int RaveHL_hasNodeByName(HL_NodeList* nodelist, const char* fmt, ...)
{
  va_list ap;
  char nodeName[1024];
  int n = 0;
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n >= 0 && n < 1024) {
    return HLNodeList_hasNodeByName(nodelist, nodeName);
  }
  return 0;
}

int RaveHL_getStringValue(HL_NodeList* nodelist, char** value, const char* fmt, ...)
{
  int result = 0;
  char nodeName[1024];
  va_list ap;
  int n = 0;
  HL_Node* node = NULL;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((value != NULL), "attribute == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  *value = NULL;

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR0("Failed to generate name for data entry");
    goto done;
  }

  node = HLNodeList_getNodeByName(nodelist, nodeName);
  if (node == NULL) {
    RAVE_ERROR1("Could not read %s", nodeName);
    goto done;
  }

  if (HLNode_getFormat(node) != HLHDF_STRING) {
    RAVE_ERROR1("%s is not of type HLHDF_STRING", nodeName);
    goto done;
  }

  *value = (char*)HLNode_getData(node);
  result = 1;

done:
  return result;
}



int RaveHL_createGroup(HL_NodeList* nodelist, const char* fmt, ...)
{
  va_list ap;
  char nodeName[1024];
  int n = 0;
  int result = 0;
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n >= 0 && n < 1024) {
    HL_Node* node = HLNode_newGroup(nodeName);
    if (node == NULL) {
      RAVE_CRITICAL1("Failed to create group with name %s", nodeName);
      goto done;
    }
    if (!HLNodeList_addNode(nodelist, node)) {
      RAVE_CRITICAL1("Failed to add group node with name %s", nodeName);
      HLNode_free(node);
      goto done;
    }
    result = 1;
  }
done:
  if (result == 0) {
    RAVE_CRITICAL0("Failed to add group node");
  }
  return result;
}

int RaveHL_createGroupUnlessExists(HL_NodeList* nodelist, const char* fmt, ...)
{
  va_list ap;
  char nodeName[1024];
  int n = 0;
  int result = 0;
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n >= 0 && n < 1024) {
    if (!HLNodeList_hasNodeByName(nodelist, nodeName)) {
      HL_Node* node = HLNode_newGroup(nodeName);
      if (node == NULL) {
        RAVE_CRITICAL1("Failed to create group with name %s", nodeName);
        goto done;
      }
      if (!HLNodeList_addNode(nodelist, node)) {
        RAVE_CRITICAL1("Failed to add group node with name %s", nodeName);
        HLNode_free(node);
        goto done;
      }
      result = 1;
    } else {
      result = 1;
    }
  }
done:
  if (result == 0) {
    RAVE_CRITICAL0("Failed to add group node");
  }
  return result;
}

int RaveHL_createStringValue(HL_NodeList* nodelist, const char* value, const char* fmt, ...)
{
  va_list ap;
  char nodeName[1024];
  int n = 0;
  int result = 0;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n >= 0 && n < 1024) {
    HL_Node* node = NULL;
    node = HLNode_newAttribute(nodeName);
    if (node == NULL) {
      RAVE_CRITICAL1("Failed to create an attribute with name %s", nodeName);
      goto done;
    }

    if (!HLNode_setScalarValue(node, strlen(value) + 1, (unsigned char*)value, "string", -1)) {
      RAVE_ERROR1("Failed to set string value for %s", nodeName);
      HLNode_free(node);
      goto done;
    }

    if (!HLNodeList_addNode(nodelist, node)) {
      RAVE_ERROR1("Failed to add node %s to nodelist", nodeName);
      HLNode_free(node);
      goto done;
    }

    result = 1;
  }

done:
  if (result == 0) {
    RAVE_ERROR0("Failed to create string attribute node");
  }
  return result;
}

/**
 * Puts an attribute in the nodelist as a hlhdf node.
 * @param[in] nodelist - the node list
 * @param[in] attribute - the attribute, the name of the attribute will be used as attr-member
 * @param[in] fmt - the root name, specified as a varargs
 * @param[in] ... - the varargs list
 * @returns 1 on success otherwise 0
 */
int RaveHL_addAttribute(HL_NodeList* nodelist, RaveAttribute_t* attribute, const char* fmt, ...)
{
  const char* attrname = NULL;
  int result = 0;
  char nodeName[1024];
  va_list ap;
  int n = 0;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR0("Failed to generate name for data entry");
    goto done;
  }

  attrname = RaveAttribute_getName(attribute);
  if (attrname != NULL) {
    char attrNodeName[2048];
    sprintf(attrNodeName, "%s/%s", nodeName, attrname);
    if (!HLNodeList_hasNodeByName(nodelist, attrNodeName)) {
      HL_Node* node = HLNode_newAttribute(attrNodeName);
      if (node == NULL) {
        RAVE_CRITICAL1("Failed to create an attribute with name %s", attrNodeName);
        goto done;
      }
      if (RaveAttribute_getFormat(attribute) == RaveAttribute_Format_Long) {
        long value;
        RaveAttribute_getLong(attribute, &value);
        result = HLNode_setScalarValue(node, sizeof(long), (unsigned char*)&value, "long", -1);
      } else if (RaveAttribute_getFormat(attribute) == RaveAttribute_Format_Double) {
        double value;
        RaveAttribute_getDouble(attribute, &value);
        result = HLNode_setScalarValue(node, sizeof(double), (unsigned char*)&value, "double", -1);
      } else if (RaveAttribute_getFormat(attribute) == RaveAttribute_Format_String) {
        char* value = NULL;
        RaveAttribute_getString(attribute, &value);
        if (value != NULL) {
          result = HLNode_setScalarValue(node, strlen(value)+1, (unsigned char*)value, "string", -1);
        } else {
          RAVE_WARNING1("Attribute %s is NULL and will be ignored", attrNodeName);
          HLNode_free(node);
          node = NULL;
          result = 1;
        }
      } else if (RaveAttribute_getFormat(attribute) == RaveAttribute_Format_LongArray) {
        long* value = NULL;
        int len = 0;
        hsize_t dims = 0;
        RaveAttribute_getLongArray(attribute, &value, &len);
        dims = len;
        if (value != NULL && len != 0) {
          result = HLNode_setArrayValue(node, sizeof(long), 1, &dims, (unsigned char*)value, "long", -1);
        } else {
          RAVE_WARNING1("Attribute %s is NULL and will be ignored", attrNodeName);
          HLNode_free(node);
          node = NULL;
          result = 1;
        }
      } else if (RaveAttribute_getFormat(attribute) == RaveAttribute_Format_DoubleArray) {
        double* value = NULL;
        int len = 0;
        hsize_t dims = 0;
        RaveAttribute_getDoubleArray(attribute, &value, &len);
        dims = len;
        if (value != NULL && len != 0) {
          result = HLNode_setArrayValue(node, sizeof(double), 1, &dims, (unsigned char*)value, "double", -1);
        } else {
          RAVE_WARNING1("Attribute %s is NULL and will be ignored", attrNodeName);
          HLNode_free(node);
          node = NULL;
          result = 1;
        }
      }
      if (result == 1 && node != NULL) {
        result = HLNodeList_addNode(nodelist, node);
        if (result == 0) {
          HLNode_free(node);
          node = NULL;
          RAVE_ERROR1("Could not add node %s", attrNodeName);
        }
      }
    } else {
      /* If attribute already has been added, we just count this as successful */
      result = 1;
    }
  }

done:
  return result;
}

RaveList_t* RaveHL_extractSubGroups(const char* attrname)
{
  char* dup = RAVE_STRDUP(attrname);
  char* r = dup; /* To remember what to delete */
  RaveList_t* result = RAVE_OBJECT_NEW(&RaveList_TYPE);
  if (dup != NULL) {
    char currentGroupName[1024];
    memset(currentGroupName, 0, 1024);
    char* tmp = strstr(dup, "/");
    while (tmp != NULL && *tmp != '\0') {
      *tmp = '\0';
      tmp++;
      if (strlen(currentGroupName) == 0) {
        strcpy(currentGroupName, dup);
      } else {
        strcat(currentGroupName, "/");
        strcat(currentGroupName, dup);
      }
      RAVE_INFO1("Adding group: %s\n", currentGroupName);
      RaveList_add(result, (char*)RAVE_STRDUP(currentGroupName));
      dup = tmp;
      tmp = strstr(dup, "/");
    }
  }
  RAVE_FREE(r);
  return result;
}

int RaveHL_addAttributes(HL_NodeList* nodelist, RaveObjectList_t* attributes, const char* name)
{
  RaveAttribute_t* attribute = NULL;
  int result = 0;
  int nattrs = 0;
  int i = 0;
  int hashow = 0, haswhat = 0, haswhere = 0;
  RaveList_t* howSubgroups = NULL;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((attributes != NULL), "attributes == NULL");

  hashow = RaveHL_hasNodeByName(nodelist, "%s/how", name);
  haswhat = RaveHL_hasNodeByName(nodelist, "%s/what", name);
  haswhere = RaveHL_hasNodeByName(nodelist, "%s/where", name);

  nattrs = RaveObjectList_size(attributes);
  for (i = 0; i < nattrs; i++) {
    const char* attrname = NULL;
    RAVE_OBJECT_RELEASE(attribute);
    attribute = (RaveAttribute_t*)RaveObjectList_get(attributes, i);
    if (attribute == NULL) {
      RAVE_WARNING1("Failed to get attribute at index %d", i);
      goto done;
    }
    attrname = RaveAttribute_getName(attribute);
    if (attrname == NULL) {
      RAVE_ERROR1("Attribute at %d has no name set", i);
      goto done;
    }
    if (haswhat==0 && strncasecmp(attrname, "what/", 5)==0) {
      haswhat = RaveHL_createGroup(nodelist, "%s/what",name);
      if (haswhat == 0) {
        RAVE_ERROR1("Failed to create group %s/what", name);
        goto done;
      }
    } else if (haswhere==0 && strncasecmp(attrname, "where/", 6)==0) {
      haswhere = RaveHL_createGroup(nodelist, "%s/where",name);
      if (haswhere == 0) {
        RAVE_ERROR1("Failed to create group %s/where", name);
        goto done;
      }
    } else if (hashow==0 && strncasecmp(attrname, "how/", 4)==0) {
      hashow = RaveHL_createGroup(nodelist, "%s/how",name);
      if (hashow == 0) {
        RAVE_ERROR1("Failed to create group %s/how", name);
        goto done;
      }
    } else {
      if (strncasecmp(attrname, "how/", 4) != 0 &&
          strncasecmp(attrname, "what/", 5) != 0 &&
          strncasecmp(attrname, "where/", 6) != 0) {
        RAVE_ERROR1("Unsupported attribute name %s", attrname);
        goto done;
      }
    }

    /** As of ODIM 2.3 we should support subgroups in how */
    if (strncasecmp(attrname, "how/", 4) == 0) {
      howSubgroups = RaveHL_extractSubGroups(attrname);
      if (howSubgroups != NULL && RaveList_size(howSubgroups) > 1) {
        int hsz = RaveList_size(howSubgroups);
        int hsi = 0;
        for (hsi = 0; hsi < hsz; hsi++) {
          char* grp = (char*)RaveList_get(howSubgroups, hsi);
          if (!RaveHL_createGroupUnlessExists(nodelist, "%s/%s",name, grp)) {
            RAVE_ERROR2("Failed to create group %s/%s", name, grp);
            goto done;
          }
        }
      }
      if (howSubgroups != NULL) {
        RaveList_freeAndDestroy(&howSubgroups);
      }
    }

    if (!RaveHL_addAttribute(nodelist, attribute, name)) {
      RAVE_ERROR2("Failed to add attribute %s/%s to nodelist", name, attrname);
      goto done;
    }
  }
  result = 1;
done:
  if (howSubgroups != NULL) {
    RaveList_freeAndDestroy(&howSubgroups);
  }
  RAVE_OBJECT_RELEASE(attribute);
  return result;
}

int RaveHL_createDataset(HL_NodeList* nodelist, void* data, long xsize, long ysize, RaveDataType dataType, const char* fmt, ...)
{
  va_list ap;
  char nodeName[1024];
  int n = 0;
  int result = 0;
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n >= 0 && n < 1024) {
    HL_Node* node = HLNode_newDataset(nodeName);
    HL_FormatSpecifier specifier = RaveHL_raveToHlhdfType(dataType);
    const char* hlhdfFormat = HL_getFormatSpecifierString(specifier);
    hsize_t dims[2];
    dims[0] = ysize;
    dims[1] = xsize;
    if (node == NULL) {
      RAVE_CRITICAL1("Failed to create dataset with name %s", nodeName);
      goto done;
    }

    if (!HLNode_setArrayValue(node,(size_t)get_ravetype_size(dataType),2,dims,data,hlhdfFormat,-1)) {
      HLNode_free(node);
      goto done;
    }

    if (!HLNodeList_addNode(nodelist, node)) {
      RAVE_CRITICAL1("Failed to add dataset node with name %s", nodeName);
      HLNode_free(node);
      goto done;
    }

    result = 1;
  }
done:
  if (result == 0) {
    RAVE_CRITICAL0("Failed to add dataset node");
  }
  return result;
}

int RaveHL_addData(
  HL_NodeList* nodelist,
  void* data,
  long xsize,
  long ysize,
  RaveDataType dataType,
  const char* fmt,
  ...)
{
  int result = 0;
  char nodeName[1024];
  va_list ap;
  int n = 0;

  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  va_start(ap, fmt);
  n = vsnprintf(nodeName, 1024, fmt, ap);
  va_end(ap);
  if (n < 0 || n >= 1024) {
    RAVE_ERROR0("Failed to generate name for data entry");
    goto done;
  }

  if (data == NULL) {
    goto done;
  }

  if (!RaveHL_createDataset(nodelist, data, xsize, ysize, dataType, "%s/data", nodeName)) {
    RAVE_CRITICAL1("Failed to create dataset with name %s/data", nodeName);
    goto done;
  }

  result = 1; // Set result to 1 now, if hdfview specific fails, result will be set back to 0.

  if (dataType == RaveDataType_UCHAR) {
    RaveAttribute_t* imgAttribute = RaveAttributeHelp_createString("CLASS", "IMAGE");
    RaveAttribute_t* verAttribute = RaveAttributeHelp_createString("IMAGE_VERSION", "1.2");
    if (imgAttribute == NULL || verAttribute == NULL) {
      result = 0;
    }
    if (result == 1) {
      result = RaveHL_addAttribute(nodelist, imgAttribute, "%s/data", nodeName);
    }
    if (result == 1) {
      result = RaveHL_addAttribute(nodelist, verAttribute, "%s/data", nodeName);
    }
    RAVE_OBJECT_RELEASE(imgAttribute);
    RAVE_OBJECT_RELEASE(verAttribute);
  }

done:
  return result;
}

HL_FormatSpecifier RaveHL_raveToHlhdfType(RaveDataType format)
{
  int index = 0;
  HL_FormatSpecifier result = HLHDF_UNDEFINED;
  while (RAVE_TO_HLHDF_MAP[index].hlhdfFormat != HLHDF_END_OF_SPECIFIERS) {
    if (RAVE_TO_HLHDF_MAP[index].raveType == format) {
      result = RAVE_TO_HLHDF_MAP[index].hlhdfFormat;
      break;
    }
    index++;
  }
  return result;
}

RaveDataType RaveHL_hlhdfToRaveType(HL_FormatSpecifier format)
{
  int index = 0;
  RaveDataType result = RaveDataType_UNDEFINED;
  while (RAVE_TO_HLHDF_MAP[index].hlhdfFormat != HLHDF_END_OF_SPECIFIERS) {
    if (RAVE_TO_HLHDF_MAP[index].hlhdfFormat == format) {
      result = RAVE_TO_HLHDF_MAP[index].raveType;
      break;
    }
    index++;
  }
  return result;
}

/**
 * Loads the attributes from the name into the RaveCoreObject. I.e.
 * name/how/..., name/where/... and name/what/...
 * @param[in] nodelist - the hlhdf list
 * @param[in] object - the object to fill
 * @param[in] fmt - the varargs name of the object
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
int RaveHL_loadAttributesAndData(HL_NodeList* nodelist, void* object, RaveHL_attr_f attrf, RaveHL_data_f dataf, const char* fmt, ...)
{
  int result = 1;
  int n = 0;
  int i = 0;
  int nameLength = 0;

  va_list ap;
  char name[1024];
  int nName = 0;
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((object != NULL), "object == NULL");

  va_start(ap, fmt);
  nName = vsnprintf(name, 1024, fmt, ap);
  va_end(ap);
  if (nName < 0 || nName >= 1024) {
    RAVE_ERROR0("NodeName would evaluate to more than 1024 characters.");
    result = 0;
  } else {
    nameLength = strlen(name);
  }

  n = HLNodeList_getNumberOfNodes(nodelist);
  for (i = 0; result == 1 && i < n; i++) {
    HL_Node* node = HLNodeList_getNodeByIndex(nodelist, i);
    const char* nodeName = HLNode_getName(node);
    int nNameLength = strlen(nodeName);
    if (nNameLength>nameLength && strncasecmp(nodeName, name, nameLength)==0) {
      if (nodeName[nameLength]=='/') {
        char* tmpptr = (char*)nodeName+(nameLength + 1);
        if (HLNode_getType(node) == ATTRIBUTE_ID &&
            (strncasecmp(tmpptr, "how/", 4)==0 ||
             strncasecmp(tmpptr, "what/", 5)==0 ||
             strncasecmp(tmpptr, "where/", 6)==0)) {
          RaveAttribute_t* attribute = RaveHL_createAttribute(node);
          if (attribute != NULL) {
            result = RaveAttribute_setName(attribute, RaveHL_convertAttributeName(tmpptr));
            if (result == 1 && attrf != NULL) {
              result = attrf(object, attribute);
            }
          }
          if (result && (strcasecmp(tmpptr, "how/beamwidth") == 0 || strcasecmp(tmpptr, "how/beamw") == 0)) { /* We convert this into how/beamwH but also need std */
            RaveAttribute_t* beamwidthAttribute = RaveHL_createAttribute(node);
            if (beamwidthAttribute != NULL) {
              result = RaveAttribute_setName(beamwidthAttribute, "how/beamwidth");
              if (result == 1 && attrf != NULL) {
                result = attrf(object, beamwidthAttribute);
              }
              RAVE_OBJECT_RELEASE(beamwidthAttribute);
            }
          }
          RAVE_OBJECT_RELEASE(attribute);
        } else if (HLNode_getType(node) == DATASET_ID &&
            strcasecmp(tmpptr, "data")==0) { /* Hopefully they have at least selected all metadata before entering here. */
          hsize_t d0 = HLNode_getDimension(node, 0);
          hsize_t d1 = HLNode_getDimension(node, 1);
          RaveDataType dataType = RaveHL_hlhdfToRaveType(HLNode_getFormat(node));
          if (dataType != RaveDataType_UNDEFINED && dataf != NULL) {
            result = dataf(object, d1, d0, HLNode_getData(node), dataType, nodeName);
          } else {
            RAVE_ERROR0("Undefined datatype for dataset");
            result = 0;
          }
        }
      }
    }
  }

  return result;
}

/*@} End of Interface functions */
