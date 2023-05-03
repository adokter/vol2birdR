/*
 * lazy_nodelist_io.c
 *
 *  Created on: Nov 6, 2020
 *      Author: anders
 */
#include <lazy_nodelist_reader.h>

#include "hlhdf_read.h"
#include "hlhdf_nodelist.h"
#include "hlhdf_node.h"
#include "hlhdf_alloc.h"
#include <string.h>
#include "rave_debug.h"
#include "rave_hlhdf_utilities.h"

/**
 * Represents the lazy nodelist loader
 */
struct _LazyNodeListReader_t {
  RAVE_OBJECT_HEAD /** Always on top */
  HL_NodeList* nodelist;
  char* filename;
};

/*@{ Private functions */
/**
 * Constructor.
 */
static int LazyNodeListReader_constructor(RaveCoreObject* obj)
{
  LazyNodeListReader_t* self = (LazyNodeListReader_t*)obj;
  self->nodelist = NULL;
  self->filename = NULL;
  return 1;
}

/**
 * Destroys the object
 * @param[in] obj - the instance
 */
static void LazyNodeListReader_destructor(RaveCoreObject* obj)
{
  LazyNodeListReader_t* self = (LazyNodeListReader_t*)obj;
  if (self->nodelist != NULL) {
    HLNodeList_free(self->nodelist);
  }
  if (self->filename != NULL) {
    HLHDF_FREE(self->filename);
  }
}

/*@} End of Private functions */

/*@{ Interface functions */

int LazyNodeListReader_init(LazyNodeListReader_t* self, HL_NodeList* nodelist)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (nodelist != NULL && self->nodelist == NULL) {
    self->nodelist = nodelist;
    self->filename = HLNodeList_getFileName(self->nodelist);
    return 1;
  }
  return 0;
}

int LazyNodeListReader_preload(LazyNodeListReader_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return LazyNodeListReader_preloadQuantities(self, NULL);
}

static int LazyNodeListReaderInternal_liststrcmp(void* a, void* b)
{
  if (strcmp((const char*)a, (const char*)b)==0) {
    return 1;
  }
  return 0;
}

int LazyNodeListReader_preloadQuantities(LazyNodeListReader_t* self, const char* quantities)
{
  RaveList_t* quantitiesToPreload = NULL;
  int i = 0, n = 0;
  int result = 0;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    if (quantities != NULL) {
      quantitiesToPreload = RaveUtilities_getTrimmedTokens(quantities, ',');
      if (quantitiesToPreload == NULL) {
        RAVE_ERROR0("Failed to handle quantities to preload");
        return 0;
      }
    }
    n = HLNodeList_getNumberOfNodes(self->nodelist);
    for (i = 0; i < n; i++) {
      HL_Node* node = HLNodeList_getNodeByIndex(self->nodelist, i);
      const char* nodeName = HLNode_getName(node);
      if (HLNode_getType(node) == DATASET_ID && !HLNode_fetched(node)) {
        int dsetid=0, did=0;
        if (sscanf(nodeName, "/dataset%d/data%d/data", &dsetid, &did) == 2) { /* This is a parameter and probably has a quantity */
          char qname[1024];
          snprintf(qname, 1024, "/dataset%d/data%d/what/quantity", dsetid, did);
          if (quantitiesToPreload != NULL && RaveList_size(quantitiesToPreload) > 0 && HLNodeList_hasNodeByName(self->nodelist, qname)) {
            const char* quantity = (const char *)HLNode_getData(HLNodeList_getNodeByName(self->nodelist, qname));
            if (RaveList_find(quantitiesToPreload, (void*)quantity, LazyNodeListReaderInternal_liststrcmp) != NULL) {
              HLNode_setMark(node, NMARK_SELECT);
            }
          } else {
            HLNode_setMark(node, NMARK_SELECT);
          }
        } else {
          if (!HLNode_fetched(node)) {
            HLNode_setMark(node, NMARK_SELECT);
          }
        }
      }
    }
    result = HLNodeList_fetchMarkedNodes(self->nodelist);
  }
  if (quantitiesToPreload != NULL) {
    RaveList_freeAndDestroy(&quantitiesToPreload);
  }
  return result;
}

RaveData2D_t* LazyNodeListReader_getDataset(LazyNodeListReader_t* self, const char* datasetname)
{
  RaveData2D_t* result = NULL;
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    HL_Node* node = HLNodeList_getNodeByName(self->nodelist, datasetname);
    if (node != NULL && HLNode_getType(node) == DATASET_ID) {
      if (!HLNode_fetched(node)) {
        node = HLNodeList_fetchNode(self->nodelist, datasetname);
        if (node == NULL) {
          RAVE_ERROR1("Failed to fetch node %s during lazy loading", datasetname);
          goto done;
        }
      }
      hsize_t d0 = HLNode_getDimension(node, 0);
      hsize_t d1 = HLNode_getDimension(node, 1);
      RaveDataType dataType = RaveHL_hlhdfToRaveType(HLNode_getFormat(node));
      if (dataType != RaveDataType_UNDEFINED) {
        result = RAVE_OBJECT_NEW(&RaveData2D_TYPE);
        if (result != NULL) {
          RaveData2D_setData(result, d1, d0, HLNode_getData(node), dataType);
        }
      }
    }
  }
