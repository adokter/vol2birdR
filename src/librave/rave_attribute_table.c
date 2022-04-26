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
 * Used for managing attributes and handle different versions.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2022-03-30
 */
#include "rave_attribute_table.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include <math.h>
#include <stdio.h>


#define SPEED_OF_LIGHT 299792458  /* m/s */

/**
 * Represents one scan in a volume.
 */
struct _RaveAttributeTable_t {
  RAVE_OBJECT_HEAD /** Always on top */
  RaveIO_ODIM_Version version;    /**< the default version */
  RaveObjectHashTable_t* attributes; /**< the attributes */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int RaveAttributeTable_constructor(RaveCoreObject* obj)
{
  RaveAttributeTable_t* attr = (RaveAttributeTable_t*)obj;
  attr->version = RAVEIO_API_ODIM_VERSION;
  attr->attributes = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (attr->attributes == NULL) {
    return 0;
  }
  return 1;
}

static int RaveAttributeTable_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  RaveAttributeTable_t* this = (RaveAttributeTable_t*)obj;
  RaveAttributeTable_t* src = (RaveAttributeTable_t*)srcobj;
  this->version = src->version;
  this->attributes = RAVE_OBJECT_CLONE(src->attributes);
  if (this->attributes == NULL) {
    return 0;
  }
  return 1;
}

/**
 * Destructor.
 */
static void RaveAttributeTable_destructor(RaveCoreObject* obj)
{
  RaveAttributeTable_t* attr = (RaveAttributeTable_t*)obj;
  RAVE_OBJECT_RELEASE(attr->attributes);
}

RaveAttribute_t* RaveAttributeTableInternal_getAttribute(RaveAttributeTable_t* self, const char* attrname)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (RaveAttribute_t*)RaveObjectHashTable_get(self->attributes, attrname);
}

