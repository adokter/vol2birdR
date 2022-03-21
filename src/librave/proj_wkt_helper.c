#include "proj_wkt_helper.h"
#include <string.h>
#include <stdio.h>

/**
 * Mapping containing a wkt mapping function.
 */
typedef struct RaveWktMappingFunction {
  RaveObjectList_t*(*proj_identifier_fun)(Projection_t*, const char* projid, const char* wktname);
  const char* projid;
  const char* wktname;
} RaveWktMappingFunction;

/**
 * Mapping between a proj.4 definition id and a wkt id. For example
 * defid = +lat_0 and wktid = latitude_of_projection_origin. The definition can be found in Table F.1. Grid Mapping Attributes in
 * CF conventions 1.7:  http://cfconventions.org/Data/cf-conventions/cf-conventions-1.7/build/apf.html
 */
typedef struct Proj4DefIdToWktIdMapping {
  const char* defid;
  const char* wktid;
} Proj4DefIdToWktIdMapping;

/**
 * Locates a defid inside the string str assuming that the format of the id might be for example
 * +lat_0, +R etc. Then it assumes that these id:s are followed by a = sign and a value that
 * can be translated into a double.
 * @param[in] str - the string to search in
 * @param[in] defid - the PROJ.4 str id, like +lat_0, +R, ...
 * @param[out] v - the found value
 * @returns 1 on success otherwise 0
 */
static int RaveWkt_getDoubleFromString(const char* str, const char* defid, double* v)
{
  int result = 0;
  char* ptr = strstr(str, defid);
  char found[512];
  if (ptr != NULL) {
    char* optr = ptr + strlen(defid) + 1; /* 1 for = sign */
    char* eptr = strpbrk(optr, " \t");
    size_t len = strlen(optr);
    double dvalue = 0.0;
    if (eptr != NULL) {
      len = (eptr - optr + 1);
    }
    strncpy(found, optr, len);
    found[len] = '\0';
    if (sscanf(found, "%lf", &dvalue) == 1) {
      *v = dvalue;
      result = 1;
    }
  }
  return result;
}

/**
 * Adds a double value (RaveAttribute_t double) from a proj.4 definition str id.
 * @param[in] str - the proj.4 definition str
 * @param[in] attrname - the name of the rave attribute
 * @param[in] defid - the wanted id, for example +lon_0
 * @param[in] objlist - the object list that will get the found attribute added.
 * @return 1 on success, 0 will only be returned if the id was found but not could be added to the object list.
 */
static int RaveWkt_addDoubleAttributeToList(const char* str, const char* attrname, const char* defid, RaveObjectList_t* objlist)
{
  int result = 1; /* Default is always ok */
  double dvalue = 0.0;
  if (RaveWkt_getDoubleFromString(str, defid, &dvalue)) {
    RaveAttribute_t* attr = RaveAttributeHelp_createDouble(attrname, dvalue);
    result = RaveObjectList_add(objlist, (RaveCoreObject*)attr);
    RAVE_OBJECT_RELEASE(attr);
  }
  return result;
}

/**
 * Tries to locate a standard_paralell definition in a proj.4 definition string. The standard_paralell
 * is defined by +lat_1 (and eventually +lat_2). If only lat_1 is found, then the attribute added to the
 * object list will be a Double, if both lat_1 and lat_2 is found it will instead be a double array.
 * @param[in] str - the proj.4 definition string
 * @param[in] objlist - the object list that the found attribute should be added to.
 * @return 1 on success, 0 will be only returned if standard_parallel was found but not could be added to the object list.
 */
