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
 * Functions that should be used when reading a HDF5 file.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-11
 */
#ifndef HLHDF_READ_H
#define HLHDF_READ_H
#include "hlhdf_types.h"

/**
 * Reads an HDF5 file with name filename from the group fromPath and downwards.
 * This function will not fetch the actual data but will only read the structure.
 * Use selectAll/selectAllNodes and fetchMarkedNodes to retrieve the data.
 * @ingroup hlhdf_c_apis
 * @param[in] filename the name of the HDF5 file
 * @param[in] fromPath the path from where the file should be read.
 * @return the read data structure on success, otherwise NULL.
 */
HL_NodeList* HLNodeList_readFrom(const char* filename, const char* fromPath);

/**
 * Reads an HDF5 file with name filename from the root group ("/") and downwards.
 * This function will not fetch the actual data but will only read the structure.
 * Use selectAll/selectAllNodes and fetchMarkedNodes to retrieve the data.
 * @ingroup hlhdf_c_apis
 * @param[in] filename the name of the HDF5 file
 * @return the read data structure on success, otherwise NULL.
 */
HL_NodeList* HLNodeList_read(const char* filename);

/**
 * Selects the node named 'name' from which to fetch data.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the nodelist where the node named name should be marked for select
 * @param[in] name the fully qualified name of the node that should be selected.
 * @return: 1 on success, otherwise 0
 */
int HLNodeList_selectNode(HL_NodeList* nodelist, const char* name);

/**
 * Marks all nodes in the nodelist for retrival.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @return 1 on success, otherwise 0
 */
int HLNodeList_selectAllNodes(HL_NodeList* nodelist);

/**
 * Selects all metadata nodes in the nodelist to be fetched, ie. dataset attributes but no dataset arrays or arrays.
 * <b>VOLATILE: Do not attempt to access dataset arrays after calling this.</b>
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @return 1 on success, otherwise 0
 */
int HLNodeList_selectMetadataNodes(HL_NodeList* nodelist);

/**
 * Selects all metadata including metadata about datasets but will exclude data for datasets..
 * <b>VOLATILE: Do not attempt to access dataset arrays after calling this.</b>
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @return 1 on success, otherwise 0
 */
int HLNodeList_selectAllMetadataNodes(HL_NodeList* nodelist);

/**
 * Only select data set nodes for fetching. This is useful if for example wanting to
 * first read metadata. Then depending on content fetch dataset nodes.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @return 1 on success, otherwise 0
 */
int HLNodeList_selectOnlyDatasetNodes(HL_NodeList* nodelist);

/**
 * De-selects the node named 'name' to be retrived when fetching data from the nodelist file.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @param[in] name the name that should be deselected
 * @return 1 on success, otherwise 0.
 */
int HLNodeList_deselectNode(HL_NodeList* nodelist, const char* name);

/**
 * Fills all nodes (marked as select) with data.
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @return 1 on success, otherwise 0
 */
int HLNodeList_fetchMarkedNodes(HL_NodeList* nodelist);

/**
 * Behaves as a combination of HLNodeList_selectNode()/fetch()/getNode().
 * @ingroup hlhdf_c_apis
 * @param[in] nodelist the node list
 * @param[in] name the name of the node that should be fetched.
 * @return the found node or NULL on failure.
 */
HL_Node* HLNodeList_fetchNode(HL_NodeList* nodelist, const char* name);

#endif
