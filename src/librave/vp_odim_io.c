/* --------------------------------------------------------------------
Copyright (C) 2012 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Polar ODIM IO functions
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-08-30
 *
 * @co-author Ulf E. Nordh, SMHI
 * @date 2017-02-23, started overhaul of the code to achieve better
 * resemblance with N2 and requirements from customer E-profile. Added code under
 * the function VpOdimIOInternal_loadDsAttribute, the code makes it possible to
 * read/write the attributes under /dataset1/what
 */
#include "vp_odim_io.h"
#include "rave_hlhdf_utilities.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include "odim_io_utilities.h"
#include <math.h>

/**
 * The Vertical Profile ODIM IO adaptor
 */
struct _VpOdimIO_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveIO_ODIM_Version version;
  int strict; /**< If strict compliance should be enforced for some attributes */
  char error_message[1024];                /**< if an error occurs during writing an error message might give you the reason */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int VpOdimIO_constructor(RaveCoreObject* obj)
{
  ((VpOdimIO_t*)obj)->version = RaveIO_ODIM_Version_2_4;
  ((VpOdimIO_t*)obj)->strict = 0;
  strcpy(((VpOdimIO_t*)obj)->error_message, "");
  return 1;
}

/**
 * Copy constructor
 */
static int VpOdimIO_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  ((VpOdimIO_t*)obj)->version = ((VpOdimIO_t*)srcobj)->version;
  ((VpOdimIO_t*)obj)->strict = ((VpOdimIO_t*)srcobj)->strict;
  strcpy(((VpOdimIO_t*)obj)->error_message, ((VpOdimIO_t*)srcobj)->error_message);
  return 1;
}

/**
 * Destroys the object
 * @param[in] obj - the instance
 */
static void VpOdimIO_destructor(RaveCoreObject* obj)
{
}

/**
 * Locates an attribute with specified name in the list of attributes.
 * @param[in] attributes - list of attributes
 * @param[in] name - name of attributes searched for
 * @return attribute if found, otherwise NULL
 */
static RaveAttribute_t* VpOdimIOInternal_getAttribute(RaveObjectList_t* attributes, const char* name)
{
  int nlen = RaveObjectList_size(attributes);
  int i = 0;
  for (i = 0; i <nlen; i++) {
    RaveAttribute_t* attr = (RaveAttribute_t*)RaveObjectList_get(attributes, i);
    const char* attrname = RaveAttribute_getName(attr);
    if (strcmp(attrname, name) == 0) {
      return attr;
    }
    RAVE_OBJECT_RELEASE(attr);
  }
  return NULL;
}

/**
 * Uppdates gain & offset for fields that are affected. Currently, HGHT and MESH.
 * @param[in] self - self
 * @param[in] attributes - list of attributes affected
 * @param[in] fromInternal - If 0, then translation if to internal format. If 1, then from internal format.
 */
static int VpOdimIOInternal_updateGainOffset(VpOdimIO_t* self, RaveObjectList_t* attributes, int fromInternal)
{
  RaveAttribute_t* quantityAttr = NULL;
  RaveAttribute_t* gainAttr = NULL;
  RaveAttribute_t* offsetAttr = NULL;
  int result = 0;

  if (self->version < RaveIO_ODIM_Version_2_4) {
    return 1;
  }

  quantityAttr = VpOdimIOInternal_getAttribute(attributes, "what/quantity");
  if (quantityAttr != NULL && RaveAttribute_getFormat(quantityAttr) == RaveAttribute_Format_String) {
    char* value = NULL;
    double gain=1.0, offset=0.0;
    RaveAttribute_getString(quantityAttr, &value);
    if (value != NULL && (strcasecmp("HGHT", value)==0 || strcasecmp("MESH", value)==0)) {
      gainAttr = VpOdimIOInternal_getAttribute(attributes, "what/gain");
      offsetAttr = VpOdimIOInternal_getAttribute(attributes, "what/offset");
      if (gainAttr == NULL) {
        RaveAttribute_t* newattr = RaveAttributeHelp_createDouble("what/gain", 1.0);
        if (newattr == NULL) {
          goto done;
        }
        gainAttr = RAVE_OBJECT_COPY(newattr);
        RaveObjectList_add(attributes, (RaveCoreObject*)newattr);
        RAVE_OBJECT_RELEASE(newattr);
      }
      if (offsetAttr == NULL) {
        RaveAttribute_t* newattr = RaveAttributeHelp_createDouble("what/gain", 0.0);
        if (newattr == NULL) {
          goto done;
        }
        offsetAttr = RAVE_OBJECT_COPY(newattr);
        RaveObjectList_add(attributes, (RaveCoreObject*)newattr);
        RAVE_OBJECT_RELEASE(newattr);
      }
      RaveAttribute_getDouble(gainAttr, &gain);
      RaveAttribute_getDouble(offsetAttr, &offset);
      if (fromInternal) {
        OdimIoUtilities_convertGainOffsetFromInternalRave((const char*)value, self->version, &gain, &offset);
      } else {
        OdimIoUtilities_convertGainOffsetToInternalRave((const char*)value, self->version, &gain, &offset);
      }
      RaveAttribute_setDouble(gainAttr, gain);
      RaveAttribute_setDouble(offsetAttr, offset);
      RAVE_OBJECT_RELEASE(gainAttr);
      RAVE_OBJECT_RELEASE(offsetAttr);
    }
  }

  result = 1;
done:
  RAVE_OBJECT_RELEASE(quantityAttr);
  RAVE_OBJECT_RELEASE(gainAttr);
  RAVE_OBJECT_RELEASE(offsetAttr);
  return result;
}

