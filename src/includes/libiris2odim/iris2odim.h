/* --------------------------------------------------------------------
Copyright (C) 2015 The Crown (i.e. Her Majesty the Queen in Right of Canada)

This file is an add-on to RAVE.

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
 * Description: These structures and macro definitions are used in 
 * iris2odim.c and iris2odim_main.c to read an "IRIS RAW" radar data 
 * file and write the data to an ODIM-H5 formatted file. Part of 
 * program iris2odim.
 * @file iris2odim.h
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-10-16
 */
#ifndef IRIS2ODIM_H
#define IRIS2ODIM_H
#include "irisdlist.h"
#include "iris2list_sigmet.h"
#include "iris2list_listobj.h"
#include "rave_alloc.h"
#include "rave_types.h"
#include "rave_io.h"
#include "rave_object.h"
#include "rave_attribute.h"
#include "polarscanparam.h"
#include "polarscan.h"
#include "polarvolume.h"
#define MY_FILL_DOUBLE (9.9692099683868690e+36)  /* From netCDF */
# define MY_ABS(x)    ((x) < 0 ? -(x) : (x))  /* used in conjunction with the relative difference function reldif */
# define MY_MAX(a, b) ((a) > (b) ? (a) : (b))  /* used in conjunction with the relative difference function reldif */
# define TOLERENCE (0.000001) /* used in conjunction with the relative difference function reldif */

/**
 * Typedef for print function
 */
typedef void(*iris_printfun)(const char* msg);

/**
 * The default printf function routing prints to stderr unless -DIRIS_NO_EXIT_OR_STDERR has been specified.
 */
void Iris_default_printf(const char* msg);

/**
 * Sets the print function to where printouts should be done. Default behaviour is to use
 * Iris_default_printf.
 * @param[in] fun - the default printer
 */
void Iris_set_printf(iris_printfun fun);

/**
 * Wraps exit into it's own function to be able to disable hard exit when used in app.
 * Will either return code or do a hard exit depending on if -DIRIS_NO_EXIT_OR_STDERR has
 * been defind or not.
 * @param[in] return_code - the return value for this function if IRIS_NO_EXIT_OR_STDERR has been defined.
 * @param[in] exit_code - the exit code used to call exit if IRIS_NO_EXIT_OR_STDERR hasn't been defined.
 * Output:
 * @return the return_code
 */
int Iris_exit_function(int return_code, int exit_code);

/**
 * Wraps fprintf into it's own function to be able to disable printouts depending on if DIRIS_NO_EXIT_OR_STDERR has been set or not
 * @param[in] fmt - a string containing the output format
 * @param[in] ... - variable argument list
 */
void Iris_printf(const char* fmt, ...);

/**
 *
 * @param[in] ifile - a string containing the input IRIS file name;
 * @param[in] file_element_pp - an empty file_element_s structure filled by this function
 * @return signed integer status indicator (zero means success)
 */
int iris2list(const char* ifile,
              file_element_s **file_element_pp);

/**
* Populates a polar scan "parameter" (aka moment, quantity, variable).
* @param[in] param - polar scan parameter to be populated
* @param[in] this_datatype_structure - structure holding ray data and ingest_data_header
* @param[in] file_element_p - file_element_s structure (a filled radar volume struct) 
* @param[out] ra_p - an pointer to a structure (allocated in function) holding "ray attributes" related arrays 
*                   for a single sweep (allocated and populated in this function when obtain_ra is true)
* @param[in] obtain_ra - integer/logical fills ra_p when TRUE 
*                        (obtained once for each sweep, on first 'datatype')
* @return signed integer status indicator (zero means success)
*/
int populateParam(PolarScanParam_t *param, 
                  datatype_element_s *this_datatype_structure,
                  file_element_s* file_element_p,
                  ra_s **ra_p, 
                  int obtain_ra);

/**
* Populates a polar scan (aka sweep).
* @param[in] scan - polar scan to be populated
* @param[in] file_element_p - file_element_s structure (a filled radar volume struct) 
* @param[in] sweep_index - index of scan being filled (origin = 1)
* @return signed integer status indicator (zero means success)
*/
int populateScan(PolarScan_t* scan, 
                 file_element_s* file_element_p, 
                 int sweep_index);

