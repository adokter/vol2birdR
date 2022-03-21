/*
 * lazy_nodelist_io.h
 *
 *  Created on: Nov 4, 2020
 *      Author: anders
 */

#ifndef LAZY_NODELIST_IO_H_
#define LAZY_NODELIST_IO_H_
#include "rave_object.h"
#include "rave_types.h"
#include "hlhdf_nodelist.h"
#include "hlhdf_node.h"
#include "rave_data2d.h"
#include "rave_alloc.h"
#include "rave_list.h"
#include "rave_attribute.h"

/**
 * Defines a LazyNodeListReader instance
 */
typedef struct _LazyNodeListReader_t LazyNodeListReader_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType LazyNodeListReader_TYPE;

/**
 * Sets up ownership of this nodelist. Creates a lazy node list reader object from a HL nodelist. NOTE! The LazyNodeListReader_t instance will
 * take over responsibility for memory so don't release the nodelist unless the init function returns 0 which means that it failed to
 * take hold of nodelist
 * @param[in] self - self
 * @param[in] nodelist - nodelist
 * @returns 1 on success otherwise 0
 */
int LazyNodeListReader_init(LazyNodeListReader_t* self, HL_NodeList* nodelist);

/**
 * Preloads everything that hasn't already been loaded.
 * @param[in] self
 * @returns 1 on success otherwise 0
 */
int LazyNodeListReader_preload(LazyNodeListReader_t* self);

/**
 * Preloads datasets according to:
 * + /datasetX/dataY/data that are paired with /datasetX/dataY/what/quantity that are in the list of quantities
 * + all datasets that doesn't have the above pairing.
 * @param[in] self - self
 * @param[in] quantities - a comma separated list of quantities that should be matched against. If quantities = NULL everything is read..
 * @returns 1 on success otherwise 0
 */
int LazyNodeListReader_preloadQuantities(LazyNodeListReader_t* self, const char* quantities);

/**
 * Gets a dataset as a rave data 2d field instance from loader.
 * @param[in] self - self
 * @param[in] datasetname - name of the dataset to be fetched
 * @returns the dataset as a rave data 2d type or NULL if it couldn't be found
 */
RaveData2D_t* LazyNodeListReader_getDataset(LazyNodeListReader_t* self, const char* datasetname);

/**
 * Gets an attribute instance from loader.
 * @param[in] self - self
 * @param[in] attributename - name of the attribute to be fetched
 * @returns the rave attribute or NULL if it couldn't be found
 */
RaveAttribute_t* LazyNodeListReader_getAttribute(LazyNodeListReader_t* self, const char* attributename);

/**
 * Checks if the specified node name has been loaded (fetched) into memory.
 * @param[in] self - self
 * @param[in] name - name of the node
 * @return 1 if it has been loaded, 0 otherwise
 */
int LazyNodeListReader_isLoaded(LazyNodeListReader_t* self, const char* name);

/**
 * Checks if the specified node name exists.
 * @param[in] self - self
 * @param[in] name - name of the node
 * @return 1 if it exists, 0 otherwise
 */
int LazyNodeListReader_exists(LazyNodeListReader_t* self, const char* name);

/**
 * Returns the internal pointer into self. NOTE. Do not release memory
 * @param[in] self - self
 * @returns the internal node list
 */
HL_NodeList* LazyNodeListReader_getHLNodeList(LazyNodeListReader_t* self);

/**
 * Returns all node names within the nodelist.
 * @param[in] self - self
 * @returns the node names
 */
RaveList_t* LazyNodeListReader_getNodeNames(LazyNodeListReader_t* self);

/**
 * Creates a lazy node list reader object from a HL nodelist. NOTE! The LazyNodeListReader_t instance will take over responsibility
 * for memory so don't release the nodelist unless returned instance is NULL.
 * @param[in] nodelist - the HL node list
 * @returns a lazy node list IO instance
 */
LazyNodeListReader_t* LazyNodeListReader_create(HL_NodeList* nodelist);


/**
 * Reads a hdf5 nodelist in lazy mode. I.e. only metadata is fetched. Other data is
 * fetched upon request. NOTE! If there are a lot of data that should be fetched, it might
 * be better to fetch everything immediately.
 * @param[in] filename - HDF5 file to read
 * @return a lazy node list io instance or NULL on failure
 */
LazyNodeListReader_t* LazyNodeListReader_read(const char* filename);

/**
 * Reads a hdf5 nodelist in preloaded mode. I.e. everything is loaded immediately.
 * @param[in] filename - HDF5 file to read
 * @return a preloaded node list io instance or NULL on failure
 */
LazyNodeListReader_t* LazyNodeListReader_readPreloaded(const char* filename);

#endif /* LAZY_NODELIST_IO_H_ */
