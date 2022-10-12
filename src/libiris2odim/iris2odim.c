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
 * Functions that create an ODIM-H5 formatted file from data saved in 
 * linked lists.  Part of program iris2odim.
 * @file iris2odim.c
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-10-16
 */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE  // To get timegm on linux
#define _DARWIN_C_SOURCE // To get timegm on mac
#include <ctype.h>
#include <stdio.h>
#include <math.h>       // M_PI
#include <stddef.h> // size_t, NULL
#include <stdlib.h> // malloc, calloc, exit, free
#include <stdarg.h>
#include <string.h>
#include <time.h>       // localtime_r
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "iris2odim.h"
#include "iris2list_listobj.h"
#include "iris2list_interface.h"
#include "irisdlist.h"
#include "rave_alloc.h"

/**
 * print function used by Iris_printf
 */
static iris_printfun iris_internal_printf_fun = Iris_default_printf;

/**
 * Default printf function.
 */
void Iris_default_printf(const char* msg)
{
#ifndef IRIS_NO_EXIT_OR_STDERR
  fprintf(stderr, "%s", msg);
#endif
}

/**
 * Wraps exit into it's own function to be able to disable hard exit when used in app.
 * Will either return code or do a hard exit depending on if -DIRIS_NO_EXIT_OR_STDERR has
 * been defind or not.
 * @param[in] return_code - the return value for this function if IRIS_NO_EXIT_OR_STDERR has been defined.
 * @param[in] exit_code - the exit code used to call exit if IRIS_NO_EXIT_OR_STDERR hasn't been defined.
 * Output:
 * @return the return_code
 */
int Iris_exit_function(int return_code, int exit_code)
{
#ifdef IRIS_NO_EXIT_OR_STDERR
    return return_code;
#else
    exit(exit_code);
    return return_code;
#endif
}

/**
 * Wraps fprintf into it's own function to be able to disable printouts depending on if DIRIS_NO_EXIT_OR_STDERR has been set or not
 * @param[in] fmt - a string containing the output format
 * @param[in] ... - variable argument list
 */
void Iris_printf(const char* fmt, ...)
{
  va_list alist;
  va_start(alist,fmt);
  char msgbuff[4096];
  int n = vsnprintf(msgbuff, 4096, fmt, alist);
  va_end(alist);
  if (n < 0 || n >= 1024) {
    return;
  }
  iris_internal_printf_fun(msgbuff);
}

/**
 * Sets the print function to where printouts should be done. Default behaviour is to use
 * Iris_default_printf.
 * @param[in] fun - the default printer
 */
void Iris_set_printf(iris_printfun fun)
{
  if (fun != NULL) {
    iris_internal_printf_fun = fun;
  }
}

/**
 * Function name: populateParam
 * Intent: A function to transfer info to RAVE objects for eventual output
 * to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an empty Toolbox sweep/moment of data
 * (2) a pointer to a specific moment in a doubly-linked-list of radar moments  
 * (3) a pointer to structure holding all data from an IRIS RAW-file
 * (4) a pointer to a pointer to a (unallocated) ray attrubutes structure
 * (5) integer serving as logical -> when true fill the ra_s
 * 
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int populateParam(PolarScanParam_t* param, 
                  datatype_element_s *this_type,
                  file_element_s* file_element_p,
                  ra_s **ra_pp,
                  int obtain_ra) {
   ra_s *ra_p = NULL;
   int ret = 0;
   int nrays, nbins, max_nrays, max_nelements;
   int max_nbins = 0;
   double gain = 1.0;
   double offset = 0.0;
   double nodata = 255.0;
   double undetect = 0.0;
   double dummy_double = 0.0;
   double nyquist_velocity = 0.0;
   char *this_quantity = NULL;
   void* data = NULL;
   uint8_t**  arr2d_uchar       = NULL;
   uint8_t*   arr2d_uchar_data  = NULL;
   uint16_t** arr2d_ushort      = NULL;
   uint16_t*  arr2d_ushort_data = NULL;
   int16_t**  arr2d_short       = NULL;
   int16_t*   arr2d_short_data  = NULL;
   double**   arr2d_double      = NULL;
   double*    arr2d_double_data = NULL;
   int ray_index = -1;
   ray_s *this_ray_structure = NULL;
   IrisDList_t *ray_list = NULL;
   IrisDListElement_t *this_ray_element = NULL;
   RaveDataType type = RaveDataType_UNDEFINED;
   double angular_resolution_degrees = (double) 
      file_element_p->ingest_header_p->tcf.scan.angular_resolution_x1000 /
      1000.0;
   idh_s *idh_p = this_type->ingest_data_header_p;
   max_nrays = (int) idh_p->resolution_as_rays_per_360_degree_sweep;
   /* get the index for identity of the moment/data-type */
   int moment_id = (int)this_type->ingest_data_header_p->data_type;
   /* get the ODIM H5 name from a lookup table/enum */
   this_quantity = mapDataType(moment_id);
   ray_list = this_type->ray_list_p;
   nrays = (int) ray_list->size;
   for(int m=0; m < nrays; m++ ) {
      if(m==0) {
         this_ray_element = ray_list->head;
      }
      else {
         this_ray_element = this_ray_element->next;
      }
      this_ray_structure = (ray_s *) this_ray_element->data;
      nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
      if( max_nbins < nbins ) max_nbins = nbins;
   }
   max_nelements = max_nrays * max_nbins;
   /*
    * allocate and partially fill ray attributes structure
    * if obtain_ra is true
    */
   if(obtain_ra && *ra_pp == NULL) {
      *ra_pp = RAVE_MALLOC(sizeof(ra_s));
      ra_p = *ra_pp;
      if( ra_p == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes structure.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      else {
         ra_p->startazA = NULL;
         ra_p->startazT = NULL;
         ra_p->stopazA = NULL;
         ra_p->stopazT = NULL;
         ra_p->elangles = NULL;
         ra_p->expected_nrays = max_nrays;
      }
      ra_p->startazA = RAVE_MALLOC(sizeof(double)*max_nrays);
      if( ra_p->startazA == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes array ra_p->startazA.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      ra_p->stopazA = RAVE_MALLOC(sizeof(double)*max_nrays);
      if( ra_p->stopazA == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes array ra_p->stopazA.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      ra_p->startazT = RAVE_MALLOC(sizeof(double)*max_nrays);
      if( ra_p->startazT == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes array ra_p->startazT.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      ra_p->stopazT = RAVE_MALLOC(sizeof(double)*max_nrays);
      if( ra_p->stopazT == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes array ra_p->stopazT.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      ra_p->elangles = RAVE_MALLOC(sizeof(double)*max_nrays);
      if( ra_p->elangles == NULL) {
         /* Tell client that the allocation failed */
         Iris_printf("Error allocating ray attributes array ra_p->elangles.\n");
         free_IRIS(&file_element_p);
         return Iris_exit_function(-1, EXIT_FAILURE);
      }
      /*
       * set the (un)fill value to MY_FILL_DOUBLE for the angle arrays 
       */
      for(int iray=0; iray < max_nrays; iray++ ) {
         ra_p->startazA[iray] = MY_FILL_DOUBLE;
         ra_p->stopazA[iray] = MY_FILL_DOUBLE;
         ra_p->startazT[iray] = MY_FILL_DOUBLE;
         ra_p->stopazT[iray] = MY_FILL_DOUBLE;
         ra_p->elangles[iray] = MY_FILL_DOUBLE;
      }
      /*
       * For each ray we received compute the ray attributes that are radar pointing angles
       * and insert then into the arrays in the ray attributes structure 
       */
      double istartazA, istopazA, istartelA, istopelA;  
      for(int iray=0; iray < nrays; iray++ ) {
         if(iray==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index based on actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index = compute_ray_index(rhd_p,
                                       angular_resolution_degrees, 
                                       max_nrays);
         /* 
          * Antenna positions at the start and end of the ray 
          * are stored as 16-bit binary angles.
          * Azimuth:
          * add 360 degrees to all negative Azimuth angles so that the 
          * range is 0 to +360 instead of -180 to +180.
          * Elevation:
          * leave any negative elevation angles as is.
          */
         istartazA = (double) rhd_p->azimuth_angle_at_beginning_of_ray
         / 65536.0 * 360.0;
         if( istartazA < 0.0 )  istartazA += 360.0;
         ra_p->startazA[ray_index] = istartazA;
         istopazA  = (double) rhd_p->azimuth_angle_at_end_of_ray
         / 65536.0 * 360.0;
         if( istopazA < 0.0 )  istopazA += 360.0;
         ra_p->stopazA[ray_index]  = istopazA; 
         istartelA = (double) rhd_p->elevation_angle_at_beginning_of_ray
         / 65536.0 * 360.0;
         istopelA  = (double) rhd_p->elevation_angle_at_end_of_ray
         / 65536.0 * 360.0;
         ra_p->elangles[ray_index] = (istartelA + istopelA) / 2.0;
      }
   }
   switch( moment_id ) { /* start outer switch */
   /* 
    * NOTE!!!
    * all of the following cases are scaling of 1-byte rays
    * so only the gain, offset, nodata, and undetect differ.
    * So we group them all together here and use an inner 
    * switch to delineate the differences.
    */
   case  1 : /* Total H power/Uncorrected reflectivity (1 byte)  */
   case  2 : /* Clutter Corrected H reflectivity (1 byte)        */
   case 61 : /* Total V Power/Uncorrected reflectivity (1 byte)  */
   case 63 : /* Clutter Corrected V Reflectivity (1 byte)        */
   case 71 : /* Total Power Enhanced (via H+V or HV) (1 byte)    */
   case 73 : /* Clutter Corrected Reflectivity Enhanced (1 byte) */
   case  5 : /* ZDR - differential reflectivity (1 byte) */
   case 16 : /* PhiDP - differential phase (1 byte)        */
   case 50 : /* PhiH  - differential phase H to V (1 byte) */
   case 52 : /* PhiV  - differential phase V to H (1 byte) */
   case 17 : /* Unfolded Velocity (1 byte) */
   case 25 : /* LDR H to V (1 byte) */
   case 27 : /* LDR V to H (1 byte) */
   case 32 : /* HEIGHT--> echo tops height (1 byte) */
   case 35 : /* SHEAR--> wind shear (1 byte) */
   case 55 : /* HCLASS--> hydro meteor class (1 byte) */
      type = RaveDataType_UCHAR;
      switch( moment_id ) { /* start inner switch */
      case  1 : /* Total H power/Uncorrected reflectivity (1 byte)  */
      case  2 : /* Clutter Corrected H reflectivity (1 byte)        */
      case 61 : /* Total V Power/Uncorrected reflectivity (1 byte)  */
      case 63 : /* Clutter Corrected V Reflectivity (1 byte)        */
      case 71 : /* Total Power Enhanced (via H+V or HV) (1 byte)    */
      case 73 : /* Clutter Corrected Reflectivity Enhanced (1 byte) */
         gain = 0.5;
         offset = -32.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case  5 : /* ZDR - differential reflectivity (1 byte) */
         gain = 1.0/16.0;
         offset = -8.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 16 : /* PhiDP - differential phase (1 byte)        */
      case 50 : /* PhiH  - differential phase H to V (1 byte) */
      case 52 : /* PhiV  - differential phase V to H (1 byte) */
         gain = 180.0/254.0;
         offset = -180.0/254.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 17 : /* Unfolded Velocity (1 byte) */
         /* 
          * DB_VELC Corrected Velocity = -75 + (N/254 * 150) 
          */
         gain = 150.0/254.0;
         offset = -180.0/254.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 25 : /* LDR H to V (1 byte) */
      case 27 : /* LDR V to H (1 byte) */
         gain = 1.0/5.0;
         offset = -45.2;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 32 : /* HEIGHT--> echo tops height (1 byte) */
         gain = 0.01;
         offset = -1.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 35 : /* SHEAR--> wind shear (1 byte) */
         gain = 0.2;
         offset = -25.6;
         nodata = 255.0;
         undetect = 0.0;
         break;
      case 55 : /* HCLASS--> hydro meteor class (1 byte) */
         gain = 1.0;
         offset = 0.0;
         nodata = 255.0;
         undetect = 0.0;
         break;
      } /*end inner-switch */

      /* assume malloc always succeeds */
      arr2d_uchar_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_uchar_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      uint8_t nodata_uint8_t = (uint8_t) nodata;
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_uchar_data[lv] = nodata_uint8_t;
      }
      arr2d_uchar = RAVE_MALLOC(max_nrays * sizeof(*arr2d_uchar));
      for(int i1 = 0; i1 < max_nrays; ++i1) {
         arr2d_uchar[i1] = &arr2d_uchar_data[i1 * max_nbins];
      }
      /* 
       * copy rays of data into the 2D array
       * Note! The number of bins in a ray can be less
       * than the number declared in the 2D array
       * especially when the 20km height limit is reached
       */
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index based on actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index = compute_ray_index(rhd_p,
                                       angular_resolution_degrees, 
                                       max_nrays);
         for(int ibin=0; ibin < max_nbins; ibin++) {
            if(ibin >= nbins) {
               /* 
                * fill array elements with the 'undetect' value 
                * when missing range bins (altitudes exceeding 20km) 
                */
               arr2d_uchar[ray_index][ibin] = (uint8_t) undetect;
            }
            else {
               arr2d_uchar[ray_index][ibin] = this_ray_structure->ray_body[ibin];
            }
         }
      }
      data = arr2d_uchar_data;
      break;
   /* 
    * NOTE!!!
    * all of the following cases are scaling of UNSIGNED 2-byte
    * rays so only the gain, offset, nodata, and undetect differ.
    * So we group them all together here and use an inner 
    * switch to delineate the differences.
    */
   case  8 : /* Total H power/Uncorrected reflectivity  (2 byte) */
   case  9 : /* Clutter Corrected H reflectivity (2 byte)        */
   case 10 : /* VEL2--> velocity (2 byte)                        */
   case 12 : /* ZDR2--> differential reflectivity (2 byte)       */
   case 15 : /* KDP2 specific differential phase  (2 byte)       */
   case 22 : /* Corrected Velocity (2 byte)                      */
   case 26 : /* LDR H to V (2 byte)                              */
   case 28 : /* LDR V to H (2 byte)                              */
   case 62 : /* Total V power/Uncorrected reflectivity (2 byte)  */
   case 64 : /* Clutter Corrected V Reflectivity (2 byte)        */
   case 72 : /* Total Power Enhanced (via H+V or HV) (2 byte)    */
   case 74 : /* Clutter Corrected Reflectivity Enhanced (2 byte) */
   case 11 : /* WIDTH2--> spectrum width  (2 byte)               */
   case 20 : /* RhoHV H-V correlation (2 byte)                   */
   case 23 : /* SQI signal quality index (2 byte)                */
   case 24 : /* PhiDP2--> differential phase  (2 byte)           */
   case 45 : /* TIME2--> time in seconds                         */
   case 56 : /* HCLASS2--> hydrometeor class (2 byte)            */
      type = RaveDataType_USHORT;
      switch( moment_id ) { /* start inner switch */
      case  8 : /* Total H power/Uncorrected reflectivity  (2 byte) */
      case  9 : /* Clutter Corrected H reflectivity (2 byte)        */
      case 10 : /* VEL2--> velocity (2 byte)                        */
      case 12 : /* ZDR2--> differential reflectivity (2 byte)       */
      case 15 : /* KDP2 specific differential phase  (2 byte)       */
      case 22 : /* Corrected/Unfolded Velocity (2 byte)             */
      case 26 : /* LDR H to V (2 byte)                              */
      case 28 : /* LDR V to H (2 byte)                              */
      case 62 : /* Total V power/Uncorrected reflectivity (2 byte)  */
      case 64 : /* Clutter Corrected V Reflectivity (2 byte)        */
      case 72 : /* Total Power Enhanced (via H+V or HV) (2 byte)    */
      case 74 : /* Clutter Corrected Reflectivity Enhanced (2 byte) */
         gain = 0.01;
         offset = -327.68;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      case 11 : /* WIDTH2--> spectrum width  (2 byte) */
         gain = 0.01;
         offset = 0.0;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      case 20 : /* RhoHV H-V correlation (2 byte) */
      case 23 : /* SQI signal quality index (2 byte) */
         gain = 1.0/65534.0;
         offset = -1.0/65534.0;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      case 24 : /* PhiDP2--> differential phase  (2 byte) */
         gain = 360.0/65534.0;
         offset = -360.0/65534.0;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      case 45 : /* TIME2--> time in seconds */
         gain = 1.0;
         offset = 0.0;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      case 56 : /* HCLASS2--> hydrometeor class (2 byte) */
         gain = 1.0;
         offset = 0.0;
         nodata = 65535.0;
         undetect = 0.0;
         break;
      } /* end inner-switch */
      /* assume malloc always succeeds */
      arr2d_ushort_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_ushort_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      uint16_t nodata_uint16_t = (uint16_t) nodata;
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_ushort_data[lv] = nodata_uint16_t;
      }
      arr2d_ushort = RAVE_MALLOC(max_nrays * sizeof(*arr2d_ushort));
      for (int i1 = 0; i1 < max_nrays; ++i1)
      {
         arr2d_ushort[i1] = &arr2d_ushort_data[i1 * max_nbins];
      }
      /* 
       * copy rays of data into the 2D array
       * Note! The number of bins in a ray can be less
       * than the number declared in the 2D array 
       */
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = IrisDList_head(ray_list);
         }
         else {
            this_ray_element = IrisDListElement_next(this_ray_element);
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from the actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees, 
                                        max_nrays);
         for(int ibin=0; ibin < max_nbins; ibin++) {
            if(ibin >= nbins) {
               /* 
                * fill array elements with the 'undetect' value 
                * when missing range bins (altitudes exceeding 20km) 
                */
               arr2d_ushort[ray_index][ibin] = (uint16_t) undetect;
            }
            else {
               uint16_t one_value;
               memcpy( &one_value, &this_ray_structure->ray_body[ibin*2], 2);
               arr2d_ushort[ray_index][ibin] = one_value;
            }
         }
      }
      data = arr2d_ushort_data;
      break;
   /* 
    * NOTE!!!
    * all of the following cases are scaling of SIGNED 2-byte
    * rays so only the gain, offset, nodata, and undetect differ.
    * So we group them all together here and use an inner 
    * switch to delineate the differences.
    */
   case 36 : /* DIVERGE2--> divergence */
   case 40 : /* DEFORM2--> deformation */
   case 41 : /* VVEL2--> vertical velocity */
   case 43 : /* HDIR2--> horozontal wind direction */
   case 44 : /* AXDIL2--> axis of dilliation */
      type = RaveDataType_SHORT;
      switch( moment_id ) { /* start inner switch */
      case 36 : /* DIVERGE2--> divergence, a signed short */
         gain = 1.0E-7;
         offset = 0.0;
         nodata = 32767.0;
         undetect = 32767.0;
         break;
      case 40 : /* DEFORM2--> deformation */
         gain = 1.0E-7;
         offset = 0.0;
         nodata = 32767.0;
         undetect = 32767.0;
         break;
      case 41 : /* VVEL2--> vertical velocity */
         gain = 1.0E-02;
         offset = 0.0;
         nodata = 32767.0;
         undetect = 32767.0;
         break;
      case 43 : /* HDIR2--> horozontal wind direction */
      case 44 : /* AXDIL2--> axis of dilliation */
         gain = 1.0E-01;
         offset = 0.0;
         nodata = 32767.0;
         undetect = 32767.0;
         break;
      } /* end of inner-switch */
      /* assume malloc always succeeds */
      arr2d_short_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_short_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      int16_t nodata_int16_t = (int16_t) nodata;
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_short_data[lv] = nodata_int16_t;
      }
      arr2d_short = RAVE_MALLOC(max_nrays * sizeof(*arr2d_short));
      for (int i1 = 0; i1 < max_nrays; ++i1)
      {
         arr2d_short[i1] = &arr2d_short_data[i1 * max_nbins];
      }
      /* 
       * copy rays of data into the 2D array
       * Note! The number of bins in a ray can be less
       * than the number declared in the 2D array 
       */
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) 
              this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees,
                                        max_nrays);
         for(int ibin=0; ibin < max_nbins; ibin++) {
            if(ibin >= nbins) {
               /* 
                * fill array elements with the 'undetect' value 
                * when missing range bins (altitudes exceeding 20km) 
                */
               arr2d_short[ray_index][ibin] = (int16_t) undetect;
            }
            else {
               int16_t one_value;
               memcpy( &one_value, &this_ray_structure->ray_body[ibin*2], 2);
               arr2d_short[ray_index][ibin] = one_value;
            }
         } /* end for bins in ray */
      } /* rays in sweep */
      data = arr2d_short_data;
      break;
   case  3 : /* Velocity (1 byte) */
      type = RaveDataType_USHORT;
      nyquist_velocity = calc_nyquist(file_element_p);
      /*
       * Note! nyquist velocity is now part of the gain and offset
       */
      gain = nyquist_velocity/127.0;
      offset = -128.0/127.0*nyquist_velocity;
      nodata = 256.0;
      undetect = 0.0;
      /* assume malloc always succeeds */
      arr2d_ushort_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_ushort_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      nodata_uint16_t = (uint16_t) nodata;
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_ushort_data[lv] = nodata_uint16_t;
      }
      arr2d_ushort = RAVE_MALLOC(max_nrays * sizeof(*arr2d_ushort));
      for (int i1 = 0; i1 < max_nrays; ++i1) {
         arr2d_ushort[i1] = &arr2d_ushort_data[i1 * max_nbins];
      }
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees,
                                        max_nrays);
         /*
          * take value of from unsigned 1-byte integer and output it into
          * an unsigned 2-bite integer, but don't change the value
          */
         for(int ibin=0; ibin < max_nbins; ibin++) {
            /* 
             * fill array elements with the 'undetect' value 
             * when missing range bins (altitudes exceeding 20km) 
             */
            if(ibin >= nbins) {
               arr2d_ushort[ray_index][ibin] =  (uint16_t) undetect;
            }
            else {
               arr2d_ushort[ray_index][ibin] =  
                  (uint16_t) this_ray_structure->ray_body[ibin]; 
            }
         }
      }
      data = arr2d_ushort_data;
      break;
   case  4 : /* Spectrum Width  (1 byte) */
      type = RaveDataType_USHORT;
      gain = 1.0/256.0;;
      offset = 0.0;
      nodata = 256.0;
      undetect = 0.0;
      /* assume malloc always succeeds */
      arr2d_ushort_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_ushort_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      nodata_uint16_t = (uint16_t) nodata;
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_ushort_data[lv] = nodata_uint16_t;
      }
      arr2d_ushort = RAVE_MALLOC(max_nrays * sizeof(*arr2d_ushort));
      for (int i1 = 0; i1 < max_nrays; ++i1) {
         arr2d_ushort[i1] = &arr2d_ushort_data[i1 * max_nbins];
      }
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees,
                                        max_nrays);
         /*
          * take value of from unsigned 1-byte integer and output it into
          * an unsigned 2-byte integer, but don't change the value
          */
         for(int ibin=0; ibin < max_nbins; ibin++) {
            /* 
             * fill array elements with the 'undetect' value 
             * when missing range bins (altitudes exceeding 20km) 
             */
            if(ibin >= nbins) {
               arr2d_ushort[ray_index][ibin] =  (uint16_t) undetect;
            }
            else {
               arr2d_ushort[ray_index][ibin] =  
               (uint16_t) this_ray_structure->ray_body[ibin]; 
            }
         }
      }
      data = arr2d_ushort_data;
      break;
   case 14 : /* KDP specific differential phase (1 byte) */
      type = RaveDataType_DOUBLE;
      gain = 1.0;
      offset = 0.0;
      nodata = -MY_FILL_DOUBLE;
      undetect = MY_FILL_DOUBLE;
      /* assume malloc always succeeds ? */
      arr2d_double_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_double_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_double_data[lv] = nodata;
      }
      arr2d_double = RAVE_MALLOC(max_nrays * sizeof(*arr2d_double));
      for (int i1 = 0; i1 < max_nrays; ++i1)
      {
         arr2d_double[i1] = &arr2d_double_data[i1 * max_nbins];
      }
      SINT4 wavelength_in_hocm = 
         file_element_p->product_header_p->
         end.wavelength_in_hundredths_of_centimeters;
      double wavelength_in_cm = (double) wavelength_in_hocm /1.0E2;
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) 
            this_ray_structure->ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees,
                                        max_nrays);
         for(int ibin=0; ibin < max_nbins; ibin++) {
            /* 
             * fill array elements with the 'undetect' value 
             * when missing range bins (altitudes exceeding 20km) 
             */
            if(ibin >= nbins) {
               arr2d_double[ray_index][ibin] = undetect;
            }
            else {
               if( this_ray_structure->ray_body[ibin] == 0 || 
                                   wavelength_in_hocm == 0) {
                  dummy_double = undetect;
               }
               else if( this_ray_structure->ray_body[ibin] == 255) {
                  dummy_double = nodata;
               }
               else if( this_ray_structure->ray_body[ibin] == 128) {
                  dummy_double = 0.0;
               }
               else if( this_ray_structure->ray_body[ibin] > 128) {
                  dummy_double = (double) (this_ray_structure->ray_body[ibin]-129);
                  dummy_double /= 126.0;
                  dummy_double = pow(600.0,dummy_double);
                  dummy_double *= 0.25;
                  dummy_double /= wavelength_in_cm;
               }
               else if( this_ray_structure->ray_body[ibin] < 128) {
                  dummy_double = (double) (127-this_ray_structure->ray_body[ibin]);
                  dummy_double /= 126.0;
                  dummy_double = pow(600.0,dummy_double);
                  dummy_double *= -0.25;
                  dummy_double /= wavelength_in_cm;
               }
               arr2d_double[ray_index][ibin] =  dummy_double;
            }
         }
      }
      data = arr2d_double_data;
      break;
   case 18 : /* SQI signal quality index  (1 byte)   */
   case 19 : /* RhoHV H-V power correlation (1 byte) */
   case 46 : /* RhoH H to V (1 byte)                 */
   case 48 : /* RhoV V to H (1 byte)                 */
      type = RaveDataType_DOUBLE;
      gain = 1.0;
      offset = 0.0;
      nodata = -MY_FILL_DOUBLE;
      undetect = MY_FILL_DOUBLE;
      /* assume malloc always succeeds ? */
      arr2d_double_data = RAVE_MALLOC(max_nelements * sizeof(*arr2d_double_data));
      /* fill all array elements with the 'nodata' value to handle missing rays */
      for(int lv=0; lv < max_nelements; ++lv) {
         arr2d_double_data[lv] = nodata;
      }
      arr2d_double = RAVE_MALLOC(max_nrays * sizeof(*arr2d_double));
      for (int i1 = 0; i1 < max_nrays; ++i1)
      {
         arr2d_double[i1] = &arr2d_double_data[i1 * max_nbins];
      }
