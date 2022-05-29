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
 * @date 2009-12-17
 */
#include "rave_types.h"
#include "rave_alloc.h"
#include "rave_debug.h"
#include "string.h"
#include "stdlib.h"
#include <stdio.h>

/**
 * Product Type constants
 */
static const char RaveTypes_ProductType_UNDEFINED_STR[] = "UNDEFINED";
static const char RaveTypes_ProductType_SCAN_STR[] = "SCAN";
static const char RaveTypes_ProductType_PPI_STR[] = "PPI";
static const char RaveTypes_ProductType_CAPPI_STR[] = "CAPPI";
static const char RaveTypes_ProductType_PCAPPI_STR[] = "PCAPPI";
static const char RaveTypes_ProductType_ETOP_STR[] = "ETOP";
static const char RaveTypes_ProductType_MAX_STR[] = "MAX";
static const char RaveTypes_ProductType_RR_STR[] = "RR";
static const char RaveTypes_ProductType_VIL_STR[] = "VIL";
static const char RaveTypes_ProductType_COMP_STR[] = "COMP";
static const char RaveTypes_ProductType_VP_STR[] = "VP";
static const char RaveTypes_ProductType_RHI_STR[] = "RHI";
static const char RaveTypes_ProductType_XSEC_STR[] = "XSEC";
static const char RaveTypes_ProductType_VSP_STR[] = "VSP";
static const char RaveTypes_ProductType_HSP_STR[] = "HSP";
static const char RaveTypes_ProductType_RAY_STR[] = "RAY";
static const char RaveTypes_ProductType_AZIM_STR[] = "AZIM";
static const char RaveTypes_ProductType_QUAL_STR[] = "QUAL";
static const char RaveTypes_ProductType_PMAX_STR[] = "PMAX";
static const char RaveTypes_ProductType_SURF_STR[] = "SURF";
static const char RaveTypes_ProductType_EBASE_STR[] = "EBASE";

/**
 * Object Type constants
 */
static const char RaveTypes_ObjectType_UNDEFINED_STR[]= "UNDEFINED";
static const char RaveTypes_ObjectType_PVOL_STR[]= "PVOL";
static const char RaveTypes_ObjectType_CVOL_STR[]= "CVOL";
static const char RaveTypes_ObjectType_SCAN_STR[]= "SCAN";
static const char RaveTypes_ObjectType_RAY_STR[]= "RAY";
static const char RaveTypes_ObjectType_AZIM_STR[]= "AZIM";
static const char RaveTypes_ObjectType_IMAGE_STR[]= "IMAGE";
static const char RaveTypes_ObjectType_COMP_STR[]= "COMP";
static const char RaveTypes_ObjectType_XSEC_STR[]= "XSEC";
static const char RaveTypes_ObjectType_VP_STR[]= "VP";
static const char RaveTypes_ObjectType_PIC_STR[]= "PIC";
/*@{ Constants */

/**
 * Mapping between a product type and the corresponding string
 */
struct RaveTypes_ProductMapping {
  Rave_ProductType type; /**< the product type */
  const char* str;       /**< the string */
};

/**
 * Mapping between a object type and the corresponding string
 */
struct RaveIO_ObjectTypeMapping {
  Rave_ObjectType type;  /**< the object type */
  const char* str;       /**< the string representation */
};

/**
 * The product type mapping table.
 */
