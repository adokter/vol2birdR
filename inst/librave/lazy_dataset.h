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
#ifndef LAZY_DATASET_H_
#define LAZY_DATASET_H_
#include "rave_object.h"
#include "rave_types.h"
#include "rave_data2d.h"
#include "rave_alloc.h"
#include "rave_list.h"
#include "rave_attribute.h"
#include "lazy_nodelist_reader.h"

/**
 * Defines a LazyNodeListReader instance
 */
typedef struct _LazyDataset_t LazyDataset_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType LazyDataset_TYPE;

/**
 * Initializes this object with the reader and the name of the node to fetch.
 * The nodename must exist within the available names in the reader otherwise initialization will fail.
 * @param[in] self - self
 * @param[in] reader - the node list reader
 * @param[in] nodename - the name of the node
 * @return 1 on success otherwise 0
 */
int LazyDataset_init(LazyDataset_t* self, LazyNodeListReader_t* reader, const char* nodename);

/**
 * Load the data upon request
 */
RaveData2D_t* LazyDataset_get(LazyDataset_t* self);

long LazyDataset_getXsize(LazyDataset_t* self);

long LazyDataset_getYsize(LazyDataset_t* self);

RaveDataType LazyDataset_getDataType(LazyDataset_t* self);

const char* LazyDataset_getNodeName(LazyDataset_t* self);
#endif /* LAZY_DATASET_H_ */