/*
      returned_pointer_to_void = 
           rave_allocate_2d_mem((void*) arr2d_double,
                                 nrays,
                                 max_nbins,
                                 type);
      arr2d_double_data = (double*) returned_pointer_to_void;
*/
      for(int irow=0; irow < nrays; irow++ ) {
         if(irow==0) {
            this_ray_element = ray_list->head;
         }
         else {
            this_ray_element = this_ray_element->next;
         }
         this_ray_structure = (ray_s *) this_ray_element->data;
         nbins = (int) this_ray_structure->
                        ray_head.actual_number_of_bins_in_ray;
         /* 
          * compute a ray index from actual azimuth angles for this ray 
          */
         rhd_s *rhd_p = &this_ray_structure->ray_head;
         ray_index  = compute_ray_index(rhd_p,
                                        angular_resolution_degrees,
                                        max_nrays);
         for(int ibin=0; ibin < max_nbins; ibin++) {
            if(ibin >= nbins) {
               /* 
                * fill array elements with the 'undetect' value 
                * when missing range bins (altitudes exceeding 20km) 
                */
               arr2d_double[ray_index][ibin] = undetect;
            }
            else {
               if( this_ray_structure->ray_body[ibin] == 0 ) {
                  dummy_double = undetect;
               }
               else if(this_ray_structure->ray_body[ibin] == 255) {
                  dummy_double = nodata;
               }
               else {
                  dummy_double = (double) this_ray_structure->ray_body[ibin];
                  dummy_double -= 1.0;
                  dummy_double /= 253.0;
                  dummy_double = sqrt(  dummy_double );
               }
               arr2d_double[ray_index][ibin] =  dummy_double; 
            }
         }
      }
      data = arr2d_double_data;
      break;
   } /* end select */
   /* 
    * Map IRIS moments to ODIM, 
    * e g. corrected horizontal reflectivity 
    */
   PolarScanParam_setQuantity(param, this_quantity);
   /* 
    * Linear scaling factor, 
    * with an example for 8-bit reflectivity 
    */
    PolarScanParam_setGain(param, gain);
   /* 
    * Linear scaling offset, 
    * with an example for 8-bit reflectivity 
    */
   PolarScanParam_setOffset(param, offset);
   /* 
    * Value for 'no data', ie. unradiated areas, 
    * with a convention used for reflectivity 
    */
   PolarScanParam_setNodata(param, nodata);
   /* 
    * Value for 'undetected', ie. areas radiated but with no echo, 
    * with a convention used for reflectivity 
    */
   PolarScanParam_setUndetect(param, undetect);
   /* 
    * Set the data in the Toolbox object 
    */
   ret = PolarScanParam_setData(param, max_nbins, max_nrays, data, type);
   /* 
    * deallocate 2D arrays allocated above 
    */
   if(arr2d_uchar != NULL) {
      RAVE_FREE(arr2d_uchar_data);
      arr2d_uchar_data = NULL;
      RAVE_FREE(arr2d_uchar);
      arr2d_uchar = NULL;
   }
   else if(arr2d_ushort != NULL) {
      RAVE_FREE(arr2d_ushort_data);
      arr2d_ushort_data = NULL;
      RAVE_FREE(arr2d_ushort);
      arr2d_ushort = NULL;
   }
   else if(arr2d_short != NULL) {
      RAVE_FREE(arr2d_short_data);
      arr2d_short_data = NULL;
      RAVE_FREE(arr2d_short);
      arr2d_short = NULL;
   }
   else if(arr2d_double != NULL) {
      RAVE_FREE(arr2d_double_data);
      arr2d_double_data = NULL;
      RAVE_FREE(arr2d_double);
      arr2d_double = NULL;
   }
   /* We'll add appropriate exception handling later */
   return ret;
} /* End function populateParam */

