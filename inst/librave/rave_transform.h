/**
 * Transformation routines.
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-08-05
 */
#ifndef RAVE_TRANSFORM_H
#define RAVE_TRANSFORM_H

#include "rave_types.h"
#include "rave_proj.h"

/**
 * Data structure when working with 2D projections
 */
typedef struct {
   PJ* inpj;                /**< Source projection */
   double inxscale;         /**< Source x-scale */
   double inyscale;         /**< Source y-scale */
   int inxmax;              /**< Source x-dimension */
   int inymax;              /**< Source y-dimension */
   UV inUL;                 /**< Source upper-left corner */

   double nodata;           /**< No data value */
   double noecho;           /**< No echo value */

   PJ* outpj;               /**< Destination projection */
   double outxscale;        /**< Destination x-scale */
   double outyscale;        /**< Destination y-scale */
   UV outUL;                /**< Destination upper-left corner */

   double R;                /**< Radius for transformation, used in cressman */
   RaveTransformationMethod method;              /**< Transformation method */

   unsigned char* data;     /**< Source data */
   RaveDataType type;       /**< Source data type */
   int stride_xsize;        /**< Source stride for length of one step y-wise, i.e. y*stride_xsize + x */

} RaveTransform2D;

/**
 * Weight for one item
 */
typedef struct {
   int x;             /**< the x position */
   int y;             /**< the y position */
   double value;      /**< the value */
   double weight;     /**< the weight */
   double distance;   /**< the distance to the origin */
} RaveWeight2D;

/**
 * All weights that should be used for calculating a value of a position
 */
typedef struct {
   RaveWeight2D* weights;  /**< all weights */
   int weightsn;           /**< Number of weights */
   double total_wsum;      /**< the total weight sum */
   double nodata;          /**< nodata value */
   double noecho;          /**< noecho value */
   int scale_weights;      /**< If the weights should be normalized */
} TransformWeight;

/**
 * Defines one field in a volume
 */
typedef struct {
   unsigned char* src; /**< The data array */
   RaveDataType type;  /**< The type of data */
   double elev;        /**< The elevation */
   int stride;         /**< The stride */
} RavePolarField;

/**
 * Defines one polar volume
 */
typedef struct {
   double beamwidth;          /**< Beamwidth */
   double nodata;             /**< No data */
   double noecho;             /**< No echo */
   double lon;                /**< Longitude for the radar */
   double lat;                /**< Latitude for the radar */
   double geo_height;         /**< Height of the radar */

   int azim_uppb;             /**< Number of beans */
   int range_uppb;            /**< Number of range bins*/
   double azimuth_offset;     /**< Azimutal difference between two nearby beans */
   double scale;              /**< Resolution for the beans */

   double cressmanxy;         /**< xy value used in cressman */
   double cressmanz;          /**< z value used in cressman */
   int got_cressmanxy;        /**< if cressmanxy is set */
   int got_cressmanz;         /**< if cressmanz is set */

   int fieldsn;               /**< number of fields */
   RavePolarField* fields;    /**< The array of fields */
} RavePolarVolume;

/**
 * Gets the value at position x,y from the data field.
 * @param[in] data the data array
 * @param[in] x the x position
 * @param[in] y the y position
 * @param[in] type the type of the data
 * @param[in] stride_xsize the stride
 */
double get_array_item_2d(
  unsigned char* data, int x, int y, RaveDataType type, int stride_xsize);

/**
 * Sets the value at position x,y in the data field.
 * @param[in] data the data array
 * @param[in] x the x position
 * @param[in] y the y position
 * @param[in] v the value that will be converted into the specified type
 * @param[in] type the type of the data
 * @param[in] stride_xsize the stride
 */
void set_array_item_2d(
  unsigned char* data, int x, int y, double v, RaveDataType type, int stride_xsize);

/**
 * Gets a value from a polar volume.
 * @param[in] pvol the polar volume
 * @param[in] e the elevation
 * @param[in] r the range bin
 * @param[in] a the azimuth
 * @return the value at the specified position
 */
double get_array_item_3d(RavePolarVolume* pvol, int e, int r, int a);

/**
 * Sets a value in a polar volume.
 * @param[in] pvol the polar volume
 * @param[in] e the elevation
 * @param[in] r the range bin
 * @param[in] a the azimuth
 * @param[in] v the value to be set
 */
void set_array_item_3d(RavePolarVolume* pvol, int e, int r, int a, double v);

/**
 * Creates a TransformWeight struct with noi number of weights allocated.
 * @param[in] noi the number of weights to be allocated
 * @return a TransformWeight struct with an array of weights allocated
 */
TransformWeight* init_tw(int noi);

/**
 * Deallocates the whole TransformWeight struct v, including
 * the weights allocated.
 * @param[in] v the structure to be deallocated
 */
void free_tw(TransformWeight* v);

/**
 * Gets the weight for the pixel closest to here_s and .
 * @param[in] x position in the out projection (only used for debugging)
 * @param[in] y position in the out projection (only used for debugging)
 * @param[in] here_s the pixel
 * @param[in] tw the transformation structure
 * @return the weight(s)
 */
TransformWeight* get_nearest_weights_2d(int x, int y, UV here_s, RaveTransform2D* tw);

/**
 * Gets the bilinear weights for the pixel surrounding here_s.
 * @param[in] x position in the out projection (only used for debugging)
 * @param[in] y position in the out projection (only used for debugging)
 * @param[in] here_s the pixel
 * @param[in] tw the transformation structure
 * @return the weight(s)
 */
TransformWeight* get_bilinear_weights_2d(int x, int y, UV here_s, RaveTransform2D *tw);

/**
 * Gets the cubic weights for the pixel surrounding here_s.
 * @param[in] x position in the out projection (only used for debugging)
 * @param[in] y position in the out projection (only used for debugging)
 * @param[in] here_s the pixel
 * @param[in] tw the transformation structure
 * @return the weight(s)
 */
TransformWeight* get_cubic_weights_2d(int x, int y, UV here_s, RaveTransform2D *tw);

/**
 * Gets the cressman weights for the pixel surrounding here_s.
 * @param[in] x position in the out projection (only used for debugging)
 * @param[in] y position in the out projection (only used for debugging)
 * @param[in] here_s the pixel
 * @param[in] tw the transformation structure
 * @return the weight(s)
 */
TransformWeight* get_cressman_weights_2d(int x, int y, UV here_s, RaveTransform2D* tw);

/**
 * Gets the appropriate weights for the current algorithm and position.
 *
 * @param[in] x position in the out projection (only used for debugging)
 * @param[in] y position in the out projection (only used for debugging)
 * @param[in] here_s the pixel
 * @param[in] tw the transformation structure.
 * @return the weight(s)
 */
TransformWeight* get_weights_2d(int x, int y, UV here_s, RaveTransform2D* tw);

/**
 * Evaluates the weights specified in the TransformWeight struct,
 * the weights will be normalized within the function by dividing the
 * given weight with the total_wsum attribute in the TransformWeight struct.
 * @param[in] tw the transformation weight(s)
 * @return the computed value
 */
double compute_weights_2d(TransformWeight* tw);

#endif /* RAVE_TRANSFORM_H */
