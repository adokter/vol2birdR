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
 * Type definitions that are used in HLHDF.
 *
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-06-10
 */
#ifndef HLHDF_TYPES_H
#define HLHDF_TYPES_H

#include <hdf5.h>

/**
 * @defgroup ValidFormatSpecifiers Valid format specifiers
 * All format specifiers are passed on as constant strings. HLHDF is always atempting to work
 * with native formats which means that what is written might not be interpreeted back to
 * the same format. For example, if a 'char' is written, it might be a 'schar' that is returned
 * or if a 'llong' is written, it might actually be written as a 'long.
 *
 * Use the function \ref HL_getFormatSpecifierString to get a string representation of the string.
 *
 * Valid character strings are:
 * <ul>
 *  <li>'char'</li>
 *  <li>'schar'</li>
 *  <li>'uchar'</li>
 *  <li>'short'</li>
 *  <li>'ushort'</li>
 *  <li>'int'</li>
 *  <li>'uint'</li>
 *  <li>'long'</li>
 *  <li>'ulong'</li>
 *  <li>'llong'</li>
 *  <li>'ullong'</li>
 *  <li>'float'</li>
 *  <li>'double'</li>
 *  <li>'ldouble'</li>
 *  <li>'hsize'</li>
 *  <li>'hssize'</li>
 *  <li>'herr'</li>
 *  <li>'hbool'</li>
 *  <li>'string'</li>
 *  <li>'compound'</li>
 * </ul>
 */
/*@{*/
/**
 * These are all valid format specifiers that are used within HLHDF. HLHDF_ARRAY is not possible
 * to use for writing and is only used when reading HDF5 files with array content.
 */
typedef enum HL_FormatSpecifier {
  HLHDF_UNDEFINED = 0,  /**< 'UNDEFINED' If no format has been specified, this string will be returned */
  HLHDF_CHAR,           /**< 'char' */
  HLHDF_SCHAR,          /**< 'schar' */
  HLHDF_UCHAR,          /**< 'uchar' */
  HLHDF_SHORT,          /**< 'short' */
  HLHDF_USHORT,         /**< 'ushort' */
  HLHDF_INT,            /**< 'int' */
  HLHDF_UINT,           /**< 'uint' */
  HLHDF_LONG,           /**< 'long' */
  HLHDF_ULONG,          /**< 'ulong' */
  HLHDF_LLONG,          /**< 'llong' */
  HLHDF_ULLONG,         /**< 'ullong' */
  HLHDF_FLOAT,          /**< 'float' */
  HLHDF_DOUBLE,         /**< 'double' */
  HLHDF_LDOUBLE,        /**< 'ldouble' */
  HLHDF_HSIZE,          /**< 'hsize' */
  HLHDF_HSSIZE,         /**< 'hssize' */
  HLHDF_HERR,           /**< 'herr' */
  HLHDF_HBOOL,          /**< 'hbool' */
  HLHDF_STRING,         /**< 'string' */
  HLHDF_COMPOUND,       /**< 'compound' */
  HLHDF_ARRAY,          /**< 'array' This is only something that will be read but is not possible to write */
  HLHDF_END_OF_SPECIFIERS
} HL_FormatSpecifier;

/*@}*/

/**
 * Defines what type of compression that should be used
 * @ingroup hlhdf_c_apis
 *  */
typedef enum HL_CompressionType {
  CT_NONE=0, /**< No compression */
  CT_ZLIB,   /**< ZLIB compression */
  CT_SZLIB   /**< SZLIB compression */
} HL_CompressionType;

/**
 * See hdf5 documentation for H5Pget_version for purpose
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   unsigned super; /**< Super block version number */
   unsigned freelist; /**< Global freelist version number */
   unsigned stab; /**< Symbol table version number */
   unsigned shhdr; /**< Shared object header version number */
} HL_PropertyVersion;

/**
 * See hdf5 documentation for H5Pset_sizes and H5Pget_sizes for purpose
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   size_t sizeof_addr; /**< Size of an object offset in bytes */
   size_t sizeof_size; /**< Size of an object length in bytes */
} HL_PropertySize;

/**
 * See hdf5 documentation for H5Pset_sym_k and H5Pget_sym_k for purpose
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   unsigned ik; /**< Symbol table tree rank. */
   unsigned lk; /**< Symbol table node size. */
} HL_PropertySymK;

/**
 * Properties that can be finely tuned when creating a HDF5 file.
 * @ingroup hlhdf_c_apis
 */
typedef struct {
  /**
   * See @ref #HL_PropertyVersion
   */
  HL_PropertyVersion version;

  /**
   * See hdf5 documentation for H5Pset_userblock and H5Pget_userblock for purpose
   */
  hsize_t userblock;

  /**
   * See @ref #HL_PropertySize
   */
  HL_PropertySize sizes;

  /**
   * See @ref #HL_PropertySymK
   */
  HL_PropertySymK sym_k;

  /**
   * See hdf5 documentation for H5Pset_istore_k and H5Pget_istore_k for purpose
   */
  unsigned istore_k;

  /** This is actually in the File access properties but atm it feels like overkill
   * to provide the user with functionality to fine tune all these variables
   * since its only the meta_block_size we have found any use for.
   * If the value of meta_block_size is 2048, then we are using default value
   * and the default FILE_ACCESS_PROPERTY will be used.
   */
  hsize_t meta_block_size;

} HL_FileCreationProperty;

