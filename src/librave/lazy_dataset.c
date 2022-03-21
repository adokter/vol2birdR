/* --------------------------------------------------------------------
Copyright (C) 2020- Swedish Meteorological and Hydrological Institute, SMHI,

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
 * This is a wrapper around a lazy nodelist reader used for fetching data from a HL_NodeList.
 * This does not support \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2020-11-07
 */
#include "lazy_dataset.h"
#include "rave_debug.h"
#include "rave_alloc.h"
#include <string.h>
#include "rave_object.h"
#include "rave_data2d.h"
#include "rave_hlhdf_utilities.h"

/**
 * Represents one lazy dataset
 */
struct _LazyDataset_t {
  RAVE_OBJECT_HEAD /** Always on top */
  LazyNodeListReader_t* reader; /**< reader */
  char* nodename; /**< name of the node to fetch from reader */
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int LazyDataset_constructor(RaveCoreObject* obj)
{
  LazyDataset_t* dataset = (LazyDataset_t*)obj;
  dataset->reader = NULL;
  dataset->nodename = NULL;
  return 1;
}

/**
 * Destructor.
 */
static void LazyDataset_destructor(RaveCoreObject* obj)
{
  LazyDataset_t* dataset = (LazyDataset_t*)obj;
  RAVE_OBJECT_RELEASE(dataset->reader);
  RAVE_FREE(dataset->nodename);
}

static HL_Node* LazyDatasetInternal_getNode(LazyNodeListReader_t* reader, const char* nodename)
{
  return HLNodeList_getNodeByName(LazyNodeListReader_getHLNodeList(reader), nodename);
}

/*@} End of Private functions */

/*@{ Interface functions */
int LazyDataset_init(LazyDataset_t* self, LazyNodeListReader_t* reader, const char* nodename)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (reader == NULL) {
    RAVE_ERROR0("Can not initialize LazyDataset with NULL reader");
    return 0;
  }
  if (nodename == NULL) {
    RAVE_ERROR0("Can not initialize LazyDataset with NULL nodename");
    return 0;
  }

  if (!LazyNodeListReader_exists(reader, nodename)) {
    RAVE_ERROR1("LazyNodeListReader does not contain node named %s", nodename);
    return 0;
  }

  if (HLNode_getType(LazyDatasetInternal_getNode(reader, nodename)) != DATASET_ID) {
    RAVE_ERROR1("%s is not a dataset", nodename);
  }

  self->nodename = RAVE_STRDUP(nodename);
  if (self->nodename == NULL) {
    return 0;
  }

  self->reader = RAVE_OBJECT_COPY(reader);
  return 1;
}

RaveData2D_t* LazyDataset_get(LazyDataset_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return LazyNodeListReader_getDataset(self->reader, self->nodename);

}

long LazyDataset_getXsize(LazyDataset_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return HLNode_getDimension(LazyDatasetInternal_getNode(self->reader, self->nodename), 1);
}

long LazyDataset_getYsize(LazyDataset_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return HLNode_getDimension(LazyDatasetInternal_getNode(self->reader, self->nodename), 0);
}

RaveDataType LazyDataset_getDataType(LazyDataset_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return RaveHL_hlhdfToRaveType(HLNode_getFormat(LazyDatasetInternal_getNode(self->reader, self->nodename)));
}

const char* LazyDataset_getNodeName(LazyDataset_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return (const char*)self->nodename;
}

/*@} End of Interface functions */

RaveCoreObjectType LazyDataset_TYPE = {
    "LazyDataset",
    sizeof(LazyDataset_t),
    LazyDataset_constructor,
    LazyDataset_destructor,
    NULL
};