static int RaveWkt_addStandardParalellToList(const char* str, RaveObjectList_t* objlist)
{
  double latv1=0.0, latv2=0.0;
  double dv[2];
  RaveAttribute_t* attr = NULL;
  int result = 1; /* Default is always ok */
  if (RaveWkt_getDoubleFromString(str, "+lat_ts", &latv1)) {
    attr = RaveAttributeHelp_createDouble("standard_parallel", latv1);
    result = RaveObjectList_add(objlist, (RaveCoreObject*)attr);
  } else if (RaveWkt_getDoubleFromString(str, "+lat_1", &latv1)) {
     dv[0] = latv1;
     if (RaveWkt_getDoubleFromString(str, "+lat_2", &latv2)) {
       dv[1] = latv2;
       attr = RaveAttributeHelp_createDoubleArray("standard_parallel", dv, 2);
     } else {
       attr = RaveAttributeHelp_createDouble("standard_parallel", dv[0]);
     }
     result = RaveObjectList_add(objlist, (RaveCoreObject*)attr);
  }
  RAVE_OBJECT_RELEASE(attr);
  return result;
}

static RaveObjectList_t* RaveWkt_translate_from_projection_with_ids(Projection_t* projection, const char* projid, const char* wktname, Proj4DefIdToWktIdMapping* defmap, size_t nDefmap)
{
  const char* str = Projection_getDefinition(projection);
  RaveObjectList_t* result = NULL;
  if (projid != NULL && wktname != NULL && strstr(str, projid)) {
    RaveAttribute_t* attr = RaveAttributeHelp_createString("grid_mapping_name", wktname);
    size_t i = 0;
    result = RAVE_OBJECT_NEW(&RaveObjectList_TYPE);
    RaveObjectList_add(result, (RaveCoreObject*)attr);
    for (i = 0; i < nDefmap; i++) {
      if (strcmp(defmap[i].wktid, "standard_paralell")==0) {
        RaveWkt_addStandardParalellToList(str, result);
      } else {
        RaveWkt_addDoubleAttributeToList(str, defmap[i].wktid, defmap[i].defid, result);
      }
    }
    RAVE_OBJECT_RELEASE(attr);
  }
  return result;
}

/**
 *  Albers Equal-Area Conic.
 */