static const struct RaveTypes_ProductMapping PRODUCT_MAPPING[] =
{
  {Rave_ProductType_UNDEFINED,  RaveTypes_ProductType_UNDEFINED_STR},
  {Rave_ProductType_SCAN,       RaveTypes_ProductType_SCAN_STR},
  {Rave_ProductType_PPI,        RaveTypes_ProductType_PPI_STR},
  {Rave_ProductType_CAPPI,      RaveTypes_ProductType_CAPPI_STR},
  {Rave_ProductType_PCAPPI,     RaveTypes_ProductType_PCAPPI_STR},
  {Rave_ProductType_ETOP,       RaveTypes_ProductType_ETOP_STR},
  {Rave_ProductType_MAX,        RaveTypes_ProductType_MAX_STR},
  {Rave_ProductType_RR,         RaveTypes_ProductType_RR_STR},
  {Rave_ProductType_VIL,        RaveTypes_ProductType_VIL_STR},
  {Rave_ProductType_COMP,       RaveTypes_ProductType_COMP_STR},
  {Rave_ProductType_VP,         RaveTypes_ProductType_VP_STR},
  {Rave_ProductType_RHI,        RaveTypes_ProductType_RHI_STR},
  {Rave_ProductType_XSEC,       RaveTypes_ProductType_XSEC_STR},
  {Rave_ProductType_VSP,        RaveTypes_ProductType_VSP_STR},
  {Rave_ProductType_HSP,        RaveTypes_ProductType_HSP_STR},
  {Rave_ProductType_RAY,        RaveTypes_ProductType_RAY_STR},
  {Rave_ProductType_AZIM,       RaveTypes_ProductType_AZIM_STR},
  {Rave_ProductType_QUAL,       RaveTypes_ProductType_QUAL_STR},
  {Rave_ProductType_PMAX,       RaveTypes_ProductType_PMAX_STR},
  {Rave_ProductType_SURF,       RaveTypes_ProductType_SURF_STR},
  {Rave_ProductType_EBASE,      RaveTypes_ProductType_EBASE_STR},
  {Rave_ProductType_ENDOFTYPES, NULL},
};

/**
 * The mapping table.
 */
static const struct RaveIO_ObjectTypeMapping OBJECT_TYPE_MAPPING[] =
{
  {Rave_ObjectType_UNDEFINED, RaveTypes_ObjectType_UNDEFINED_STR},
  {Rave_ObjectType_PVOL, RaveTypes_ObjectType_PVOL_STR},
  {Rave_ObjectType_CVOL, RaveTypes_ObjectType_CVOL_STR},
  {Rave_ObjectType_SCAN, RaveTypes_ObjectType_SCAN_STR},
  {Rave_ObjectType_RAY, RaveTypes_ObjectType_RAY_STR},
  {Rave_ObjectType_AZIM, RaveTypes_ObjectType_AZIM_STR},
  {Rave_ObjectType_IMAGE, RaveTypes_ObjectType_IMAGE_STR},
  {Rave_ObjectType_COMP, RaveTypes_ObjectType_COMP_STR},
  {Rave_ObjectType_XSEC, RaveTypes_ObjectType_XSEC_STR},
  {Rave_ObjectType_VP, RaveTypes_ObjectType_VP_STR},
  {Rave_ObjectType_PIC, RaveTypes_ObjectType_PIC_STR},
  {Rave_ObjectType_ENDOFTYPES, NULL}
};

/*@} End of Constants */

/*@{ Interface functions */
int get_ravetype_size(RaveDataType type)
{
  switch(type) {
  case RaveDataType_CHAR:
    return sizeof(char);
  case RaveDataType_UCHAR:
    return sizeof(unsigned char);
  case RaveDataType_SHORT:
    return sizeof(short);
  case RaveDataType_USHORT:
    return sizeof(unsigned short);
  case RaveDataType_INT:
    return sizeof(int);
  case RaveDataType_UINT:
    return sizeof(unsigned int);
  case RaveDataType_LONG:
    return sizeof(long);
  case RaveDataType_ULONG:
    return sizeof(unsigned long);
  case RaveDataType_FLOAT:
    return sizeof(float);
  case RaveDataType_DOUBLE:
    return sizeof(double);
  default:
    return -1;
  }
}

Rave_ProductType RaveTypes_getProductTypeFromString(const char* name)
{
  Rave_ProductType result = Rave_ProductType_UNDEFINED;
  if (name != NULL) {
    int index = 0;
    while (PRODUCT_MAPPING[index].str != NULL) {
      if (strcmp(PRODUCT_MAPPING[index].str, name) == 0) {
        result = PRODUCT_MAPPING[index].type;
        break;
      }
      index++;
    }

  }
  return result;
}