static RaveAttribute_t* RaveAttributeTableInternal_translateFromInternal(RaveAttributeTable_t* self, RaveAttribute_t* attr, RaveIO_ODIM_Version version)
{
  RaveAttribute_t* newattr = NULL;
  const char* attrname = RaveAttribute_getName(attr);

  if (version >= RaveIO_ODIM_Version_2_4) {
    if (strcasecmp("how/rpm", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/antspeed", version);
    } else if (strcasecmp("how/S2N", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/SNR_threshold", version);
    } else if (strcasecmp("how/startazT", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/startT", version);
    } else if (strcasecmp("how/stopazT", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/stopT", version);
    } else if (strcasecmp("how/wavelength", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/frequency", version);
    } else if (strcasecmp("how/melting_layer_top", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/melting_layer_top_A", version);
    } else if (strcasecmp("how/melting_layer_bottom", attrname) == 0) {
      newattr = RaveAttributeTable_getAttributeVersion(self, "how/melting_layer_bottom_A", version);
    } else {
      newattr = RaveAttributeTable_getAttributeVersion(self, attrname, version);
    }
  } else {
    newattr = RaveAttributeTable_getAttributeVersion(self, attrname, version);
  }

  return newattr;
}

static RaveList_t* RaveAttributeTableInternal_getInternalNames(RaveAttributeTable_t* self, RaveIO_ODIM_Version version)
{
  RaveList_t* rlist = RaveObjectHashTable_keys(self->attributes);
  if (rlist != NULL) {
    int nlen = 0, i = 0;
    nlen = RaveList_size(rlist);
    for (i = 0; i < nlen; i++) {
      char* name = (char*)RaveList_get(rlist, i);
      if (version >= RaveIO_ODIM_Version_2_4) {
        if (strcasecmp("how/rpm", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/antspeed"));
        } else if (strcasecmp("how/S2N", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/SNR_threshold"));
        } else if (strcasecmp("how/startazT", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/startT"));
        } else if (strcasecmp("how/stopazT", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/stopT"));
        } else if (strcasecmp("how/wavelength", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/frequency"));
        } else if (strcasecmp("how/_melting_layer_top", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/melting_layer_top"));
        } else if (strcasecmp("how/melting_layer_top", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/melting_layer_top_A"));
        } else if (strcasecmp("how/melting_layer_bottom", name) == 0) {
          char* x = RaveList_remove(rlist, i);
          RAVE_FREE(x);
          RaveList_insert(rlist, i, RAVE_STRDUP("how/melting_layer_bottom_A"));
        }
      }
    }
  }
  return rlist;
}

/*@} End of Private functions */

/*@{ Interface functions */
int RaveAttributeTable_setVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (version >= RaveIO_ODIM_Version_2_2) {
    self->version = version;
    return 1;
  }
  return 0;
}

RaveIO_ODIM_Version RaveAttributeTable_getVersion(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->version;
}

int RaveAttributeTable_addAttribute(RaveAttributeTable_t* self, RaveAttribute_t* attr, RaveAttribute_t** translation)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_addAttributeVersion(self, attr, self->version, translation);
}

int RaveAttributeTable_addAttributeVersion(RaveAttributeTable_t* self, RaveAttribute_t* attr, RaveIO_ODIM_Version version, RaveAttribute_t** translation)
{
  RaveAttribute_t* newattr = NULL;
  int result = 0;

  RAVE_ASSERT((self != NULL), "self == NULL");
  const char* attrname = RaveAttribute_getName(attr);

  if (strcasecmp("how/antspeed", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double) {
    double v = 0.0;
    RaveAttribute_getDouble(attr, &v);
    newattr = RaveAttributeHelp_createDouble("how/rpm", v/6);
  } else if (strcasecmp("how/SNR_threshold", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double) {
    double v = 0.0;
    RaveAttribute_getDouble(attr, &v);
    newattr = RaveAttributeHelp_createDouble("how/S2N", v);
  } else if ((strcasecmp("how/startT", attrname) == 0 || strcasecmp("how/stopT", attrname) == 0) &&
      RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray) {
    double* darr = NULL;
    int nlen = 0;
    RaveAttribute_getDoubleArray(attr, &darr, &nlen);
    if (strcasecmp("how/startT", attrname) == 0) {
      newattr = RaveAttributeHelp_createDoubleArray("how/startazT", darr, nlen);
    } else {
      newattr = RaveAttributeHelp_createDoubleArray("how/stopazT", darr, nlen);
    }
  } else if (strcasecmp("how/frequency", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double) {
    double v = 0.0;
    RaveAttribute_getDouble(attr, &v);
    if (v != 0.0) {
      /* f = C/λ, f = frequency Hz, C = speed of light m/s, λ = wavelength in meters */
      newattr = RaveAttributeHelp_createDouble("how/wavelength", (SPEED_OF_LIGHT / v)*100.0);
    }
  } else if (strcasecmp("how/melting_layer_top", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double && version >= RaveIO_ODIM_Version_2_4) {
    double v = 0.0;
    RaveAttribute_getDouble(attr, &v);
    newattr = RaveAttributeHelp_createDouble("how/_melting_layer_top", v);
  } else if (strcasecmp("how/melting_layer_bottom", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double && version >= RaveIO_ODIM_Version_2_4) {
    double v = 0.0;
    RaveAttribute_getDouble(attr, &v);
    newattr = RaveAttributeHelp_createDouble("how/_melting_layer_bottom", v);
  } else if (strcasecmp("how/melting_layer_top_A", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray && version >= RaveIO_ODIM_Version_2_4) {
    double* darr = NULL;
    int nlen = 0;
    RaveAttribute_getDoubleArray(attr, &darr, &nlen);
    newattr = RaveAttributeHelp_createDoubleArray("how/melting_layer_top", darr, nlen);
    if (newattr != NULL) {
      int i = 0;
      RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
      for (i = 0; i < nlen; i++) {
        darr[i] /= 1000.0;
      }
    }
  } else if (strcasecmp("how/melting_layer_bottom_A", attrname) == 0 && RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray && version >= RaveIO_ODIM_Version_2_4) {
    double* darr = NULL;
    int nlen = 0;
    RaveAttribute_getDoubleArray(attr, &darr, &nlen);
    newattr = RaveAttributeHelp_createDoubleArray("how/melting_layer_bottom", darr, nlen);
    if (newattr != NULL) {
      int i = 0;
      RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
      for (i = 0; i < nlen; i++) {
        darr[i] /= 1000.0;
      }
    }
  }

  if (version >= RaveIO_ODIM_Version_2_4) {
    /* Since all attribute units within rave are according to 2.3. */
    if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(attr, &v);
      if (strcasecmp("how/gasattn", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v * 1000.0); /* dB/m => dB/km */
      } else if (strcasecmp("how/minrange", attrname)==0 ||
          strcasecmp("how/maxrange", attrname)==0 ||
          strcasecmp("how/radhoriz", attrname)==0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v / 1000.0); /* m => km */
      } else if (strcasecmp("how/nomTXpower", attrname) == 0 ||
          strcasecmp("how/peakpwr", attrname) == 0 ||
          strcasecmp("how/avgpwr", attrname) == 0) {
        if (v != 0) {
          newattr = RaveAttributeHelp_createDouble(attrname, pow(10.0, (v - 30.0)/10.0)/1000.0); /* dbM => kW */
        }
      } else if (strcasecmp("how/pulsewidth", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v * 1000000.0); /* s => micros */
      } else if (strcasecmp("how/RXbandwidth", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v / 1000000.0); /* Hz => MHz */
      }
    } else if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray && strcasecmp("how/TXpower", attrname) == 0) {
      newattr = RAVE_OBJECT_CLONE(attr);
      if (newattr != NULL) {
        double* darr = NULL;
        int nlen = 0, i = 0;
        RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
        for (i = 0; i < nlen; i++) {
          if (darr[i] != 0) {
            darr[i] = pow(10.0, (darr[i] - 30.0)/10.0)/1000.0; /* dBm => kW */
          }
        }
      }
    }
  }

  if (newattr != NULL && translation != NULL) {
    *translation = RAVE_OBJECT_COPY(newattr);
  }

  if (newattr == NULL) {
    newattr = RAVE_OBJECT_COPY(attr);
  }

  result = RaveObjectHashTable_put(self->attributes, RaveAttribute_getName(newattr), (RaveCoreObject*)newattr);

  RAVE_OBJECT_RELEASE(newattr);

  return result;
}

RaveAttribute_t* RaveAttributeTable_getAttribute(RaveAttributeTable_t* self, const char* attrname)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_getAttributeVersion(self, attrname, self->version);
}

RaveAttribute_t* RaveAttributeTable_getAttributeVersion(RaveAttributeTable_t* self, const char* attrname, RaveIO_ODIM_Version version)
{
  RaveAttribute_t* newattr = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (strcasecmp("how/antspeed", attrname) == 0) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/rpm");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      newattr = RaveAttributeHelp_createDouble("how/antspeed", v*6);
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/SNR_threshold", attrname) == 0) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/S2N");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      newattr = RaveAttributeHelp_createDouble("how/SNR_threshold", v);
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/startT", attrname) == 0 || strcasecmp("how/stopT", attrname) == 0) {
    RaveAttribute_t* internal = NULL;
    if (strcasecmp("how/startT", attrname) == 0) {
      internal = RaveAttributeTableInternal_getAttribute(self, "how/startazT");
    } else {
      internal = RaveAttributeTableInternal_getAttribute(self, "how/stopazT");
    }
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_DoubleArray) {
      double* darr = NULL;
      int nlen = 0;
      RaveAttribute_getDoubleArray(internal, &darr, &nlen);
      if (strcasecmp("how/startT", attrname) == 0) {
        newattr = RaveAttributeHelp_createDoubleArray("how/startT", darr, nlen);
      } else {
        newattr = RaveAttributeHelp_createDoubleArray("how/stopT", darr, nlen);
      }
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/melting_layer_top", attrname) == 0 && version >= RaveIO_ODIM_Version_2_4) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/_melting_layer_top");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      newattr = RaveAttributeHelp_createDouble("how/melting_layer_top", v);
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/melting_layer_bottom", attrname) == 0 && version >= RaveIO_ODIM_Version_2_4) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/_melting_layer_bottom");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      newattr = RaveAttributeHelp_createDouble("how/melting_layer_bottom", v);
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/melting_layer_top_A", attrname) == 0) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/melting_layer_top");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_DoubleArray) {
      double* darr = NULL;
      int nlen = 0, i = 0;
      RaveAttribute_getDoubleArray(internal, &darr, &nlen);
      newattr = RaveAttributeHelp_createDoubleArray("how/melting_layer_top_A", darr, nlen);
      if (newattr != NULL) {
        RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
        for (i = 0; i < nlen; i++) {
          darr[i] = darr[i] * 1000.0;
        }
      } else {
        RAVE_ERROR0("Failed to create newattr");
        RAVE_OBJECT_RELEASE(internal);
        goto done;
      }
    }
    RAVE_OBJECT_RELEASE(internal);
  } else if (strcasecmp("how/melting_layer_bottom_A", attrname) == 0) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/melting_layer_bottom");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_DoubleArray) {
      double* darr = NULL;
      int nlen = 0, i = 0;
      RaveAttribute_getDoubleArray(internal, &darr, &nlen);
      newattr = RaveAttributeHelp_createDoubleArray("how/melting_layer_bottom_A", darr, nlen);
      if (newattr != NULL) {
        RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
        for (i = 0; i < nlen; i++) {
          darr[i] = darr[i] * 1000.0;
        }
      } else {
        RAVE_ERROR0("Failed to create newattr");
        RAVE_OBJECT_RELEASE(internal);
        goto done;
      }
    }
    RAVE_OBJECT_RELEASE(internal);
  }

  if (strcasecmp("how/frequency", attrname) == 0) {
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, "how/wavelength");
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      if (v != 0.0) {
        /* f = C/λ, f = frequency MHz, C = speed of light m/s, λ = wavelength in meters */
        v = v/100.0; /* From cm to meters */
        newattr = RaveAttributeHelp_createDouble("how/frequency", SPEED_OF_LIGHT / v);
      }
    }
    RAVE_OBJECT_RELEASE(internal);
  }

  if (version >= RaveIO_ODIM_Version_2_4) {
    /* Since all attribute units within rave are according to new . We have to change unit to old one */
    RaveAttribute_t* internal = RaveAttributeTableInternal_getAttribute(self, attrname);
    if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_Double) {
      double v = 0.0;
      RaveAttribute_getDouble(internal, &v);
      if (strcasecmp("how/gasattn", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v / 1000.0); /* dB/km => dB/m */
      } else if (strcasecmp("how/minrange", attrname)==0 ||
          strcasecmp("how/maxrange", attrname)==0 ||
          strcasecmp("how/radhoriz", attrname)==0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v * 1000.0); /* km => m */
      } else if (strcasecmp("how/nomTXpower", attrname) == 0 ||
          strcasecmp("how/peakpwr", attrname) == 0 ||
          strcasecmp("how/avgpwr", attrname) == 0) {
        if (v != 0.0) {
          newattr = RaveAttributeHelp_createDouble(attrname, 10 * log10(1000.0*v) + 30); /* dBm => kw */
        }
      } else if (strcasecmp("how/pulsewidth", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v / 1000000.0); /* micros => s */
      } else if (strcasecmp("how/RXbandwidth", attrname) == 0) {
        newattr = RaveAttributeHelp_createDouble(attrname, v * 1000000.0); /* MHz => Hz */
      }
    } else if (internal != NULL && RaveAttribute_getFormat(internal) == RaveAttribute_Format_DoubleArray && strcasecmp("how/TXpower", attrname) == 0) {
      newattr = RAVE_OBJECT_CLONE(internal);
      if (newattr != NULL) {
        double* darr = NULL;
        int nlen = 0, i = 0;
        RaveAttribute_getDoubleArray(newattr, &darr, &nlen);
        for (i = 0; i < nlen; i++) {
          darr[i] = 10 * log10(1000.0*darr[i]) + 30; /* kw => dBm */ //    pow(10.0, (darr[i] - 30.0)/10.0)/1000.0; /* dBm => kw */
        }
      }
    }
    RAVE_OBJECT_RELEASE(internal);
  }

  if (newattr == NULL) {
    newattr = (RaveAttribute_t*)RaveObjectHashTable_get(self->attributes, attrname);
  }

done:
  return newattr;
}

int RaveAttributeTable_size(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_size(self->attributes);
}

int RaveAttributeTable_hasAttribute(RaveAttributeTable_t* self, const char* key)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_exists(self->attributes, key);
}