static RaveObjectList_t* albers_equal_area_conic(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping aeaConicMapping[6] = {
      {"+lon_0", "longitude_of_central_meridian"},
      {"+lat_0", "latitude_of_projection_origin"},
      {"+x_0",   "false_easting"},
      {"+y_0",   "false_northing"},
      {"+R",     "earth_radius"},
      {"",       "standard_paralell"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=aea", "albers_conical_equal_area", aeaConicMapping, 6);
}

/**
 *  Azimuthal equidistant
 */
static RaveObjectList_t* azimuthal_equidistant(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping aeqdMapping[5] = {
      {"+lon_0", "longitude_of_projection_origin"},
      {"+lat_0", "latitude_of_projection_origin"},
      {"+x_0",   "false_easting"},
      {"+y_0",   "false_northing"},
      {"+R",     "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=aeqd", "azimuthal_equidistant", aeqdMapping, 5);
}

/**
 *  Lambert azimuthal equal area
 */
static RaveObjectList_t* lambert_azimuthal_equal_area(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping laeaMapping[5] = {
      {"+lon_0", "longitude_of_projection_origin"},
      {"+lat_0", "latitude_of_projection_origin"},
      {"+x_0",   "false_easting"},
      {"+y_0",   "false_northing"},
      {"+R",     "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=laea", "lambert_azimuthal_equal_area", laeaMapping, 5);
}

/**
 *  Lambert conformal
 */
static RaveObjectList_t* lambert_conformal_conic(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping lccMapping[6] = {
      {"+lon_0", "longitude_of_central_meridian"},
      {"+lat_0", "latitude_of_projection_origin"},
      {"+x_0",   "false_easting"},
      {"+y_0",   "false_northing"},
      {"+R",     "earth_radius"},
      {"",       "standard_paralell"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=lcc", "lambert_conformal_conic", lccMapping, 6);
}

/**
 *  Lambert cylindrical equal area
 */

static RaveObjectList_t* lambert_cylindrical_equal_area(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping lceaMapping[6] = {
      {"+lon_0",  "longitude_of_central_meridian"},
      {"+k_0",    "scale_factor_at_projection_origin"},
      {"",        "standard_paralell"},
      {"+x_0",    "false_easting"},
      {"+y_0",    "false_northing"},
      {"+R",      "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=lcea", "lambert_cylindrical_equal_area", lceaMapping, 6);
}

/**
 *  Mercator
 */

static RaveObjectList_t* mercator(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping mercatorMapping[6] = {
      {"+lon_0",  "longitude_of_projection_origin"},
      {"+k_0",    "scale_factor_at_projection_origin"},
      {"",        "standard_paralell"},
      {"+x_0",    "false_easting"},
      {"+y_0",    "false_northing"},
      {"+R",      "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=merc", "mercator", mercatorMapping, 6);
}

/**
 * Ortographic
 */
static RaveObjectList_t* orthographic(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping orthographicMapping[5] = {
      {"+lon_0",  "longitude_of_projection_origin"},
      {"+lat_0",  "latitude_of_projection_origin"},
      {"+x_0",    "false_easting"},
      {"+y_0",    "false_northing"},
      {"+R",      "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=ortho", "orthographic", orthographicMapping, 5);
}

/**
 * Polar stereographic
 */
static RaveObjectList_t* polar_stereographic(Projection_t* projection, const char* projid, const char* wktname)
{
  Proj4DefIdToWktIdMapping psMapping[7] = {
      {"+lon_0",  "straight_vertical_longitude_from_pole"},
      {"+lat_0",  "latitude_of_projection_origin"},
      {"",        "standard_paralell"},
      {"+k_0",    "scale_factor_at_projection_origin"},
      {"+x_0",    "false_easting"},
      {"+y_0",    "false_northing"},
      {"+R",      "earth_radius"}
  };
  return RaveWkt_translate_from_projection_with_ids(projection, "+proj=stere", "polar_stereographic", psMapping, 7);
}

/**
 * All supported projection mappings. The two other parameters defid, mapping_name will be passed to the
 * specified identififier function. Useful when specifying a mapping with the general lookup function.
 */
static RaveWktMappingFunction RAVE_WKT_MAPPINGS[] = {
    {albers_equal_area_conic,        "+proj=aea",    "albers_conical_equal_area"},
    {azimuthal_equidistant,          "+proj=aeqd",   "azimuthal_equidistant"},
    {lambert_azimuthal_equal_area,   "+proj=laea",   "lambert_azimuthal_equal_area"},
    {lambert_conformal_conic,        "+proj=lcc",    "lambert_conformal_conic"},
    {lambert_cylindrical_equal_area, "+proj=lcea",   "lambert_cylindrical_equal_area"},
    {mercator,                       "+proj=merc",   "mercator"},
    {orthographic,                   "+proj=ortho",  "orthographic"},
    {polar_stereographic,            "+proj=stere",  "polar_stereographic"},
    /* Need to add support for
     * grid_mapping_name = latitude_longitude
     * grid_mapping_name = rotated_latitude_longitude
     * grid_mapping_name = stereographic
     * grid_mapping_name = transverse_mercator
     * grid_mapping_name = vertical_perspective
     *
     * http://cfconventions.org/Data/cf-conventions/cf-conventions-1.7/build/apf.html
     * http://geotiff.maptools.org/proj_list/
     */
    {NULL, NULL, NULL}
};

RaveObjectList_t* RaveWkt_translate_from_projection(Projection_t* projection)
{
  RaveObjectList_t* result = NULL;
  int i = 0;
  while (RAVE_WKT_MAPPINGS[i].proj_identifier_fun != NULL && result == NULL) {
    result = RAVE_WKT_MAPPINGS[i].proj_identifier_fun(projection, RAVE_WKT_MAPPINGS[i].projid, RAVE_WKT_MAPPINGS[i].wktname);
    i++;
  }
  return result;
}