/**
 * Function name: polpulateScan
 * Intent: A function to transfer info to RAVE objects for eventual output
 * to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an empty Toolbox polar scan object
 * (2) a pointer to a structure holding IRIS RAW-file data
 * (3) a signed integer, the scan/sweep that this function call is handling
 * Note! The origin of this sweep index is zero, not 1
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int populateScan(PolarScan_t* scan, 
                 file_element_s* file_element_p, 
                 int sweep_index) {
   char sdate[9];
   char etime[7];
   UINT4 sfm = 0;
   int ret = 0;
   int ncopied = 0;
   IrisDList_t *sweep_list = NULL;
   IrisDListElement_t *a_sweep_element = NULL;
   sweep_element_s *sweep_data = NULL;
   IrisDList_t *types_list = NULL;
   IrisDListElement_t *this_type_element = NULL;
   datatype_element_s* this_type = NULL;
   idh_s *idh_p = NULL;
   mtv_s my_mtv;
   mtv_s *my_mtv_p = &my_mtv;
   ymd_s *sweep_start_ymd_p = NULL;
   ymd_s *sweep_end_ymd_p = NULL;
   ymd_s *product_generation_ymd_p = NULL;
   mtv_s *product_generation_mtv_p = NULL;
   char stime[7];
   /* 
    * np = Number of moments/Parameters recorded in whatever
    * the current sweep/scan that we are currently dealing with.
    * In list lingo this is the same as the number of datatype 
    * elements
    */
   int np;
   RaveCoreObject* object = (RaveCoreObject*)scan;
   sweep_list = file_element_p->sweep_list_p;
   int ns = sweep_list->size;
   size_t n_sweeps_in_volume = (size_t) ns;
   /* point to the current sweep */
   for (int jj=0; jj <= sweep_index; jj++) {
      /* set a pointer to point to the current scan/sweep */
      if(jj==0) {
         a_sweep_element = IrisDList_head(sweep_list);
      }
      else {
         a_sweep_element = IrisDListElement_next(a_sweep_element);
      }
   }
   sweep_data = (sweep_element_s *) a_sweep_element->data;
   /* here 'types' refers to moments or scanned-parameters */ 
   types_list = sweep_data->types_list_p;
   np = IrisDList_size(types_list);
   /* set pointer to the first node in the parameter list */	  
   this_type_element = types_list->head;
   /* set pointer to the data in this list node */
   this_type = (datatype_element_s *) this_type_element->data;
   /* point to ingest data header for this sweep */
   idh_p = this_type->ingest_data_header_p;
   /* azimuthal offset SHOULD always be zero */
   ret = addDoubleAttribute(object, "how/astart", 0.0);
   /*
    * Put the product generation time into a time structure.  Although we do
    * not use this time in this program, this time can be used as an
    * approximation of the end of the volume scan since all rays happen before
    * this time.
    */
   product_generation_ymd_p =
      &(file_element_p->product_header_p->pcf.product_GenTime_UTC);
   /***********************************************************************
    *                                                                     *
    * the function 'create_consistency_check_arrays' allocates            *
    * both structure and arrays.  Integer arrays are initialised          *
    * to zero.                                                            *
    *                                                                     *
    **********************************************************************/
   cci_s* cci_p = create_consistency_check_arrays(n_sweeps_in_volume);
   /***********************************************************************
    *                                                                     *
    * Note! The function 'do_consistency_check' returns nothing but       *
    * does change the contents of variables which are pointed to by       *
    * members of the cci_s structure pointed to by cci_p                  *
    * (ie this function has side-effects).                                *
    *                                                                     *
    **********************************************************************/
   do_consistency_check(cci_p, n_sweeps_in_volume, file_element_p);
   /*
    * Current contents of cci_s:
    * 
    * UINT2 *index_of_first_ray_timewise_p;  [number_of_sweeps]
    * UINT2 *ray_highest_integral_seconds_p; [number_of_sweeps]
    * mtv_s *(*sweep_start_times_mtv_p); [MAX_SWEEPS]
    * 
    */
   if(cci_p->sweep_start_times_mtv_p[sweep_index] != NULL) {
      my_mtv_p->isdst = cci_p->sweep_start_times_mtv_p[sweep_index]->isdst;
      sweep_start_ymd_p = mtv_to_ymd(cci_p->sweep_start_times_mtv_p[sweep_index]);
      if(sweep_start_ymd_p != NULL) {
         ncopied = snprintf(sdate,9,"%4.4i%2.2i%2.2i", 
                            sweep_start_ymd_p->year,
                            sweep_start_ymd_p->month, 
                            sweep_start_ymd_p->day);
         if(ncopied <= 1) {
            Iris_printf("Error when formatting volume start date.\n");
            Iris_printf("Will attempt to continue.\n");
         }
         sfm = sweep_start_ymd_p->seconds_since_midnight;
         RAVE_FREE(sweep_start_ymd_p);
      }
   }
   /* transfer formatted volume start time to string */
   UINT4 hours, minutes, seconds;
   hours = sfm/3600;
   minutes = (sfm - hours*3600)/60;
   seconds = sfm - hours*3600 - minutes*60;
   ncopied = snprintf(stime,7,"%2.2i%2.2i%2.2i", hours,minutes,seconds);
   if(ncopied <= 1) {
      Iris_printf("Error when formatting volume start time.\n");
      Iris_printf("Will attempt to continue.\n");
   }
   /*
    * Note! The following version of sweep start time is the fractional
    * number of seconds after midnight Jan011970
    */
   double sweep_start_time = 0.0;
   if( cci_p->sweep_start_times_mtv_p[sweep_index] != NULL) {
      sweep_start_time = 
        (double) cci_p->sweep_start_times_mtv_p[sweep_index]->tv_sec +
        (double) cci_p->sweep_start_times_mtv_p[sweep_index]->tv_usec / 1.0E6;
   }
   double sweep_stop_time = 0.0;
   if( sweep_index == ns-1) { /* if this is the last sweep */
      /* the product generation time is recorded after the last sweep */
      if(product_generation_ymd_p != NULL) {
         product_generation_mtv_p = ymd_to_mtv(product_generation_ymd_p);
         if(product_generation_mtv_p != NULL) {
            sweep_stop_time = 
               (double) product_generation_mtv_p->tv_sec +
               (double) product_generation_mtv_p->tv_usec / 1.0E6;
               RAVE_FREE(product_generation_mtv_p);
               product_generation_mtv_p = NULL;
         }
      }
      else {
         Iris_printf("Error obtaining volume stop time.\n");
         Iris_printf("Could be a result of an allocation failure.\n");
         Iris_printf("Will attempt to continue.\n");
      }
   }
   else {
      if(cci_p->sweep_start_times_mtv_p[sweep_index+1] != NULL) {
         sweep_stop_time = 
            (double) cci_p->sweep_start_times_mtv_p[sweep_index+1]->tv_sec +
            (double) cci_p->sweep_start_times_mtv_p[sweep_index+1]->tv_usec / 1.0E6;
      }
   }
   double nsecs, sweep_stop_time_est;
   nsecs = (double) cci_p->ray_highest_integral_seconds_p[sweep_index];
   sweep_stop_time_est = sweep_start_time + (nsecs+1.0);
   if(sweep_stop_time_est < sweep_stop_time) { 
      sweep_stop_time = sweep_stop_time_est;
   }

   my_mtv_p->tv_sec = (time_t) floor(sweep_stop_time);
   my_mtv_p->tv_usec = 
        (sweep_stop_time - (double) floor(sweep_stop_time))*1.0E06;
   my_mtv_p->isdst = 0; /* AHE: Better than unitialized but is it correct? */

   sweep_end_ymd_p = mtv_to_ymd(my_mtv_p);
   char edate[9];
   sfm = 0;
   if( sweep_end_ymd_p != NULL) {
      ncopied = snprintf(edate,9,"%4.4i%2.2i%2.2i", sweep_end_ymd_p->year,
                         sweep_end_ymd_p->month, sweep_end_ymd_p->day);
      if(ncopied <= 1) {
         Iris_printf("Error when formatting sweep end date.\n");
         Iris_printf("Will attempt to continue.\n");
      }
      /* transfer formatted sweep end time to string */
      sfm = sweep_end_ymd_p->seconds_since_midnight;
      RAVE_FREE(sweep_end_ymd_p);
   }
   hours = sfm/3600;
   minutes = (sfm - hours*3600)/60;
   seconds = sfm - hours*3600 - minutes*60;
   ncopied = snprintf(etime,7,"%2.2i%2.2i%2.2i", hours,minutes,seconds);
   if(ncopied <= 1) {
      Iris_printf("Error when formatting sweep end time.\n");
      Iris_printf("Will attempt to continue.\n");
   }
   /* 
    * Attributes that are set at the scan level are common to all 
    * moments/quantities/parameters. 
    * 
    * Set select mandatory 'what' attributes. 
    * (See Table 13 in the ODIM_H5 spec) 
    */
   ret = PolarScan_setStartDate(scan, sdate);
   ret = PolarScan_setStartTime(scan, stime);
   ret = PolarScan_setEndDate(scan, edate);
   ret = PolarScan_setEndTime(scan, etime);
   /* Set mandatory 'where' attributes, Table 4 in the ODIM_H5 spec. */
   long a1gate = (long) cci_p->index_of_first_ray_timewise_p[sweep_index];
   PolarScan_setA1gate(scan, a1gate);
   double fixed_angle, fixed_angle_radians;
   fixed_angle = (double) idh_p->fixed_angle_of_sweep / 65536.0 * 360.0;
   fixed_angle_radians = fixed_angle * 3.14159265358979323846 / 180.0;
   UINT2 antenna_scan_mode =
      file_element_p->ingest_header_p->tcf.scan.antenna_scan_mode;
   /* Antenna Scan Mode (TASK_SCAN_xxx)
    *  1:PPI sector
    *  2:RHI sector
    *  3:Manual
    *  4:PPI full
    *  5:file
    *  6:exec
    *  7:RHI full
    */
   switch(antenna_scan_mode)
   {
   case(1):
   case(4):
      PolarScan_setElangle(scan, fixed_angle_radians);  /* Radians */
      break;
   case(2):
   case(7):
      /*
       * will have to figure out what to do for the case of RHIs
       */
      break;
   }
   /*
   double range_of_first_bin_in_km = file_element_p->ingest_header_p->
                                     tcf.rng.range_of_first_bin_in_cm/1.0E6;
    */
   double range_of_first_bin_in_km = 0.0;
   PolarScan_setRstart(scan, range_of_first_bin_in_km);   /* Kilometers */
   int step_between_output_bins_in_cm = 
      file_element_p->ingest_header_p->tcf.rng.step_between_output_bins_in_cm;
   double step_between_output_bins_in_m = ((double)
      step_between_output_bins_in_cm) / 100.0;
   PolarScan_setRscale(scan, step_between_output_bins_in_m);   /* Meters */
   long scan_index = (long) idh_p->sweep_number;
   ret = addLongAttribute(object, "how/scan_index", scan_index);
   /* Determine number of scans:
    * 
    * Decide when to use 'sweeps_completeded' as the number of
    * sweeps. Around ~ June 23 2014 after updating some sigmet 
    * software, the sweeps_completed started to be reported wrong
    * in the IRIS RAW files. So we are using sweeps_to_perform
    * 
    *        int sweeps_completed = (int) file_element_p->ingest_header_p->
    *                                     icf.number_of_sweeps_completed;
    */
   long sweeps_to_perform = (long) file_element_p->ingest_header_p->
                                   tcf.scan.number_of_sweeps_to_perform;
   ret = addLongAttribute(object, "how/scan_count", sweeps_to_perform);
   /* Loop through the moments, populating a Toolbox object for each */
    this_type_element = NULL;
   ra_s *ra_p = NULL;
   int obtain_ra;
   for(int i = 0; i < np; i++) {
      if(i==0) obtain_ra = TRUE;
      else obtain_ra = FALSE;
      PolarScanParam_t* param = RAVE_OBJECT_NEW(&PolarScanParam_TYPE);
      if( i == 0 ) {
         this_type_element = types_list->head;
      }
      else {
         this_type_element = this_type_element->next;
      }
      /* set pointer to moment/data element */
      this_type = (datatype_element_s *) this_type_element->data;
      ret = populateParam(param, 
                          this_type, 
                          file_element_p, 
                          &ra_p, 
                          obtain_ra);
      ret = PolarScan_addParameter(scan, param);
      RAVE_OBJECT_RELEASE(param);
   }
   /* 
    * Detailed ray readout az and el angles and acquisition times. 
    * Helper function below. 
    */
   ret = setRayAttributes(scan, 
                          file_element_p,
                          cci_p,
                          sweep_index,
                          &ra_p);
   destroy_consistency_check_arrays(cci_p, n_sweeps_in_volume);
   /* 
    * free all ray attribute related arrays and the ra structure itself
    * if any are non-null
    */
   if(ra_p != NULL) {
      RAVE_FREE(ra_p->elangles);
      RAVE_FREE(ra_p->startazA);
      RAVE_FREE(ra_p->startazT);
      RAVE_FREE(ra_p->stopazA);
      RAVE_FREE(ra_p->stopazT);
      RAVE_FREE(ra_p);
      ra_p = NULL;
   }
   return ret;
} /* end of populateScan */

