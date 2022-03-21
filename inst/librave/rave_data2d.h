/* --------------------------------------------------------------------
Copyright (C) 2009 Swedish Meteorological and Hydrological Institute, SMHI,

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
 * Represents a 2-dimensional data array.
 * This object supports \ref #RAVE_OBJECT_CLONE.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-12-17
 */
#ifndef RAVE_DATA2D_H
#define RAVE_DATA2D_H
#include "rave_object.h"
#include "rave_types.h"

/**
 * Defines a Rave 2-dimensional data array
 */
typedef struct _RaveData2D_t RaveData2D_t;

/**
 * Type definition to use when creating a rave object.
 */
extern RaveCoreObjectType RaveData2D_TYPE;

/**
 * Sets if the operations should take into account nodata value or not.
 * Default is turned off
 * @param[in] self - self
 * @param[in] use - 1 if nodata controls should be performed, otherwise it's turned off
 */
void RaveData2D_useNodata(RaveData2D_t* self, int use);

/**
 * Returns if the calculations are taking into account the nodata value
 * @param[in] self - self
 * @returns if nodata value is taken into account when performing the operations
 */
int RaveData2D_usingNodata(RaveData2D_t* self);

/**
 * @param[in] self - self
 * @param[in] nodata - the nodata value to use.
 */
void RaveData2D_setNodata(RaveData2D_t* self, double nodata);

/**
 * @param[in] self - self
 * @returns the nodata value
 */
double RaveData2D_getNodata(RaveData2D_t* self);

/**
 * Returns the xsize
 * @param[in] self - self
 * @return the xsize
 */
long RaveData2D_getXsize(RaveData2D_t* self);

/**
 * Returns the ysize
 * @param[in] self - self
 * @return the ysize
 */
long RaveData2D_getYsize(RaveData2D_t* self);

/**
 * Returns the data type
 * @param[in] self - self
 * @return the data type
 */
RaveDataType RaveData2D_getType(RaveData2D_t* self);

/**
 * Returns a pointer to the internal data storage.
 * @param[in] self - self
 * @return the internal data pointer (NOTE! Do not release this pointer)
 */
void* RaveData2D_getData(RaveData2D_t* self);

/**
 * Sets the data.
 * @param[in] self  - self
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] data  - the data
 * @param[in] type  - the data type
 * @returns 1 on success, otherwise 0
 */
int RaveData2D_setData(RaveData2D_t* self, long xsize, long ysize, void* data, RaveDataType type);

/**
 * Creates a data field with the specified size and type.
 * @param[in] self  - self
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] type  - the data type
 * @param[in] value - initial value to set for all positions in the data field
 * @returns 1 on success, otherwise 0
 */
int RaveData2D_createData(RaveData2D_t* self, long xsize, long ysize, RaveDataType type, double value);

/**
 * Fills the field with the specified value
 * @param[in] self - self
 * @param[in] v - the value to fill with
 * @returns 1 on success, otherwise 0
 */
int RaveData2D_fill(RaveData2D_t* self, double v);

/**
 * Sets the value at the specified coordinates.
 * @param[in] self - self
 * @param[in] x - the x-position
 * @param[in] y - the y-position
 * @param[in] v - the value to set
 * @return 1 if the value could be set, otherwise 0
 */
int RaveData2D_setValue(RaveData2D_t* self, long x, long y, double v);

/**
 * Same as RaveData2D_setValue but there is no boundary checking performed.
 * I.e. unless you know what you are doing you might be accessing unreserved memory.
 * @param[in] self - self
 * @param[in] x - the x-position
 * @param[in] y - the y-position
 * @param[in] v - the value to set
 * @return 1 if the value could be set, otherwise 0
 */
int RaveData2D_setValueUnchecked(RaveData2D_t* self, long x, long y, double v);

/**
 * Returns the value at the specified x and y position. If coordinates is outside
 * the boundaries, v will be left as is so initialize it before calling this function.
 * @param[in] self - self
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return 1 if the value could be extracted and returned, otherwise 0
 */
int RaveData2D_getValue(RaveData2D_t* self, long x, long y, double* v);

