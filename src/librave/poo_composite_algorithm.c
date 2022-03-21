/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * POO compositing algorithm.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2011-10-28
 */
#include "poo_composite_algorithm.h"
#include "composite.h"
#include "polarvolume.h"
#include <stdio.h>
#include "raveobject_hashtable.h"
#include "raveobject_list.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include "rave_datetime.h"
#include <string.h>

/**
 * The name of self
 */
static const char* POO_NAME = "POO";

/**
 * Represents the cartesian product.
 */
struct _PooCompositeAlgorithm_t {
  RAVE_OBJECT_HEAD /** Always on top */
  COMPOSITE_ALGORITHM_HEAD /**< composite specifics */

  RaveObjectHashTable_t* sources; /**< the composite objects */
  RaveValueType type; /**< the currents positions type */
  double pooheight; /**< the value to be used */
  double mindist; /**< the minimium distance used when going for NEAREST/HEIGHT*/
  CompositeSelectionMethod_t method; /**< the method used */
};

/*@{ Private functions */
/**
 * Constructor.
 * @param[in] obj - the created object
 */
static int PooCompositeAlgorithm_constructor(RaveCoreObject* obj)
{
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)obj;
  this->getName = PooCompositeAlgorithm_getName;
  this->supportsProcess = PooCompositeAlgorithm_supportsProcess;
  this->process = PooCompositeAlgorithm_process;
  this->initialize = PooCompositeAlgorithm_initialize;
  this->reset = PooCompositeAlgorithm_reset;
  this->supportsFillQualityInformation = PooCompositeAlgorithm_supportsFillQualityInformation;
  this->fillQualityInformation = PooCompositeAlgorithm_fillQualityInformation;
  this->sources = NULL;
  return 1;
}

/**
 * Copy constructor.
 * @param[in] obj - the created object
 * @param[in] srcobj - the source (that is copied)
 */
static int PooCompositeAlgorithm_copyconstructor(RaveCoreObject* obj, RaveCoreObject* srcobj)
{
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)obj;
  PooCompositeAlgorithm_t* src = (PooCompositeAlgorithm_t*)srcobj;
  this->getName = src->getName;
  this->supportsProcess = src->supportsProcess;
  this->process = src->process;
  this->initialize = src->initialize;
  this->reset = src->reset;
  this->supportsFillQualityInformation = src->supportsFillQualityInformation;
  this->fillQualityInformation = src->fillQualityInformation;
  this->sources = RAVE_OBJECT_CLONE(src->sources);
  if (this->sources == NULL) {
    goto error;
  }
  return 1;
error:
  RAVE_OBJECT_RELEASE(this->sources);
  return 0;
}

/**
 * Destructor
 * @param[in] obj - the object to destroy
 */
static void PooCompositeAlgorithm_destructor(RaveCoreObject* obj)
{
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)obj;
  this->getName = NULL;
  this->supportsProcess = NULL;
  this->process = NULL;
  this->initialize = NULL;
  this->reset = NULL;
  this->supportsFillQualityInformation = NULL;
  this->fillQualityInformation = NULL;
  RAVE_OBJECT_RELEASE(this->sources);
}

/**
 * Will traverse all objects in the list and atempt to find a scan that contains a
 * quality field that has got a how/task value == se.smhi.detector.poo.
 * All scans that contains such a field will get a scan set in the resulting
 * hash table with the quality data set as the default (and only) parameter.
 * @param[in] composite - the composite
 * @return a hash table
 */
static RaveObjectHashTable_t* PooCompositeAlgorithmInternal_getPooScanFields(Composite_t* composite)
{
  RaveObjectHashTable_t* result = NULL;
  RaveObjectHashTable_t* scans = NULL;
  int nrobjs = 0, i = 0;
  int status = 1;

  scans = RAVE_OBJECT_NEW(&RaveObjectHashTable_TYPE);
  if (scans == NULL) {
    RAVE_ERROR0("Failed to allocate memory for object hash table");
    goto done;
  }
  nrobjs = Composite_getNumberOfObjects(composite);

  for (i = 0; status == 1 && i < nrobjs; i++) {
    RaveCoreObject* obj = Composite_get(composite, i);
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE)) {
      RaveField_t* field = PolarScan_findAnyQualityFieldByHowTask((PolarScan_t*)obj, "se.smhi.detector.poo");
      if (field != NULL) {
        PolarScan_t* scan = PolarScan_createFromScanAndField((PolarScan_t*)obj, field);
        if (scan == NULL || !RaveObjectHashTable_put(scans, PolarScan_getSource(scan), (RaveCoreObject*)scan)) {
          RAVE_ERROR0("Failed to add poo scan to hash table");
          status = 0;
        }
        RAVE_OBJECT_RELEASE(scan);
      }
      RAVE_OBJECT_RELEASE(field);
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE)) {
      PolarScan_t* pooscan = PolarVolume_findAnyScanWithQualityFieldByHowTask((PolarVolume_t*)obj, "se.smhi.detector.poo");
      if (pooscan != NULL) {
        RaveField_t* field = PolarScan_findAnyQualityFieldByHowTask(pooscan, "se.smhi.detector.poo");
        if (field != NULL) {
          PolarScan_t* scan = PolarScan_createFromScanAndField(pooscan, field);
          if (scan == NULL || !RaveObjectHashTable_put(scans, PolarScan_getSource(scan), (RaveCoreObject*)scan)) {
            RAVE_ERROR0("Failed to add poo scan to hash table");
            status = 0;
          }
          RAVE_OBJECT_RELEASE(scan);
        }
        RAVE_OBJECT_RELEASE(field);
      }
      RAVE_OBJECT_RELEASE(pooscan);
    }
    RAVE_OBJECT_RELEASE(obj);
  }
  result = RAVE_OBJECT_COPY(scans);