RaveAttribute_t* RaveAttributeTable_removeAttribute(RaveAttributeTable_t* self, const char* key)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (RaveAttribute_t*)RaveObjectHashTable_remove(self->attributes, key);
}

void RaveAttributeTable_clear(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  RaveObjectHashTable_clear(self->attributes);
}

RaveList_t* RaveAttributeTable_getAttributeNames(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_getAttributeNamesVersion(self, self->version);
}

RaveList_t* RaveAttributeTable_getAttributeNamesVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version)
{
  RaveList_t* rlist = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");

  rlist = RaveAttributeTableInternal_getInternalNames(self, version);

  return rlist;
}

RaveObjectList_t* RaveAttributeTable_getValues(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveAttributeTable_getValuesVersion(self, self->version);
}

RaveObjectList_t* RaveAttributeTable_getValuesVersion(RaveAttributeTable_t* self, RaveIO_ODIM_Version version)
{
  RaveObjectList_t* values = NULL;
  RaveObjectList_t* rlist = NULL;
  RaveObjectList_t* result = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");

  values = RaveObjectHashTable_values(self->attributes);
  rlist = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);

  if (values != NULL && rlist != NULL) {
    int nlen = RaveObjectList_size(values), i = 0;
    for (i = 0; i < nlen; i++) {
      RaveAttribute_t* attr = (RaveAttribute_t*)RaveObjectList_get(values, i);
      RaveAttribute_t* translated = RaveAttributeTableInternal_translateFromInternal(self, attr, version);
      if (translated != NULL) {
        if (!RaveObjectList_add(rlist, (RaveCoreObject*)translated)) {
          RAVE_OBJECT_RELEASE(attr);
          RAVE_OBJECT_RELEASE(translated);
          goto fail;
        }
      }
      RAVE_OBJECT_RELEASE(attr);
      RAVE_OBJECT_RELEASE(translated);
    }
  }

  result = RAVE_OBJECT_CLONE(rlist);