/**
 * Function name: polpulateObject
 * Intent: A function to transfer info to RAVE objects for eventual output
 * to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an empty Toolbox core object (object type to be determined below)
 * (2) a pointer to a structure holding IRIS RAW-file data
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int populateObject(RaveCoreObject* object, file_element_s* file_element_p) {
   int ret = 0;
   int nscans = 0;
   int i, ncopied;
   IrisDList_t *sweep_list = NULL;
   IrisDListElement_t *a_sweep_element = NULL;
   sweep_element_s *this_sweep_data = NULL;
   IrisDList_t *types_list = NULL;
   IrisDListElement_t *a_type_element = NULL;
   datatype_element_s* this_type = NULL;
   struct ymds_time *sweep_start_ymd_p = NULL;
   struct ymds_time *product_generation_ymd_p = NULL;
   int last_day[12] = {31,28,30,30,31,28,31,31,30,31,30,31};
   char *source;
   /* here to prepare the radar 'source' (ie the radar site name)
    * the pointer 'site_name' points to a character array, not a
    * string. So we copy it to char array 'sname', and terminate it with
    * a NUL character to make it a string, then we trim the excess spaces
    * off the end with the 'trim' function. Finally, we set a pointer to
    * point to the array, and pass the pointer to the mapSource2Nod 
    * function (which is just a lookup of rave site name from IRIS site 
    * name)
    */
   char sname[17];
   ncopied = snprintf(sname,16,"%s",
      file_element_p->product_header_p->end.site_name);
   if(ncopied > 0 ) rltrim(sname);
   source = mapSource2Nod(sname);
   /*
    * The top-level 'what' group has a time and date that are referred to
    * in the OPERA Data Information Model for HDF as 'Nominal' date and time.
    * In this case we have decided that 'Nominal' designates a time based on
    * a 10 minute scan cycle, so that every scan in a scan-cycle has the
    * same nominal time.  So Nominal times on even 10s of minutes, for example
    * 190000, 191000, 192000, 193000, 194000, 195000, 200000, 201000, etc.
    * Our cycle starts 5 minutes before the Nominal time and ends 10 after the
    * Nominal time. Assuming volume scans start near the beginning of a cycle
    * we round the end of the volume scan to the nearest 10 minute point.
    * For non-volume scans (single sweep scans) we round the start time to the
    * nearest 10 minute point.
    */
   char ntime[7];
   char ndate[9];
   int hourz, minutez, secondz, nominal_time_in_sfm;
   int yearz=0, monthz=0, dayz=0;
   double time_in_seconds = 0.0;
   double time_in_minutes = 0.0;
   double nominal_time_in_minutes;
   sweep_list = file_element_p->sweep_list_p;
   nscans = sweep_list->size;
   if(RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      /* the product generation time is recorded after the last sweep */
      product_generation_ymd_p =
         &(file_element_p->product_header_p->pcf.product_GenTime_UTC);
      time_in_seconds = (double) 
         product_generation_ymd_p->seconds_since_midnight;
      yearz = (int) product_generation_ymd_p->year;
      monthz = (int) product_generation_ymd_p->month;
      dayz = (int) product_generation_ymd_p->day;
   }
   else if(RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
      a_sweep_element = sweep_list->head;
      this_sweep_data =  (sweep_element_s *) a_sweep_element->data;
      types_list = this_sweep_data->types_list_p;;
      a_type_element = types_list->head;
      this_type = (datatype_element_s *) a_type_element->data;
      sweep_start_ymd_p = &(this_type->ingest_data_header_p->sweep_start_time);
      time_in_seconds = (double) sweep_start_ymd_p->seconds_since_midnight;
      yearz = (int) sweep_start_ymd_p->year;
      monthz = (int) sweep_start_ymd_p->month;
      dayz = (int) sweep_start_ymd_p->day;
   }
   
   time_in_minutes = time_in_seconds / 60.0;
   nominal_time_in_minutes = round(time_in_minutes/10.)*10.;
   nominal_time_in_sfm = (int) (nominal_time_in_minutes*60.0);
   hourz = nominal_time_in_sfm/3600;
   minutez = (nominal_time_in_sfm - hourz*3600)/60;
   secondz = nominal_time_in_sfm - hourz*3600 - minutez*60;
   if(hourz == 24) {
      hourz = 0;
      dayz += 1;
      if((yearz%400==0) || (yearz%4==0)) {
         /* it is a leap year */
         if( (monthz == 2) && (dayz > 29) ) {
            monthz += 1;
            dayz = 1;
         }
         else {
            if(dayz > last_day[monthz-1]) {
               monthz += 1;
               dayz = 1;
            }
         }
      }
      else {
         /* it's not leap year */
         if(dayz > last_day[monthz-1]) {
            monthz += 1;
            dayz = 1;
         }
      }
      if(monthz > 12) {
         monthz = 1;
         yearz += 1;
      }
   }
   ncopied = snprintf(ntime,7,"%2.2i%2.2i%2.2i",hourz,minutez, secondz);
   if(ncopied <= 1) {
      Iris_printf("Error while formulating nominal time.\n");
      Iris_printf("Will attempt to continue!\n");
   }
   ncopied = snprintf(ndate,9,"%4.4i%2.2i%2.2i", yearz,monthz,dayz);
   if(ncopied <= 1) {
      Iris_printf("Error when formatting nominal date.\n");
      Iris_printf("Will attempt to continue.\n");
   }
   /* 
    * Assign some Top-level 'what' attributes 
    * (see Table 1 of the ODIM_H5 spec). 
    */
   if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
     ret = PolarVolume_setDate((PolarVolume_t*)object, ndate);
     ret = PolarVolume_setTime((PolarVolume_t*)object, ntime);
     ret = PolarVolume_setSource((PolarVolume_t*)object, source);
   } else if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
     ret = PolarScan_setDate((PolarScan_t*)object, ndate);
     ret = PolarScan_setTime((PolarScan_t*)object, ntime);
     ret = PolarScan_setSource((PolarScan_t*)object, source);
   }

   /* 
    * set some top level where attrubutes for polar data types
    * See Table 4 of the OPERA Information Model for HDF5
    */
   double site_lat_degrees = (double)
   file_element_p->product_header_p->end.latitude_of_center/
        (4294967296.0 / 360.0);
   if(site_lat_degrees > 180.0) site_lat_degrees -= 360.0;
   double site_lon_degrees = (double) 
        file_element_p->product_header_p->end.longitude_of_center/
        (4294967296.0 / 360.0);
   if(site_lon_degrees > 180.0) site_lon_degrees -= 360.0;
   double site_msl_height_in_m = (double)
        file_element_p->product_header_p->
        end.signed_ground_height_relative_to_sea_level;
   double radar_agl_height_in_m = (double)
        file_element_p->product_header_p->
        end.height_of_radar_above_the_ground_in_meters;
   double total_above_sea_level_height = site_msl_height_in_m + radar_agl_height_in_m;
   double site_lat_radians = site_lat_degrees * 3.14159265358979323846 / 180.0;
   double site_lon_radians = site_lon_degrees * 3.14159265358979323846 / 180.0;        
   if(RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      PolarVolume_setLatitude((PolarVolume_t*)object, site_lat_radians);
      PolarVolume_setLongitude((PolarVolume_t*)object, site_lon_radians);
      PolarVolume_setHeight((PolarVolume_t*)object, total_above_sea_level_height);
    } else if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
      PolarScan_setLatitude((PolarScan_t*)object, site_lat_radians);
      PolarScan_setLongitude((PolarScan_t*)object, site_lon_radians);
      PolarScan_setHeight((PolarScan_t*)object, total_above_sea_level_height);
    }
   /* Set optional 'how' attributes. 
    * There are lots! See Table 8 in the ODIM_H5 spec. */
   /*
    * We put the name of the volume scan, eg 'DOPVOL1_A' into the how 
    * attribute 'task'. Use snprintf to change a non-terminated character
    * array into a terminated string.
    */
   char task_name[13];
   ncopied = snprintf(task_name,12,"%s",
                      file_element_p->product_header_p->pcf.dataGen_task_name);
   if(ncopied > 0 ) rltrim(task_name);
   ret = addStringAttribute(object, "how/task", task_name);
   char radar_system[8];
   ncopied = snprintf(radar_system,8,"VAIS%3.3i",
             file_element_p->product_header_p->end.type_of_signal_processor_used);
   if(ncopied > 0 ) rltrim(radar_system);
   ret = addStringAttribute(object, "how/system", radar_system);
   /* for now, hardwire this attribute */
   ret = addStringAttribute(object, "how/TXtype", "magnetron");
   int polmode = (int) file_element_p->ingest_header_p->
                       tcf.misc.type_of_polarization;
   switch(polmode)
   {
   case(0):
      ret = addStringAttribute(object, "how/polmode", "single-H");
      break;
   case(1):
      ret = addStringAttribute(object, "how/polmode", "single-V");
      break;
   case(2):
      ret = addStringAttribute(object, "how/polmode", "switched-dual");
      break;
   case(3):
      ret = addStringAttribute(object, "how/polmode", "simultaneous-dual");
      break;
   }
   ret = addStringAttribute(object, "how/software", "IRIS");
   /* iris version number should be something like 8.13 */
   char iversion[5];
   ncopied = snprintf(iversion,5,"%s",
             file_element_p->product_header_p->end.IRIS_version_product_maker);
   if(ncopied > 0 ) rltrim(iversion);
   double IRIS_version = 0.0;
   size_t siz = sscanf(iversion,"%lf",&IRIS_version);
   if(siz > 0) {
      ret = addStringAttribute(object, "how/sw_version", iversion);
   }
   ret = addStringAttribute(object, "how/simulated", "False");
   double wavelength_in_cm = (double) file_element_p->
        product_header_p->end.wavelength_in_hundredths_of_centimeters;
   wavelength_in_cm =wavelength_in_cm / 100.0;
   ret = addDoubleAttribute(object, "how/wavelength", wavelength_in_cm);
   UINT2 antenna_scan_mode = 
       file_element_p->ingest_header_p->tcf.scan.antenna_scan_mode;
   /* 
    * The scan speed in degrees per second is only available in the PPI
    * scan modes.
    *
    * Antenna Scan Mode (TASK_SCAN_xxx)
    *  1:PPI sector
    *  2:RHI sector
    *  3:Manual
    *  4:PPI full
    *  5:file
    *  6:exec
    *  7:RHI full
    */
   switch(antenna_scan_mode)
   {
   case(1):
   case(4):
      {
      double scan_speed_DegPerSec = (double) 
      file_element_p->ingest_header_p->tcf.scan.scan_speed
            / 65536.0 * 360.0;
      double rpm = scan_speed_DegPerSec / 360.0 * 60.;
      ret = addDoubleAttribute(object, "how/rpm", rpm);
      }
      break;
   case(2):
   case(7):
      /*
       * will have to figure out what to do for the case of RHIs
       */
       break;
   }
   double pw_us = (double) file_element_p->product_header_p->end.
   pulse_width_in_hundredths_of_microseconds /100.0;
   ret = addDoubleAttribute(object, "how/pulsewidth", pw_us);
        
   double band_width_MHz = (double) file_element_p->
        product_header_p->end.receiver_bandwidth_in_kHz / 1000.0;
   ret = addDoubleAttribute(object, "how/RXbandwidth", band_width_MHz);
        
   int prfcontrol = (int) file_element_p->ingest_header_p->
        tcf.dsp.multi_PRF_mode_flag;
   double PRF_hi_hz = (double)
        file_element_p->product_header_p->end.PRF_in_hertz;
   double PRF_lo_hz = 0.0;
   /*
    * Is there is more than one PRF ?
    * PRF_FIXED   (0)         Fixed trigger rate
    * PRF_2_3     (1)         2:3 PRF Ratio
    * PRF_3_4     (2)         3:4 PRF Ratio
    * PRF_4_5     (3)         4:5 PRF Ratio
    */
   switch(prfcontrol)
   {
   case(1):
      PRF_lo_hz = PRF_hi_hz /3.0 * 2.0;
      break;    
   case(2):
      PRF_lo_hz = PRF_hi_hz/4.0 * 3.0;
      break;
   case(3):
      PRF_lo_hz = PRF_hi_hz/5.0 * 4.0;
      break;
   }
   /* Note! Only 1 or 2 PRFs are aloowed VAISALA/IRIS radars */
   switch(prfcontrol)
   {
   case(0):
      ret = addDoubleAttribute(object, "how/lowprf",  PRF_hi_hz);
      ret = addDoubleAttribute(object, "how/midprf",  PRF_hi_hz);
      ret = addDoubleAttribute(object, "how/highprf", PRF_hi_hz);
      break;
   case(1):
   case(2):
   case(3):
      ret = addDoubleAttribute(object, "how/lowprf",  PRF_lo_hz);
      ret = addDoubleAttribute(object, "how/midprf",  PRF_hi_hz);
      ret = addDoubleAttribute(object, "how/highprf", PRF_hi_hz);
      break;
   }
   /*
    *        ret = addDoubleAttribute(object, "how/TXlossH", 1.0);
    *        ret = addDoubleAttribute(object, "how/TXlossV", 1.0);
    *        ret = addDoubleAttribute(object, "how/injectlossH", 1.0);
    *        ret = addDoubleAttribute(object, "how/injectlossV", 1.0);
    *        ret = addDoubleAttribute(object, "how/RXlossH", 1.0);
    *        ret = addDoubleAttribute(object, "how/RXlossV", 1.0);
    *        ret = addDoubleAttribute(object, "how/radomelossH", 1.0);
    *        ret = addDoubleAttribute(object, "how/radomelossV", 1.0);
    */
   /*
    * Is the gain fixed ?
    * GAIN_FIXED   (0)         Fixed receiver gain
    * GAIN_STC     (1)         STC Gain
    * GAIN_AGC     (2)         AGC Gain
    *
    * Note! The fixed gain seems to be always zero (?)
    * so we are commenting the antenna gain part out until
    * further notice!
    * 
    *        int gaincontrol = (int)file_element_p->ingest_header_p->
    *                                       tcf.dsp.gain_control_flag;
    *        double fixed_gain_level = (double) file_element_p->
    *               ingest_header_p->tcf.dsp.fixed_gain / 1000.F;
    *        if(gaincontrol == 0) {
    *          ret = addDoubleAttribute(object, "how/antgainH", fixed_gain_level);
    *          ret = addDoubleAttribute(object, "how/antgainV", fixed_gain_level);
              }
    */
        
   /* 
    * IRIS supplies the angular beam width for both horizontal and
    * vertical polarizations (they tend to be identical).
    * Note! No beam widths were recorded in really old versions of IRIS, 
    * (pre-7.18).
    */
   if( IRIS_version >= 7.18 ) {
      double hor_beam_width_angle = 
         (double) file_element_p->ingest_header_p->
         tcf.misc.horizontal_beam_width/4294967296.0 * 360.0;
      double ver_beam_width_angle = 
         (double) file_element_p->ingest_header_p->
         tcf.misc.vertical_beam_width/4294967296.0 * 360.0;
      ret = addDoubleAttribute(object, "how/beamwH", hor_beam_width_angle);
      ret = addDoubleAttribute(object, "how/beamwidth", hor_beam_width_angle);
      ret = addDoubleAttribute(object, "how/beamwV", ver_beam_width_angle);
   }
   /* gas attenuation (dB/km) */
   double gas_attenuation = 
      (double) file_element_p->ingest_header_p->
      tcf.dsp.gas_attenuation/ 100000.;
        
   ret = addDoubleAttribute(object, "how/gasattn", gas_attenuation);
        
   double radarc_H = (double) file_element_p->ingest_header_p->
        tcf.cal.radar_constant_Horiz_in_hundredths_of_dB/100.;
   ret = addDoubleAttribute(object, "how/radconstH", radarc_H);
   double radarc_V = (double) file_element_p->ingest_header_p->
        tcf.cal.radar_constant_Vert_in_hundredths_of_dB/100.;
   ret = addDoubleAttribute(object, "how/radconstV", radarc_V);
   double tpower = (double) file_element_p->ingest_header_p->
        tcf.misc.transmit_power_in_watts /1000.;
   ret = addDoubleAttribute(object, "how/nomTXpower", tpower);
   /* must confirm that this should be always zero */
   ret = addDoubleAttribute(object, "how/powerdiff", 0.0);
   double nyquist_velocity = calc_nyquist(file_element_p);
   ret = addDoubleAttribute(object, "how/NI", nyquist_velocity);
   long nsamples = (long) file_element_p->
      product_header_p->end.number_of_samples_used;
   ret = addLongAttribute(object, "how/Vsamples", nsamples);
   /*
    *        ret = addStringAttribute(object, "how/azmethod", "AVERAGE");
    *        ret = addStringAttribute(object, "how/elmethod", "AVERAGE");
    *        ret = addStringAttribute(object, "how/binmethod", "AVERAGE");
    */
   /* is angle synching enabled ? */
   if(0 != (GIS_SYENAB & file_element_p->ingest_header_p->GParm.istat_i)) {
      /* yes angle synching IS enabled
       * are the sych table values elevation angles?
       */
      if(0 != (GIS_SYEL & file_element_p->ingest_header_p->GParm.istat_i)) {
         ret = addStringAttribute(object, "how/anglesync", "elevation");
      }
      else {
         ret = addStringAttribute(object, "how/anglesync", "azimuth");
      }
   }
/*
   ret = addDoubleAttribute(object, "how/anglesyncRes", 1.0);
   // Corrupt scans need to be caught 
   ret = addStringAttribute(object, "how/malfunc", "False");
   // BITE message if malfunc=True 
   ret = addStringAttribute(object, "how/radar_msg", "");
   ret = addDoubleAttribute(object, "how/NEZH", 1.0);
   ret = addDoubleAttribute(object, "how/NEZV", 1.0);
   ret = addStringAttribute(object, "how/clutterType", "Daniel's");
   ret = addStringAttribute(object, "how/clutterMap", "/daniel/clutter.map");
   ret = addDoubleAttribute(object, "how/zcalH", 1.0);
   ret = addDoubleAttribute(object, "how/zcalV", 1.0);
   ret = addDoubleAttribute(object, "how/nsampleH", 1.0);
   ret = addDoubleAttribute(object, "how/nsampleV", 1.0);
*/
   double sqi_thresh = (double) file_element_p->ingest_header_p->
        tcf.cal.SQI_threshold / 256.F;
   ret = addDoubleAttribute(object, "how/SQI", sqi_thresh);
   double ccor_threshold = (double) file_element_p->ingest_header_p->
        tcf.cal.clutter_correction_threshold/ 16.F;
   ret = addDoubleAttribute(object, "how/CSR", ccor_threshold);
   double log_threshold = (double) file_element_p->ingest_header_p->
        tcf.cal.reflectivity_noise_threshold / 16.F;
   ret = addDoubleAttribute(object, "how/LOG", log_threshold);
   double transmitP = (double) file_element_p->ingest_header_p->
        tcf.misc.transmit_power_in_watts;
   ret = addDoubleAttribute(object, "how/avgpwr", transmitP);
   int sweep_ind = 0;
   if(nscans > 1) {
      /* Populate each scan of a PVOL */
      for( i=0; i < nscans; i++) {
         PolarScan_t* scan = RAVE_OBJECT_NEW(&PolarScan_TYPE);
         sweep_ind = i;
         ret = populateScan(scan, file_element_p, sweep_ind);
         if(ret != 0) {
            /* 
             * assume that if we cannot successfully populate 
             * the scan then we might as well quit processing 
             * this file
             */
            RAVE_OBJECT_RELEASE(scan);
            if(file_element_p != NULL) {
               free_IRIS(&file_element_p);
            }
            return -1;
         }
         ret = PolarVolume_addScan((PolarVolume_t*)object, scan);
         RAVE_OBJECT_RELEASE(scan);
      }
   } 
   else {
      /* Only one scan to populate */
      ret = populateScan((PolarScan_t*)object, file_element_p, sweep_ind);
      if(ret != 0) {
         /* 
          * assume that if we cannot successfully populate 
          * this scan then we might as well quit processing 
          * this file
          */
         RAVE_OBJECT_RELEASE(object);
         if(file_element_p != NULL) {
            free_IRIS(&file_element_p);
         }
         return -1;
      }
   }
   return 0;
} /* end function populateObject */

