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
 * Functions for working with HL_Node's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-11
 */
#ifndef HLHDF_NODE_H
#define HLHDF_NODE_H
#include "hlhdf_types.h"

/**
 * Creates a new HL_Node instance.
 * @ingroup hlhdf_c_apis
 * @param[in] name the name of this node, should be fully qualified name. I.e. /a/b/c
 * @return the allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_new(const char* name);

/**
 * Deallocates the provided node.
 * @ingroup hlhdf_c_apis
 * @param[in] node the node that should be deallocated
 */
void HLNode_free(HL_Node* node);

/**
 * Creates an empty group HL_Node of type @ref HL_Type::GROUP_ID.
 * @ingroup hlhdf_c_apis
 * @param[in] name the fully qualified name
 * @return The allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_newGroup(const char* name);

/**
 * Creates an empty attribute HL_Node of type @ref HL_Type::ATTRIBUTE_ID.
 * @ingroup hlhdf_c_apis
 * @param[in] name the fully qualified name
 * @return The allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_newAttribute(const char* name);

/**
 * Creates an empty data set HL_Node of type @ref HL_Type::DATASET_ID.
 * @ingroup hlhdf_c_apis
 * @param[in] name the fully qualified name
 * @return The allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_newDataset(const char* name);

/**
 * Creates an empty data type HL_Node of type @ref HL_Type::TYPE_ID.
 * @ingroup hlhdf_c_apis
 * @param[in] name the fully qualified name
 * @return The allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_newDatatype(const char* name);

/**
 * Creates an empty reference HL_Node of type @ref HL_Type::REFERENCE_ID.
 * @ingroup hlhdf_c_apis
 * @param[in] name the fully qualified name
 * @return The allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_newReference(const char* name);

/**
 * Copies an HL_Node.
 * @ingroup hlhdf_c_apis
 * @param[in] node the node that should be copied
 * @return the allocated node on success, otherwise NULL.
 */
HL_Node* HLNode_copy(HL_Node* node);

/**
 * Sets a scalar value in the specified node.
 * @ingroup hlhdf_c_apis
 * @param[in] node the node that should get its value set.
 * @param[in] sz the size of the data
 * @param[in] value the data
 * @param[in] fmt the format specifier, @see ref ValidFormatSpecifiers "here" for valid formats.
 * @param[in] typid if a custom made type should be used for writing the data, otherwise use -1.
 * @return 1 if everything was ok, otherwise 0
 */
int HLNode_setScalarValue(HL_Node* node,size_t sz,unsigned char* value,const char* fmt,hid_t typid);

/**
 * Sets an array value in the specified node.
 * @ingroup hlhdf_c_apis
 * @param[in] node the node that should get its value set
 * @param[in] sz the size of the type
 * @param[in] ndims the rank
 * @param[in] dims the dimension
 * @param[in] value the data buffer
 * @param[in] fmt the format specifier, @see ref ValidFormatSpecifiers "here" for valid formats.
 * @param[in] typid if a custom made type should be used for writing the data, otherwise use -1.
 * @return 1 if everything was ok, otherwise 0
 */
int HLNode_setArrayValue(HL_Node* node,size_t sz,int ndims,hsize_t* dims,unsigned char* value,
      const char* fmt,hid_t typid);

/**
 * Returns the node name.
 * @param[in] node the node
 * @return the name of this node, <b>If this node is freed, then the returned name can not be relied on</b>.
 */
const char* HLNode_getName(HL_Node* node);

/**
 * Returns the internal data pointer for this node.
 * @param[in] node the node
 * @return the internal data (<b>Do not release and be careful so that the node does not change when holding the data pointer.</b>).
 */
unsigned char* HLNode_getData(HL_Node* node);

/**
 * Returns the type size for the data format.
 * @param[in] node the node
 * @return the type size for the data format
 */
size_t HLNode_getDataSize(HL_Node* node);

/**
 * Returns the internal rawdata pointer for this node.
 * @param[in] node the node
 * @return the internal data (<b>Do not release and be careful so that the node does not change when holding the data pointer.</b>).
 */
unsigned char* HLNode_getRawdata(HL_Node* node);

/**
 * Returns the type size for the raw data format.
 * @param[in] node the node
 * @return the type size for the raw data format
 */
size_t HLNode_getRawdataSize(HL_Node* node);

/**
 * Compares the nodes name with the provided name.
 * @param[in] node the node
 * @param[in] name the name to test
 * @return 1 if the name equals, otherwise 0
 */
int HLNode_nameEquals(HL_Node* node, const char* name);

