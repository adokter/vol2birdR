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
#ifndef HLHDF_NODELIST_H
#define HLHDF_NODELIST_H
#include "hlhdf_types.h"

/**
 * Creates a new HL_NodeList instance
 * @ingroup hlhdf_c_apis
 * @return the allocated node list on success, otherwise NULL.
 */
HL_NodeList* HLNodeList_new(void);

/**
 * Releasing all resources associated with this node list including the node list itself.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the list that should be released.
 */
void HLNodeList_free(HL_NodeList* nodelist);

/**
 * Sets the filename in the HL_NodeList instance
 * @param[in] nodelist - the nodelist
 * @param[in] filename - the filename that should be used
 * @return 1 on success, otherwise 0
 */
int HLNodeList_setFileName(HL_NodeList* nodelist, const char* filename);

/**
 * Returns the filename of this nodelist.
 * @param[in] nodelist - the nodelist
 * @return the filename for this nodelist or NULL if no filename is set or failed to allocate memory.
 */
char* HLNodeList_getFileName(HL_NodeList* nodelist);

/**
 * Returns the number of nodes that exists in the provided nodelist.
 * @param[in] nodelist - the node list
 * @return the number of nodes or a negative value on failure.
 */
int HLNodeList_getNumberOfNodes(HL_NodeList* nodelist);

/**
 * Returns the node at the specified index.
 * @param[in] nodelist - the node list
 * @param[in] index - the index of the node
 * @return the node if it exists, otherwise NULL. <b>Do not free since it is an internal pointer</b>
 */
HL_Node* HLNodeList_getNodeByIndex(HL_NodeList* nodelist, int index);

/**
 * Marks all nodes in the nodelist with the provided mark.
 * @param[in] nodelist - the nodelist to be updated.
 * @param[in] mark - the mark each node should have.
 */
void HLNodeList_markNodes(HL_NodeList* nodelist, const HL_NodeMark mark);

/**
 * Adds a node to the nodelist.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should get a node added
 * @param[in] node the node that should be added to the node list
 * @return 1 On success, otherwise 0
 */
int HLNodeList_addNode(HL_NodeList* nodelist, HL_Node* node);

/**
 * Locates a node called nodeName in the nodelist and returns a pointer
 * to this node. I.e. Do not delete it!
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should be searched in
 * @param[in] nodeName the name of the node that should be located
 * @return the node if it could be found, otherwise NULL.
 */
HL_Node* HLNodeList_getNodeByName(HL_NodeList* nodelist,const char* nodeName);

/**
 * Returns if the nodelist contains a node with the specified name or not.
 * @param[in] nodelist - the nodelist
 * @param[in] nodeName - the name of the node that is searched for
 * @returns 1 if the node could be found, otherwise 0
 */
int HLNodeList_hasNodeByName(HL_NodeList* nodelist, const char* nodeName);

/**
 * Searches the nodelist for any type node, that has got the same object id as objno0 and objno1.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist that should be searched
 * @param[in] objno0 identifier 0
 * @param[in] objno1 identifier 1
 * @return The compound type description if found, otherwise NULL.
 */
HL_CompoundTypeDescription* HLNodeList_findCompoundDescription(HL_NodeList* nodelist,
                  unsigned long objno0,
                  unsigned long objno1);

#endif /* HLHDF_NODELIST_H */