/**
 * Function name: readIRIS
 * Intent: This function alocates a structure that will hold all data from 
 * an input file. First it allocates the structure itself (which contains
 * pointers only), then allocates the structures corresponding to each pointer
 * in the main structure.  Then the top level structure allocated in this 
 * function and the file to be read are passed to a function that reads the 
 * input file and fills the top level structure.  The top level structure
 * is then passed back to the calling program.
 * 
 * Input:
 * (1) a pointer to a string of characters holding the input file name to read
 * Output:
 * (1) a pointer to a structure that holds the contents of an input file
 */
file_element_s* readIRIS(const char* ifile) {
   int ret = -1;
   file_element_s* file_element_p = NULL;
   if (!is_regular_file(ifile)) {
     return NULL;
   }
   /*****************************************************************************
    *                                                                           *
    * allocate space for a file element structure, the file space pointed to by *
    * pointer 'file_element_p' is then ready for use                            *
    *                                                                           *
    ****************************************************************************/
   file_element_p = RAVE_MALLOC(sizeof(file_element_s));
   if(file_element_p == NULL) {
      /* inform client about allocation failure */
      Iris_printf("Error allocating 'file_element' structure.\n");
      return NULL;
   }
   file_element_p->product_header_p = NULL;
   file_element_p->ingest_header_p = NULL;
   file_element_p->sweep_list_p = NULL;
   /*****************************************************************************
    *                                                                           *
    * allocate space for a file product_header structure inside the             *
    * file_element structure.                                                   *
    *                                                                           *
    ****************************************************************************/
   file_element_p->product_header_p = RAVE_MALLOC(sizeof(phd_s));
   if(file_element_p->product_header_p == NULL) {
      /* inform client about allocation failure */
      Iris_printf("Error allocating 'product_header' structure.\n");
      RAVE_FREE(file_element_p);
      return NULL;
   }
   /*****************************************************************************
    * likewise                                                                  *
    * allocate space for a ingest_header structure inside the                   *
    * file_element structure.                                                   *
    *                                                                           *
    ****************************************************************************/
   file_element_p->ingest_header_p = RAVE_MALLOC(sizeof(ihd_s));
   if(file_element_p->ingest_header_p == NULL) {
      /* inform client about allocation failure */
      Iris_printf("Error allocating 'ingest_header' structure.\n");
      free_IRIS(&file_element_p);
      return NULL;
   }
   /*****************************************************************************
    *  Each file element has a sweep list in it                                 *
    *  allocate space and initialize the sweep list structure                   *
    *                                                                           *
    ****************************************************************************/
   file_element_p->sweep_list_p = IrisDList_create();
   if( file_element_p->sweep_list_p == NULL) {
      /* Tell client that the allocation failed */
      Iris_printf("Error allocating sweep IrisDList_t structure.\n");
      free_IRIS(&file_element_p);
      return NULL;
   }

   /*****************************************************************************
    * Read IRIS file and put all file contents into a doubly-linked list        *
    * and return a pointer to that list.  Note! A lot of memory allocation goes *
    * on in the called function and the functions it calls, but all those are   *
    * deallocated if something goes wrong, so that the returned integer is non- *
    * zero and the file_element_p pointer is set to NULL.                       *
    *                                                                           *
    ****************************************************************************/
   ret = iris2list(ifile,&file_element_p);
   if (ret == 0) return file_element_p;
   else return NULL;
} /* end Function readIRIS */

/**
 * Function name: objectTypeFromIRIS
 * Intent: Based on number of scans in the payload, return the corresponding RAVE ObjectType enum
 * for SCAN or PVOL
 */
int objectTypeFromIRIS(file_element_s* file_element_p) {
   if (file_element_p != NULL) {
      IrisDList_t *sweep_list = file_element_p->sweep_list_p;
      int sweeps_received = sweep_list->size;
      if(sweeps_received > 1) return Rave_ObjectType_PVOL;
      else return Rave_ObjectType_SCAN;
   } 
   else {
      return Rave_ObjectType_UNDEFINED;
   }
} /* End function objectTypeFromIRIS */

/**
 * Function name: free_IRIS
 * Intent: frees resources used to represent IRIS payload in memory
 * Specifically this function deallocates the entire file element structure
 * Note: an ingest header is NOT the same as an ingest data header
 */
void free_IRIS(file_element_s  **file_element_pp) {
   IrisDList_t *sweep_list = NULL;
   IrisDList_t *types_list = NULL;
   IrisDList_t *rays_list = NULL;
   IrisDListElement_t *this_sweep_element = NULL;
   IrisDListElement_t *this_type_element = NULL;
   IrisDListElement_t *this_ray_element = NULL;
   datatype_element_s *this_type = NULL;
   sweep_element_s *this_sweep_data = NULL;
   idh_s *this_ingest_data_header = NULL;
   ray_s* ray_structure_p = NULL;
   int nsweeps = 0;
   int ntypes = 0;
   int nrays = 0;
   /* 
    * if the file element pointer is NULL then 
    * the file element was aready freed or was never allocated 
    */
   if(*file_element_pp == NULL) return; 
   /* 
    * free 2 of 3 elements of the file element structure.
    * These are straight forward because they have no lists
    * or pointers, just structures holding structures.
    */
   if((*file_element_pp)->ingest_header_p != NULL) {
      RAVE_FREE((*file_element_pp)->ingest_header_p);
      (*file_element_pp)->ingest_header_p = NULL;
   }
   if((*file_element_pp)->product_header_p != NULL) {
      RAVE_FREE((*file_element_pp)->product_header_p);
      (*file_element_pp)->product_header_p = NULL;
   }
   /*
    * if the sweep list pointer is NULL then
    * it's OK to skip past the next part
    */
   if((*file_element_pp)->sweep_list_p == NULL) goto x;
   
   /* use a shorter named pointer to the sweep list */
   sweep_list = (*file_element_pp)->sweep_list_p;
   nsweeps = sweep_list->size;
   /*
    * if the sweep list is empty then
    * it's OK to skip past the next part
    */
   if(nsweeps == 0) goto w;
   /*
    * loop through the sweep list removing each sweep until none remain
    */
   for(int l=0; l < nsweeps; l++) {
      /* point to this element of the sweep list */
      if(l==0) this_sweep_element = IrisDList_head(sweep_list);
      else this_sweep_element = IrisDListElement_next(this_sweep_element);
      /* 
       * if this sweep element is NULL then this list has ended pre-maturely
       * exit the loop
       * Note! This should never actually happen
       */
      if(this_sweep_element == NULL) {
         Iris_printf("Error! Premature end of sweep "
                        "list in free_IRIS function.\n");
         break;
      }
      this_sweep_data = (sweep_element_s *) this_sweep_element->data;
      /* 
       * if data pointed to by this sweep element is NULL 
       * then there is nothing to free in this sweep
       * skip this sweep element
       * Note! This should never actually happen
       */
      if(this_sweep_data == NULL) {
         Iris_printf("Error! Sweep element with no data "
                 "encountered in free_IRIS function.\n");
         continue;
      }
      types_list = this_sweep_data->types_list_p;
      /* 
       * if the type list pointed to by this sweep element is NULL 
       * then there is nothing to free in this sweep
       * skip this sweep element
       * Note! This should never actually happen
       */
      if(types_list == NULL) {
         Iris_printf("Error! Sweep element with no types list "
                 "encountered in free_IRIS function.\n");
         continue;
      }
      ntypes = types_list->size;
      /* 
       * if the type list pointed to by this sweep element has zero length
       * then there are no type elements to free in this sweep
       * OK to skip ahead
       * Note! This should never actually happen
       */
      if(ntypes == 0) {
         Iris_printf("Error! Zero length types list "
                 "encountered in free_IRIS function.\n");
         goto u;
      }
      /*
       * For each datatype (dBT for example) free 
       * (1) an ingest data header structure
       * (2) list of ray_s structures.  
       * Note! a ray_s structure has no pointers and has a known size. 
       */
      this_type_element = NULL;
      for(int m=0; m < ntypes; m++) {
         /* point to this element of the type list */
         if(m==0) this_type_element = IrisDList_head(types_list);
         else this_type_element = IrisDListElement_next(this_type_element);
         /* 
          * if this type element is NULL then this list has ended pre-maturely
          * exit the loop
          * Note! This should never actually happen
          */
         if(this_type_element == NULL) {
            Iris_printf("Error! Premature end of type "
                    "list in free_IRIS function.\n");
            break;
         }
         /* make a pointer named 'this_type' to the data structure in this element */
         this_type = (datatype_element_s *) this_type_element->data;
         /* 
          * if data pointed to by this type element is NULL 
          * then there is nothing to free in this type
          * skip this type element
          * Note! This should never actually happen
          */
         if(this_type == NULL) {
            Iris_printf("Error! Data-type element with no data "
                    "encountered in free_IRIS function.\n");
            continue;
         }
         /* set a pointer to the ray list in the data structure of this element  */
         rays_list = this_type->ray_list_p;
         /* set a pointer to the ingest_data_header in the data structure of this element */
         this_ingest_data_header = this_type->ingest_data_header_p;
         /*
          * free the ingest data header structure if pointer is non-NULL
          */
         if(this_ingest_data_header != NULL) {
            RAVE_FREE(this_ingest_data_header);
            this_ingest_data_header = NULL;
         }
         /* 
          * if the rays list pointed to by this type element is NULL 
          * then there is nothing to free
          * skip this type element
          * Note! This should never actually happen
          */
         if(rays_list == NULL) {
            Iris_printf("Error! Type element with no rays list "
                    "encountered in free_IRIS function.\n");
            continue;
         }
         nrays = rays_list->size;
         /* 
          * if the rays list pointed to by this data-type element has zero length
          * then there are no rays to free in this data-type
          * OK to skip ahead
          * Note! This should never actually happen
          */
         if(nrays == 0) {
            Iris_printf("Error! Zero length rays list "
                    "encountered in free_IRIS function.\n");
            goto t;
         }
         
         if(rays_list->size == 0) goto t;
         /* 
          * free the rays list itself
          */
         while( nrays > 0) {
            /*
             * point to the last ray element in the ray list
             */
            this_ray_element = rays_list->tail;
            /*
             * point to the data in that ray element in the ray list
             */
            ray_structure_p = (ray_s *) this_ray_element->data;
            /* 
             * adjust the list so that the last element is
             * no longer in the list
             */
            if(this_ray_element == rays_list->head) {
               rays_list->head = this_ray_element->next;
               if(rays_list->head == NULL) {
                  rays_list->tail = NULL;
               }
               else {
                  this_ray_element->next->prev = NULL;
               }
            }
            else {
               this_ray_element->prev->next = this_ray_element->next;
               if(this_ray_element->next == NULL) {
                  rays_list->tail = this_ray_element->prev;
               }
               else { 
                  this_ray_element->next->prev = this_ray_element->prev;
               }
            }
            nrays--;
            /* 
             * free the ray_s structure, which is the 'data' part of 
             * this ray list element of the rays list 
             */
            if( ray_structure_p != NULL) {
               RAVE_FREE(ray_structure_p);
               ray_structure_p = NULL;
            }
            /* 
             * free the memory associated with this ray list element
             */
            if( this_ray_element != NULL) { 
               RAVE_FREE(this_ray_element);
               this_ray_element = NULL;
            }
            /* repeat for all elements in the rays list */
         } /* end while rays_list->size > 0 */
         /*
          * free the ray list structure
          */
t:       if(rays_list != NULL) RAVE_FREE(rays_list);
         rays_list = NULL;
      } /* end for m < ntypes */
      /* 
       * free the types list structure
       * Note! All of the data in the list elements have been freed above
       * 
       * Note! Program does not get to this location if the pointer 
       * to the types list is NULL, or the length of the list is zero.
       */
      while(ntypes > 0) {
         /*
          * point to the last data-type element in the type list
          */
         this_type_element = types_list->tail;
         /*
          * point to the data in that data-type element in the type list
          */
         this_type = (datatype_element_s *) this_type_element->data;
         /* 
          * adjust the list so that the last element is
          * no longer in the list
          */
         if (this_type_element == types_list->head) {
            types_list->head = this_type_element->next;
            if(types_list->head == NULL) types_list->tail = NULL;
            else this_type_element->next->prev = NULL;
         }
         else {
            this_type_element->prev->next = this_type_element->next;
            if (this_type_element->next == NULL) 
               types_list->tail = this_type_element->prev;
            else 
               this_type_element->next->prev = this_type_element->prev;
         }
         ntypes--;
         /* free the memory associated with the data structure */
         if( this_type != NULL) { 
            RAVE_FREE(this_type);
            this_type = NULL;
         }
         /* free the memory associated with the list element */
         if( this_type_element != NULL) { 
            RAVE_FREE(this_type_element);
            this_type_element = NULL;
         }
      }  /* end while ntypes > 0 */
      /*
       * free the types list structure
       */
u:    if(types_list != NULL) RAVE_FREE(types_list);
      types_list = NULL;
   } /* end for-loop of all sweeps in the sweep list */
   /* 
    * free the sweep list itself
    * Note! All of the data in the list elements have been freed above
    */
   while(nsweeps > 0) {
      this_sweep_element = sweep_list->tail;
      this_sweep_data = (sweep_element_s *) this_sweep_element->data;
      /* 
       * adjust the list so that the last element is
       * no longer in the list
       */
      if(this_sweep_element == sweep_list->head) {
         sweep_list->head = this_sweep_element->next;
         if (sweep_list->head == NULL) sweep_list->tail = NULL;
         else this_sweep_element->next->prev = NULL;
      }
      else {
         this_sweep_element->prev->next = this_sweep_element->next;
         if (this_sweep_element->next == NULL) 
            sweep_list->tail = this_sweep_element->prev;
         else 
            this_sweep_element->next->prev = this_sweep_element->prev;
      }
      nsweeps--;
      /* 
       * free the data structure in this sweep element
       * Note! The contents of the data structure for sweeps is a types list
       * So this data structure should already have been freed
       */
      if( this_sweep_data != NULL) { 
         RAVE_FREE(this_sweep_data);
            this_sweep_data = NULL;
      }
      /* free the memory associated with this sweep list element */
      if( this_sweep_element != NULL) { 
         RAVE_FREE(this_sweep_element);
            this_sweep_element = NULL;
      }
      /* repeat for all elements in the sweep list */
   } /* end while */
   /*
    * free the sweeps list structure
    */
w: if(sweep_list != NULL) RAVE_FREE(sweep_list);
   sweep_list = NULL;

   /* 
    * free the file element structure itself
    * which is just 3 pointers that point to things
    * that have been freed in the above code
    */
x: if(*file_element_pp != NULL) { 
      RAVE_FREE(*file_element_pp);
      *file_element_pp = NULL;
   }
   return;
} /* end function free_IRIS */