/**
 * Vp root attributes.
 * @param[in] object - the OdimIoUtilityArg pointing to a polar scan
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int VpOdimIOInternal_loadRootAttribute(void* object, RaveAttribute_t* attribute)
{
  VerticalProfile_t* vp = (VerticalProfile_t*)((OdimIoUtilityArg*)object)->object;
  RaveIO_ODIM_Version version = ((OdimIoUtilityArg*)object)->version;
  const char* name;
  int result = 0;

  RAVE_ASSERT((attribute != NULL), "attribute == NULL");

  name = RaveAttribute_getName(attribute);

  if (strcasecmp("what/date", name)==0) {
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR0("Failed to extract what/date as a string");
      goto done;
    }
    result = VerticalProfile_setDate(vp, value);
  } else if (strcasecmp("what/time", name)==0) {
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR0("Failed to extract what/time as a string");
      goto done;
    }
    result = VerticalProfile_setTime(vp, value);
  } else if (strcasecmp("what/source", name)==0) {
    char* value = NULL;
    if (!RaveAttribute_getString(attribute, &value)) {
      RAVE_ERROR0("Failed to extract what/source as a string");
      goto done;
    }
    result = VerticalProfile_setSource(vp, value);
  } else if (strcasecmp("where/lon", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/lon as a double");
      goto done;
    }
    VerticalProfile_setLongitude(vp, value * M_PI/180.0);
  } else if (strcasecmp("where/lat", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/lat as a double");
      goto done;
    }
    VerticalProfile_setLatitude(vp, value * M_PI/180.0);
  } else if (strcasecmp("where/height", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/height as a double");
      goto done;
    }
    VerticalProfile_setHeight(vp, value);
  } else if (strcasecmp("where/levels", name)==0) {
    long value = 0;
    if (!(result = RaveAttribute_getLong(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/levels as a long");
      goto done;
    }
    VerticalProfile_setLevels(vp, value);
  } else if (strcasecmp("where/interval", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/interval as a double");
      goto done;
    }
    VerticalProfile_setInterval(vp, value);
  } else if (strcasecmp("where/minheight", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/minheight as a double");
      goto done;
    }
    VerticalProfile_setMinheight(vp, value);
        
  } else if (strcasecmp("where/maxheight", name)==0) {
    double value = 0.0;
    if (!(result = RaveAttribute_getDouble(attribute, &value))) {
      RAVE_ERROR0("Failed to extract where/maxheight as a double");
      goto done;
    }
    VerticalProfile_setMaxheight(vp, value);
  } else if (strcasecmp("what/object", name) == 0) {
    result = 1;
  } else {
    VerticalProfile_addAttributeVersion(vp, attribute, version);
    result = 1;
  }

done:
  return result;
}

/**
 * Called when an attribute belonging to a dataset in a vertical profile
 * is found. Not, overrides any existing one in the root....
 * @param[in] object - the OdimIoUtilityArg pointing to a vertical profile
 * @param[in] attribute - the attribute found
 * @return 1 on success otherwise 0
 */