/**
 * Marks a node.
 * @param[in] node - the node
 * @param[in] mark - the node mark
 */
void HLNode_setMark(HL_Node* node, const HL_NodeMark mark);

/**
 * Returns the mark of a node.
 * @param[in] node - the node
 * @return the node mark.
 */
HL_NodeMark HLNode_getMark(HL_Node* node);

/**
 * Returns if the data for this node has been extracted from the hdf5 file or not.
 * Useful if implementing for example lazy-loading or similar features.
 * @param[in] node - the node
 * @return 1 if data has been fetched from HDF5 file, otherwise 0
 */
int HLNode_fetched(HL_Node* node);

/**
 * Sets if this node has got its data fetched or not.
 * @param[in] node - the node
 * @param[in] fetched - 1 if data fetched, otherwise 0
 */
void HLNode_setFetched(HL_Node* node, int fetched);


/**
 * Gets the type of the node
 * @param[in] node the node
 * @return the type of the node
 */
HL_Type HLNode_getType(HL_Node* node);

/**
 * Gets the string represenation of the data format.
 * @param[in] node the node
 * @return the format specifier or the string representation for HLHDF_UNDEFINED
 */
const char* HLNode_getFormatName(HL_Node* node);

/**
 * Returns the format specifier for the node
 * @param[in] node the node
 * @return the format specifier
 */
HL_FormatSpecifier HLNode_getFormat(HL_Node* node);

/**
 * Sets the data type of the node
 * @param[in] node the node
 * @param[in] datatype the datatype
 * @return the data type of the node
 */
void HLNode_setDataType(HL_Node* node, HL_DataType datatype);

/**
 * Returns the data type of the node
 * @param[in] node the node
 * @return the data type of the node
 */
HL_DataType HLNode_getDataType(HL_Node* node);

/**
 * Sets the node dimensions.
 * @param[in] node the node
 * @param[in] ndims the rank
 * @param[in] dims the dimensions
 * @return 1 on success, otherwise 0
 */
int HLNode_setDimensions(HL_Node* node, int ndims, hsize_t* dims);

/**
 * Gets the node dimensions
 * @param[in] node the node
 * @param[out] ndims the rank
 * @param[out] dims the dimensions
 */
void HLNode_getDimensions(HL_Node* node, int* ndims, hsize_t** dims);

/**
 * Returns the rank (number of dimensions).
 * @param[in] node the node
 * @return the rank.
 */
int HLNode_getRank(HL_Node* node);

/**
 * Returns the dimension of the specified index.
 * @param[in] node the node
 * @param[in] index the index
 * @return the size of the specified index. (If index < 0 or >= the rank then 0 is returned).
 */
hsize_t HLNode_getDimension(HL_Node* node, int index);

/**
 * Returns the number of value points that this node has.
 * If rank == 0, then 1 is returned.
 * If rank > 0 and dims is set, then 1*dims[0]*...*dims[rank-1] is returned.
 * If rank > 0 and dims == NULL, which not should happen, but then 0 is returned
 * @param[in] node the node
 * @return the number of value points.
 */
hsize_t HLNode_getNumberOfPoints(HL_Node* node);

/**
 * Sets the description for this node.
 * @param[in] node the node
 * @param[in] descr the compound description <b>(memory taken over so do not release it)</b>
 */
void HLNode_setCompoundDescription(HL_Node* node, HL_CompoundTypeDescription* descr);

/**
 * Returns the compound description for the node (if any).
 * @param[in] node the node
 * @return the compound description if any, otherwise NULL. (<b>Do not release since it points to internal memory</b>
 */
HL_CompoundTypeDescription* HLNode_getCompoundDescription(HL_Node* node);

/**
 * Returns the compression object for this node.
 * @param[in] node the node
 * @return the compression object (<b>points to internal memory so do not release</b>).
 */
HL_Compression* HLNode_getCompression(HL_Node* node);

/**
 * Sets the compression object for this node.
 * @param[in] node the node
 * @param[in] compression the compression object (<b>HL_Node takes responsibility so do not free after call</b>).
 */
void HLNode_setCompression(HL_Node* node, HL_Compression* compression);

/**
 * Will mark the node to be committed, only applicable on type nodes.
 * @ingroup hlhdf_c_apis
 * @param[in] node the node that should be commited
 * @param[in] typid the HDF5 type identifier
 * @return 1 on success, otherwise 0
 * @deprecated this function will be removed sooner or later.
 */
int HLNode_commitType(HL_Node* node,hid_t typid);

#endif