/**
 * Same as RaveData2D_getValue but there is no boundary checking performed.
 * I.e. unless you know what you are doing you might be accessing unreserved memory.
 * @param[in] self - self
 * @param[in] x - the x index
 * @param[in] y - the y index
 * @param[out] v - the data at the specified index
 * @return 1 if the value could be extracted and returned, otherwise 0
 */
int RaveData2D_getValueUnchecked(RaveData2D_t* self, long x, long y, double* v);

/**
 * Returns if this object contains data and a xsize and ysize > 0.
 * @param[in] self - self
 * @returns 1 if this object contains data, otherwise 0.
 */
int RaveData2D_hasData(RaveData2D_t* self);

/**
 * Returns min in self
 * @param[in] self - self
 * @returns min value
 */
double RaveData2D_min(RaveData2D_t* self);

/**
 * Returns max in self
 * @param[in] self - self
 * @returns max value
 */
double RaveData2D_max(RaveData2D_t* self);

/**
 * Concatenates field with other horizontally and returns the new field.
 * The field's and other's y-dimensions must be the same as well as the data
 * type.
 * @param[in] field - self
 * @param[in] other - the field to contatenate
 * @returns the concatenated field on success otherwise NULL
 */
RaveData2D_t* RaveData2D_concatX(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Concatenates field with other vertically and returns the new field.
 * The field's and other's x-dimensions must be the same as well as the data
 * type.
 * @param[in] field - self
 * @param[in] other - the field to contatenate
 * @returns the concatenated field on success otherwise NULL
 */
RaveData2D_t* RaveData2D_concatY(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Circular shift of the field in x & y dimension.
 * @param[in] field - the field to be shifted
 * @param[in] nx - the number of steps to be shifted in x-direction. Can be both positive and negative
 * @param[in] ny - the number of steps to be shifted in y-direction. Can be both positive and negative
 * @returns the shifted field
 */
RaveData2D_t* RaveData2D_circshift(RaveData2D_t* field, int nx, int ny);

/**
 * Circular shift of the internal field in x & y dimension.
 * @param[in] field - the field to be shifted
 * @param[in] nx - the number of steps to be shifted in x-direction. Can be both positive and negative
 * @param[in] ny - the number of steps to be shifted in y-direction. Can be both positive and negative
 * @returns 1 if shift was successful otherwise 0
 */
int RaveData2D_circshiftData(RaveData2D_t* field, int nx, int ny);

/**
 * Adds the specified value to field and returns a new field
 * @param[in] field - self
 * @param[in] v - the value to add
 * @return the new field or NULL on error
 */
RaveData2D_t* RaveData2D_addNumber(RaveData2D_t* field, double v);

/**
 * Adds the array value to field and returns a new field. Other's x&y size must either be 1 or the same as selfs xsize/ysize.
 * @param[in] field - self
 * @param[in] other - the other array to add
 * @return the new field or NULL on error
 */
RaveData2D_t* RaveData2D_add(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Substracts the specified value from field and returns a new field
 * @param[in] field - self
 * @param[in] v - the value to substract
 * @return the new field or NULL on error
 */
RaveData2D_t* RaveData2D_subNumber(RaveData2D_t* field, double v);

/**
 * Substracts the array value from field and returns a new field. Other's x&y size must either be 1 or the same as selfs xsize/ysize.
 * @param[in] field - self
 * @param[in] other - the other array to substract
 * @return the new field or NULL on error
 */
RaveData2D_t* RaveData2D_sub(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Multiplicates all values in field with v and returns a new field
 * @param[in] field - self
 * @param[in] v - the value to multply with
 */
RaveData2D_t* RaveData2D_mulNumber(RaveData2D_t* field, double v);

/**
 * Element wise multiplication. I.e. not a real matrix multiplication but instead it
 * atempts to perform item wise multiplication row or columnwise. This means that others
 * x&y size must either be 1 or the same as selfs xsize/ysize.
 * @param[in] field - self
 * @param[in] other - the other array to use for multiplication
 */
RaveData2D_t* RaveData2D_emul(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Element wise pow on a matrix.
 * @param[in] field - self
 * @param[in] v - the pow value
 */
RaveData2D_t* RaveData2D_powNumber(RaveData2D_t* field, double v);

/**
 * Element wise pow on a matrix This means that others
 * x&y size must either be 1 or the same as selfs xsize/ysize.
 * @param[in] field - self
 * @param[in] other - the other array to use for multiplication
 */
RaveData2D_t* RaveData2D_epow(RaveData2D_t* field, RaveData2D_t* other);

/**
 * Executes a median filtering in 2D. I.e. takes surrounding pixels in winXsize, winYsize and determines the median value for each
 * pixel in the field. I.e. box will be x +/- winXsize/2, y +/- winYsize/2
 * @param[in] field - self
 * @param[in] winXsize - the number of pixels in x-direction
 * @param[in] winYsize - the number of pixels in y-direction
 * @returns the filtered data field
 */
RaveData2D_t* RaveData2D_medfilt2(RaveData2D_t* field, long winXsize, long winYsize);

/**
 * Runs a cummulative sum of either columns or rows. If dir == 1, then it's columnwise
 * sum. Else if dir == 2, then it's row based cummulative sum.
 * @param[in] field - self
 * @param[in] if it's goind to be column (1) or row (2) based cummulation.
 * @returns a new field with the cummulative sum
 */
RaveData2D_t* RaveData2D_cumsum(RaveData2D_t* field, int dir);

/**
 * Computes the standard deviation over a given number of pixels (nx * ny). If nx or ny = 0, it will be row or col wise std.
 * @param[in] field - self
 * @param[in] nx - number of pixels in x-dim
 * @param[in] ny - number of pixels in y-dim
 */
RaveData2D_t* RaveData2D_movingstd(RaveData2D_t* field, long nx, long ny);

/**
 * Creates a histogram of field with bins number of bins. The histogram will be determined as
 * Calculate bin ranges as scale = (max - min) / nbins
 * Bin1: max >= x <= max + scale
 * Bin2: max + scale > x <= max + 2*scale
 * Bin3: max + 2*scale > x <= max + 3*scale
 * and so on.....
 *
 * Example: y=[-7  -5  -3  -2  -2  -1   0   1   2   3   3   4   5   5   6   7]
 *          nbins = 4
 *
 * scale = (7 - (-7)) / 4 = 3.5
 * =>
 * b1 = -7 => -3.5
 * b2 = -3.49 => 0
 * b3 = 0.01 => 3.5
 * b4 = 3.51 => 7
 *
 * @param[in] field - self
 * @param[in] bins - number of bins
 * @param[in,out] nnodata - If using nodata, this will be number of nodata values since they won't be in the histogram counts.
 */
long* RaveData2D_hist(RaveData2D_t* field, int bins, long* nnodata);

/**
 * Calculate the entropy value for the field. Entropy is a statistical measure of randomness.
 * @param[in] field - self
 * @param[in] bins - the number of bins
 * @param[in,out] entropy - the calculated entropy
 * @returns 1 on success otherwise 0
 */
int RaveData2D_entropy(RaveData2D_t* field, int bins, double* entropy);

/**
 * Helper function to print field to stdout
 * @param[in] field - field
 */
void RaveData2D_disp(RaveData2D_t* field);

/**
 * Helper function that returns the matrix as a string. Note, this is not
 * reentrant since it's using a static buffer.
 * @param[in] field - self
 * @returns matrix as a string
 */
const char* RaveData2D_str(RaveData2D_t* field);

/**
 * Replaces all occurances of v with v2 in the provided data field
 * @param[in] field - self
 * @param[in] v - the value to search for
 * @param[in] v2 - the value that should be used as replacement
 */
void RaveData2D_replace(RaveData2D_t* field, double v, double v2);

/**
 * Utility function for creating a new rave data 2d object filled with zeros
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] type  - the data type
 * @returns The instance on success, otherwise NULL
 */
RaveData2D_t* RaveData2D_zeros(long xsize, long ysize, RaveDataType type);

/**
 * Utility function for creating a new rave data 2d object filled with ones
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] type  - the data type
 * @returns The instance on success, otherwise NULL
 */
RaveData2D_t* RaveData2D_ones(long xsize, long ysize, RaveDataType type);

/**
 * Utility function for creating a new rave data 2d object.
 * @param[in] xsize - x-size
 * @param[in] ysize - y-size
 * @param[in] type  - the data type
 * @returns The instance on success, otherwise NULL
 */
RaveData2D_t* RaveData2D_createObject(long xsize, long ysize, RaveDataType type);

#endif /* RAVE_DATA2D_H */