/**
* Populates a Toolbox object, which can be either a polar volume or polar scan (aka sweep).
* @param[in] object - Toolbox core object, containing either a polar volume or a polar scan, to be populated
* @param[in] *file_element_p - file_element_s structure (a filled iris-radar volume struct) 
* @return signed integer status indicator (zero means success)
*/
int populateObject(RaveCoreObject* object, file_element_s *file_element_p);

/**
 * Reads an IRIS file into memory.
 * @param[in] ifile - input IRIS file string
 * it is NULL.
 * @return file_element_s* - IRIS payload in memory
 */
file_element_s* readIRIS(const char* ifile);

/**
 * Determines the Rave_ObjectType from the IRIS payload.
 * @param[in] **file_element_pp - structure representing the IRIS file contents
 * @return int - Rave_ObjectType_PVOL, Rave_ObjectType_SCAN upon success, or Rave_ObjectType_UNDEFINED upon failure
 */
int objectTypeFromIRIS(file_element_s* file_element_p);

/**
 * Frees memory used to represent IRIS payload
 * @param[in] **file_element_pp - structure representing the IRIS file contents
 */
void free_IRIS(file_element_s **file_element_pp);

/**
 * Determines whether the given path is to a regular file.
 * @param[in] file string to query
 * @return integer 1 on success, 0 on failure
 */
int is_regular_file(const char *path);

/**
 * Determines if the given file is intended to be in IRIS format.
 * @param[in] file string to query
 * @return integer - 0 on success, -1 on failure
 */
int isIRIS(const char *path);

/**
* Helper function : reads ray acquisition angles and times from IRIS and maps to equivalent ODIM 1-D arrays.
* @param[in] scan - polar scan to which to write the ray header metadata
* @param[in] file_element_p - file_element_s structure (a filled iris-radar volume struct) 
* @param[in] cci_p - consistency-check-info structure
* @param[in] sweep_index - index of scan being filled (origin = 1)
* @param[in] ra_pp - a pointer to a pointer to a structure holding "ray attributes" related arrays  
* @return signed integer status indicator (zero means success)
*/
int setRayAttributes(PolarScan_t* scan,
                     file_element_s* file_element_p,
                     cci_s *cci_p,
                     int sweep_index, 
                     ra_s **ra_pp);

/**
* Helper function : adds a long integer attribute to an ODIM object.
* @param[in] object - Toolbox core polar object, either a polar volume or polar scan
* @param[in] name - string containing the ODIM attribute name, e.g. something starting with 'how/'
* @param[in] value - long integer value
* @return signed integer status indicator (zero means success)
*/
int addLongAttribute(RaveCoreObject* object, const char* name, long value);

/**
* Helper function : adds a double-precision floating point attribute to an ODIM object.
* @param[in] object - Toolbox core polar object, either a polar volume or polar scan
* @param[in] name - string containing the ODIM attribute name, e.g. something starting with 'how/'
* @param[in] value - double-precision floating point value
* @return signed integer status indicator (zero means success)
*/
int addDoubleAttribute(RaveCoreObject* object, const char* name, double value);

/**
 * Helper function : adds a string attribute to an ODIM object.
 * @param[in] object - Toolbox core polar object, either a polar volume or polar scan
 * @param[in] name - string containing the ODIM attribute name, e.g. something starting with 'how/'
 * @param[in] value - string (unterminated)
 * @return signed integer status indicator (zero means success)
 */
int addStringAttribute(RaveCoreObject* object, const char* name, const char* value);

/**
 * Helper function : actually adds the attribute to the Toolbox object.
 * @param[in] object - Toolbox core polar object, either a polar volume or polar scan
 * @param[in] attr - Toolbox Attribute object, containing the attribute value (long, double, or string)
 * @return signed integer status indicator (zero means success)
 */
int addAttribute(RaveCoreObject* object, RaveAttribute_t* attr);