done:
  RAVE_OBJECT_RELEASE(scans);
  return result;
}
/*@} End of Private functions */

/*@{ Interface functions */
const char* PooCompositeAlgorithm_getName(CompositeAlgorithm_t* self)
{
  return POO_NAME;
}

void PooCompositeAlgorithm_reset(CompositeAlgorithm_t* self, int x, int y)
{
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)self;
  RAVE_ASSERT((this != NULL), "this == NULL");
  this->pooheight = 1e10;
  this->mindist = 1e10;
  this->type = RaveValueType_NODATA;
}

int PooCompositeAlgorithm_supportsProcess(CompositeAlgorithm_t* self)
{
  return 0;
}

int PooCompositeAlgorithm_process(CompositeAlgorithm_t* self, \
  RaveCoreObject* obj, const char* quantity, double olon, double olat, double dist, RaveValueType* otype, double* ovalue, \
  PolarNavigationInfo* navinfo)
{
  return 0;
}

int PooCompositeAlgorithm_initialize(CompositeAlgorithm_t* self, struct _Composite_t* composite)
{
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)self;
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");

  RAVE_OBJECT_RELEASE(this->sources);
  this->sources = PooCompositeAlgorithmInternal_getPooScanFields(composite);
  if (this->sources == NULL) {
    RAVE_ERROR0("Failed to prepare poo fields");
    goto done;
  }
  this->method = Composite_getSelectionMethod(composite);

  result = 1;
done:
  return result;
}

int PooCompositeAlgorithm_supportsFillQualityInformation(CompositeAlgorithm_t* self, const char* howtask)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (strcmp("se.smhi.detector.poo", howtask) == 0) {
    return 1;
  }
  return 0;
}

int PooCompositeAlgorithm_fillQualityInformation(CompositeAlgorithm_t* self,
                                                 RaveCoreObject* obj,
                                                 const char* howtask,
                                                 const char* quantity,
                                                 RaveField_t* field,
                                                 long x,
                                                 long y,
                                                 PolarNavigationInfo* navinfo,
                                                 double gain,
                                                 double offset)
{
  int result = 0;
  PolarScan_t* pooscan = NULL;
  PooCompositeAlgorithm_t* this = (PooCompositeAlgorithm_t*)self;

  RAVE_ASSERT((self != NULL), "self == NULL");
  RAVE_ASSERT((obj != NULL), "obj == NULL");
  RAVE_ASSERT((field != NULL), "field == NULL");
  RAVE_ASSERT((navinfo != NULL), "navinfo == NULL");
  RAVE_ASSERT((gain != 0.0), "gain == 0.0");

  if (strcmp("se.smhi.detector.poo", howtask) == 0) {
    if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarVolume_TYPE) && navinfo->ei >= 0 && navinfo->ri >= 0 && navinfo->ai >= 0) {
      const char* source = PolarVolume_getSource((PolarVolume_t*)obj);
      if (source != NULL) {
        pooscan = (PolarScan_t*)RaveObjectHashTable_get(this->sources, source);
      }
    } else if (RAVE_OBJECT_CHECK_TYPE(obj, &PolarScan_TYPE) && navinfo->ri >= 0 && navinfo->ai >= 0) {
      const char* source = PolarScan_getSource((PolarScan_t*)obj);
      if (source != NULL ) {
        pooscan = (PolarScan_t*)RaveObjectHashTable_get(this->sources, source);
      }
    }
    if (pooscan != NULL) {
      double v = 0.0;
      double convertedval = 0.0;
      RaveValueType t = PolarScan_getNearest(pooscan, navinfo->lon, navinfo->lat, 1, &v);

      if (t != RaveValueType_DATA) {
        v = 0.0;
      }
      convertedval = (v - offset) / gain;
      RaveField_setValue(field, x, y, convertedval);

      result = 1;
    }
  }

  RAVE_OBJECT_RELEASE(pooscan);
  return result;
}


/*@} End of Interface functions */

RaveCoreObjectType PooCompositeAlgorithm_TYPE = {
    "PooCompositeAlgorithm",
    sizeof(PooCompositeAlgorithm_t),
    PooCompositeAlgorithm_constructor,
    PooCompositeAlgorithm_destructor,
    PooCompositeAlgorithm_copyconstructor
};
