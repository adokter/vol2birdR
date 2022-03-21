#ifndef PROJ_WKT_HELPER_H
#define PROJ_WKT_HELPER_H

#include "raveobject_list.h"
#include "rave_attribute.h"
#include "projection.h"

/**
 * Takes a Projection_t and tries to translate it into a list of WKT-compatible attributes.
 * For example, if the projection definition is:
 * +proj=merc +lat_ts=0 +lon_0=0 +k_0=1.0 +R=6378137.0 +no_defs
 *
 * The returned object list will be containing the following \ref #RaveAttribut_t.
 * grid_mapping_name = mercator (string)
 * standard_parallel = 0 (double)
 * scale_factor_at_projection_origin = 0
 * earth_radius = 6378137.0
 *
 * @param[in] projection - the projecton to translate
 * @returns the translated attributes in WKT format. If no translation could be done a NULL list will be returned
 *
 */
RaveObjectList_t* RaveWkt_translate_from_projection(Projection_t* projection);

#endif
