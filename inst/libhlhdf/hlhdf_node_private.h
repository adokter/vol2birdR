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
 * Private functions for working with HL_Node's.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-24
 */
#ifndef HLHDF_NODE_PRIVATE_H
#define HLHDF_NODE_PRIVATE_H

/**
 * Sets data and datasize in the node. When this function has been called,
 * responsibility for the data has been taken over so do not release that memory.
 * @param[in] node the node (MAY NOT BE NULL)
 * @param[in] datasize the size of the data type as get by H5Tget_size.
 * @param[in] data the data (<b>responsibility taken over so do not release after call</b>).
 */
void HLNodePrivate_setData(HL_Node* node, size_t datasize, unsigned char* data);

/**
 * Sets rawdata and rawdatasize in the node. When this function has been called,
 * responsibility for the data has been taken over so do not release that memory.
 * @param[in] node the node (MAY NOT BE NULL)
 * @param[in] datasize the size of the data type as get by H5Tget_size.
 * @param[in] data the data (<b>responsibility taken over so do not release after call</b>).
 */
void HLNodePrivate_setRawdata(HL_Node* node, size_t datasize, unsigned char* data);

/**
 * Copies the typid and sets it in the node and also atempts to derive
 * the format name.
 * @param[in] node the node
 * @param[in] typid the type identifier
 * @return 1 on success, otherwise 0.
 */
int HLNodePrivate_setTypeIdAndDeriveFormat(HL_Node* node, hid_t typid);

/**
 * Sets the HDF identifier.
 * @param[in] node the node
 * @param[in] hdfid the hdf id. (<b>The responsibility is taken over so do not delete</b>)
 */
void HLNodePrivate_setHdfID(HL_Node* node, hid_t hdfid);

/**
 * Returns the HDF identifier.
 * @param[in] node the node
 * @return the hdf identifier (<b>Do not close</b>).
 */
hid_t HLNodePrivate_getHdfID(HL_Node* node);

/**
 * Returns an internal pointer to the dimensions.
 * @param[in] node the node
 * @return the internal dimension pointer (<b>Do not free and be careful when using it</b>).
 */
const hsize_t* HLNodePrivate_getDims(HL_Node* node);

/**
 * Returns the internal type id.
 * @param[in] node the  node
 * @return the internal type identifier (<b>Do not close</b>).
 */
hid_t HLNodePrivate_getTypeId(HL_Node* node);

#endif /* HLHDF_NODE_PRIVATE_H */