/**
 * Compression properties.
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   /**
    * The wanted compression type.
    * If @ref HL_CompressionType#CT_ZLIB is specified, then level needs to be set.
    * if @ref HL_CompressionType#CT_SZLIB is specified, then szlib_mask and szlib_px_per_block needs to be set.
    */
   HL_CompressionType type;

   /**
    * The compression level when using ZLIB compression,
    * compression is indicated by values between 1-9, if set
    * to 0 this will not be seen as compression */
   int level;

   /**
    * Mask when using szlib compression, the mask can be set up from two
    * different sets of options.
    * <table>
    * <tr><td>H5_SZIP_CHIP_OPTION_MASK</td><td>Compresses exactly as in hardware.</td></tr>
    * <tr><td>H5_SZIP_ALLOW_K13_OPTION_MASK</td><td>Allows k split = 13 compression mode. (Default)</td></tr>
    * <tr><td>&nbsp;</td><td>&nbsp;</td></tr>
    * <tr><td>H5_SZIP_EC_OPTION_MASK</td><td>Selects entropy coding method. (Default)</td></tr>
    * <tr><td>H5_SZIP_NN_OPTION_MASK</td><td>Selects nearest neighbor coding method.</td></tr>
    * </table>
    * Where the paired options are mutual exclusive, i.e. it is possible
    * to set the szlib_mask to H5_SZIP_CHIP_OPTION_MASK|H5_SZIP_EC_OPTION_MASK
    * but not to H5_SZIP_CHIP_OPTION_MASK|H5_SZIP_ALLOW_K13_OPTION_MASK
    */
   unsigned int szlib_mask;

   /**
    * The block size must be be even, with typical values being 8, 10, 16, and 32.
    * The more pixel values vary, the smaller this number should be.
    */
   unsigned int szlib_px_per_block;
} HL_Compression;

/**
 * This is an enumeration variable designed to identify the type of a given node.
 * @ingroup hlhdf_c_apis
 */
typedef enum HL_Type {
  UNDEFINED_ID=-1, /**< An undefined type */
  ATTRIBUTE_ID=0,  /**< Attribute type (corresponds to H5A) */
  GROUP_ID,        /**< Group type (corresponds to H5G) */
  DATASET_ID,      /**< Data set type (corresponds to H5D) */
  TYPE_ID,         /**< Type type (corresponds to H5T) */
  REFERENCE_ID     /**< Reference type (corresponds to H5R) */
} HL_Type;

/**
 * This is an enumeration variable designed to identify the type of data in a given node.
 * When new nodes are initiated, they are initialized with DTYPE_UNDEFINED_ID.
 * @ingroup hlhdf_c_apis
 */
typedef enum HL_DataType {
  DTYPE_UNDEFINED_ID=-1, /**< Undefined data type */
  HL_SIMPLE=0,           /**< If the value is a scalar */
  HL_ARRAY               /**< If the value is of array type */
} HL_DataType;

/**
 * This is an enumeration variable designed to keep track of the status of a given node.
 * @ingroup hlhdf_c_apis
 */
typedef enum HL_NodeMark {
  NMARK_UNDEFINED=-1, /**< Undefined type */
  NMARK_ORIGINAL=0, /**< Nothing has been done on the node, e.g. it has been read but nothing else */
  NMARK_CREATED,    /**< If a node has been created but not been written */
  NMARK_CHANGED,    /**< If a nodes value has been changed and needs to be written */
  NMARK_SELECT,     /**< The node has been marked for fetching but the read has not been performed yet. */
  NMARK_SELECTMETA  /**< Special variant for marking datasets that we are only interested in the metadata */
} HL_NodeMark;

/**
 * This type is designed to describe an individual node with a complicated structure, ie.
 * one which consists of more than atomic data types. It contains all the information required
 * to interpret the contents of the node.
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   char attrname[256]; /**< name of the attribute */
   size_t offset;      /**< offset in the structure, use HOFFSET in HDF5 */
   size_t size;        /**< size of the data field */
   char format[256];   /**< format specifier, @ref ValidFormatSpecifiers "format specifier"*/
   int ndims;          /**< number of dimensions */
   size_t dims[4];     /**< dimensions, max 4 */
} HL_CompoundTypeAttribute;

/**
 * This type is a list of <b>HL_CompoundTypeAttribute</b>s. The reason why it's
 * called "Description" is that it acts more like meta data than actual
 * data.
 * @ingroup hlhdf_c_apis
 */
typedef struct {
   char hltypename[256];    /**< this types name if any */
   unsigned long objno[2];  /**< the unique identifier for this type */
   size_t size;             /**< the size of this type */
   int nAttrs;              /**< the number of attributes defining this type */
   int nAllocAttrs;         /**< the number of allocated attributes */
   HL_CompoundTypeAttribute** attrs; /**< points at the different attributes that defines this type, max index is always nAttrs-1 */
} HL_CompoundTypeDescription;

/**
 * Each entry and type in a HDF5 file is represented by a HL_Node.
 * @ingroup hlhdf_c_apis
 */
typedef struct _HL_Node HL_Node;

/**
 * Represents a HDF5 file.
 * @ingroup hlhdf_c_apis
 */
typedef struct _HL_NodeList HL_NodeList;

#endif
