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
 * Common functions for working with an HDF5 file through the HL-HDF API.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-10
 */
#ifndef HLHDF_H
#define HLHDF_H
#include <hdf5.h>
#include "hlhdf_types.h"
#include "hlhdf_node.h"
#include "hlhdf_nodelist.h"
#include "hlhdf_read.h"
#include "hlhdf_write.h"
#include "hlhdf_compound.h"

/**
 * Define for FALSE unless it already has been defined.
 */
#ifndef FALSE
#define FALSE 0
#endif

/**
 * Define for TRUE unless it already has been defined.
 */
#ifndef TRUE
#define TRUE 1
#endif

/**
 * @defgroup hlhdf_c_apis HLHDF C-API Reference Manual
 * This is the currently provided APIs available in C. This group
 * is a subset of all defined functions and if more information on
 * all source code is wanted, please refer to the files section of
 * the documentation.
 */

/**
 * Disables error reporting.
 * @ingroup hlhdf_c_apis
 */
void HL_disableErrorReporting(void);

/**
 * Enables error reporting
 * @ingroup hlhdf_c_apis
 */
void HL_enableErrorReporting(void);


/**
 * Returns if error reporting is enabled or not.
 * @ingroup hlhdf_c_apis
 */
int HL_isErrorReportingEnabled(void);

/**
 * Initializes the HLHDF handler functions.
 * <b>This always needs to be done before doing anything else when using HLHDF.</b>
 * @ingroup hlhdf_c_apis
 */
void HL_init(void);

/**
 * Toggles the debug mode for HLHDF. Possible values of flag are:
 * <ul>
 *   <li>0 = No debugging</li>
 *   <li>1 = Debug only the HLHDF library</li>
 *   <li>2 = Debug both HLHDF and HDF5 library</li>
 * </ul>
 * @ingroup hlhdf_c_apis
 * @param[in] flag the level of debugging
 */
void HL_setDebugMode(int flag);

/**
 * Verifies if the provided filename is a valid HDF5 file or not.
 * @ingroup hlhdf_c_apis
 * @param[in] filename the full path of the file to check
 * @return TRUE if file is an HDF5 file, otherwise FALSE
 */
int HL_isHDF5File(const char* filename);

/**
 * Creates a file property instance that can be passed on to createHlHdfFile when
 * creating a HDF5 file.
 * @ingroup hlhdf_c_apis
 * @return the allocated file property instance, NULL on failure. See @ref HLFileCreationProperty_free for deallocation.
 */
HL_FileCreationProperty* HLFileCreationProperty_new(void);

/**
 * Deallocates the HL_FileCreationProperty instance.
 * @ingroup hlhdf_c_apis
 * @param[in] prop The property to be deallocated
 */
void HLFileCreationProperty_free(HL_FileCreationProperty* prop);

/**
 * Calculates the size in bytes of the provided @ref ValidFormatSpecifiers "format specifiers".
 * The exception is string and compound type since they needs to be analyzed to get the size.
 * @ingroup hlhdf_c_apis
 * @param[in] format The @ref ValidFormatSpecifiers "format specifier".
 * @return the size in bytes or -1 on failure.
 */
int HL_sizeOfFormat(const char* format);

/**
 * Verifies if the format name is supported by HLHDF.
 * Does not see string and compound as a supported format since they are not
 * possible to determine without a hid.
 * @ingroup hlhdf_c_apis
 * @param[in] format the format name string
 * @return TRUE if it is supported, otherwise FALSE.
 */
int HL_isFormatSupported(const char* format);

/**
 * Returns the format specifier for the provided format string.
 * @param[in] format the format name
 * @return the format specifier. HLHDF_UNDEFINED is also returned on error
 */
HL_FormatSpecifier HL_getFormatSpecifier(const char* format);

/**
 * Returns the string representation of the provided specifier.
 * @param[in] specifier - the specifier
 * @return NULL if not a valid specifier, otherwise the format.
 */
const char* HL_getFormatSpecifierString(HL_FormatSpecifier specifier);

/**
 * Returns the version that was used for building this binary.
 * @return the version string.
 */
const char* HL_getHDF5Version(void);

/**
 * Creates an allocated and initialized instance of HL_Compression.
 * @ingroup hlhdf_c_apis
 * @param[in] type the compression type to use
 * @return the created instance, NULL on failure.
 */
HL_Compression* HLCompression_new(HL_CompressionType type);

/**
 * Creates a copy of the provided HL_Compression instance.
 * @ingroup hlhdf_c_apis
 * @param[in] inv the instance to be cloned
 * @return the cloned instance or NULL if parameter was NULL or memory not could be allocated.
 */
HL_Compression* HLCompression_clone(HL_Compression* inv);

/**
 * Initializes a HL_Compressiosn instance.
 * @ingroup hlhdf_c_apis
 * @param[in] inv The compression object to be initialized
 * @param[in] type the type of compression the object should be initialized with
 */
void HLCompression_init(HL_Compression* inv,HL_CompressionType type);

/**
 * Deallocates the HL_Compression instance.
 * @ingroup hlhdf_c_apis
 * @param inv The instance that should be deallocated
 */
void HLCompression_free(HL_Compression* inv);

#endif
