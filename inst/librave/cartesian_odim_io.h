/* --------------------------------------------------------------------
Copyright (C) 2010 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Adaptor for cartesian ODIM H5 files.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2010-09-09
 */
#ifndef CARTESIAN_ODIM_IO_H
#define CARTESIAN_ODIM_IO_H
#include "rave_object.h"
#include "hlhdf.h"
#include "cartesian.h"
#include "cartesianvolume.h"
#include "lazy_nodelist_reader.h"

/**
 * Defines the odim h5 adaptor for cartesian products
 */
typedef struct _CartesianOdimIO_t CartesianOdimIO_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType CartesianOdimIO_TYPE;

/**
 * Sets the version that this io class should handle.
 * @param[in] self - self
 * @param[in] version - the odim version
 */
void CartesianOdimIO_setVersion(CartesianOdimIO_t* self, RaveIO_ODIM_Version version);

/**
 * Returns the version that this io class handles.
 * @param[in] self - self
 * @returns - the odim version
 */
RaveIO_ODIM_Version CartesianOdimIO_getVersion(CartesianOdimIO_t* self);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] self - self
 * @param[in] strict - if writing should be performed strictly or not
 */
void CartesianOdimIO_setStrict(CartesianOdimIO_t* self, int strict);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] self - self
 * @returns if writing should be performed strictly or not
 */
int CartesianOdimIO_isStrict(CartesianOdimIO_t* self);

/**
 * If an error occurs during writing, you might get an indication for why
 * by checking the error message.
 * @param[in] self - self
 * @returns the error message (will be an empty string if nothing to report).
 */
const char* CartesianOdimIO_getErrorMessage(CartesianOdimIO_t* self);

/**
 * Reads a cartesian from the nodelist and sets the data in the cartesian.
 * @param[in] self - self
 * @param[in] lazyReader - the wrapper around the hdf5 node list
 * @param[in] cartesian - the cartesian that should get the attribute and data set
 * @returns 1 on success otherwise 0
 */
int CartesianOdimIO_readCartesian(CartesianOdimIO_t* self, LazyNodeListReader_t* lazyReader, Cartesian_t* cartesian);

/**
 * Reads a volume from the nodelist and sets the data in the volume.
 * @param[in] self - self
 * @param[in] lazyReader - the wrapper around the hdf5 node list
 * @param[in] volume - the volume that should get the attribute and data set
 * @returns 1 on success otherwise 0
 */
int CartesianOdimIO_readVolume(CartesianOdimIO_t* self, LazyNodeListReader_t* lazyReader, CartesianVolume_t* volume);

/**
 * Fills a HL nodelist with information from a cartesian product.
 * @param[in] self - self
 * @param[in] nodelist - the node list
 * @param[in] cartesian - the cartesian product
 * @returns 1 on success, 0 otherwise
 */
int CartesianOdimIO_fillImage(CartesianOdimIO_t* self, HL_NodeList* nodelist, Cartesian_t* cartesian);

/**
 * Fills a HL nodelist with information from a cartesian volume.
 * @param[in] self - self
 * @param[in] nodelist - the node list
 * @param[in] volume - the cartesian volume
 * @returns 1 on success, 0 otherwise
 */
int CartesianOdimIO_fillVolume(CartesianOdimIO_t* self, HL_NodeList* nodelist, CartesianVolume_t* volume);


/**
 * Validates an image in order to verify if it contains necessary information
 * for writing.
 * @param[in] self - self
 * @param[in] cartesian - the cartesian product to validate
 * @param[in] msg - message array to get indication what is wrong
 * @param[in] maxlen - max length of message array
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidImageAddMsg(Cartesian_t* cartesian, char* msg, int maxlen);

/**
 * Validates an image in order to verify if it contains necessary information
 * for writing.
 * @param[in] self - self
 * @param[in] cartesian - the cartesian product to validate
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidImage(Cartesian_t* cartesian);

/**
 * Validates an image belonging to a volume in order to verify if it
 * contains necessary information for writing.
 * @param[in] self - self
 * @param[in] cartesian - the cartesian product to validate
 * @param[in] msg - message array to get indication what is wrong
 * @param[in] maxlen - max length of message array
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidVolumeImageAddMsg(Cartesian_t* cartesian, char* msg, int maxlen);

/**
 * Validates an image belonging to a volume in order to verify if it
 * contains necessary information for writing.
 * @param[in] self - self
 * @param[in] cartesian - the cartesian product to validate
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidVolumeImage(Cartesian_t* cartesian);

/**
 * Validates an volume in order to verify if it contains necessary information
 * for writing.
 * @param[in] self - self
 * @param[in] volume - the volume to validate
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidVolumeAddMsg(CartesianVolume_t* volume, char* msg, int maxlen);

/**
 * Validates an volume in order to verify if it contains necessary information
 * for writing.
 * @param[in] self - self
 * @param[in] volume - the volume to validate
 * @param[in] msg - message array to get indication what is wrong
 * @param[in] maxlen - max length of message array
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_isValidVolume(CartesianVolume_t* volume);

/**
 * Validates a volume according to strictness and version in order to verify if it contains necessary information.
 * @param[in] self - self
 * @param[in] volume - the volume to validate
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_validateVolumeHowAttributes(CartesianOdimIO_t* self, CartesianVolume_t* volume);

/**
 * Validates an image according to strictness and version in order to verify if it contains necessary information.
 * @param[in] self - self
 * @param[in] image - the image to validate
 * @return 1 if valid otherwise 0
 */
int CartesianOdimIO_validateCartesianHowAttributes(CartesianOdimIO_t* self, Cartesian_t* image);

#endif /* CARTESIAN_ODIM_IO_H */