const char* RaveTypes_getStringFromProductType(Rave_ProductType type)
{
  int index = 0;
  while (PRODUCT_MAPPING[index].str != NULL) {
    if (type == PRODUCT_MAPPING[index].type) {
      return PRODUCT_MAPPING[index].str;
    }
    index++;
  }
  return NULL;
}

Rave_ObjectType RaveTypes_getObjectTypeFromString(const char* name)
{
  Rave_ObjectType result = Rave_ObjectType_UNDEFINED;
  if (name != NULL) {
    int index = 0;
    while (OBJECT_TYPE_MAPPING[index].str != NULL) {
      if (strcmp(OBJECT_TYPE_MAPPING[index].str, name) == 0) {
        result = OBJECT_TYPE_MAPPING[index].type;
        break;
      }
      index++;
    }

  }
  return result;
}

const char* RaveTypes_getStringFromObjectType(Rave_ObjectType type)
{
  int index = 0;
  while (OBJECT_TYPE_MAPPING[index].str != NULL) {
    if (type == OBJECT_TYPE_MAPPING[index].type) {
      return OBJECT_TYPE_MAPPING[index].str;
    }
    index++;
  }
  return NULL;
}

void RaveTypes_FreePolarObservationLinkedList(PolarObservationLinkedList* obs)
{
  if (obs != NULL) {
    RaveTypes_FreePolarObservationLinkedList(obs->next);
    RAVE_FREE(obs);
  }
}

PolarObservation* RaveTypes_PolarObservationLinkedListToArray(PolarObservationLinkedList* llobs, int* nritems)
{
  PolarObservation* result = NULL;
  if (llobs != NULL) {
    int nlen = 0, index = 0;
    PolarObservationLinkedList* pobs = llobs;
    while (pobs != NULL) {
      nlen++;
      pobs = pobs->next;
    }
    result = RAVE_MALLOC(sizeof(PolarObservation) * nlen);
    if (result == NULL) {
      RAVE_ERROR0("Failed to allocate memory for polar observations");
      goto done;
    }
    pobs = llobs;
    while (pobs != NULL) {
      result[index++] = pobs->obs;
      pobs = pobs->next;
    }
    *nritems = nlen;
  }

done:
  return result;
}

PolarObservation* RaveTypes_FilterPolarObservationDataValues(PolarObservation* observations, int nobservations, int* ndataobservations)
{
  PolarObservation* result = NULL;
  if (observations != NULL) {
    int nlen = 0, index = 0, i = 0;
    for (index = 0; index < nobservations; index++) {
      if (observations[index].vt == RaveValueType_DATA) {
        nlen++;
      }
    }
    result = RAVE_MALLOC(sizeof(PolarObservation) * nlen);
    if (result == NULL) {
      RAVE_ERROR0("Failed to allocate memory for polar observations");
      goto done;
    }
    index = 0;
    for (i = 0; i < nobservations; i++) {
      if (observations[i].vt == RaveValueType_DATA) {
        if (observations[i].v == 0.0 || observations[i].v == 255.0) {
          Rave_printf("Value counted as data even though value was either 0 or 255\n");
        }
        result[index++] = observations[i];
      }
    }
    *ndataobservations = nlen;
  }
done:
  return result;
}

static int RaveTypesInternal_sort_polar_observation_ascending(const void* a, const void* b)
{
  PolarObservation* poa = (PolarObservation*)a;
  PolarObservation* pob = (PolarObservation*)b;
  if (poa->vt == RaveValueType_DATA && pob->vt != RaveValueType_DATA) {
    return -1;
  } else if (poa->vt != RaveValueType_DATA && pob->vt == RaveValueType_DATA) {
    return 1;
  } else if (poa->vt == RaveValueType_DATA && pob->vt == RaveValueType_DATA) {
    if (poa->v < pob->v) {
      return -1;
    } else if (poa->v > pob->v) {
      return 1;
    }
  }
  return 0;
}

void RaveTypes_SortPolarObservations(PolarObservation* observations, int nobservations)
{
  qsort(observations, nobservations, sizeof(PolarObservation), RaveTypesInternal_sort_polar_observation_ascending);
}
/*@} End of Interface functions */