static int VpOdimIOInternal_loadDsAttribute(void* object, RaveAttribute_t* attribute)
{
  VerticalProfile_t* vp = NULL;
  const char* name;
  int result = 0;
  RaveIO_ODIM_Version version;

  RAVE_ASSERT((object != NULL), "object == NULL");
  RAVE_ASSERT((attribute != NULL), "attribute == NULL");
     
  vp = (VerticalProfile_t*)((OdimIoUtilityArg*)object)->object;
  name = RaveAttribute_getName(attribute);
  version = ((OdimIoUtilityArg*)object)->version;
  
  /* The following functions added by Ulf to be able to access the new
     attributes starttime, endtime, startdate, enddate and product */
  if (name != NULL) {
    if (strcasecmp("what/starttime", name)==0) {
      char* value = NULL;
      if (!RaveAttribute_getString(attribute, &value)) {
        RAVE_ERROR0("Failed to extract what/starttime as a string");
        goto done;
      }
      if(!(result = VerticalProfile_setStartTime(vp, value))) {
        RAVE_ERROR1("Failed to set what/starttime with value = %s", value);
      }
    } else if (strcasecmp("what/endtime", name)==0) {
      char* value = NULL;
      if (!RaveAttribute_getString(attribute, &value)) {
        RAVE_ERROR0("Failed to extract what/endtime as a string");
        goto done;
      }
      if(!(result = VerticalProfile_setEndTime(vp, value))) {
        RAVE_ERROR1("Failed to set what/endtime with value = %s", value);
      }
    } else if (strcasecmp("what/startdate", name)==0) {
      char* value = NULL;
      if (!RaveAttribute_getString(attribute, &value)) {
        RAVE_ERROR0("Failed to extract what/startdate as a string");
        goto done;
      }
      if(!(result = VerticalProfile_setStartDate(vp, value))) {
        RAVE_ERROR1("Failed to set what/startdate with value = %s", value);
      }
    } else if (strcasecmp("what/enddate", name)==0) {
      char* value = NULL;
      if (!RaveAttribute_getString(attribute, &value)) {
        RAVE_ERROR0("Failed to extract what/enddate as a string");
        goto done;
      }
      if(!(result = VerticalProfile_setEndDate(vp, value))) {
        RAVE_ERROR1("Failed to set what/enddate with value = %s", value);
      }
    } else if (strcasecmp("what/product", name) == 0) {
      char* value = NULL;
      if (!RaveAttribute_getString(attribute, &value)) {
        RAVE_ERROR0("Failed to extract what/product as a string");
        goto done;
      }
      if (RaveTypes_getObjectTypeFromString(value) != Rave_ObjectType_VP) {
        RAVE_WARNING0("what/product did not identify as a VP!");
      }
      result = 1;
    } else {
      VerticalProfile_addAttributeVersion(vp, attribute, version);
      result = 1;
    }
  }
done:
  return result;
}    
    