/**
 * Function name: is_regular_file
 * Intent: determines whether the given path is to a regular file
 */
int is_regular_file(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
} /* end function is_regular_file */


/**
 * Function name: isIRIS
 * Intent: determines if the given file is intended to be in IRIS format
 */
int isIRIS(const char *path) {

   FILE *file = fopen(path, "rb" );

   /* fopen returns 0, the NULL pointer, on failure */
   if (file == 0) { /* if failed to open file, tell the client */
      Iris_printf("Could not open file %s. Will abort!\n",path );
      return -1;
   }
   else {
      char c[2];
      c[0] = fgetc(file);
      c[1] = fgetc(file);
      fclose(file);
      short *structID_p;
      structID_p= (short *) &c;
      if( *structID_p == 27) {
         /*
          * The file is an IRIS file AND
          * The file is little-endian.
          */
         return 0;
      } 
      else return -1;
   }
} /* end function isIRIS */


/**
 * Function name: setRayAttributes
 * Intent: A function to transfer info to RAVE objects for eventual output
 * to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an empty Toolbox polar scan object
 * (2) a pointer to an object holding IRIS RAW-file data
 * (3) a pointer to the list of ray structures for 1st moment of this sweep
 * (4) a pointer to a structure holding consistency-check info
 * (5) a signed integer, the scan/sweep that this function call is handling
 * Note! The origin of the sweep index is zero, not 1
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int setRayAttributes(PolarScan_t* scan, 
                     file_element_s* file_element_p,
                     cci_s *cci_p,
                     int sweep_index,
                     ra_s **ra_pp) {
   ra_s *ra_p = *ra_pp;
   IrisDList_t *sweep_list = file_element_p->sweep_list_p;
   int n_sweeps_in_volume = IrisDList_size(sweep_list);
   long max_nrays = ra_p->expected_nrays;
   double istartazT, istopazT;
   /* fill the ray start and stop times */
   int first_ray_in_sweep = (int) 
      cci_p->index_of_first_ray_timewise_p[sweep_index];
   /*
    * Note! The following version of sweep start time is the fractional
    * number of seconds after midnight Jan011970
    * Since we want to determine the sweep duration these times are sufficient
    */
   double sweep_start_time = 
      (double) cci_p->sweep_start_times_mtv_p[sweep_index]->tv_sec +
      (double) cci_p->sweep_start_times_mtv_p[sweep_index]->tv_usec / 1.0E6;
   double sweep_stop_time;
   /*
    * Set the stoptime to the next start time
    * OR if there use no next sweep
    * set the stop time to the product generation time
    */
   if( sweep_index == n_sweeps_in_volume-1) {
      /* the product generation time is recorded after the last sweep */
      ymd_s *product_generation_ymd_p =
              &(file_element_p->product_header_p->pcf.product_GenTime_UTC);
      mtv_s *product_generation_mtv_p = ymd_to_mtv(product_generation_ymd_p);
      sweep_stop_time = 
         (double) product_generation_mtv_p->tv_sec +
         (double) product_generation_mtv_p->tv_usec / 1.0E6;
         if(product_generation_mtv_p != NULL) 
            RAVE_FREE(product_generation_mtv_p);
   }
   else {
      sweep_stop_time = 
         (double) cci_p->sweep_start_times_mtv_p[sweep_index+1]->tv_sec +
         (double) cci_p->sweep_start_times_mtv_p[sweep_index+1]->tv_usec / 1.0E6;
      /* 
       * subtract a tenth of second from sweep_stop_time since we
       * cannot have one sweep end at the exact same time as the next sweep
       * starts.
       */
      sweep_stop_time -= 0.1;
   }
   double last_whole_second = (double)
      cci_p->ray_highest_integral_seconds_p[sweep_index];
   /*
    * make an estimated-stop-time that is the last whole second from start time
    * (that we get from ray header data) plus one second
    */
   double sweep_stop_time_est = sweep_start_time + (last_whole_second+1.0);
   /*
    * if the estimated estimated stop time is less than the next-sweep
    * start-time then use the estimated stop time (this should happen 
    * ~all of the time).
    */
   if(sweep_stop_time_est < sweep_stop_time) sweep_stop_time = sweep_stop_time_est;
   double sweep_duration = sweep_stop_time - sweep_start_time;
   /*
    * Since IRIS does not record the start and end time of each
    * ray in it's RAW Product file, we designate some appropriate
    * start and end times that are not that accurate since around
    * of 1/4 of the time between sweeps may be used just to arrive
    * at the new fixed angle.
    */
   double nextazT = -1.0;
   for(int jj=0; jj < max_nrays; jj++) {
      if(jj==0) {
         istartazT = sweep_start_time + 
           (double) jj / (double) max_nrays * sweep_duration;
         nextazT = sweep_start_time + 
           (double) (jj+1) / (double) max_nrays * sweep_duration;
      }
      else {
         istartazT = nextazT;
         nextazT = sweep_start_time + 
           (double) (jj+1) / (double) max_nrays * sweep_duration;
      }
      istopazT = nextazT - 0.001;
      int ii = first_ray_in_sweep + jj;
      if( ii > (max_nrays-1) ) ii -= max_nrays;
      ra_p->startazT[ii] = istartazT;
      ra_p->stopazT[ii] = istopazT;
   }
   /*
    * At this point the times should be all there but
    * there could be missing rays/data
    * in the arrays holding pointing angles
    */
   double angular_resolution_degrees = (double) 
   file_element_p->ingest_header_p->tcf.scan.angular_resolution_x1000 /
   1000.0;
   for(int jj=0; jj < max_nrays; jj++) {
      if(reldif(ra_p->startazA[jj],MY_FILL_DOUBLE) <= TOLERENCE ) {
         int start_index = jj;
         int end_index = -1;
         for(int kk=jj+1; kk < max_nrays; kk++) {
            if(!(reldif(ra_p->startazA[kk],MY_FILL_DOUBLE) <= TOLERENCE) ) {
               end_index = kk-1;
               break;
            }
         }
         if(end_index == -1) {
            /* missing values upto and including the last index 
             * if values do exist at the beginning of arrays
             * compute angles for indecies with missing values upto and including the end of array(s)
             */
            if((start_index-1) >= 0) {
               for(int mm=start_index; mm < max_nrays; mm++) {
                  ra_p->startazA[mm] = ra_p->startazA[mm-1] + angular_resolution_degrees;
                  ra_p->stopazA[mm] = ra_p->stopazA [mm-1] + angular_resolution_degrees;
                  ra_p->elangles[mm] = ra_p->elangles[mm-1];
               }
            }
            else {
               /*
                * all values are missing so make guesses and otherwise do what you can
                */
               for(int nn=0; nn < max_nrays; nn++) {
                  ra_p->startazA[nn] = angular_resolution_degrees*nn - 
                                       angular_resolution_degrees/2.;
                  ra_p->stopazA[nn] = angular_resolution_degrees*nn + 
                                       angular_resolution_degrees/2.;
                  if(ra_p->startazA[nn] < 0.0) ra_p->startazA[nn] += 360.0;
                  if(ra_p->startazA[nn] >= 360.0) ra_p->startazA[nn] -= 360.0;
                  ra_p->elangles[nn] = MY_FILL_DOUBLE;
               }
            }
         }
         else {
            /*
             * interval ends before we get to end of the array(s)
             * compute angles by using the single (good) value that follows this interval; 
             * extrapolate backwards until we get to the start of the interval 
             */
            for(int pp=end_index; pp >= start_index; pp--) {
               ra_p->startazA[pp] = ra_p->startazA[pp+1] - angular_resolution_degrees;
               ra_p->stopazA[pp] = ra_p->stopazA [pp+1] - angular_resolution_degrees;
               ra_p->elangles[pp] = ra_p->elangles[pp+1];
            }
         }
      }
   }
   /* 
    * Note! There are also startelT, stopelT, and TXpower to consider 
    */
   RaveAttribute_t* startazA_attr = 
      RaveAttributeHelp_createDoubleArray("how/startazA", ra_p->startazA, max_nrays);
   RaveAttribute_t* stopazA_attr = 
      RaveAttributeHelp_createDoubleArray("how/stopazA", ra_p->stopazA, max_nrays);
   RaveAttribute_t* startazT_attr = 
      RaveAttributeHelp_createDoubleArray("how/startazT", ra_p->startazT, max_nrays);
   RaveAttribute_t* stopazT_attr = 
      RaveAttributeHelp_createDoubleArray("how/stopazT", ra_p->stopazT, max_nrays);
   RaveAttribute_t* elangles_attr =
      RaveAttributeHelp_createDoubleArray("how/elangles", ra_p->elangles, max_nrays);
   PolarScan_addAttribute(scan, startazA_attr);
   PolarScan_addAttribute(scan, stopazA_attr);
   PolarScan_addAttribute(scan, startazT_attr);
   PolarScan_addAttribute(scan, stopazT_attr);
   PolarScan_addAttribute(scan, elangles_attr);
   RAVE_OBJECT_RELEASE(startazA_attr);
   RAVE_OBJECT_RELEASE(stopazA_attr);
   RAVE_OBJECT_RELEASE(startazT_attr);
   RAVE_OBJECT_RELEASE(stopazT_attr);
   RAVE_OBJECT_RELEASE(elangles_attr);
   return 0;
} /* end of function setRayAttributes */


/**
 * Function name: addLongAttribute
 * Intent: A function to transfer a long integer attribute to a RAVE 
 * object for eventual output to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an existing RAVE Toolbox core object
 * (2) a pointer to the name of an attribute to be stored
 * (3) a long integer holding the value of the attribute to be stored 
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int addLongAttribute(RaveCoreObject* object, const char* name, long value) {
   int ret = 0;
   RaveAttribute_t* attr;
   attr = RaveAttributeHelp_createLong(name, value);
   ret = addAttribute(object, attr);
   RAVE_OBJECT_RELEASE(attr);
   return ret;
}


/**
 * Function name: addDoubleAttribute
 * Intent: A function to transfer a double/floating-point-number attribute 
 * to a RAVE object for eventual output to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an existing RAVE Toolbox core object
 * (2) a pointer to the name of an attribute to be stored
 * (3) a double floating-point number holding the value of an attribute to be stored 
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int addDoubleAttribute(RaveCoreObject* object, const char* name, double value) {
   int ret = 0;
   RaveAttribute_t* attr;
   attr = RaveAttributeHelp_createDouble(name, value);
   ret = addAttribute(object, attr);
   RAVE_OBJECT_RELEASE(attr);
   return ret;
}


/**
 * Function name: addStringAttribute
 * Intent: A function to transfer a charcter-string attribute 
 * to a RAVE object for eventual output to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an existing RAVE Toolbox core object
 * (2) a pointer to the name of an attribute to be stored
 * (3) a pointer to a character-string holding the value of an attribute to be stored 
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int addStringAttribute(RaveCoreObject* object, const char* name, const char* value) {
   int ret = 0;
   RaveAttribute_t* attr;
   attr = RaveAttributeHelp_createString(name, value);
   ret = addAttribute(object, attr);
   RAVE_OBJECT_RELEASE(attr);
   return ret;
}


/**
 * Function name: addAttribute
 * Intent: A function to transfer a previously filled attribute 
 * to a RAVE Toolbox object for eventual output to an HDF-5 structured output file 
 * Input:
 * (1) a pointer to an existing RAVE Toolbox core object
 * (2) a pointer to a structure of attribute details to be stored
 * Output:
 * (1) a signed integer, a function status index where zero means 'no errors'
 */
int addAttribute(RaveCoreObject* object, RaveAttribute_t* attr) {
   int ret = 0;
   if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      ret = PolarVolume_addAttribute((PolarVolume_t*)object, attr);
   } 
   else {
      if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScan_TYPE)) {
         ret = PolarScan_addAttribute((PolarScan_t*)object, attr);
      } 
      else {
         if (RAVE_OBJECT_CHECK_TYPE(object, &PolarScanParam_TYPE)) {
            ret = PolarScanParam_addAttribute((PolarScanParam_t*)object, attr);
         } 
         else ret = 1;
      }
   }
   return ret;
}


/**
 * Function name: mapSource2Nod
 * Intent: A function to Mapping of IRIS site key name to ODIM node identifier.
 * Input:
 * (1) a pointer to a character string holding an IRIS radar site name
 * Output:
 * (1) a pointer to the address of a literal holding an ODIM node identifier.
 */
char* mapSource2Nod(const char* key) {
   if      (!strcmp(key,"BRITT"))            return "NOD:cawbi,PLC:Britt ON";
   else if (!strcmp(key,"MONTREAL_RIVER"))   return "NOD:cawgj,PLC:Montreal River ON";
   else if (!strcmp(key,"CWHK"))             return "NOD:cawhk,PLC:Carvel AB";
   else if (!strcmp(key,"CARVEL"))           return "NOD:cawhk,PLC:Carvel AB";
   else if (!strcmp(key,"JIMMY_LAKE"))       return "NOD:cawhn,PLC:Jimmy Lake AB";
   else if (!strcmp(key,"KING"))             return "NOD:cawkr,PLC:King ON";
   else if (!strcmp(key,"LAC_CASTOR"))       return "NOD:cawbm,PLC:Lac Castor QC";
   else if (!strcmp(key,"MCGILL"))           return "NOD:cawmn,PLC:McGill QC";
   else if (!strcmp(key,"WMN"))              return "NOD:cawmn,PLC:McGill QC";
   else if (!strcmp(key,"EXETER"))           return "NOD:cawso,PLC:Exeter ON";
   else if (!strcmp(key,"HOLYROOD"))         return "NOD:cawtp,PLC:Holyrood NL";
   else if (!strcmp(key,"ALDERGROVE"))       return "NOD:cawuj,PLC:Aldergrove BC";
   else if (!strcmp(key,"VILLEROY"))         return "NOD:cawvy,PLC:Villeroy QC";
   else if (!strcmp(key,"CWWW"))             return "NOD:cawww,PLC:Spirit River AB";
   else if (!strcmp(key,"SPIRIT_RIVER"))     return "NOD:cawww,PLC:Spirit River AB";
   else if (!strcmp(key,"GOOSEBAY"))         return "NOD:cawyr,PLC:Goose Bay NL";
   else if (!strcmp(key,"VALDIRENE"))        return "NOD:caxam,PLC:Valdirene QC";
   else if (!strcmp(key,"Bethune"))          return "NOD:caxbe,PLC:Bethune SK";
   else if (!strcmp(key,"SCHULER"))          return "NOD:caxbu,PLC:Schuler AB";
   else if (!strcmp(key,"DRYDEN"))           return "NOD:caxdr,PLC:Dryden ON";
   else if (!strcmp(key,"FRANKTOWN"))        return "NOD:caxft,PLC:Franktown ON";
   else if (!strcmp(key,"FOXWARREN"))        return "NOD:caxfw,PLC:Foxwarren MB";
   else if (!strcmp(key,"GORE"))             return "NOD:caxgo,PLC:Halifax NS";
   else if (!strcmp(key,"LANDRIENNE"))       return "NOD:caxla,PLC:Landrienne QC";
   else if (!strcmp(key,"MARIONBRIDGE"))     return "NOD:caxmb,PLC:Marion Bridge NS";
   else if (!strcmp(key,"MARBLEMT"))         return "NOD:caxme,PLC:Marble Mountain NL";
   else if (!strcmp(key,"CHIPMAN"))          return "NOD:caxnc,PLC:Chipman NB";
   else if (!strcmp(key,"LASSETER"))         return "NOD:caxni,PLC:Lasseter ON";
   else if (!strcmp(key,"PRINCE_GEORGE"))    return "NOD:caxpg,PLC:Prince George BC";
   else if (!strcmp(key,"RADISSON"))         return "NOD:caxra,PLC:Radisson SK";
   else if (!strcmp(key,"MTSICKER"))         return "NOD:caxsi,PLC:Mt Sicker BC";
   else if (!strcmp(key,"STRATHMORE"))       return "NOD:caxsm,PLC:Strathmore AB";
   else if (!strcmp(key,"SILVER_STAR"))      return "NOD:caxss,PLC:Silver Star BC";
   else if (!strcmp(key,"TIMMINS"))          return "NOD:caxti,PLC:Timmins ON";
   else if (!strcmp(key,"WOODLANDS"))        return "NOD:caxwl,PLC:Woodlands MB";
   else                                      return "NOD:xxxxx,PLC:Unknown";
   return NULL;
}