/**
 * Helper function : provides ODIM "/what/source" attribute for a given set of radars, based on their IRIS site identifiers.
 * This is a primitive formulation but it works.
 * @param[in] key - string containing the IRIS site identifier
 * @return string containing the /what/source for that radar
 */
char* mapSource2Nod(const char* key);

/**
 * Helper function : provides ODIM Quantity to set for PolarScanParam, based on their IRIS moment-id index
 * Not all IRIS moment IDs have a matching ODIM quantity.
 * @param[in] irisType - index of an IRIS moment/data_type identifier
 * @return literal/string containing an ODIM Quantity
 */
char* mapDataType(int irisType);

/**
 * Helper function : trims all spaces to the right of alphanumeric characters in string 
 * @param[in] str - string to modify
 * @return nothing
 */
void rtrim(char *str);

/**
 * Helper function : trims all spaces to the left of alphanumeric characters in string 
 * @param[in] str - string to modify
 * @return nothing
 */
void ltrim(char *str);

/**
 * Helper function : trims all spaces to the left and right of alphanumeric characters in string 
 * @param[in] str - string to modify
 * @return nothing
 */
void rltrim(char *str);

/**
 * Helper function : allocates a structure and all array members
 * @param[in] nsweeps - number of sweeps/scans in volume
 * @return an unfilled cci_s structure
 */
cci_s *create_consistency_check_arrays(size_t nsweeps);

/**
 * Helper function : dallocates a structure and all array members, exits with pointer pointing to NULL
 * @param[in] cci_p - consistency-check-info structure to deallocate
 * @param[in] nsweeps - number of sweeps/scans in volume
 * @return nothing
 */
void destroy_consistency_check_arrays(cci_s *cci_p,
                                        size_t nsweeps);
/**
 * Helper function : writes data obtained from a ymd_s date/time structure to an mtv_s date/time structure
 * @param[in] ymd_p - a structure containing gregorian date and time info
 * @return a structure containing seconds from midnight Jan011970 and integral number of microseconds
 */
mtv_s *ymd_to_mtv(ymd_s *ymd_p);

/**
 * Helper function : fills a cci_s structure with assorted non-trivial info from an iris volume scan
 * @param[in] cci_p - consistency-check-info structure to populate
 * @param[in] nsweeps - number of sweeps/scans in volume
 * @param[in] file_element_p - file_element_s structure (a filled iris-radar volume struct) 
 * @return nothing
 */
void do_consistency_check(cci_s *cci_p,
                          size_t nsweeps,
                          file_element_s *file_element_p);

/**
 * Helper function : writes data obtained from a mtv_s date/time structure to an ymd_s date/time structure
 * @param[in] mtv_p - a structure containing seconds from midnight Jan011970 and integral number of microseconds
 * @return a structure containing gregorian date and time info
 */
ymd_s *mtv_to_ymd( mtv_s *mtv_p );


/**
 * Helper function : calculates the nyquist frequency of a radar pulse train (iris scans are identical for entire volume)
 * @param[in] file_element_p - file_element_s structure (a filled iris-radar volume struct) 
 * @return a single number that is the nyquist frequency 
 */
double calc_nyquist(file_element_s *file_element_p);

/**
 * Helper function : computes an index based on start and end Azimuth angles in degrees 
 * @param[in] rhd_p - pointer to a rhd_s structure (a filled iris-radar ray-header struct)
 * @param[in] angular_resolution_degrees - number of angular degrees swept for pulses comprising a ray
 * @return a long integer holding a ray index (index starts at 0)
 */
long compute_ray_index( rhd_s *this_ray_header,
                        double angular_resolution_degrees, 
                        long max_index);

/**
 * Helper function : It returns the relative difference of two real numbers: 
 * 0.0 if they are exactly the same, otherwise the ratio of the difference to the larger of the two. 
 * @param[in] a - first floating-point-double in comparison
 * @param[in] b - second floating-point-double in comparison
 * @return a floating-point-double value (the relative difference)
 */
double reldif(double a, double b);


#endif
