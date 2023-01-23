/* --------------------------------------------------------------------
Copyright (C) 2012 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Adaptor for VP ODIM H5 files.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2012-08-30
 */
#ifndef VP_ODIM_IO_H
#define VP_ODIM_IO_H
#include "rave_object.h"
#include "hlhdf.h"
#include "vertical_profile.h"
#include "lazy_nodelist_reader.h"

/**
 * Defines the odim h5 adaptor for vp products
 */
typedef struct _VpOdimIO_t VpOdimIO_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType VpOdimIO_TYPE;

/**
 * Sets the version that this io class should handle.
 * @param[in] self - self
 * @param[in] version - the odim version
 */
void VpOdimIO_setVersion(VpOdimIO_t* self, RaveIO_ODIM_Version version);

/**
 * Returns the version that this io class handles.
 * @param[in] self - self
 * @returns - the odim version
 */
RaveIO_ODIM_Version VpOdimIO_getVersion(VpOdimIO_t* self);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] self - self
 * @param[in] strict - if writing should be performed strictly or not
 */
void VpOdimIO_setStrict(VpOdimIO_t* self, int strict);

/**
 * If writing should be done strictly. From ODIM H5 2.4 several how-attributes are mandatory. If
 * any of these are missing and strict is set to true, then the writing will fail.
 * @param[in] self - self
 * @returns if writing should be performed strictly or not
 */
int VpOdimIO_isStrict(VpOdimIO_t* self);

/**
 * If an error occurs during writing, you might get an indication for why
 * by checking the error message.
 * @param[in] self - self
 * @returns the error message (will be an empty string if nothing to report).
 */
const char* VpOdimIO_getErrorMessage(VpOdimIO_t* self);

/**
 * Reads a vp from the nodelist and sets the data in the vp.
 * @param[in] self - self
 * @param[in] nodelist - the hdf5 node list
 * @param[in] vp - the vertical profile that should get the attribute and data set
 * @returns 1 on success otherwise 0
 */
int VpOdimIO_read(VpOdimIO_t* self, LazyNodeListReader_t* lazyReader, VerticalProfile_t* vp);

/**
 * Fills a nodelist with information about a vertical profile.
 * @param[in] self - self
 * @param[in] vp - the vertical profile
 * @param[in] nodelist - the hlhdf nodelist to fill
 * @return 1 on success otherwise 0
 */
int VpOdimIO_fill(VpOdimIO_t* self, VerticalProfile_t* vp, HL_NodeList* nodelist);

/**
 * Validates a vertical profile according to strictness and version in order to verify if it contains necessary information.
 * @param[in] self - self
 * @param[in] vp - the vertical profile to validate
 * @return 1 if valid otherwise 0
 */
int VpOdimIO_validateVpHowAttributes(VpOdimIO_t* self, VerticalProfile_t* vp);

#endif