fail:
  RAVE_OBJECT_RELEASE(rlist);
  RAVE_OBJECT_RELEASE(values);
  return result;
}

RaveObjectList_t* RaveAttributeTable_getInternalValues(RaveAttributeTable_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveObjectHashTable_values(self->attributes);
}

int RaveAttributeTable_shiftAttribute(RaveAttributeTable_t* self, const char* name, int nx)
{
  RaveAttribute_t* attr = NULL;
  int result = 0;

  attr = RaveAttributeTable_getAttribute(self, name);
  if (attr != NULL) {
    if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_LongArray ||
        RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray) {
      result = RaveAttribute_shiftArray(attr, nx);
    }
  }
  RAVE_OBJECT_RELEASE(attr);

  return result;
}

int RaveAttributeTable_shiftAttributeIfExists(RaveAttributeTable_t* self, const char* name, int nx)
{
  RaveAttribute_t* attr = NULL;
  int result = 1;

  attr = RaveAttributeTable_getAttribute(self, name);
  if (attr != NULL) {
    if (RaveAttribute_getFormat(attr) == RaveAttribute_Format_LongArray ||
        RaveAttribute_getFormat(attr) == RaveAttribute_Format_DoubleArray) {
      result = RaveAttribute_shiftArray(attr, nx);
    }
  }
  RAVE_OBJECT_RELEASE(attr);

  return result;
}

void RaveAttributeTable_destroyKeyList(RaveList_t* l)
{
  RaveObjectHashTable_destroyKeyList(l);
}

/*@} End of Interface functions */

RaveCoreObjectType RaveAttributeTable_TYPE = {
  "RaveAttributeTable",
  sizeof(RaveAttributeTable_t),
  RaveAttributeTable_constructor,
  RaveAttributeTable_destructor,
  RaveAttributeTable_copyconstructor
};
