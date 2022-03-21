/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

This file is part of HLHDF.

HLHDF is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

HLHDF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

/**
 * Functions for working with HL_NodeList's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-24
 */
#include "hlhdf.h"
#include "hlhdf_alloc.h"
#include "hlhdf_defines_private.h"
#include "hlhdf_debug.h"
#include "hlhdf_node.h"
#include <string.h>
#include <stdlib.h>

/*@{ Structs */
/**
 * Represents a HDF5 file.
 * @ingroup hlhdf_c_apis
 */
struct  _HL_NodeList {
   char* filename;     /**< The file name */
   int nNodes;         /**< Number of nodes */
   int nAllocNodes;    /**< Number of allocated nodes */
   HL_Node** nodes;    /**< The list of nodes (max size is nNodes - 1) */
};

/*@{ End of Structs */

/*@{ Interface functions */
HL_NodeList* HLNodeList_new(void)
{
  HL_NodeList* retv = NULL;
  int i;
  HL_SPEWDEBUG0("ENTER: newHL_NodeList");
  if (!(retv = (HL_NodeList*) HLHDF_MALLOC(sizeof(HL_NodeList)))) {
    HL_ERROR0("Failed to allocate memory for NODE");
    return NULL;
  }
  retv->filename = NULL;

  if (!(retv->nodes = (HL_Node**) HLHDF_MALLOC(sizeof(HL_Node*) * DEFAULT_SIZE_NODELIST))) {
    HL_ERROR0("Failed to allocate memory for HL_NodeList");
    HLHDF_FREE(retv);
    return NULL;
  }
  for (i = 0; i < DEFAULT_SIZE_NODELIST; i++) {
    retv->nodes[i] = NULL;
  }
  retv->nNodes = 0;
  retv->nAllocNodes = DEFAULT_SIZE_NODELIST;
  return retv;
}

void HLNodeList_free(HL_NodeList* nodelist)
{
  int i;
  HL_SPEWDEBUG0("ENTER: HLNodeList_free");
  if (!nodelist)
    return;

  if (nodelist->nodes) {
    for (i = 0; i < nodelist->nNodes; i++) {
      HLNode_free(nodelist->nodes[i]);
    }
    HLHDF_FREE(nodelist->nodes);
  }
  HLHDF_FREE(nodelist->filename);
  HLHDF_FREE(nodelist);
  HL_SPEWDEBUG0("EXIT: HLNodeList_free");
}

int HLNodeList_setFileName(HL_NodeList* nodelist, const char* filename)
{
  int status = 0;
  char* newfilename = NULL;

  if (nodelist == NULL || filename == NULL) {
    HL_ERROR0("Inparameters NULL");
    goto fail;
  }
  if ((newfilename = HLHDF_STRDUP(filename)) == NULL) {
    HL_ERROR1("Failed to allocate memory for file %s", filename);
    goto fail;
  }
  HLHDF_FREE(nodelist->filename);
  nodelist->filename = newfilename;
  newfilename = NULL; // Hand over memory

  status = 1;
fail:
  HLHDF_FREE(newfilename);
  return status;
}

char* HLNodeList_getFileName(HL_NodeList* nodelist)
{
  char* retv = NULL;

  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    return NULL;
  }

  if (nodelist->filename != NULL) {
    if ((retv = HLHDF_STRDUP(nodelist->filename)) == NULL) {
      HL_ERROR1("Failed to allocate memory for filename: %s", nodelist->filename);
    }
  }

  return retv;
}

int HLNodeList_getNumberOfNodes(HL_NodeList* nodelist)
{
  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    return -1;
  }
  return nodelist->nNodes;
}

HL_Node* HLNodeList_getNodeByIndex(HL_NodeList* nodelist, int index)
{
  if (nodelist == NULL) {
    HL_ERROR0("Inparameters NULL");
    return NULL;
  } else if (index < 0 || index >= nodelist->nNodes) {
    HL_ERROR0("index out of range");
    return NULL;
  }
  return nodelist->nodes[index];
}

void HLNodeList_markNodes(HL_NodeList* nodelist, const HL_NodeMark mark)
{
  int i = 0;
  if (nodelist != NULL) {
    for (i = 0; i < nodelist->nNodes; i++) {
      HLNode_setMark(nodelist->nodes[i], mark);
    }
  }
}

/*
 * If 1 is returned, then responsibility has been taken, otherwise
 * the caller has to free both nodelist and node by him self
 */
