/* --------------------------------------------------------------------
Copyright (C) 2009, 2011 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Functions for performing rave related IO operations, mostly ODIM-formatted HDF5 files.
 * This object does NOT support \ref #RAVE_OBJECT_CLONE.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-11-12
 */
#ifndef RAVE_IO_H
#define RAVE_IO_H

#include "rave_object.h"
#include "rave_types.h"

/**
 * The file format of the data that has been read.
 * When reading a file through rave io, you might sometime
 * read exotic formats like BUFR. This type provides information
 * on what type of format we found it to be.
 */
typedef enum RaveIO_ODIM_FileFormat {
  RaveIO_ODIM_FileFormat_UNDEFINED = -1, /**< undefined */
  RaveIO_ODIM_FileFormat_HDF5 = 0,       /**< HDF 5 */
  RaveIO_ODIM_FileFormat_BUFR = 1,       /**< BUFR */
  RaveIO_FileFormat_CF = 2               /** CF Conventions / Radial */
} RaveIO_ODIM_FileFormat;

/**
 * Defines a Rave IO instance
 */
typedef struct _RaveIO_t RaveIO_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveIO_TYPE;

/**
 * Closes the HDF5 file but will keep the RaveIO instance.
 * @param[in] raveio - the rave IO instance
 */
void RaveIO_close(RaveIO_t* raveio);

/**
 * Opens a supported HDF5 file and loads it into the RaveIO instance.
 * Same as:
 * RaveIO_t* instance = RAVE_OBJECT_NEW(&RaveIO_TYPE);
 * RaveIO_setFilename(instance, filename);
 * RaveIO_load(instance);
 *
 * @param[in] filename - the HDF5 file to open
 * @param[in] lazyLoading - if file should be loaded in lazy mode or not
 * @param[in] preloadQuantities - if lazy loading, then these quantities will be loaded immediately.
 * @returns The raveio instance on success, otherwise NULL.
 */
RaveIO_t* RaveIO_open(const char* filename, int lazyLoading, const char* preloadQuantities);

/**
 * Loads the HDF5 file into the raveio instance.
 * @param[in] raveio - self
 * @param[in] lazyLoading - if file should be loaded in lazy mode or not
 * @param[in] preloadQuantities - if lazy loading, then these quantities will be loaded immediately.*
 * @returns the opened object
 */
int RaveIO_load(RaveIO_t* raveio, int lazyLoading, const char* preloadQuantities);

/**
 * Saves a rave object as specified according to ODIM HDF5 format specification.
 * @param[in] raveio - self
 * @param[in] filename - the filename to save with. May be NULL, same as calling RaveIO_setFilename followed by RaveIO_save(..., NULL)
 * @returns 1 on success, otherwise 0
 */
int RaveIO_save(RaveIO_t* raveio, const char* filename);

/**
 * Sets the object to be saved.
 * @param[in] raveio - self
 * @param[in] object - the object to be saved
 */
void RaveIO_setObject(RaveIO_t* raveio, RaveCoreObject* object);

/**
 * Returns the loaded object/object to be saved.
 * @param[in] raveio - self
 * @returns the object
 */
RaveCoreObject* RaveIO_getObject(RaveIO_t* raveio);

/**
 * Sets the filename that should be used when saving the object.
 * @param[in] raveio - self
 * @param[in] filename - the filename that should be used when saving.
 * @returns 1 on success, otherwise 0
 */
int RaveIO_setFilename(RaveIO_t* raveio, const char* filename);

/**
 * Returns the current filename.
 * @param[in] raveio - self
 * @returns the current filename
 */
const char* RaveIO_getFilename(RaveIO_t* raveio);

/**
 * Returns the object type for the currently opened file. Requires that
 * a RaveCoreObject has been set.
 * @param[in] raveio - the Rave IO instance
 * @returns the object type or Rave_ObjectType_UNDEFINED on error.
 */
Rave_ObjectType RaveIO_getObjectType(RaveIO_t* raveio);

/**
 * Sets the ODIM version to use when saving the file.
 * @param[in] raveio - self
 * @param[in] version - the version to be used
 * @returns 1 if the specified version is supported, otherwise 0.
 */
int RaveIO_setOdimVersion(RaveIO_t* raveio, RaveIO_ODIM_Version version);

/**
 * Returns the ODIM version that will be used to write the file.
 * @param[in] raveio - self
 * @returns the ODIM version
 */
RaveIO_ODIM_Version RaveIO_getOdimVersion(RaveIO_t* raveio);

/**
 * Returns the ODIM version of the file that was read.
 * @param[in] raveio - self
 * @returns the ODIM version
 */
RaveIO_ODIM_Version RaveIO_getReadOdimVersion(RaveIO_t* raveio);

/**
 * Sets the ODIM h5rad version to use when saving the file. Currently, the only
 * supported version is 2.0.
 * @param[in] raveio - self
 * @param[in] version - the version to be used
 * @returns 1 if the specified version is supported, otherwise 0.
 */
