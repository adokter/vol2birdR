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
 * iris2list.c to read "IRIS RAW" radar data files. Part of program 
 * iris2odim.
 * @file iris2list_listobj.h
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-11-25
 */
#ifndef IRIS2LIST_LISTOBJ_H
#define IRIS2LIST_LISTOBJ_H
#include <time.h>
#include "iris2list_sigmet.h"
#include "irisdlist.h"

// define structure ray_attributes and type ra_s
typedef struct ray_attributes {
   int expected_nrays;
   double *startazA;
   double *stopazA;
   double *startazT;
   double *stopazT;
   double *elangles; 
 } ra_s;

// define structure file_element and type file_element_s
typedef struct file_element {
  phd_s *product_header_p;  // pointer to a product header structure
  ihd_s *ingest_header_p;   // pointer to an ingest header structure
  IrisDList_t *sweep_list_p; // pointer to a doubly-linked list of sweep_element structures
} file_element_s;

  
// define the structure 'ray_header' and the type 'rhd_s'
typedef struct ray_header {
// Antenna positions at the start and end of the ray are stored as 16-bit binary
// angles.
  BIN2 azimuth_angle_at_beginning_of_ray;   // If dual-PRF then
                                            // bit0=Ray's PRF was high
  BIN2 elevation_angle_at_beginning_of_ray;     //If trigger blanking on then
                                            // bit0=Trigger was not blanked
  BIN2 azimuth_angle_at_end_of_ray;
  BIN2 elevation_angle_at_end_of_ray;
  SINT2 actual_number_of_bins_in_ray;
  UINT2 time_in_seconds_from_start_of_sweep;  // unsigned
} rhd_s;

// define the structure 'ray' and the type 'ray_s'
typedef struct ray {
  rhd_s ray_head;
  UINT1 ray_body[MAX_RAY_BODY_SIZE];
  UINT2 ray_body_size_in_bytes;
  UINT2 normal_ray_end;
  UINT2 abandon_ray;
  UINT2 abandon_buf;
} ray_s;

//define sweep_element structure and type sweep_element_s
typedef struct sweep_element {
  IrisDList_t *types_list_p; // pointer to a doubly-linked list of data types recorded (structures that include ray data)
} sweep_element_s;

// define structure 'IRISbuf' and type 'IRISbuf'
typedef struct IRISbuf {
  UINT1 bufIRIS[IRIS_BUFFER_SIZE];
  UINT2 bytesCopied;
  SINT2 errorInd;
  UINT2 numberSkipped;
} IRISbuf;

// define the structure 'rayplus' and the type 'rayplus_s'
typedef struct rayplus {
  sweep_element_s *new_sweep_element_p;
  ray_s *ray;
  UINT2 updated_offset;
  rpb_s *new_rpb_p;
  IRISbuf *new_IRISbuf_p;
} rayplus_s;

//define 'datatype_element' structure and type 'datatype_element_s'
typedef struct datatype_element {
  idh_s *ingest_data_header_p; // pointer to a single 'ingest data header structure'
  IrisDList_t *ray_list_p;          // pointer to a doubly-linked list of rays (of one datatype, from one sweep)
} datatype_element_s;

// define structure 'my_timeval' and type 'mtv_s'
typedef struct my_timeval {
  time_t tv_sec;
  time_t tv_usec;
  int isdst;
} mtv_s;

// define structure 'consistency_check_info' and type 'cci_s'
typedef struct consistency_check_info {
  UINT2 *index_of_first_ray_timewise_p; /* [number_of_sweeps] */
  UINT2 *ray_highest_integral_seconds_p;/* [number_of_sweeps] */
  mtv_s *(*sweep_start_times_mtv_p);    /* [number_of_sweeps] */
} cci_s;

#endif