/**
 * Fills the scan with information from the dataset and below. I.e. root
 * attributes are not read.
 * @param[in] lazyReader - the wrapper around a hlhdf node list
 * @param[in] scan - the scan
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
static int VpOdimIOInternal_fillVpDataset(VpOdimIO_t* self, LazyNodeListReader_t* lazyReader, VerticalProfile_t* vp, const char* fmt, ...)
{
  int result = 0;
  OdimIoUtilityArg arg;

  va_list ap;
  char name[1024];
  int nName = 0;
  int pindex = 1;

  RAVE_ASSERT((lazyReader != NULL), "lazyReader == NULL");
  RAVE_ASSERT((vp != NULL), "vp == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  va_start(ap, fmt);
  nName = vsnprintf(name, 1024, fmt, ap);
  va_end(ap);
  if (nName < 0 || nName >= 1024) {
    RAVE_ERROR0("NodeName would evaluate to more than 1024 characters.");
    goto done;
  }

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)vp;
  arg.version = self->version;
  if (!RaveHL_loadAttributesAndData(arg.nodelist,
                                    &arg,
                                    VpOdimIOInternal_loadDsAttribute,
                                    NULL,
                                    "%s",
                                    name)) {
    RAVE_ERROR1("Failed to load attributes for vertical profile at %s level", name);
    goto done;
  }

  result = 1;
  pindex = 1;
  while (result == 1 && RaveHL_hasNodeByName(arg.nodelist, "%s/data%d", name, pindex)) {
    RaveField_t* field = OdimIoUtilities_loadField(lazyReader, self->version, "%s/data%d", name, pindex);
    if (field != NULL) {
      RaveObjectList_t* attributes = RaveField_getInternalAttributeValues(field);
      if (attributes != NULL) {
        result = VpOdimIOInternal_updateGainOffset(self, attributes, 0);
      }
      if (result) {
        result = VerticalProfile_addField(vp, field);
      }
      RAVE_OBJECT_RELEASE(attributes);
    } else {
      result = 0;
    }
    pindex++;
    RAVE_OBJECT_RELEASE(field);
  }

done:
  return result;
}

/**
 * Adds a vp parameter to the nodelist.
 *
 * @param[in] field - the field
 * @param[in] nodelist - the hlhdf node list
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
static int VpOdimIOInternal_addParameter(VpOdimIO_t* self, RaveField_t* field, HL_NodeList* nodelist, const char* fmt, ...)
{
  int result = 0;
  RaveObjectList_t* attributes = NULL;
  va_list ap;
  char name[1024];
  int nName = 0;

  RAVE_ASSERT((field != NULL), "field == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  va_start(ap, fmt);
  nName = vsnprintf(name, 1024, fmt, ap);
  va_end(ap);
  if (nName < 0 || nName >= 1024) {
    RAVE_ERROR0("NodeName would evaluate to more than 1024 characters.");
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, name)) {
    if (!RaveHL_createGroup(nodelist, name)) {
      goto done;
    }
  }
  if ((attributes = RaveField_getAttributeValuesVersion(field, self->version)) == NULL) {
    goto done;
  }

  VpOdimIOInternal_updateGainOffset(self, attributes, 1);

  if (!RaveHL_addAttributes(nodelist, attributes, name)) {
    goto done;
  }

  result = RaveHL_addData(nodelist,
                          RaveField_getData(field),
                          RaveField_getXsize(field),
                          RaveField_getYsize(field),
                          RaveField_getDataType(field),
                          name);
done:
  RAVE_OBJECT_RELEASE(attributes);
  return result;
}

/**
 * Adds the fields to the nodelist.
 *
 * @param[in] vp - the vertical profile
 * @param[in] nodelist - the hlhdf node list
 * @param[in] fmt - the varargs format string
 * @param[in] ... - the varargs
 * @return 1 on success otherwise 0
 */
static int VpOdimIOInternal_addFields(VpOdimIO_t* self, VerticalProfile_t* vp, HL_NodeList* nodelist, const char* fmt, ...)
{
  int result = 0;
  RaveObjectList_t* fields = NULL;
  int nparams = 0;

  va_list ap;
  char name[1024];
  int nName = 0;
  int pindex = 1;

  RAVE_ASSERT((vp != NULL), "vp == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");
  RAVE_ASSERT((fmt != NULL), "fmt == NULL");

  va_start(ap, fmt);
  nName = vsnprintf(name, 1024, fmt, ap);
  va_end(ap);
  if (nName < 0 || nName >= 1024) {
    RAVE_ERROR0("NodeName would evaluate to more than 1024 characters.");
    goto done;
  }

  if ((fields = VerticalProfile_getFields(vp)) == NULL) {
    goto done;
  }
  nparams = RaveObjectList_size(fields);

  result = 1;
  for (pindex = 0; result == 1 && pindex < nparams; pindex++) {
    RaveField_t* field = (RaveField_t*)RaveObjectList_get(fields, pindex);
    if (field != NULL) {
      result = VpOdimIOInternal_addParameter(self, field, nodelist, "%s/data%d", name, (pindex+1));
    } else {
      result = 0;
    }
    RAVE_OBJECT_RELEASE(field);
  }


done:
  RAVE_OBJECT_RELEASE(fields);
  return result;
}

/*@} End of Private functions */

/*@{ Interface functions */
void VpOdimIO_setVersion(VpOdimIO_t* self, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->version = version;
}

RaveIO_ODIM_Version VpOdimIO_getVersion(VpOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->version;
}

void VpOdimIO_setStrict(VpOdimIO_t* self, int strict)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  self->strict = strict;
}

int VpOdimIO_isStrict(VpOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->strict;
}