int RaveIO_setH5radVersion(RaveIO_t* raveio, RaveIO_ODIM_H5rad_Version version);

/**
 * Returns the h5rad version.
 * @param[in] raveio - self
 * @returns the h5rad version
 */
RaveIO_ODIM_H5rad_Version RaveIO_getH5radVersion(RaveIO_t* raveio);

/**
 * Will return the file format that this file was read as. Note, it is currently
 * not possible to save data files in any other formats than HDF5.
 * @param[in] raveio - self
 * @return the file format.
 */
RaveIO_ODIM_FileFormat RaveIO_getFileFormat(RaveIO_t* raveio);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] raveio - self
 * @param[in] strict - if writing should be performed strictly or not
 */
void RaveIO_setStrict(RaveIO_t* raveio, int strict);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] raveio - self
 * @returns if writing should be performed strictly or not
 */
int RaveIO_isStrict(RaveIO_t* raveio);

/**
 * Sets what file format to use.
 * @param[in] raveio - self
 * @param[in] format - the file format to use
 * @returns 1 on success otherwise 0
 */
int RaveIO_setFileFormat(RaveIO_t* raveio, RaveIO_ODIM_FileFormat format);

/**
 * Sets the compression level.
 * @param[in] raveio - self
 * @param[in] lvl - the compression level (0..9)
 */
void RaveIO_setCompressionLevel(RaveIO_t* raveio, int lvl);

/**
 * Returns the compression level
 * @param[in] raveio- self
 * @returns the compression level
 */
int RaveIO_getCompressionLevel(RaveIO_t* raveio);

/**
 * Sets the user block.
 * @param[in] raveio - self
 * @param[in] userblock - the user block
 */
void RaveIO_setUserBlock(RaveIO_t* raveio, unsigned long long userblock);

/**
 * Returns the user block.
 * @param[in] raveio - self
 * @returns the user block
 */
unsigned long long RaveIO_getUserBlock(RaveIO_t* raveio);

/**
 * Sets the sizes
 * @param[in] raveio - self
 * @param[in] sz - same as sizes.sizeof_size
 * @param[in] addr - same as sizes.sizeof_addr
 */
void RaveIO_setSizes(RaveIO_t* raveio, size_t sz, size_t addr);

/**
 * Returns the sizes
 * @param[in] raveio - self
 * @param[in] sz - same as sizes.sizeof_size
 * @param[in] addr - same as sizes.sizeof_addr
 */
void RaveIO_getSizes(RaveIO_t* raveio, size_t* sz, size_t* addr);

/**
 * Sets the symk
 * @param[in] raveio - self
 * @param[in] ik - same as sym_k.ik
 * @param[in] lk - same as sym_k.lk
 */
void RaveIO_setSymk(RaveIO_t* raveio, int ik, int lk);

/**
 * Returns the symk
 * @param[in] raveio - self
 * @param[in] ik - same as sym_k.ik
 * @param[in] lk - same as sym_k.lk
 */
void RaveIO_getSymk(RaveIO_t* raveio, int* ik, int* lk);

/**
 * Sets the istore_k value.
 * @param[in] raveio - self
 * @param[in] k - the istore_k value
 */
void RaveIO_setIStoreK(RaveIO_t* raveio, long k);

/**
 * Returns the istore_k value
 * @param[in] raveio - self
 * @returns the istore_k value
 */
long RaveIO_getIStoreK(RaveIO_t* raveio);

/**
 * Sets the meta block size
 * @param[in] raveio - self
 * @param[in] sz - the meta block size
 */
void RaveIO_setMetaBlockSize(RaveIO_t* raveio, long sz);

/**
 * Returns the meta block size
 * @param[in] raveio - self
 * @returns the meta block size
 */
long RaveIO_getMetaBlockSize(RaveIO_t* raveio);

/**
 * Sets the bufr table directory to use when reading bufr files.
 * This function is only relevant if BUFR support has been enabled
 * otherwise it will just be a setter that isn't used.
 *
 * @param[in] raveio - self
 * @param[in] dname - the directory name
 * @return 1 on success otherwise 0
 */
int RaveIO_setBufrTableDir(RaveIO_t* raveio, const char* dname);

/**
 * Returns the bufr table directory.
 * @param[in] raveio - self
 * @return the bufr table directory
 */
const char* RaveIO_getBufrTableDir(RaveIO_t* raveio);

/**
 * If an error occurs during writing, you might get an indication for why
 * by checking the error message.
 * @param[in] raveio - rave io
 * @returns the error message (will be an empty string if nothing to report).
 */
const char* RaveIO_getErrorMessage(RaveIO_t* raveio);

/**
 * Returns if the raveio supports the provided file format.
 * @param[in] format - the inquiried file format
 * @return 1 if rave io supports the format, otherwise 0
 */
int RaveIO_supports(RaveIO_ODIM_FileFormat format);



#endif