/**
 * Function name: mapDataType
 * Intent: A function to Mapping of IRIS data/moment type to ODIM data/moment type.
 * Input:
 * (1) an signed integer indicating an IRIS data/moment type (corresponding to the 
 * iris_data_type enum)
 * Output:
 * (1) a pointer to the address of a literal holding an ODIM data/moment type .
 * Mapping of IRIS data/moment type to ODIM 'Quantity' identifier.
 * Input: A signed integer corresponding to the iris_data_type enum
 * Return: The function returns the address of a literal that corrosponds to a 
 * data-type/moment name that can be found in the ODIM Data Information Model for H5
 * (see Table 16 in that document).
 * Note! some of the IRIS data-types/moments cannot be found in table 16 so we had to 
 * make some up.
 */
char* mapDataType(int irisType) {
   if( (irisType==DB_DBT)   || (irisType==DB_DBT2) ) return "TH";
   if( (irisType==DB_DBZ)   || (irisType==DB_DBZ2) || (irisType==DB_DBZC) || (irisType==DB_DBZC2) ) return "DBZH";
   if( (irisType==DB_VEL)   || (irisType==DB_VEL2) ) return "VRADH";
   if( (irisType==DB_WIDTH) || (irisType==DB_WIDTH2) ) return "W";
   if( (irisType==DB_ZDR)   || (irisType==DB_ZDR2) || (irisType==DB_ZDRC) || (irisType==DB_ZDRC2) ) return "ZDR";
   if( irisType==DB_RAINRATE2 ) return "RATE";
   if( (irisType==DB_KDP)   || (irisType==DB_KDP2) ) return "KDP";
   if( (irisType==DB_PHIDP) || (irisType==DB_PHIDP2) ) return "PHIDP";
   if( (irisType==DB_VELC)  || (irisType==DB_VELC2) ) return "VRADDH";
   if( (irisType==DB_SQI)   || (irisType==DB_SQI2) ) return "SQIH";
   if( (irisType==DB_RHOHV) || (irisType==DB_RHOHV2) ) return "RHOHV";
   if( (irisType==DB_LDRH)  || (irisType==DB_LDRH2) ) return "LDRH";  /* Not ODIM */
   if( (irisType==DB_LDRV)  || (irisType==DB_LDRV2) ) return "LDRV";  /* Not ODIM */
   if( (irisType==DB_FLAGS) || (irisType==DB_FLAGS2) ) return "QIND";  /* This is surely wrong */
   if( irisType==DB_HEIGHT) return "HGHT";
   if( (irisType==DB_VIL2)  || (irisType==DB_FLIQUID2) ) return "VIL";  /* FLIQUID part is probably wrong */
   if( irisType==DB_DIVERGE2) return "div";
   if( irisType==DB_DEFORM2)  return "def";
   if( irisType==DB_VVEL2)    return "w";
   if( irisType==DB_HVEL2)    return "ff";
   if( irisType==DB_HDIR2)    return "dd";
   if( irisType==DB_AXDIL2)   return "ad";
   if( (irisType==DB_RHOH)   || (irisType==DB_RHOH2) ) return "RHOH";  /* Not ODIM */
   if( (irisType==DB_RHOV)   || (irisType==DB_RHOV2) ) return "RHOV";  /* Not ODIM */
   if( (irisType==DB_PHIH)   || (irisType==DB_PHIH2) ) return "PHIH";  /* Not ODIM */
   if( (irisType==DB_PHIV)   || (irisType==DB_PHIV2) ) return "PHIV";  /* Not ODIM */
   if( (irisType==DB_HCLASS) || (irisType==DB_HCLASS2) ) return "CLASS";
   if( irisType==DB_TEMPERATURE16) return "TEMP";  /* Not ODIM */
   if( irisType==DB_VIR16) return "VIR";  /* Not ODIM */
   if( (irisType==DB_DBTV8)   || (irisType==DB_DBTV16) ) return "TV";
   if( (irisType==DB_DBZV8)   || (irisType==DB_DBZV16) ) return "DBZV";
   if( (irisType==DB_SNR8)    || (irisType==DB_SNR16) ) return "SNRH";
   if( (irisType==DB_ALBEDO8) || (irisType==DB_ALBEDO16) ) return "ALBEDO";
   if( irisType==DB_VILD16) return "VILD";  /* Not ODIM */
   if( irisType==DB_TURB16) return "TURB";  /* Not ODIM */
   if( (irisType==DB_DBTE8) || (irisType==DB_DBTE16) ) return "TE";  /* Not ODIM */
   if( (irisType==DB_DBZE8) || (irisType==DB_DBZE16) ) return "DBZE";  /* Not ODIM */
   return NULL;
}

/**
 * Function name: rtrim
 * Intent: A function to trim trailing spaces from an input string.
 * Input:
 * (1) a pointer to a nul terminated character string 
 * Output:
 * None, with the exception of the side-effect is that the input string 
 * is changed.
 */
/*****************************************************************************
*                                                                            *
*  ---------------------------------- rtrim -------------------------------  *
*                                                                            *
*****************************************************************************/
void rtrim(char *str) {
   size_t n;
   n = strlen(str);
   while (n > 0 && isspace((unsigned char)str[n - 1])) {
      n--;
   }
   str[n] = '\0';
   return;
}

/**
 * Function name: ltrim
 * Intent: A function to trim leading spaces from an input string.
 * Input:
 * (1) a pointer to a nul terminated character string 
 * Output:
 * None, with the exception of the side-effect is that the input string 
 * is changed.
 */
/*****************************************************************************
*                                                                            *
*  ---------------------------------- ltrim -------------------------------  *
*                                                                            *
*****************************************************************************/
void ltrim(char *str) {
   size_t n;
   n = 0;
   while (str[n] != '\0' && isspace((unsigned char)str[n])) {
      n++;
   }
   memmove(str, str + n, strlen(str) - n + 1);
   return;
}

/**
 * Function name: rltrim
 * Intent: A function to trim both leading and trailing spaces from an 
 * input string.
 * Input:
 * (1) a pointer to a nul terminated character string 
 * Output:
 * None, with the exception of the side-effect is that the input string 
 * is changed.
 */
/*****************************************************************************
*                                                                            *
*  ----------------------------------- rltrim -------------------------------  *
*                                                                            *
*****************************************************************************/
void rltrim(char *str)
{
   rtrim(str);
   ltrim(str);
   return;
}

/**
 * Function name: create_consistency_check_arrays
 * Intent: A function to allocate a structure and elements belonging to it.
 * Input:
 * (1) a 'size_t' type of unsigned integer that indicates the number of 
 * sweeps/scans in the radar volume
 * Output:
 * (1) a pointer to an allocated, but not filled structure.
 * Note! The calling program must free the returned structure built by 
 * this function.
 */
/*****************************************************************************
*                                                                            *
*  ------------------- create_consistency_check_arrays --------------------  *
*                                                                            *
*****************************************************************************/
cci_s *create_consistency_check_arrays(size_t nsweeps) {
   /* 
    * first allocate the structure that will be returned from this function 
    */
   cci_s *out;
   out = RAVE_MALLOC(sizeof *out);
   if( !out ) {
      Iris_printf(
         "Error! Unable to allocate a consistency check info structure.\n");
      return NULL;
   }

   /*
    * allocate a 1D array
    * index_of_first_ray_timewise[number_of_sweeps] 
    */
   UINT2 *dummy_UINT2_p;
   dummy_UINT2_p = (UINT2 *) RAVE_CALLOC(nsweeps, sizeof *dummy_UINT2_p);
   if( !dummy_UINT2_p ) {
      Iris_printf(
         "Error! Unable to allocate index_of_first_ray_timewise array.\n");
      return NULL;
   }
   out->index_of_first_ray_timewise_p = dummy_UINT2_p;
   dummy_UINT2_p = NULL;

   /*
    * allocate a pointer to a 1D array 
    * ray_highest_integral_seconds[number_of_sweeps] 
    */
   dummy_UINT2_p = (UINT2 *) RAVE_CALLOC(nsweeps, sizeof *dummy_UINT2_p);
   if( !dummy_UINT2_p ) {
      Iris_printf(
         "Error! Unable to allocate ray_highest_integral_seconds array.\n");
      return NULL;
   }
   out->ray_highest_integral_seconds_p = dummy_UINT2_p;
   dummy_UINT2_p = NULL;
   

   /* 
    * Allocate a 2D array by allocating an array of pointers
    * and then for each element, allocate a structure.
    * So mtv_array_p points to an array of pointers to mtv structures.
    * The array is allocated, then filled, then the pointer to the array
    * is assigned to pointer 'sweep_start_times_mtv'.
    * Note! The array length is "number_of_sweeps", not MAX_SWEEPS
    */
   mtv_s **mtv_array_p = NULL;
   mtv_array_p = (mtv_s **) RAVE_MALLOC( nsweeps*sizeof(mtv_s *) );
   if( !mtv_array_p ) {
      Iris_printf("Error! Unable to allocate mtv_s array of pointers.\n");
      return NULL;
   }
   /*
    * Loop through the array of pointers and allocate
    * a structure for each element
    */
   for(size_t kv=0; kv < nsweeps; kv++) {
      /* point to an allocated structure */
      mtv_array_p[kv] = (mtv_s *) RAVE_MALLOC(sizeof(mtv_s));
      if( !mtv_array_p[kv] ) {
         Iris_printf("Error! Unable to allocate mtv_s structure.\n");
         return NULL;
      }
      memset(mtv_array_p[kv], 0, sizeof(mtv_s));
   }
   out->sweep_start_times_mtv_p = mtv_array_p;
   return out;
} /* end of create_consistency_check_arrays */

/**
 * Function name: destroy_consistency_check_arrays
 * Intent: A function to de-allocate/free a structure and elements 
 * belonging to it.
 * Input:
 * (1) a pointer to the structure to be deallocated
 * (2) a 'size_t' type of unsigned integer that indicates the number of 
 * sweeps/scans in the radar volume
 * Output:
 * Nothing, except for the side-effect that the input structure is freed. 
 */
/*****************************************************************************
 *                                                                            *
 *  ------------------- destroy_consistency_check_arrays --------------------  *
 *                                                                            *
 *****************************************************************************/
void destroy_consistency_check_arrays(cci_s *cci_p,
                                      size_t nsweeps) 
{  
   mtv_s *this_time = NULL;
   if(cci_p == NULL) return;
   /*
    * free the 1D array 'index_of_first_ray_timewise_p'
    */
   if(cci_p->index_of_first_ray_timewise_p != NULL) {
      RAVE_FREE(cci_p->index_of_first_ray_timewise_p);
      cci_p->index_of_first_ray_timewise_p = NULL;
   }
   /*
    * free the 1D array 'ray_highest_integral_seconds_p'
    */
   if(cci_p->ray_highest_integral_seconds_p != NULL) {
      RAVE_FREE(cci_p->ray_highest_integral_seconds_p);
      cci_p->ray_highest_integral_seconds_p = NULL;
   }
   /*
    * free each element of the 2D array 'sweep_start_times_mtv_p'
    * Note! each element points to a structure
    */
   for(size_t kv=0; kv < nsweeps; kv++) {
      this_time = cci_p->sweep_start_times_mtv_p[kv];
      if(this_time != NULL) {
         RAVE_FREE(this_time);
         cci_p->sweep_start_times_mtv_p[kv] = NULL;
      }
   }
   /*
    * free the 1D array of pointers comprising
    * the 2D array 'sweep_start_times_mtv_p'
    */
   if(cci_p->sweep_start_times_mtv_p != NULL) {
      RAVE_FREE(cci_p->sweep_start_times_mtv_p);
      cci_p->sweep_start_times_mtv_p = NULL;
   }
   /*
    * free the cci structure
    */
   if(cci_p != NULL) {
      RAVE_FREE(cci_p);
      cci_p = NULL;
   }
   return;
}

/**
 * Function name: ymd_to_mtv
 * Intent: A function to write data obtained from a ymd_s date/time structure to a
 * mtv_s date/time structure. 
 * belonging to it.
 * Input:
 * (1) a pointer to a previously populated ymd_s structure
 * Output:
 * (1) a pointer to an allocated and filled mtv_s structure
 * Note! The mtv_s structure is allocated in this function and must be 
 * freed by the calling program.  This function returns a null pointer 
 * if the allocation fails OR if the call to the "mktime" functions fails.
 * Note! A ymd_s date/time structure is an IRIS date-time structure with
 * date and time elements that are seperate.  A mtv_s date/time structure is
 * structure where time and date are referenced to Jan011970;
 * As of June22 2016, mtv_s carries the isdst indicator obtained from the
 * ymd structure.
 */
/*****************************************************************************
*                                                                            *
*  ---------------------------- ymd_to_mtv    ----------------------------   *
*                                                                            *
*****************************************************************************/
mtv_s *ymd_to_mtv( ymd_s *ymd_p ) {
   /* allocate the structure to return */
   mtv_s *mtv_p;
   mtv_p = RAVE_MALLOC(sizeof *mtv_p);
   if( !mtv_p ) {
      Iris_printf("Error! Unable to allocate mtv_s structure.\n");
      return NULL;
   }
   struct tm a_tm_struct;
   struct tm *sometime = &a_tm_struct;
   UINT4 since_midnight, hours, minutes, secs;
   time_t seconds;
   sometime->tm_year = ymd_p->year-1900;
   sometime->tm_mon = ymd_p->month-1;
   sometime->tm_mday = ymd_p->day;
   since_midnight = ymd_p->seconds_since_midnight;
   hours = since_midnight / 3600L;
   sometime->tm_hour = hours;
   minutes = (since_midnight-hours*3600L) / 60L;
   sometime->tm_min = minutes;
   secs = since_midnight-hours*3600L-minutes*60L;
   sometime->tm_sec = secs;
   /* 
    * isdst = -1 means I don't know if it's DST
    * isdst =  0 means that it is NOT DST
    * isdst = +1 means that it is DST
    */
   if( UTC_FROM_MILLS(ymd_p->milliseconds_and_UTC_DST_indication))
      sometime->tm_isdst = 0;
   else if( DST_FROM_MILLS(ymd_p->milliseconds_and_UTC_DST_indication))
      sometime->tm_isdst = +1;
   else if( LDST_FROM_MILLS(ymd_p->milliseconds_and_UTC_DST_indication))
      sometime->tm_isdst = +1;
   else
      /* As of June102016 we never set the daylight savings time indicator
       * to -1 because it appears to set the indicator to whatever it is
       * on the date that this program is run, which is a awful guess.
       * Instead set the indicator to 0 if there is no indication that DST
       * is in effect
       */
//      sometime->tm_isdst = -1;
       sometime->tm_isdst = 0;
#ifndef _WIN32
   seconds = timegm( sometime );
#else
   seconds = _mkgmtime( sometime );
#endif

   if( seconds == -1) {
      Iris_printf("Function mktime failed in function ymd_to_mtv.\n");
      Iris_printf(
          "Will return a NULL instead of a my_time_value structure.\n");
      return NULL;
   }
   UINT2 millisecs = MS_FROM_MILLS(ymd_p->milliseconds_and_UTC_DST_indication);
   mtv_p->tv_sec = seconds;
   mtv_p->tv_usec =  (time_t) (millisecs)*1000L;
   mtv_p->isdst = sometime->tm_isdst;
   return mtv_p;
}