const char* VpOdimIO_getErrorMessage(VpOdimIO_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->error_message;
}

int VpOdimIO_read(VpOdimIO_t* self, LazyNodeListReader_t* lazyReader, VerticalProfile_t* vp)
{
  int result = 0;
  OdimIoUtilityArg arg;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((lazyReader != NULL), "lazyReader == NULL");
  RAVE_ASSERT((vp != NULL), "vp == NULL");

  arg.lazyReader = lazyReader;
  arg.nodelist = LazyNodeListReader_getHLNodeList(lazyReader);
  arg.object = (RaveCoreObject*)vp;
  arg.version = self->version;

  if (!RaveHL_hasNodeByName(arg.nodelist, "/dataset1") ||
      !RaveHL_hasNodeByName(arg.nodelist, "/dataset1/data1")) {
    RAVE_ERROR0("VP file does not contain vertical profile data...");
    goto done;
  }

  if (!RaveHL_loadAttributesAndData(arg.nodelist, &arg,
                                    VpOdimIOInternal_loadRootAttribute,
                                    NULL,
                                    "")) {
    RAVE_ERROR0("Failed to load attributes for vertical profile at root level");
    goto done;
  }

  if (!VpOdimIOInternal_fillVpDataset(self, lazyReader, vp, "/dataset1")) {
    RAVE_ERROR0("Failed to fill vertical profile");
    goto done;
  }

  result = 1;
done:
  return result;
}

int VpOdimIO_fill(VpOdimIO_t* self, VerticalProfile_t* vp, HL_NodeList* nodelist)
{
  int result = 0;
  RaveObjectList_t* attributes = NULL;
  RaveObjectList_t* qualityfields = NULL;
  char* source = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((vp != NULL), "vp == NULL");
  RAVE_ASSERT((nodelist != NULL), "nodelist == NULL");

  strcpy(self->error_message, "");

  if (!VpOdimIO_validateVpHowAttributes(self, vp)) {
    RAVE_ERROR0("Could not validate vertical profile how-attributes");
    goto done;
  }

  if (!RaveHL_hasNodeByName(nodelist, "/Conventions")) {
    if (!RaveHL_createStringValue(nodelist, RaveHL_getOdimVersionString(self->version), "/Conventions")) {
      goto done;
    }
  }

  attributes = VerticalProfile_getAttributeValuesVersion(vp, self->version);
  if (attributes != NULL) {
    const char* objectType = RaveTypes_getStringFromObjectType(Rave_ObjectType_VP);
    if (!RaveUtilities_addStringAttributeToList(attributes, "what/object", objectType) ||
        !RaveUtilities_replaceStringAttributeInList(attributes, "what/version", RaveHL_getH5RadVersionStringFromOdimVersion(self->version))) {
      RAVE_ERROR0("Failed to add what/object or what/version to attributes");
      goto done;
    }
  } else {
    RAVE_ERROR0("Failed to aquire attributes for vertical profile");
    goto done;
  }

  source = RaveUtilities_handleSourceVersion(VerticalProfile_getSource(vp), self->version);
  if (self->strict && !RaveUtilities_isSourceValid(source, self->version)) {
    strcpy(self->error_message, "what/source is not valid, missing ORG or NOD?");
    goto done;
  }

  if (!RaveUtilities_replaceStringAttributeInList(attributes, "what/date", VerticalProfile_getDate(vp)) ||
      !RaveUtilities_replaceStringAttributeInList(attributes, "what/time", VerticalProfile_getTime(vp)) ||
      !RaveUtilities_replaceStringAttributeInList(attributes, "what/source", source) ||
      !RaveUtilities_replaceLongAttributeInList(attributes, "where/levels", VerticalProfile_getLevels(vp)) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/interval", VerticalProfile_getInterval(vp)) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/minheight", VerticalProfile_getMinheight(vp)) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/maxheight", VerticalProfile_getMaxheight(vp)) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/height", VerticalProfile_getHeight(vp)) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/lat", VerticalProfile_getLatitude(vp)*180.0/M_PI) ||
      !RaveUtilities_replaceDoubleAttributeInList(attributes, "where/lon", VerticalProfile_getLongitude(vp)*180.0/M_PI)) {
    goto done;
  }

  if (!VerticalProfile_hasAttribute(vp, "how/software")) {
    if (!RaveUtilities_addStringAttributeToList(attributes, "how/software", "BALTRAD")) {
      RAVE_ERROR0("Failed to add how/software to attributes");
    }
  }

  if (attributes == NULL || !RaveHL_addAttributes(nodelist, attributes, "")) {
    goto done;
  }

  if (!RaveHL_createGroup(nodelist, "/dataset1")) {
    goto done;
  }
  
  if (!RaveHL_createGroup(nodelist, "/dataset1/what")) {
    goto done;  
  }

  RaveObjectList_clear(attributes);

  if (self->version >= RaveIO_ODIM_Version_2_3) {
    if (VerticalProfile_getProdname(vp) == NULL) {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", "BALTRAD vp")) {
        goto done;
      }
    } else {
      if (!RaveUtilities_addStringAttributeToList(attributes, "what/prodname", VerticalProfile_getProdname(vp))) {
        goto done;
      }
    }
  }

  if (!RaveHL_addAttributes(nodelist, attributes, "/dataset1")) {
    goto done;
  }

  /* The following code added by Ulf to be able to access the new
   attributes starttime, endtime, startdate, enddate and product */
  if (VerticalProfile_getStartTime(vp) != NULL) {
    if (!RaveHL_createStringValue(nodelist, VerticalProfile_getStartTime(vp), "/dataset1/what/starttime")) {
      goto done;
    }
  }

  if (VerticalProfile_getEndTime(vp) != NULL) {
    if (!RaveHL_createStringValue(nodelist, VerticalProfile_getEndTime(vp), "/dataset1/what/endtime")) {
      goto done;
    }
  }

  if (VerticalProfile_getStartDate(vp) != NULL) {
    if (!RaveHL_createStringValue(nodelist, VerticalProfile_getStartDate(vp), "/dataset1/what/startdate")) {
      goto done;
    }
  }

  if (VerticalProfile_getEndDate(vp) != NULL) {
    if (!RaveHL_createStringValue(nodelist, VerticalProfile_getEndDate(vp), "/dataset1/what/enddate")) {
      goto done;
    }
  }

  if (VerticalProfile_getProduct(vp) != NULL) {
    if (!RaveHL_createStringValue(nodelist, VerticalProfile_getProduct(vp), "/dataset1/what/product")) {
      goto done;
    }
  }
  
  
  RAVE_OBJECT_RELEASE(attributes);
  attributes = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
  if (attributes == NULL) {
    goto done;
  }

  result = VpOdimIOInternal_addFields(self, vp, nodelist, "/dataset1");