done:
  return result;
}

RaveAttribute_t* LazyNodeListReader_getAttribute(LazyNodeListReader_t* self, const char* attributename)
{
  RaveAttribute_t* attr = NULL;
  RaveAttribute_t* result = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    HL_Node* node = HLNodeList_getNodeByName(self->nodelist, attributename);
    if (node != NULL && HLNode_getType(node) == ATTRIBUTE_ID) {
      if (!HLNode_fetched(node)) { /* This is currently only theoretical since all metadata is loaded on read */
        node = HLNodeList_fetchNode(self->nodelist, attributename);
        if (node == NULL) {
          RAVE_WARNING1("Failed to fetch node %s in during lazy loading", attributename);
          goto done;
        }
      }
      attr = RaveHL_createAttribute(node);
      if (attr != NULL) {
        if (!RaveAttribute_setName(attr, attributename)) {
          goto done;
        }
      }
    }
  }
  result = RAVE_OBJECT_COPY(attr);
done:
  return result;
}

int LazyNodeListReader_isLoaded(LazyNodeListReader_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    HL_Node* node = HLNodeList_getNodeByName(self->nodelist, name);
    if (node != NULL) {
      return HLNode_fetched(node);
    }
  }
  return 0;
}

int LazyNodeListReader_exists(LazyNodeListReader_t* self, const char* name)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    return HLNodeList_hasNodeByName(self->nodelist, name);
  }
  return 0;
}

RaveList_t* LazyNodeListReader_getNodeNames(LazyNodeListReader_t* self)
{
  RaveList_t* result = NULL;

  RAVE_ASSERT((self != NULL), "self == NULL");
  if (self->nodelist != NULL) {
    result = RAVE_OBJECT_NEW(&RaveList_TYPE);
    if (result != NULL) {
      int i = 0;
      int nlen = HLNodeList_getNumberOfNodes(self->nodelist);
      for (i = 0; i < nlen; i++) {
        HL_Node* node = HLNodeList_getNodeByIndex(self->nodelist, i);
        char* nodeName = RAVE_STRDUP(HLNode_getName(node));
        if (nodeName == NULL || !RaveList_add(result, nodeName)) {
          RAVE_ERROR0("Failed to add node name to list of node names");
          RaveList_freeAndDestroy(&result);
          return NULL;
        }
      }
    }
  }
  return result;
}

HL_NodeList* LazyNodeListReader_getHLNodeList(LazyNodeListReader_t* self)
{
  RAVE_ASSERT((self != NULL), "self == NULL");
  return self->nodelist;
}

LazyNodeListReader_t* LazyNodeListReader_create(HL_NodeList* nodelist)
{
  LazyNodeListReader_t* result = NULL;
  if (nodelist != NULL) {
    result = RAVE_OBJECT_NEW(&LazyNodeListReader_TYPE);
    if (result != NULL) {
      if (!LazyNodeListReader_init(result, nodelist)) {
        RAVE_OBJECT_RELEASE(result);
      }
    }
  }
  return result;
}

LazyNodeListReader_t* LazyNodeListReader_read(const char* filename)
{
  HL_NodeList* nodelist = HLNodeList_read(filename);

  if (nodelist == NULL) {
    RAVE_ERROR1("Failed to read %s", nodelist);
    return NULL;
  }

  HLNodeList_selectAllMetadataNodes(nodelist);
  //HLNodeList_selectAllNodes(nodelist); 0.5 LDR
  if (!HLNodeList_fetchMarkedNodes(nodelist)) {
    RAVE_ERROR1("Failed to load hdf5 file '%s'", filename);
    HLNodeList_free(nodelist);
    return NULL;
  }

  return LazyNodeListReader_create(nodelist);
}

LazyNodeListReader_t* LazyNodeListReader_readPreloaded(const char* filename)
{
  HL_NodeList* nodelist = HLNodeList_read(filename);

  if (nodelist == NULL) {
    RAVE_ERROR1("Failed to read %s", nodelist);
    return NULL;
  }

  HLNodeList_selectAllNodes(nodelist);
  if (!HLNodeList_fetchMarkedNodes(nodelist)) {
    RAVE_ERROR1("Failed to load hdf5 file '%s'", filename);
    HLNodeList_free(nodelist);
    return NULL;
  }

  return LazyNodeListReader_create(nodelist);
}

/*@} End of Interface functions */

RaveCoreObjectType LazyNodeListReader_TYPE = {
    "LazyNodeListReader",
    sizeof(LazyNodeListReader_t),
    LazyNodeListReader_constructor,
    LazyNodeListReader_destructor,
    NULL
};