/**
 * Function name: do_consistency_check
 * Intent: A function to check that the number of moments and rays received 
 * are less than or equal to the number expected (etc).
 * Input:
 * (1) a pointer to a previously populated ymd_s structure
 * Output:
 * Nothing,but has the side effect of filling a (cci) structure that holds 
 * variables used elsewhere in the program. 
 */
/*****************************************************************************
*                                                                            *
*  ------------------------ do_consistency_check --------------------------  *
*                                                                            *
*****************************************************************************/
void do_consistency_check(cci_s *cci_p, // <-- structure to fill
                          size_t nsweeps,
                          file_element_s *file_element_p) {
   IrisDList_t *ray_list = NULL;
   IrisDListElement_t *datatype_element = NULL;
   IrisDListElement_t *this_ray_element = NULL;
   datatype_element_s *datatype_current = NULL;
   ray_s *this_ray_structure = NULL;
   ymd_s *sweep_start_ymd_p = NULL;
   UINT2 rays_in_this_sweep = 0;
   mtv_s *dummy_mtv_p = NULL;
   IrisDListElement_t *this_sweep_element = NULL;
   IrisDList_t *types_list = NULL;
   IrisDList_t *sweep_list = file_element_p->sweep_list_p;
   sweep_element_s *sweep_data_p = NULL;
   /* 
    * loop through the sweeps in this volume scan 
    */
   for(size_t mm = 0 ; mm < nsweeps; mm++ ) {
      if( mm == 0) {
         this_sweep_element = IrisDList_head(sweep_list);
      }
      else {
         this_sweep_element = IrisDListElement_next(this_sweep_element);
      }
      sweep_data_p = (sweep_element_s *) this_sweep_element->data;
      types_list = sweep_data_p->types_list_p;
      /* 
       * point to the first data-type structure in the datatype list
       * for this sweep 
       */
      datatype_element = IrisDList_head(types_list);
      datatype_current = (datatype_element_s *) datatype_element->data;
      ray_list = datatype_current->ray_list_p;

      rays_in_this_sweep = (UINT2) IrisDList_size(ray_list);
      /* sfs is seconds-from-start   (of current sweep) */
      UINT2 sfs_this_ray = 0;
      UINT2 sfs_last_ray = 0;
      /*
       * when seconds from start is zero at k=0
       * then ray_highest_integral_seconds[mm]
       * never gets assigned in the loop below
       * so  index_of_first_ray_timewise[mm] = 0
       * is correct
       */
      cci_p->index_of_first_ray_timewise_p[mm] = 0;
      /*
       * ray_highest_integral_seconds[mm] is just
       * the maximum value of sfs_this_ray encountered
       * in this sweep
       */
      cci_p->ray_highest_integral_seconds_p[mm] = 0;    
      /* loop through the rays in this sweep */
      for(UINT2 k = 0; k < rays_in_this_sweep; k++) {
         if( k == 0) {
            this_ray_element = IrisDList_head(ray_list);
         }
         else {
            this_ray_element = IrisDListElement_next(this_ray_element);
         }

         this_ray_structure = (ray_s *) this_ray_element->data;
         if( k > 0 ) {
            sfs_last_ray = sfs_this_ray;
         }
         sfs_this_ray = this_ray_structure->
            ray_head.time_in_seconds_from_start_of_sweep;
         if(sfs_this_ray == 0) {
            if(sfs_last_ray > 0) {
               cci_p->index_of_first_ray_timewise_p[mm]=k;
            }
         }
         else {
            if(sfs_this_ray > cci_p->ray_highest_integral_seconds_p[mm]) {
               cci_p->ray_highest_integral_seconds_p[mm] = sfs_this_ray;
            }
         }  
      } /* end-for-loop rays */
      /*
       * save pointers to mtv structures that hold sweep start times
       * Note! Each mtv structure is allocated inside the 'ymd_to_mtv'
       * function and a pointer to the structure is returned by the
       * function.
       */
      sweep_start_ymd_p =
         &datatype_current->ingest_data_header_p->sweep_start_time;
      dummy_mtv_p = ymd_to_mtv(sweep_start_ymd_p);
      /* copy the structure content */
      if(dummy_mtv_p != NULL) {
         cci_p->sweep_start_times_mtv_p[mm]->tv_sec = dummy_mtv_p->tv_sec;
         cci_p->sweep_start_times_mtv_p[mm]->tv_usec = dummy_mtv_p->tv_usec;
         cci_p->sweep_start_times_mtv_p[mm]->isdst = dummy_mtv_p->isdst;
         RAVE_FREE(dummy_mtv_p);
      }
   } /* end for-loop sweeps */
   return;
} /* end function do_consistency_check */

/**
 * Function name: mtv_to_ymd
 * Intent: A function to write data obtained from a mtv_s date/time structure 
 * to a ymd_s date/time structure.
 * Input:
 * (1) a pointer to a previously populated mtv_s structure
 * Output:
 * (1) a pointer to an allocated and filled ymd_s structure
 * Note! The mtv_s structure is allocated in this function and must be 
 * freed by the calling program.  This function returns a null pointer 
 * if the allocation fails OR if the call to the "mktime" functions fails.
 * Note! A ymd_s structure is allocated in this function and must be freed 
 * by the calling program.  This function returns a null pointer if the 
 * allocation fails OR if the call to the "gmtime_r" functions fails.
 * Note! On June 22 2016 a DST indicator was added to the mtv structure
 * and applied in this fucntion. Prior to that DST was determined by the
 * date and location that the this function was called, which was not correct
 * much of the time.
 */
/*****************************************************************************
*                                                                            *
*  ---------------------------- mtv_to_ymd    ----------------------------   *
*                                                                            *
*****************************************************************************/
ymd_s *mtv_to_ymd( mtv_s *mtv_p ) {
   int last_day[12] = {31,28,30,30,31,28,31,31,30,31,30,31};
   /* allocate a tm structure */
   struct tm *my_tm_p;
   my_tm_p = RAVE_MALLOC(sizeof *my_tm_p);
   if( my_tm_p == NULL ) {
      Iris_printf("Error! Unable to allocate 'tm' structure.\n");
      return NULL;
   }
   /* set a pointer to the input number of seconds */
   time_t *my_timet_p = &(mtv_p->tv_sec);
   my_tm_p = gmtime_r(my_timet_p, my_tm_p);
   if(my_tm_p == NULL ) return NULL;
   ymd_s *ymd_p;
   ymd_p = RAVE_MALLOC(sizeof *ymd_p);
   if( ymd_p == NULL) {
      Iris_printf("Error! Unable to allocate 'ymd' structure.\n");
      return NULL;
   }
   int my_yr =   my_tm_p->tm_year+1900;
   int my_mn =   my_tm_p->tm_mon+1;
   int my_dy =   my_tm_p->tm_mday;
   int my_hour = my_tm_p->tm_hour;
   int my_min =  my_tm_p->tm_min;
   int my_sec =  my_tm_p->tm_sec;
   /* 
    * convert input number of micro-seconds 
    * to input number of milli-seconds 
    */ 
   double my_milli = (double) mtv_p->tv_usec / 1000.;
   /* 
    * if the local date/time is daylight savings time
    * then the localtime_r function adds an hour to the
    * time, so we have to un-do that
    */
   if(my_tm_p->tm_isdst !=  mtv_p->isdst) {
      if( (my_tm_p->tm_isdst==1) &&  (mtv_p->isdst==0) ) {
         my_hour -= 1;
         /* of course that may send the hour beyond the limit */
         if(my_hour < 0) {
            my_hour = 23;
            my_dy -= 1;
            if(my_dy < 1) {
               my_mn -= 1;
               if(my_mn < 1) {
                  my_mn = 12;
                  my_yr -= 1;
               }
               my_dy = last_day[my_mn-1];
               if(my_mn == 2) {
                  if( (my_yr%4 ==0 && my_yr%100==0 ) ||
                      ( my_yr%400==0 ) ) my_dy = 29;
               }
            }
         }
      }
      else if( (my_tm_p->tm_isdst==0) &&  (mtv_p->isdst==1) ) {
         /* of course that may send the hour beyond the limit */
         if(my_hour > 23) {
            my_hour = 0;
            my_dy += 1;
            int last_dy = last_day[my_mn-1];
            if(my_mn == 2) {
               if( (my_yr%4 ==0 && my_yr%100==0 ) ||
                  ( my_yr%400==0 ) ) last_dy = 29;
            }
            if(my_dy > last_dy) {
               my_mn += 1;
               if(my_mn > 12) {
                  my_mn = 1;
                  my_yr += 1;
               }
               if(my_mn == 2) {
                  if( (my_yr%4 ==0 && my_yr%100==0 ) ||
                     ( my_yr%400==0 ) ) my_dy = 29;
               }
            }
         }
      }
   }
   ymd_p->year = (UINT2) my_yr;
   ymd_p->month= (UINT2) my_mn;
   ymd_p->day  =(UINT2) my_dy;
   ymd_p->seconds_since_midnight = (UINT4)
      (my_hour*3600L + my_min*60L + my_sec);
   ymd_p->milliseconds_and_UTC_DST_indication = (UINT2) my_milli ;
   /* Note we did not set the DST related flags here */
   if(my_tm_p != NULL) {
      RAVE_FREE(my_tm_p);
      my_tm_p = NULL;
   }
   return ymd_p;
}

/**
 * Function name: calc_nyquist
 * Intent: Calculate the nyquist velocity of scan/volume
 * Input:
 * (1) a pointer to structure holding IRIS RAW data from a single file
 * Output:
 * (1) a double/floating-point number holding the nyquist (radial) velocity
 */
double calc_nyquist(file_element_s *file_element_p) {
   double nyquist_velocity;
   double PRT1 = 0.;
   double PRT2 = 0.;
   double PRF1_hz, PRF2_hz;
   SINT4 PRF1_hz_int = file_element_p->product_header_p->end.PRF_in_hertz;
   PRF1_hz = (double) PRF1_hz_int;
   if( PRF1_hz_int != 0) PRT1 = 1. / PRF1_hz;
   double wavelength_meters = (double)
      file_element_p->product_header_p->
      end.wavelength_in_hundredths_of_centimeters /1.0E4;
   UINT2 prfcontrol = file_element_p->ingest_header_p->
      tcf.dsp.multi_PRF_mode_flag;
   nyquist_velocity = wavelength_meters * (PRF1_hz / 4.0);
   /*
    * Is there is more than one PRF ?
   * PRF_FIXED   (0)         Fixed trigger rate
    * PRF_2_3     (1)         2:3 PRF Ratio
    * PRF_3_4     (2)         3:4 PRF Ratio
    * PRF_4_5     (3)         4:5 PRF Ratio
    *
    */
   if( prfcontrol != PRF_FIXED) {
      if( prfcontrol == PRF_2_3) {
         nyquist_velocity *= 2.;
      }
      else if( prfcontrol == PRF_3_4) {
         nyquist_velocity *= 3.;
      }
      else if( prfcontrol == PRF_4_5) {
         nyquist_velocity *= 4.;
      }
      else {
         UINT2 PRF2_hz_int = 
            file_element_p->ingest_header_p->tcf.dsp.u.batch.low_PRF_in_hz;
         PRF2_hz = (double) PRF2_hz_int / 65536.;
         if( PRF2_hz_int != 0) PRT2 = 1. / PRF2_hz;
         nyquist_velocity = 0.;
         if(PRF1_hz_int != 0  && PRF2_hz_int != 0) {
            nyquist_velocity = wavelength_meters /
                (4. * fabs(PRT1 - PRT2));
         }
      }
   }
   return nyquist_velocity;
}
/*****************************************************************************
 *                                                                           *
 *  ------------------------ compute_ray_index ---------------------------   *
 *                                                                           *
 *****************************************************************************/
/* Return computed ray index based on start and end azimuth angles
 * Note! The returned ray index should be from 0 to 359 and is a long integer
 * ======================================================================== */
long compute_ray_index( rhd_s *this_ray_header,
                        double angular_resolution_degrees,
                        long max_index ) {
   double a;
   double azi_angle_mid_degrees, ray_index_dbl;
   double azi_angle_beg_deg_m1802p180, azi_angle_end_deg_m1802p180;
   double azi_angle_beg_deg_0to360, azi_angle_end_deg_0to360;
   long returned_index = -1;
   /* calculate the angle end points in degrees */
   azi_angle_beg_deg_m1802p180 =
      ((double)this_ray_header->azimuth_angle_at_beginning_of_ray) /
      (65536.0 / 360.0);
   azi_angle_end_deg_m1802p180 =
      ((double)this_ray_header->azimuth_angle_at_end_of_ray) /
      (65536.0 / 360.0);
   /* same except in 0 to 360 range */
   if(azi_angle_beg_deg_m1802p180 < 0.0 ) {
      azi_angle_beg_deg_0to360 = azi_angle_beg_deg_m1802p180 + 360.0;
   }
   else {
      azi_angle_beg_deg_0to360 = azi_angle_beg_deg_m1802p180;
   }
   if(azi_angle_end_deg_m1802p180 < 0.0 ) {
      azi_angle_end_deg_0to360 = azi_angle_end_deg_m1802p180 + 360.0;
   }
   else {
      azi_angle_end_deg_0to360 = azi_angle_end_deg_m1802p180;
   }
   /*
    * calculate mid angle
    * Note! There are 2 exceptional cases
    * (1) the start and end angles straddle the 180 degree mark (use 0 to 360 range)
    * (2) the start and end angles straddle the 0 degree mark (use -180 to +180 range)
    * 
    */
   if( (azi_angle_beg_deg_0to360<=90.0 && azi_angle_end_deg_0to360>=270.0) ||
       (azi_angle_beg_deg_0to360>=270.0 && azi_angle_end_deg_0to360<=90.0) ) {
      a = (azi_angle_end_deg_m1802p180 + azi_angle_beg_deg_m1802p180) / 2.0;
      /* a is in -180 to +180 range, switch back to 0 to 360 range */
      if(a < 0.0) {
         azi_angle_mid_degrees = a + 360.0;
      }
      else {
         azi_angle_mid_degrees = a;
      }
   }
   else {
      /* a is in 0 to 360 range */
      a = (azi_angle_end_deg_0to360 + azi_angle_beg_deg_0to360) / 2.0;
      azi_angle_mid_degrees = a;
   }
   /* 
    * make sure angle ranges from 0 upto (but not including) 360 range
    */
   if(azi_angle_mid_degrees < 0.0) {
      azi_angle_mid_degrees += 360.0;
   }
   else {
      if( a >= 360.0 ) {
         azi_angle_mid_degrees -= 360.0;
      }
   }
   ray_index_dbl= azi_angle_mid_degrees / angular_resolution_degrees;
   returned_index = (long) round(ray_index_dbl);
   if( returned_index == max_index) returned_index -= max_index;
   return returned_index;
}

/*****************************************************************************
 *                                                                           *
 *  ---------------------------- reldif ----------------------------------   *
 *                                                                           *
 *****************************************************************************/
/* Returns the relative difference of two real numbers: 0.0 if they are 
 * exactly the same, otherwise the ratio of the difference to the larger of 
 * the two. 
 * ======================================================================== */
double reldif(double a, double b)
{
   double c = MY_ABS(a);
   double d = MY_ABS(b);
   double e = MY_MAX(c, d);
   double f = 0.0 ? 0.0 : MY_ABS(a - b) / e;
   return f;
}

/* END HELPER FUNCTIONS */