int HLNodeList_addNode(HL_NodeList* nodelist, HL_Node* node)
{
  int newallocsize;
  int i;
  char* tmpStr = NULL;
  char* tmpPtr;
  int treeStructureOk = 0;
  int status = 0;
  HL_Type type;
  HL_SPEWDEBUG0("ENTER: addNode");

  if (nodelist == NULL || node == NULL) {
    HL_ERROR0("Inparameters NULL");
    return 0;
  }
  type = HLNode_getType(node);
  if (HLNode_getName(node)) {
    tmpStr = HLHDF_STRDUP(HLNode_getName(node));
  }
  if (tmpStr == NULL) {
    HL_ERROR0("Failed to get node name");
    goto fail;
  }

  if (HLNodeList_getNodeByName(nodelist, tmpStr) != NULL) {
    HL_ERROR1("Node %s already exists", tmpStr);
    goto fail;
  }

  if (!(tmpPtr = strrchr(tmpStr, '/'))) {
    HL_ERROR1("Could not extract '/' from node name %s",tmpStr);
    goto fail;
  } else {
    tmpPtr[0] = '\0';
    if (strcmp(tmpStr, "") != 0) {
      HL_Node* node = HLNodeList_getNodeByName(nodelist, tmpStr);
      if (node != NULL) {
        HL_Type nType = HLNode_getType(node);

        if ((nType == GROUP_ID) ||
            (nType == DATASET_ID && (type == ATTRIBUTE_ID || type == REFERENCE_ID))) {
          treeStructureOk = 1;
        }
      }
    } else {
      treeStructureOk = 1;
    }
  }

  if (treeStructureOk == 0) {
    HL_ERROR1("Tree structure not built correct, missing group or dataset %s",tmpStr);
    goto fail;
  }

  if (nodelist->nNodes < nodelist->nAllocNodes - 1) {
    nodelist->nodes[nodelist->nNodes++] = node;
  } else {
    newallocsize = nodelist->nAllocNodes + DEFAULT_SIZE_NODELIST;
    if (!(nodelist->nodes = HLHDF_REALLOC(nodelist->nodes, sizeof(HL_Node*) * newallocsize))) {
      HL_ERROR0("Serious memory error occured when reallocating Node list");
      goto fail;
    }

    for (i = nodelist->nAllocNodes; i < newallocsize; i++) {
      nodelist->nodes[i] = NULL;
    }
    nodelist->nAllocNodes = newallocsize;
    nodelist->nodes[nodelist->nNodes++] = node;
  }

  status = 1;
fail:
  HLHDF_FREE(tmpStr);
  return status;
}

HL_Node* HLNodeList_getNodeByName(HL_NodeList* nodelist, const char* nodeName)
{
  int i;
  HL_SPEWDEBUG0("ENTER: getNode");
  if (!nodelist || !nodeName) {
    HL_ERROR0("Can't get node when either nodelist or nodeName is NULL");
    return NULL;
  }
  for (i = 0; i < nodelist->nNodes; i++) {
    if (HLNode_nameEquals(nodelist->nodes[i], nodeName)) {
      return nodelist->nodes[i];
    }
  }

  HL_DEBUG1("Could not locate node '%s'",nodeName);

  return NULL;
}

int HLNodeList_hasNodeByName(HL_NodeList* nodelist, const char* nodeName)
{
  int result = 0;
  int i = 0;

  if (!nodelist || !nodeName) {
    HL_ERROR0("Can't locate node when either nodelist or nodeName is NULL");
    return 0;
  }
  for (i = 0; result == 0 && i < nodelist->nNodes; i++) {
    if (HLNode_nameEquals(nodelist->nodes[i], nodeName)) {
      result = 1;
    }
  }

  return result;
}

HL_CompoundTypeDescription* HLNodeList_findCompoundDescription(
  HL_NodeList* nodelist, unsigned long objno0, unsigned long objno1)
{
  int i;
  HL_CompoundTypeDescription* retv = NULL;
  HL_SPEWDEBUG0("ENTER: findHL_CompoundTypeDescription");
  for (i = 0; i < nodelist->nNodes; i++) {
    if (HLNode_getType(nodelist->nodes[i]) == TYPE_ID) {
      HL_CompoundTypeDescription* descr = HLNode_getCompoundDescription(nodelist->nodes[i]);
      if (descr != NULL) {
        if (objno0 == descr->objno[0] && objno1 == descr->objno[1]) {
          retv = descr;
          goto done;
        }
      }
    }
  }
done:
  HL_SPEWDEBUG0("EXIT: findHL_CompoundTypeDescription");
  return retv;
}