done:
  RAVE_OBJECT_RELEASE(attributes);
  RAVE_OBJECT_RELEASE(qualityfields);
  RAVE_FREE(source);
  return result;
}

int VpOdimIO_validateVpHowAttributes(VpOdimIO_t* self, VerticalProfile_t* vp)
{
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (!self->strict) {
    return 1;
  }
  if (self->version >= RaveIO_ODIM_Version_2_4) {
    int gotSimulated = VerticalProfile_hasAttribute(vp, "how/simulated");
    if (!gotSimulated) {
      RaveObjectList_t* fields = VerticalProfile_getFields(vp);
      if (fields != NULL) {
        int i = 0, nrfields = 0;
        gotSimulated = 1;
        nrfields = RaveObjectList_size(fields);
        for (i = 0; i < nrfields && gotSimulated; i++) {
          RaveField_t* field = (RaveField_t*)RaveObjectList_get(fields, i);
          gotSimulated = RaveField_hasAttribute(field, "how/simulated");
          RAVE_OBJECT_RELEASE(field);
        }
      } else {
        RAVE_ERROR0("Failed to validate vertical profile");
        gotSimulated = 0;
      }
      RAVE_OBJECT_RELEASE(fields);
    }
    if (!gotSimulated) {
      RAVE_ERROR0("Failed to validate how attributes for cartesian image. Missing required attribute.");
      strcpy(self->error_message, "Failed to validate how attributes for cartesian image. Missing required attribute how/simulated");
    }
    result = gotSimulated;
  }
  return result;
}


/*@} End of Interface functions */

RaveCoreObjectType VpOdimIO_TYPE = {
    "VpOdimIO",
    sizeof(VpOdimIO_t),
    VpOdimIO_constructor,
    VpOdimIO_destructor,
    VpOdimIO_copyconstructor
};
