/* ---------------------------------------------------------------------------
Copyright (C) 2015 The Crown (i.e. Her Majesty the Queen in Right of Canada)

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
----------------------------------------------------------------------------*/
/**
 * Description: Functions used to read an IRIS raw data file and save the 
 * contents in linked lists. Part of program iris2odim.
 * @file iris2list.c
 * @author Daniel Michelson and Mark Couture, Environment Canada
 * @date 2015-10-21
 */
//#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <math.h>       // M_PI
#include <stddef.h> // size_t, NULL
#include <stdlib.h> // malloc, calloc, exit, free
#include <string.h>
#include <time.h>       // localtime_r
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "iris2odim.h" // this includes rave_alloc and other rave related
#include "iris2list_listobj.h"
#include "iris2list_sigmet.h"
#include "iris2list_interface.h"
#include "rave_alloc.h"

/*****************************************************************************
*                                                                            *
*  ------------------------------ iris2list -------------------------------  *
* INPUT:                                                                     *
* (1) ifile :a pointer to an array of charecters holding the input file name *
* (2) file_element_pp :a pointer to a pointer to an empty file_element_s     *
* structure                                                                  *
* RETURN:                                                                    *
* An signed 4-byte integer with a values set to zero if no errors encountered*
*                                                                            *
* INTENDED RESULT:                                                           *
* The structure passed into this function is filled with contents of the     *
* input file                                                                 *
*                                                                            *
*****************************************************************************/
int iris2list(const char *ifile,
              file_element_s **file_element_pp) {
   IrisDList_t *sweeplist = (*file_element_pp)->sweep_list_p;
   sweep_element_s *sweep_list_element = NULL;
   sweep_element_s *sweep_list_element_new = NULL;
   IrisDList_t *datatypelist = NULL;
   datatype_element_s *datatype_data_p = NULL;
   IrisDList_t *raylist = NULL;
   ray_s *ray_list_element_p = NULL;
   IrisDListElement_t *datatype_current = NULL;
   rpb_s *rpb_p = NULL;
   rayplus_s *rayplus_p = NULL;
   _Bool target_is_big_endian;
   _Bool save_and_exit = 0;
   SINT2 recNum;
   UINT1 myRecNumBytes[2], b1,b2;
   SINT2* rn_p = NULL;
   FILE *fpIn = NULL;
   rn_p = (SINT2*) &myRecNumBytes[0];
   int ray_count[MAX_DATA_TYPES_IN_FILE];
   for(int nnn=0; nnn < (MAX_DATA_TYPES_IN_FILE); nnn++) ray_count[nnn] = 0;
   int type_index = 0;
   /*****************************************************************************
    *                                                                           *
    * open an input file for reading,  abort the program if unable to open file.*
    *                                                                           *
    * try to ascertain if the file is big_endian or little_endian               *
    *                                                                           *
    ****************************************************************************/
   fpIn = fopen( ifile, "rb" );
   if( fpIn == NULL) {        /* if failed to open file, tell the user */
      Iris_printf("Failed to open IRIS file %s.\n",ifile);
      free_IRIS(file_element_pp);
      return -1;
   }
   char c[2];
   c[0] = fgetc(fpIn);
   c[1] = fgetc(fpIn);
   short *structID_p;
   structID_p= (short *) &c;
   if( *structID_p != 27) {
      char dummyc;
      dummyc = c[0];
      c[0] = c[1];
      c[1] = dummyc;
      if( *structID_p != 27) {
         Iris_printf(
         "Failed to establish endian status of input file.\n"
         "Likely this is because the input file is NOT an IRIS file.\n");
         free_IRIS(file_element_pp);
         if (fpIn != NULL) fclose(fpIn);
         return -1;
      }
      else {
         target_is_big_endian = 1;
      }
   }
   else {
      target_is_big_endian = 0;
   }
   rewind(fpIn);
  
   /* make sure that the pointer to a sweep is NULL at this point */
   sweep_list_element = NULL;
   /*****************************************************************************
    * The origin for sweep number is '1', however by initializing to 0 will     *
    * trigger initialization related actions                                    *
    ****************************************************************************/
   SINT2 current_sweep = 0;
   /* origin for recCnt is '1', set to '0' so initial increment works */
   UINT2 recCnt = 0;
   /*****************************************************************************
    * Here we declare and set to NULL a void pointer which will be used to point*
    * to an input buffer, which is an array of 1-byte integers                  *
    ****************************************************************************/
   UINT1 *bufIRIS_p = NULL;
   /* declare a pointer to an IRISbuf structure */
   IRISbuf *IRISbuf_p = NULL;
   SINT2 types_in_sweep;
   UINT2 bytes2Copy;
   /*****************************************************************************
    *  get the product_hdr structure (only) from the IRIS input file            *
    *  process one input record/buffer at a time...                             *
    *                                                                           *
    ****************************************************************************/
   bytes2Copy  = PRODUCT_HDR_SIZE;
   IRISbuf_p = getabuf( fpIn, bytes2Copy );
   if( IRISbuf_p == NULL ) {
      Iris_printf(
             "First call to 'getabuf' returned no product_hdr structure.\n");
      if(*file_element_pp != NULL) {
         free_IRIS(file_element_pp);
         *file_element_pp = NULL;
      }
      if(fpIn != NULL) {
         fclose(fpIn);
         fpIn = NULL;
      }
      return -1;
   }
   if(IRISbuf_p->errorInd == 1) {
      Iris_printf(
             "First call to 'getabuf' returned a read error.\n");
      if(*file_element_pp != NULL) {
         free_IRIS(file_element_pp);
         *file_element_pp = NULL;
      }
      if(fpIn != NULL) {
         fclose(fpIn);
         fpIn = NULL;
      }
      if( IRISbuf_p != NULL) {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }
      return -1;
   }
   else if( IRISbuf_p->errorInd == 2 ) {
      Iris_printf("Hit EOF during first call to 'getabuf'.\n");
      if(*file_element_pp != NULL) {
         free_IRIS(file_element_pp);
         *file_element_pp = NULL;
      }
      if(fpIn != NULL) {
         fclose(fpIn);
         fpIn = NULL;
      }
      if( IRISbuf_p != NULL) {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }
      return -1;
   }
   recCnt++;
   bufIRIS_p = &IRISbuf_p->bufIRIS[0];
   phd_s *phd_p = NULL;
   /*
    * extract product header structure from the IRIS buffer
    */
   phd_p = extract_product_hdr(IRISbuf_p,target_is_big_endian);
   if( phd_p == NULL) {     /* inform client about allocation failure */
      Iris_printf("Error allocating 'product_hdr' structure.\n");
      if(*file_element_pp != NULL) {
         free_IRIS(file_element_pp);
         *file_element_pp = NULL;
      }
      if(fpIn != NULL) {
         fclose(fpIn);
         fpIn = NULL;
      }
      if( IRISbuf_p != NULL) {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }
      return -1;
   }

   /*****************************************************************************
    * if this input file is a 'RAW' type of IRIS file, then skip by the rest of *
    * the bytes in this record by calling getabuf.
    ****************************************************************************/
   if( phd_p->pcf.product_type_code == 15) {
      bytes2Copy  = IRIS_BUFFER_SIZE - PRODUCT_HDR_SIZE;
      if( IRISbuf_p != NULL) {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }
      IRISbuf_p = getabuf( fpIn, bytes2Copy );
      if( IRISbuf_p == NULL ) {
         Iris_printf("First call 'getabuf' "
            "after extracting product_hdr structure failed.\n");
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      if(IRISbuf_p->errorInd == 1) {
         Iris_printf("First call to 'getabuf' "
            "after extracting product_hdr structure returned a read error.\n");
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         return -1;
      }
      else if( IRISbuf_p->errorInd == 2 ) {
         Iris_printf("Hit EOF during first call to 'getabuf'"
            "after extracting product_hdr structure.\n");
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         return -1;
      }
      recCnt++;
      bufIRIS_p = &IRISbuf_p->bufIRIS[0];
   }
   /* 
    * deep copy this product header structure extracted above to the 
    * file element structure tree
    */
   deep_copy_product_header(phd_p, file_element_pp);
   /* 
    * free memory allocated in extract_product_hdr function 
    */
   if( phd_p != NULL) RAVE_FREE(phd_p);
   
   /*****************************************************************************
    *  get the ingest_header structure from the IRIS input file                 *
    *  ONLY if this IRIS file is a 'RAW' type of IRIS file                      *
    *                                                                           *
    ****************************************************************************/
   if((*file_element_pp)->product_header_p->pcf.product_type_code == 15) {
      bytes2Copy  = IRIS_BUFFER_SIZE;
      if( IRISbuf_p != NULL) {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }
      IRISbuf_p = getabuf( fpIn, bytes2Copy );
      if( IRISbuf_p == NULL ) {
         Iris_printf("Second call to 'getabuf' "
                 "returned no ingest_header structure.\n");
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         return -1;
      }
      if(IRISbuf_p->errorInd == 1) {
         Iris_printf("Second call to 'getabuf' returned a read error.\n");
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         return -1;
      }
      else if( IRISbuf_p->errorInd == 2 ) {
         Iris_printf("Hit EOF during second call to 'getabuf'.\n");
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      recCnt++;
      bufIRIS_p = &IRISbuf_p->bufIRIS[0];
      /*
       * extract the ingest header structure from the IRIS buffer 
       */
      ihd_s *ihd_p  = extract_ingest_header(IRISbuf_p, target_is_big_endian);
      if( ihd_p == NULL) {     /* inform client about allocation failure */
         Iris_printf("Error allocating 'ingest_header' structure.\n");
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      /* 
       * deep copy this ingest header structure extracted above to the file 
       * element structure tree
       */
      deep_copy_ingest_header(ihd_p, file_element_pp);
      /* free memory allocated in extract_ingest_header function */
      if(ihd_p != NULL) RAVE_FREE(ihd_p);
   }

   /* throw away the remainder of this buffer */
   if( IRISbuf_p != NULL) {
      RAVE_FREE(IRISbuf_p);
      IRISbuf_p = NULL;
   }
   /*****************************************************************************
    *  while able to get a buffer from the input file                           *
    *  process one input record/buffer at a time...                             *
    *                                                                           *
    ****************************************************************************/
   bytes2Copy  = IRIS_BUFFER_SIZE;
   while( (IRISbuf_p = getabuf( fpIn , bytes2Copy)) != NULL) {
      if( IRISbuf_p == NULL ) {
         Iris_printf("Call to 'getabuf' returned no IRIS buffer structure.");
         if(file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      if(IRISbuf_p->errorInd == 1) {
         Iris_printf("Call to 'getabuf' returned a read error.\n");
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
         if(rpb_p != NULL) {
            RAVE_FREE(rpb_p);
            rpb_p = NULL;
         }
         if(file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      else if( IRISbuf_p->errorInd == 2 ) {
         /* 
          * Hit EOF while extracting a regular sized IRIS buffer
          * Exit the while loop if there's nothing in this ray to work with.
          */
         if(IRISbuf_p->bytesCopied < 2 || IRISbuf_p->numberSkipped > 0) {
            save_and_exit = 1;
            goto se;
         }
      }
      recCnt++;
      bufIRIS_p = &IRISbuf_p->bufIRIS[0];

      /* Note! at this point we copied at least 2 bytes to the buffer */
      if(IRISbuf_p->bytesCopied > 1 &&
         (*file_element_pp)->product_header_p->pcf.product_type_code == 15) {
         /*****************************************************************************
          * For all records after the first 2 records in IRIS RAW files, the first 2  *
          * bytes of the buffer is a 2-byte integer variable that holds the record    *
          * number.  It should never be negative.                                     *    
          ****************************************************************************/
         memcpy(&myRecNumBytes, bufIRIS_p,2);
         if(target_is_big_endian) {
            b1 = myRecNumBytes[0];
            b2 = myRecNumBytes[1];
            myRecNumBytes[0] = b2;
            myRecNumBytes[1] = b1;
         }
         recNum = *rn_p;
         /*****************************************************************************
          *  Note! This record number starts at zero and the record number that we    *
          * actually use starts at one. This record number is just used as a sanity   *
          * check. It should never be negative.                                       *
          ****************************************************************************/
         if( recNum < 0 ) {
            Iris_printf(
                    "No read error but negative record number in input file\n"
                    "indicates 'bad record read'.\nWill attempt to continue");
         }
      }
      /*****************************************************************************
       * all the rest of the records begin with a 'raw_prod_bhdr' structure,       *
       * possibly followed by ingest_data_header structures, followed by ray data. *
       ****************************************************************************/
      rpb_p  = extract_raw_prod_bhdr(IRISbuf_p,target_is_big_endian);
      if( rpb_p == NULL) {
         Iris_printf("Exit from extract_raw_prod_bhdr"
                        " with Null rpb_p structure.\n");
         Iris_printf("It usually means the program was unable to allocate"
                        " an rpb structure.\n");
         if( IRISbuf_p != NULL) {
            bufIRIS_p = NULL;
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(*file_element_pp != NULL) {
            free_IRIS(file_element_pp);
            *file_element_pp = NULL;
         }
         if(fpIn != NULL) {
            fclose(fpIn);
            fpIn = NULL;
         }
         return -1;
      }
      /* start at the end of the raw_prod_bhdr */
      UINT2 byte_offset = RAW_PROD_BHDR_SIZE;
      /*****************************************************************************
       * If the sweep number (in the raw_prod_bhdr structure) has increased, then a 
       * new sweep has started and a set of ingest_data_header structure(s) needs to 
       * be read.
       * The function handle_data_headers will insert the last sweep element into 
       * the sweep list, and then allocate and return a pointer to the new sweep 
       * list element. It will also read ingest data headers (one for each data 
       * type) and save those in the new sweep. Type lists and rays lists that are 
       * part of the new sweep element are also allocated in the function.
       ****************************************************************************/
      if( rpb_p->sweep_number > current_sweep ) {
         sweep_list_element_new =
            handle_ingest_data_headers(&sweeplist,
                                      &sweep_list_element,
                                      IRISbuf_p,
                                      target_is_big_endian);
         if(sweep_list_element_new == NULL) {
            Iris_printf("Exit from handle_ingest_data_headers"
                           " with Null sweep_list_element_new structure.\n");
            Iris_printf("It usually means the program was unable to allocate"
                           " a sweep_list_element structure.\n");
            free_IRIS(file_element_pp);
            if (fpIn != NULL) fclose(fpIn);
            return -1;
         }
         /* set a pointer to the current sweep element */
         sweep_list_element = sweep_list_element_new;
         sweep_list_element_new = NULL;
         /* update the current_sweep */
         current_sweep = rpb_p->sweep_number;
         /* point to the type list for this sweep */
         datatypelist = sweep_list_element->types_list_p;
         /*****************************************************************************
          * whenever we start a new sweep, set the datatype_current pointer to NULL 
          ****************************************************************************/
         datatype_current = NULL;
         /* isolate the number of data types saved in the sweep */
         types_in_sweep = (SINT2) IrisDList_size(datatypelist);
         /* set the byte offset to what it "should" be */
         byte_offset = RAW_PROD_BHDR_SIZE +
            (UINT2) types_in_sweep * INGEST_DATA_HEADER_SIZE;
      }
      /*****************************************************************************
       * the ray_prod_bhdr lists an offset to where the ray data starts, if the 
       * offset = -1 means no rays in this record, so skip the rest and read the 
       * next record
       ****************************************************************************/
      if(rpb_p->offset_of_first_ray_in_record == -1 ) continue;
      /*****************************************************************************
       * copy the byte offset into the input array to a separate variable
       * (ie initialize b_offset)
       ****************************************************************************/
      UINT2 b_offset;
      /* default case: where no ingest_data_headers exist in rec */
      b_offset = (UINT2) rpb_p->offset_of_first_ray_in_record;
      if(byte_offset > b_offset) b_offset = byte_offset;
      /*****************************************************************************
       * In the case where ingest_data_headers don't exist, get the ray-data from 
       * the rest of the record, one ray at a time, using an endless loop
       ****************************************************************************/
      for( ; ; ) {
         /*****************************************************************************
          * if there's not enough bytes left in the buffer for a ray header
          * (let alone a ray), then it means we are finished processing this buffer
          ****************************************************************************/
         if( b_offset >= IRISbuf_p->bytesCopied ) break;
         /* extract a ray from the input buffer */
         rayplus_p = extract_rayplus(&IRISbuf_p,
                                     b_offset,
                                     &sweeplist,
                                     &sweep_list_element,
                                     current_sweep,
                                     fpIn,
                                     target_is_big_endian);
         /* exit the program if nothing returned from extract_rayplus */
         if( rayplus_p == NULL) {
            Iris_printf(
                "Exit from extract_rayplus with Null rayplus structure?\n");
            Iris_printf(
               "It usually means the program was unable to allocate a structure.\n");
            if(*file_element_pp != NULL) {
               free_IRIS(file_element_pp);
               *file_element_pp = NULL;
            }
            if (fpIn != NULL) {
               fclose(fpIn);
               fpIn = NULL;
            }
            return -1;
         }
         /*
          * If we hit end-of-file while reading rays 
          * then exit the loop gracefully
          */
         if( rayplus_p->ray->abandon_buf == 2) {
/*         if( rayplus_p->ray->abandon_buf == 2 && 
             rayplus_p->ray->abandon_ray == 1) {
*/
            if(rpb_p != NULL) {
               RAVE_FREE(rpb_p);    // free old rpb sruct 
               rpb_p = NULL;
            }
            if(rayplus_p->new_rpb_p != NULL) {
               RAVE_FREE(rayplus_p->new_rpb_p);
               rayplus_p->new_rpb_p = NULL;
            }
            if(rayplus_p->new_sweep_element_p != NULL) {
               RAVE_FREE(rayplus_p->new_sweep_element_p);
               rayplus_p->new_sweep_element_p = NULL;
            }
            /*
             * Note!  IRISbuf_p and rayplus_p->new_IRISbuf_p (both) point
             * to the same allocated buffer, free one but not both
             */
            if(rayplus_p->new_IRISbuf_p != NULL) {
               RAVE_FREE(rayplus_p->new_IRISbuf_p);
               rayplus_p->new_IRISbuf_p = NULL;
            }
            if(IRISbuf_p != NULL) {
               RAVE_FREE(IRISbuf_p);
               IRISbuf_p = NULL;
            }
            bufIRIS_p = NULL;
            /*
             * don't free the pointer to the last ray_s struct 
             * (it's in the ray list now)
             * instead just null pointer to that ray
             */
            ray_list_element_p = NULL;

            if(rayplus_p->ray != NULL) {
               RAVE_FREE(rayplus_p->ray);
               rayplus_p->ray = NULL;
            }
            
            RAVE_FREE(rayplus_p);
            rayplus_p = NULL;

            break;
         }
         /*****************************************************************************
          * if the returned ray-product-bheader pointer is non-null
          * then free then set our version of the pointer to it
          ****************************************************************************/
         if(rayplus_p->new_rpb_p != NULL)  {
            if(rpb_p != NULL) RAVE_FREE(rpb_p); // free old one if need be
            rpb_p =rayplus_p->new_rpb_p;        // assign a pointer to the new one
            rayplus_p->new_rpb_p = NULL;        // null the pointer to it in rayplus
         }
         if(rayplus_p->new_IRISbuf_p != NULL) { 
            bufIRIS_p = NULL;
            if(IRISbuf_p == NULL) {                        // this should never happen
               IRISbuf_p = rayplus_p->new_IRISbuf_p;       // set a pointer to the new buffer
            }
            rayplus_p->new_IRISbuf_p = NULL;  // null this pointer so we can free rayplus_p later
            /*
             * set a pointer to the buffer/byte-array inside the IRISbuf structure 
             * NOTE! IRISbuf_p->bufIRIS is an array, not a pointer
             */
            bufIRIS_p = &IRISbuf_p->bufIRIS[0];
            /*
             * increment the record count because a new buffer was
             * read in with the extract_rayplus function call
             */
            recCnt++;
         }

         /* 
          * if the sweep number has increased, then we start a new sweep, which has
          * already been allocated inside the function 'extract_rayplus'. 
          * Make sure the pointer to it is valid 
          */
         if( rpb_p->sweep_number > current_sweep ) {
            /*****************************************************************************
             *  a valid sweep pointer.  Replace our current pointer
             *  with the one passed back from the function.
             ****************************************************************************/
            if( rayplus_p->new_sweep_element_p == NULL) {
               Iris_printf("Exit from extract_rayplus with Null "
                              "sweep_element_p structure?\n");
               Iris_printf("It usually means the program was unable to allocate"
                              " a sweep_element_s structure.\n");
               free_IRIS(file_element_pp);
               if (fpIn != NULL) fclose(fpIn);
               return -1;
            }
            /* set a pointer to the current sweep element */
            sweep_list_element = rayplus_p->new_sweep_element_p;
            rayplus_p->new_sweep_element_p = NULL;
            /* update the current_sweep counter */
            current_sweep = rpb_p->sweep_number;
            /* point to the type list for this sweep */
            datatypelist = sweep_list_element->types_list_p;
            /*****************************************************************************
             * whenever we start a new sweep, set the datatype_current pointer to null 
             ****************************************************************************/
            datatype_current = NULL;
            /* isolate the number of data types saved in this sweep */
            types_in_sweep = (SINT2) IrisDList_size(datatypelist);
            /* re-initialize the ray count array */
            for(int nnn=0; nnn < (MAX_DATA_TYPES_IN_FILE); nnn++) ray_count[nnn] = 0;
            /* set the type index to zero, which is the first element of a type array */
            type_index = 0;
         }
         /*****************************************************************************
          * copy a pointer to a ray_s structure so that we can add this to the ray
          * list. Then set the pointer in rayplus structure to Null so that we can 
          * deallocated the rayplus structure (later) without losing the actual data
          ****************************************************************************/
         ray_list_element_p = rayplus_p->ray;
         rayplus_p->ray = NULL;
         /*****************************************************************************
          * update the byte offset into the input array which is the location to 
          * look for the next ray in the next call to extract_rayplus
          ****************************************************************************/
         b_offset = rayplus_p->updated_offset;

         if(ray_list_element_p->abandon_buf > 0) {
            /*
             * free the current IRIS buffer and the current ray
             * will leave the current ray list intact
             */
            if(IRISbuf_p != NULL) {
               RAVE_FREE(IRISbuf_p);
               IRISbuf_p = NULL;
            }
            /* free the current ray struct */
            RAVE_FREE(ray_list_element_p);
            ray_list_element_p = NULL;
            break;
         }

         if(ray_list_element_p->abandon_ray > 0) {
            /*
             * free the current ray only 
             */
            RAVE_FREE(ray_list_element_p);
            ray_list_element_p = NULL;
            /* skip to end of the for-loop */
            continue;
         }

         /* did we hit the end-of-ray in a normal way ? */
         if(ray_list_element_p->normal_ray_end != 1) {
             /*****************************************************************************
             * no, there was no error but we did not hit the end-of-ray code ???
             * for now, tell user, and continue, see what happens
             ****************************************************************************/
            Iris_printf("Did not encounter end-of-ray code? Will try to continue. \n");
         }
         /*****************************************************************************
          * if the pointer 'datatype_current' is currently not pointing to anything, 
          * then set the pointer to the head element of the data type list
          ****************************************************************************/
         if(datatype_current == NULL) {
            if(IrisDList_head(datatypelist) != NULL) {
               datatype_current = IrisDList_head(datatypelist);
            }
            else {
               Iris_printf("IrisDList_head(datatypelist) == NULL? "
                              "Should never happen. \n");
               free_IRIS(file_element_pp);
               if (fpIn != NULL) fclose(fpIn);
               return -1;
            }
         }
         else {
            /*****************************************************************************
             * else pointer 'datatype_current' is currently pointing to something. If it 
             * is currently pointing to the tail element,then set it to point to the head 
             * element
             ****************************************************************************/
            if(IrisDListElement_next(datatype_current) == NULL) { /* tail */
               datatype_current = IrisDList_head(datatypelist);
               type_index = 0;
            }
            /*****************************************************************************
             * else it is currently pointing to a non-tail element so set pointer to 
             * point to the next element in the list
             ****************************************************************************/
            else {
               datatype_current = IrisDListElement_next(datatype_current);
               type_index++;
            }
         }
         /*****************************************************************************
          * if the pointer 'datatype_current' is valid (it is) then store this ray in 
          * the appropriate ray list
          ****************************************************************************/
         if(ray_list_element_p->ray_body_size_in_bytes > 0 && 
            datatype_current != NULL ) {
                datatype_data_p = (datatype_element_s *) datatype_current->data;
            raylist = datatype_data_p->ray_list_p;
            if(IrisDList_size(raylist) == 0) {
               /*
                * if the ray list is empty then
                * insert ray before head element of list
                */
               (void)IrisDList_addFront(raylist, ray_list_element_p);
            }
            else {
               /* else insert ray after tail element of list */
               (void)IrisDList_addEnd(raylist, ray_list_element_p);
            }
            ray_list_element_p = NULL;
            ray_count[type_index]++;
         }
         else {
            if(ray_list_element_p != NULL) {
               RAVE_FREE(ray_list_element_p);
               ray_list_element_p = NULL;
            }
            ray_count[type_index]++;
         }
         /*
          * NUll all pointers inside the rayplus structure before
          * (they should already be null)
          */
         if(rayplus_p != NULL) {
            rayplus_p->ray = NULL;
            if(rayplus_p->new_IRISbuf_p       != NULL) { 
               RAVE_FREE(rayplus_p->new_IRISbuf_p);
            }
            if(rayplus_p->new_rpb_p           != NULL) {
               RAVE_FREE(rayplus_p->new_rpb_p);
            }
            if(rayplus_p->new_sweep_element_p != NULL)  {
               RAVE_FREE(rayplus_p->new_sweep_element_p);
            }
            RAVE_FREE(rayplus_p);
            rayplus_p = NULL;
         }
      } /* end of for-loop for ray extraction from an input buffer */
      if(rpb_p != NULL) {
         RAVE_FREE(rpb_p);
         rpb_p = NULL;
      }
      recCnt++;
      /*****************************************************************************
       * If the end of the input file was reached while reading an input buffer, 
       * then IRISbuf_p is NULL so exit the while loop.
       * Otherwise free the current IRIS buffer because we are about to get another
       * buffer (at the beginning of while loop) 
       ****************************************************************************/
se:   if(IRISbuf_p == NULL) break;
      else {
         bufIRIS_p = NULL;
         RAVE_FREE(IRISbuf_p);
         IRISbuf_p = NULL;
      }

      /* 
       * this pointer should be NULL at this point but if not then
       * the something abnormal happened above and we can free the
       * pointer 
       */
      if( ray_list_element_p != NULL ) {
         RAVE_FREE(ray_list_element_p);
         ray_list_element_p = NULL;
      }
      if(save_and_exit) break;
   } /* end of "while still able to read something" */

   /* insert the last sweep into the sweep list */
   if(sweep_list_element != NULL) {
      if(IrisDList_size(sweeplist) == 0) {
         (void)IrisDList_addFront(sweeplist, sweep_list_element);
      }
      else {
         (void)IrisDList_addEnd(sweeplist, sweep_list_element);
      }
   }
   if (fpIn != NULL) fclose(fpIn);
   return 0;
}

/*****************************************************************************
*                                                                            *
*  -------------------- handle_ingest_data_headers ------------------------  *
*                                                                            *
*****************************************************************************/
sweep_element_s  *handle_ingest_data_headers(
                              IrisDList_t **sweeplist,
                              sweep_element_s **sweep_list_element_pp,
                              IRISbuf *IRISbuf_p,
                              _Bool target_is_big_endian) {
   UINT1 myPeakBytes[2], b1,b2;
   SINT2 *id_p = NULL;
   id_p = (SINT2 *) &myPeakBytes[0];
   SINT2 sneak_a_peak;
   UINT1 *bufIRIS_p = NULL;
   /* simplify by setting a pointer to the input array of bytes */
   bufIRIS_p = &IRISbuf_p->bufIRIS[0];
   /* set the initial byte offset to be the size of RAW_PROD_BHDR */
   UINT2 byte_offset = RAW_PROD_BHDR_SIZE;
   /*
    * if the current sweep_list_element is non-null
    * then insert the last sweep_list_elelment into the sweep list
    */
   if(*sweep_list_element_pp != NULL) {
      if(IrisDList_size(*sweeplist) == 0) {
         (void)IrisDList_addFront(*sweeplist, *sweep_list_element_pp);
      }
      else {
         (void)IrisDList_addEnd(*sweeplist, *sweep_list_element_pp);
      }
      /* set the pointer to NULL now that the structure is in the list */
      *sweep_list_element_pp = NULL;
   }
   /* allocate a new sweep list element for the sweep list */
   sweep_element_s *new_sweep = RAVE_MALLOC(sizeof *new_sweep);
   if( !new_sweep ) {
      Iris_printf(
         "Error! Unable to allocate 'sweep_element' structure in"
         " function 'handle_ingest_data_headers'.\n");
      return NULL;
   }
   /*****************************************************************************
    *  Each sweep is composed of a list of 'data-type' elements                  *
    *  Create space and initialize the doubly-linked list of structures which    *
    *  will hold data according to acquisition type (make the data-type list.    *
    *                                                                            *
    *****************************************************************************/
   /* allocate space for a doubly linked list structure, and point to it */
   IrisDList_t *datatypelist = IrisDList_create();
   if( !datatypelist ) {
      Iris_printf(
         "Error! Unable to allocate 'datatypelist' IrisDList structure in"
         " function 'handle_ingest_data_headers'.\n");
      return NULL;
   }

   /* attach this data type list to the current sweep element */
   new_sweep->types_list_p = datatypelist;
   /* 
    * set a pointer to a (yet-to-be-designated) date-type
    * list-element to NULL
    */
   datatype_element_s *type_list_element;
   /*****************************************************************************
    * if ingest_data_header structures follow, then each one starts with a two  *
    * byte integer variable 'Structure identifier' = 24 (Ingest_data_header).   *
    * check that this is the case... if so read in the ingest_data_header.      +
    *****************************************************************************/
   memcpy( &myPeakBytes, (bufIRIS_p+byte_offset), 2);
   sneak_a_peak = *id_p;
   if(target_is_big_endian) {
      b1 = myPeakBytes[0];
      b2 = myPeakBytes[1];
      myPeakBytes[0] = b2;
      myPeakBytes[1] = b1;
   }
   sneak_a_peak = *id_p;
   while( sneak_a_peak == 24) { /* while there is an ingest_data_header to read*/
      /* allocate space for a data type element structure, and point to it */
      type_list_element = RAVE_MALLOC(sizeof *type_list_element);
      if( !type_list_element ) {
         Iris_printf(
            "Error! Unable to allocate 'type_list_element' IrisDList structure in"
            " function 'handle_ingest_data_headers'.\n");
         return NULL;
      }
      /*
       *  ready to fill in the data-type list element
       * copy an ingest_data_header from the main buffer to a structure
       */
      idh_s *idh_p;
      idh_p = extract_ingest_data_header(IRISbuf_p,
                                         byte_offset,
                                         target_is_big_endian);
      /* exit program if function above returned a NULL pointer
       * (likely because it was unable to allocate the required structure)
       */
      if( idh_p == NULL) return NULL;
      /* attach this ingest_data_header structure to the data-type list-element*/
      type_list_element->ingest_data_header_p = idh_p;
      /*****************************************************************************
       *  Each data-type element/structure has a ray-list as a component             *
       *  Create space and initialize the doubly-linked list of structures which    *
       *  will hold ray data.                                                       *
       *                                                                            *
       *****************************************************************************/
      IrisDList_t *raylist = IrisDList_create();
      if( !raylist ) {
         Iris_printf(
            "Error! Unable to allocate 'raylist' DList structure in"
            " function 'handle_ingest_data_headers'.\n");
         return NULL;
      }

      /*
       * the empty ray list is ready so...
       * attach this ray list to the current data type list-element
       */
      type_list_element->ray_list_p = raylist;
      /*
       *  insert this data-type list-element to the data-type list
       * even though the ray list is empty at this point
       */
      if(IrisDList_size(datatypelist) == 0) {
        (void)IrisDList_addFront(datatypelist, type_list_element);
      }
      else {
        (void)IrisDList_addEnd(datatypelist, type_list_element);
      }
      /* 
       * update the input record byte_offset and see if 
       * the next Structure identifier' = 24 (Ingest_data_header)
       */
      byte_offset += INGEST_DATA_HEADER_SIZE;
      memcpy( &myPeakBytes, (bufIRIS_p+byte_offset), 2);
      if(target_is_big_endian) {
         b1 = myPeakBytes[0];
         b2 = myPeakBytes[1];
         myPeakBytes[0] = b2;
         myPeakBytes[1] = b1;
      }
      sneak_a_peak = *id_p;
   } /* end of while there is an ingest header to read */
   return new_sweep;
}

/*****************************************************************************
*                                                                            *
*  -------------------------- extract_rayplus -----------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an 'rayplus'
 * structure. The function is passed a pointer to a
 * 'IRISbuf' structure, and an offset into the input
 * byte-array to where the encoded ray data starts.
 * The function takes some other input parameters that
 * only come into play if we hit the end of the input
 * buffer before getting the complete ray.  In that
 * case we must re-fill the input buffer and continue.
 * If an allocation failure occurs, a pointer to null
 * is returned and an error message is printed        */
/* ======================================================================== */
rayplus_s *extract_rayplus(IRISbuf **IRISbuf_pp,
                             UINT2 offset,
                             IrisDList_t **sweeplist_pp,
                             sweep_element_s **sweep_list_element_pp,
                             SINT2 current_sweep,
                             FILE *fp,
                             _Bool target_is_big_endian ) {
   UINT1 *ptr_s1;
   UINT1 my12bytes[12];
   UINT1 *my12bytes_p = NULL;
   UINT1 b1,b2,dummyU1;
   rpb_s *rpb_p = NULL;
   my12bytes_p = &my12bytes[0];
   UINT1 *bufIRIS_p = NULL;
   SINT2 codeWord = 0;
   SINT2 lastCodeWord = 0;
   UINT1 my12ByteCount = 0;
   SINT2 lowBits = 0;
   SINT2 lastLowBits = 0;
   IRISbuf *IRISbuf_p = NULL;
   UINT1 dummy_uint1 = 0;
   UINT1 myCodeBytes[2];
   SINT2 *cw_p = NULL;
   IrisDList_t *sweeplist = NULL;
   sweep_element_s *sweep_list_element = NULL;
   /*
    *  Initially we set IRISbuf_p to point to the 
    *  structure passed into this function
    */
   IRISbuf_p = *IRISbuf_pp;
   sweeplist = *sweeplist_pp;
   sweep_list_element = *sweep_list_element_pp;
   cw_p = (SINT2 *) &myCodeBytes[0];
   for(int z = 0 ; z < 12 ; z++) {
      dummy_uint1 = (UINT1) z+1;
      my12bytes[z] = dummy_uint1;
   }
   /*
    * allocate a ray_s structure that will be pointed
    * to by 'ray' in the rayplus structure returned by
    * this function
    */
   ray_s *this_ray = NULL;
   this_ray = (ray_s*) RAVE_MALLOC(sizeof(ray_s));
   if( this_ray == NULL ) {
      Iris_printf(
              "Error! Unable to allocate 'ray_s' structure in"
              " function 'extract_rayplus'.\n");
      return NULL;
   }
   this_ray->abandon_buf = 0;
   this_ray->abandon_ray = 0;
   this_ray->normal_ray_end = 0;
   this_ray->ray_body_size_in_bytes = 0;
   /*
    * allocate a 'rayplus' structure and
    * call the pointer to it 'out'
    */
   rayplus_s *out = NULL;
   out = RAVE_MALLOC( sizeof *out );
   if( out == NULL ) {
      Iris_printf(
          "Error! Unable to allocate 'rayplus' structure in"
          " function 'extract_rayplus'.\n");
      return NULL;
   }
   out->ray =  this_ray;
   this_ray = NULL; /* now OK to destroy this pointer on exit */
   out->new_rpb_p = NULL;
   out->new_IRISbuf_p = NULL;
   out->new_sweep_element_p = NULL;
   /* simplify by setting a pointer to the input array of bytes */
   bufIRIS_p = &IRISbuf_p->bufIRIS[0];
   /*
    * prt_s1 points to a location within the input data buffer
    * where ray data (starting with a ray header) is located
    */
   ptr_s1 = bufIRIS_p + offset;
   /* keep a new variable of the offset to the ray data we wish to read */
   UINT2 current_offset = offset;
   /* 'ray_bytes_filled' will hold the number of bytes of ray data extracted */
   UINT2 ray_bytes_filled = 0;
   /* set a pointer to the start of the ray body storage */
   UINT1 *out_ray_p = &out->ray->ray_body[0];
   /* set a pointer to the ray structure */
   for( ray_bytes_filled = 0; ray_bytes_filled < MAX_RAY_BODY_SIZE; ) {
      /*
       * If  we reached the end of the input buffer
       * then get another input buffer and start reading from that.
       */
      if(current_offset >= IRISbuf_p->bytesCopied) {
         RAVE_FREE(*IRISbuf_pp);
         *IRISbuf_pp = NULL;
         IRISbuf_p = NULL;
         bufIRIS_p = NULL;
         UINT2 bytes2Copy  = IRIS_BUFFER_SIZE;
         out->new_IRISbuf_p = getabuf( fp, bytes2Copy);
         if( out->new_IRISbuf_p == NULL) {
            Iris_printf("Unable to allocate a buffer in 'getabuf.\n");
            Iris_printf("Exiting program.\n");
            if(this_ray != NULL) {  // ray_s *this_ray
               RAVE_FREE(this_ray);
               this_ray = NULL;
            }
            if(out != NULL) {  // rayplus_s *out
               RAVE_FREE(out);
               out = NULL;
            }
            return NULL;
         }
         /*
          * Set IRISbuf_p to point to the new IRISbuf structure
          * that will be returned with the structure returned by
          * this function
          */
         IRISbuf_p = out->new_IRISbuf_p;
         bufIRIS_p = &IRISbuf_p->bufIRIS[0];
         /*
          * if an error occurred while reading the input buffer ...
          * at the very least return with some sort of error indication
          */
         if(IRISbuf_p->errorInd == 1) {
            Iris_printf(
               "Unknown error occurred while reading input file'\n");
            Iris_printf(
               "Will abandon ray and abandon buffer and try to continue.\n");
            out->ray->abandon_buf = 1;
            out->ray->abandon_ray = 1;
            return out;
         }
         else if(IRISbuf_p->errorInd == 2) {
            /*
             * If the end-of-file was reached while reading data
             * into the input buffer
             */
            if(IRISbuf_p->bytesCopied < RAW_PROD_BHDR_SIZE ||
               IRISbuf_p->numberSkipped > 0) {
               /* don't try to do anything with the last record if
                * (i) there is too little or
                * (ii) we skipped records until we hit the end
                */
               out->ray->abandon_buf = 2;
               out->ray->abandon_ray = 1;
               return out;
            }
         } /* end if  IRISbuf_p->errorInd == 2 */
         /* continue because we have enough to read a raw_prof_bhdr */
         if( rpb_p != NULL) RAVE_FREE(rpb_p);
         out->new_rpb_p  = extract_raw_prod_bhdr(IRISbuf_p,target_is_big_endian);
         if( out->new_rpb_p == NULL) {
            Iris_printf(
               "Unable to allocate a structure in 'extract_raw_prod_bhdr.\n");
            Iris_printf("Exiting program.\n");
            if(out->new_IRISbuf_p != NULL) {
               RAVE_FREE(out->new_IRISbuf_p);
               out->new_IRISbuf_p = NULL;
            }
            if(out != NULL) {  // rayplus_s *out
               RAVE_FREE(out);
               out = NULL;
            }
            if(this_ray != NULL) {  // ray_s *this_ray
               RAVE_FREE(this_ray);
               this_ray = NULL;
            }
            return NULL;
         }
         else {
            /* else pass back a pointer to the new/current raw_prod_bhdr structure */
            rpb_p=out->new_rpb_p;
         }
         current_offset = RAW_PROD_BHDR_SIZE;
         ptr_s1 = bufIRIS_p + current_offset;
         /* 
          * if the sweep number (in the raw_prod_bhdr structure) has increased then
          * a new sweep has started and a set of ingest_data_header structure(s)
          * needs to be read
          */
         if( rpb_p->sweep_number > current_sweep ) {
            out->new_sweep_element_p =
              handle_ingest_data_headers(&sweeplist,
                                         &sweep_list_element,
                                         IRISbuf_p,
                                         target_is_big_endian);
              if(out->new_sweep_element_p == NULL) {
               Iris_printf(
                  "Unable to allocate a structure in function"
                  "'handle_ingest_data_headers'.\n");
               Iris_printf("Exiting program.\n");
               if(IRISbuf_p!= NULL) {  // IRISbuf *IRISbuf_p
                  RAVE_FREE(IRISbuf_p);
                  IRISbuf_p = NULL;
               }
               if(out != NULL) {  // rayplus_s *out
                  RAVE_FREE(out);
                  out = NULL;
               }
               if(this_ray != NULL) {  // ray_s *this_ray
                  RAVE_FREE(this_ray);
                  this_ray = NULL;
               }
               return NULL;
            }
            else {
               /* re-assign the old pointer to a new sweep element */
               sweep_list_element = out->new_sweep_element_p;
            }
            IrisDList_t *datatypelist = sweep_list_element->types_list_p;
            SINT2 types_in_sweep = (SINT2) IrisDList_size(datatypelist);
            current_offset = RAW_PROD_BHDR_SIZE +
               (UINT2) types_in_sweep * INGEST_DATA_HEADER_SIZE;
            ptr_s1 = bufIRIS_p + current_offset;
            my12ByteCount = 0;
         } /* end-if rpb_p->sweep_number > current_sweep */
      } /* end if we reached the end of the input buffer */
      lastCodeWord = codeWord;
      lastLowBits = lowBits;
      /*
       * get the codeWord by copying two bytes
       * Note! Both src and destination objects passed to memcpy 
       * are considered to be 'unsigned char'
       */
      memcpy(&myCodeBytes,ptr_s1,2);
      if(target_is_big_endian) {
         b1 = myCodeBytes[0];
         b2 = myCodeBytes[1];
         myCodeBytes[0] = b2;
         myCodeBytes[1] = b1;
      }
      codeWord = *cw_p;
      /*
       * update the input pointer to point to whatever follows this codeWord and
       * update the offset/bytes that the pointer is into the input buffer
       */
      ptr_s1 += 2;
      current_offset += 2;
      /*
       * after adding 32768 to the codeWord the variable 'lowBits' will hold
       * a positive value that is the number of 2-byte words to write
       * Note! When the codeWord is positive lowBits = codeWord
       */
      if(codeWord < 0) {
         lowBits=32768+codeWord;
      }
      else {
         lowBits=codeWord;
      }
      /*
       * If lowBits is more than the maximum ray body size, tell user
       * Note ! This should NEVER occur
       */
      if( lowBits  > MAX_RAY_BODY_SIZE ) {
         Iris_printf("Error! lowBits greater than MAX_RAY_BODY_SIZE.\n");
         Iris_printf("lowBits = %i; lastLowBits = %i;"
                           "codeWord = %i; lastCodeWord = %i; \n",
                           lowBits,lastLowBits,codeWord, lastCodeWord);
         Iris_printf("ray_bytes_filled = %i; current_offset = %u.\n",
                  ray_bytes_filled, current_offset);
         Iris_printf("Exiting program.\n");
         if(IRISbuf_p!= NULL) {  // IRISbuf *IRISbuf_p
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(this_ray != NULL) {  // ray_s *this_ray
            RAVE_FREE(this_ray);
            this_ray = NULL;
         }
         if(out != NULL) {  // rayplus_s *out
            RAVE_FREE(out);
            out = NULL;
         }
         return NULL;
      } /*  end if lowBits  > MAX_RAY_BODY_SIZE */
      /*
       * If lowBits is less than zero then there is a problem
       * Note ! This should NEVER occur
       */
      else if( lowBits < 0 ) {
         Iris_printf("Error! Negative lowBits in ray decode-word.\n");
         Iris_printf("lowBits = %i; lastLowBits = %i;"
                           "codeWord = %i; lastCodeWord = %i; \n",
                           lowBits,lastLowBits,codeWord, lastCodeWord);
         Iris_printf("ray_bytes_filled = %i; current_offset = %u.\n",
                  ray_bytes_filled, current_offset);
         Iris_printf("Exiting program.\n");
         if(IRISbuf_p!= NULL) {  // IRISbuf *IRISbuf_p
            RAVE_FREE(IRISbuf_p);
            IRISbuf_p = NULL;
         }
         if(this_ray != NULL) {  // ray_s *this_ray
            RAVE_FREE(this_ray);
            this_ray = NULL;
         }
         if(out != NULL) {  // rayplus_s *out
            RAVE_FREE(out);
            out = NULL;
         }
         return NULL;
      }  /* end else if( lowBits < 0  */
      /*
       * if codeWord = 0, then skip this codeWord
       * Note! This can happen at the end of a sweep where the rest of
       * the buffer is padded out with zeros
       */
      if( codeWord == 0 ) {
         continue;
      }
      /*
       * if the codeWord = 1, then we have encountered the 'official'
       *  end of ray. Done filling ray body. Return to calling program
       *  with this ray.
       */
      else if( codeWord == 1 ) {
         out->ray->ray_body_size_in_bytes = ray_bytes_filled;
         out->ray->normal_ray_end = 1;
         break;
      }
      /*
       * if the value of the codeWord > 0,
       * we write consecutive zeros to output
       */
      else if( codeWord > 0) {
         /*
          * if value of lowBits of code-word = 3 to 32767;
          * then the lowBits holds the number of zeros to write !
          */
         if(lowBits > 2 ) {
            /*  use loop to write zero values bytes at a time */
            for(int i = 0; i < lowBits; i++) {
               /* set 2 bytes to int 0 */
               memset(out_ray_p, 0, 2);
               out_ray_p += 2;
               ray_bytes_filled += 2;
               /*
                * if writing the next byte of data would overflow the 
                * output ray (this should never happen) then tell the 
                * user there must be a problem, and abandon this ray 
                * and return with what we have.
                */
               if( (ray_bytes_filled + 1) >= MAX_RAY_BODY_SIZE ) {
                  Iris_printf("Potential output ray overflow, abandoning ray.\n");
                  Iris_printf("lowBits = %i; lastLowBits = %i;"
                           "codeWord = %i; lastCodeWord = %i; \n",
                           lowBits,lastLowBits,codeWord, lastCodeWord);
                  Iris_printf("ray_bytes_filled = %i; current_offset = %u.\n",
                  ray_bytes_filled, current_offset);
                  out->ray->ray_body_size_in_bytes = ray_bytes_filled;
                  out->updated_offset = current_offset;
                  out->ray->abandon_ray = 1;
                  return out;
               }
            } /* end of for loop */
            /* note lowBits == 2 falls through these if statements */
         } /* end if lowBits > 2 */
      }  /* end of if(codeWord > 0) */
      else if( codeWord < 0 ) {
         /*
          * if the value of the lowBits = 1 to 32767 (like it should be)
          * then the value of the lowBits holds the number of data words to write
          * transfer the first 12 bytes (the ray header)  into an special array
          */
         for(int i = 0; i < lowBits; i++) {
            /*
             * before doing anything, make sure we have a valid buffer location
             * to draw upon.  If not then read in a new input buffer and proceed
             */
            if( current_offset >= IRISbuf_p->bytesCopied ) {
               RAVE_FREE(*IRISbuf_pp);
               *IRISbuf_pp = NULL;
               IRISbuf_p = NULL;
               bufIRIS_p = NULL;
               UINT2 myBytes2Copy = IRIS_BUFFER_SIZE;
               out->new_IRISbuf_p = getabuf( fp, myBytes2Copy );
               if( out->new_IRISbuf_p == NULL) {
                  Iris_printf("Unable to allocate a buffer in 'getabuf.\n");
                  Iris_printf("Exiting program.\n");
                  if(this_ray != NULL) {  // ray_s *this_ray
                     RAVE_FREE(this_ray);
                     this_ray = NULL;
                  }
                  if(out != NULL) {  // rayplus_s *out
                     RAVE_FREE(out);
                     out = NULL;
                  }
                  return NULL;
               }
               /*
                * Set local IRISbuf pointer to point to 
                * member of outgoing structure
                */
               IRISbuf_p = out->new_IRISbuf_p;
               bufIRIS_p = &IRISbuf_p->bufIRIS[0];
               /*
                *  if an error occurred while reading the input file in getabuf
                *  at the very least return with some sort of error indication
                */
               if(IRISbuf_p->errorInd == 1) {
                  Iris_printf(
                     "Unknown error occurred while reading file in 'getabuf.\n");
                  Iris_printf(
                     "Will abandon both ray and buffer and try to continue.\n");
                  out->ray->abandon_buf = 1;
                  out->ray->abandon_ray = 1;
                  return out;
               }
               /*
                * end of file reached while reading input buffer
                */
               else if(IRISbuf_p->errorInd == 2) {
                  if(IRISbuf_p->bytesCopied < RAW_PROD_BHDR_SIZE || 
                     IRISbuf_p->numberSkipped > 0) {
                     /* 
                      * don't try to do anything with the last record if
                      * (i)there is too little or
                      * (ii) we skipped records until we hit the end
                      */
                     out->ray->abandon_buf = 2;
                     return out;
                  }
                  /* continue if we have enough to read a raw_prof_bhdr */
               }
               if( rpb_p != NULL) RAVE_FREE(rpb_p);
               out->new_rpb_p  = extract_raw_prod_bhdr(IRISbuf_p,target_is_big_endian);
               if( out->new_rpb_p == NULL) {
                  Iris_printf(
                           "Unable to allocate a structure in 'extract_raw_prod_bhdr.\n");
                  Iris_printf("Exiting program.\n");
                  if(out->new_IRISbuf_p != NULL) {
                     RAVE_FREE(out->new_IRISbuf_p);
                     out->new_IRISbuf_p = NULL;
                  }
                  if(out != NULL) {  // rayplus_s *out
                     RAVE_FREE(out);
                     out = NULL;
                  }
                  if(this_ray != NULL) {  // ray_s *this_ray
                     RAVE_FREE(this_ray);
                     this_ray = NULL;
                  }
                  return NULL;
               }
               else {
                  /* else pass back a pointer to the new/current raw_prod_bhdr structure */
                  rpb_p=out->new_rpb_p;
               }
               current_offset = RAW_PROD_BHDR_SIZE;
               ptr_s1 = bufIRIS_p + current_offset;
               /*
                * if the sweep number (in the raw_prod_bhdr structure) 
                * has increased then a new sweep has started and a set 
                * of ingest_data_header structure(s) needs to be read
                */
               if( out->new_rpb_p->sweep_number > current_sweep ) {
                  out->new_sweep_element_p =
                  handle_ingest_data_headers(&sweeplist,
                                             &sweep_list_element,
                                             IRISbuf_p,
                                             target_is_big_endian);
                  if(out->new_sweep_element_p == NULL) {
                     Iris_printf(
                              "Unable to allocate a structure in function"
                              "'handle_ingest_data_headers'.\n");
                     Iris_printf("Exiting program.\n");
                     if(IRISbuf_p!= NULL) {  // IRISbuf *IRISbuf_p
                        RAVE_FREE(IRISbuf_p);
                        IRISbuf_p = NULL;
                     }
                     if(out != NULL) {  // rayplus_s *out
                        RAVE_FREE(out);
                        out = NULL;
                     }
                     if(this_ray != NULL) {  // ray_s *this_ray
                        RAVE_FREE(this_ray);
                        this_ray = NULL;
                     }
                     return NULL;
                  }
                  else {
                     /* re-assign the old pointer to a new sweep element */
                     sweep_list_element = out->new_sweep_element_p;
                  }
                  IrisDList_t *datatypelist = sweep_list_element->types_list_p;
                  SINT2 types_in_sweep = (SINT2) IrisDList_size(datatypelist);
                  current_offset = RAW_PROD_BHDR_SIZE +
                  (UINT2) types_in_sweep * INGEST_DATA_HEADER_SIZE;
                  ptr_s1 = bufIRIS_p + current_offset;
                  my12ByteCount = 0;
               } /* end if out->new_rpb_p->sweep_number > current_sweep  */
            } /* end if( current_offset >= IRISbuf_p->bytesCopied ) */
            if( my12ByteCount < 12) {         /* have we filled the ray header ? */
               /* No, so copy 2 bytes a special 12-byte array */
               memcpy(my12bytes_p, ptr_s1, 2); /* copy 2 bytes */
               /* swap the bytes manually if required */
               if(target_is_big_endian) {
                  dummyU1 = my12bytes[my12ByteCount];
                  my12bytes[my12ByteCount] = my12bytes[my12ByteCount+1];
                  my12bytes[my12ByteCount+1] = dummyU1;
               }
               my12bytes_p += 2;      /* update the output pointer */
               my12ByteCount += 2;    /* update the output counter */
               ptr_s1 += 2;           /* update the input pointer */
               current_offset += 2;   /* keep track of the offset in input array */
               /*
                * if we just finished collecting the whole ray header array
                * then extract variables from it
                */
               if( my12ByteCount == 12) {
                  /* set the pointer to the start of array */
                  my12bytes_p = &my12bytes[0];
                  rhd_s *rhd_p  = extract_ray_header(my12bytes_p);
                  if( rhd_p != NULL) {
                     out->ray->ray_head = *rhd_p; /* copy structure (it's small) */
                     RAVE_FREE(rhd_p); /* free allocated memory of structure */
                  }
                  else {
                     /* Tell user about that the allocation failed */
                     Iris_printf("Error allocating 'ray_header' structure.\n");
                     if(IRISbuf_p!= NULL) {  // IRISbuf *IRISbuf_p
                        RAVE_FREE(IRISbuf_p);
                        IRISbuf_p = NULL;
                     }
                     if(this_ray != NULL) {  // ray_s *this_ray
                        RAVE_FREE(this_ray);
                        this_ray = NULL;
                     }
                     if(out != NULL) {  // rayplus_s *out
                        RAVE_FREE(out);
                        out = NULL;
                     }
                     return NULL;
                  }
               } /* end if my12ByteCount == 12 */
            } /* end if my12ByteCount < 12 */
            else {
               /*
                * else done filling ray header, so copy to the ray body
                */
               memcpy(out_ray_p, ptr_s1, 2);  /* copy 2-bytes to the ray data */
               /* swap these 2 bytes (manually) if required */
               if(target_is_big_endian) {
                  b1 = *out_ray_p;
                  b2 = *(out_ray_p+1);
                  *out_ray_p = b2;
                  *(out_ray_p+1) = b1;
               }
               out_ray_p += 2;                /* update pointer to input data */
               ray_bytes_filled += 2;         /* update counter */
               ptr_s1 += 2;                   /* update the input pointer */
               current_offset += 2;           /* keep track of the offset in input array */
            }
            if( (ray_bytes_filled + 2) > MAX_RAY_BODY_SIZE ) {
               /* writing 2 more bytes of data would overflow the ray body */
               /* so there must be a problem, tell user */
               Iris_printf("Potential output ray overflow.\n");
               Iris_printf("lowBits = %i; lastLowBits = %i;"
                  "codeWord = %i; lastCodeWord = %i; \n",
                  lowBits,lastLowBits,codeWord, lastCodeWord);
               Iris_printf("ray_bytes_filled = %i, current_offset = %u;\n",
                        ray_bytes_filled, current_offset);
               Iris_printf("Abandoning ray, will attempt to continue.\n");
               out->ray->ray_body_size_in_bytes = ray_bytes_filled;
               out->updated_offset = current_offset;
               out->ray->abandon_ray = 1;
               return out;
            }
         } /* end-for  i <lowBits loop */
      } /* end else (if codeWord < 0) */
   } /* end of for ray_bytes_filled < MAX_RAY_BODY_SIZE loop */
   /* return the location to start looking for the next ray */
   out->updated_offset = current_offset;
   return out;
}


/*****************************************************************************
*                                                                            *
*  --------------------------- extract_product_hdr ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * product_hdr type structure. The function is passed
 * a pointer to an 'IRISbuf' structure. If an allocation
 * failure occurs, a pointer to null is returned and
 * error messages are printed in this function  */
/* ======================================================================== */
phd_s *extract_product_hdr(IRISbuf *IRISbuf_p,
                           _Bool target_is_big_endian) {
  UINT1 *bufIRIS_p;
  bufIRIS_p = &IRISbuf_p->bufIRIS[0];
  /* declare some pointers to be used below */
  UINT1 *ptr_s0, *ptr_s1, *ptr_s2;

  /*
   * prt_s0 points to beginning of the input data buffer,
   * which is the start of 'structure_header' data
   */
  ptr_s0 = bufIRIS_p;

  /* prt_s1 points to the start of 'product_configuration' data */
  ptr_s1 = ptr_s0 + STRUCT_HEADER_SIZE;

  /* prt_s2 points to the start of 'product_end' data */
  ptr_s2 = ptr_s1 + PRODUCT_CONFIGURATION_SIZE;

  /* allocate a 'product_hdr' structure and call it 'out' */
  phd_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'phd_s' structure in"
          " function 'extract_product_hdr'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  shd_s *shd_p;
  shd_p = extract_structure_header(ptr_s0,
                                   target_is_big_endian);
  if( shd_p ) {
      out->hdr = *shd_p; // copy structure
      RAVE_FREE(shd_p); // free allocated memory of structure
  }
  pcf_s *pcf_p;
  pcf_p = extract_product_configuration(ptr_s1,
                                        target_is_big_endian);
  if( pcf_p ) {
      out->pcf = *pcf_p; // copy structure
      RAVE_FREE(pcf_p); // free allocated memory of structure
  }
  ped_s *ped_p;
  ped_p = extract_product_end(ptr_s2,
                              target_is_big_endian);
  if( ped_p ) {
      out->end = *ped_p; // copy structure
      RAVE_FREE(ped_p); // free allocated memory of structure
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_structure_header ----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a structure_header structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
shd_s *extract_structure_header(UINT1 *s1, _Bool target_is_big_endian) {
  shd_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'shd_s' structure in"
          " function 'extract_structure_header'.\n");
      return NULL;
  }
  /* copy values into the output structure */
  memcpy( &out->structure_identifier, s1, 2);
  memcpy( &out->format_version_number, s1+2, 2);
  memcpy( &out->bytes_in_entire_struct, s1+4, 4);
  memcpy( &out->flags, s1+10, 2);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->structure_identifier =
          Swap2BytesSigned( (UINT2)
              out->structure_identifier);
      out->format_version_number =
          Swap2BytesSigned( (UINT2)
              out->format_version_number);
      out->bytes_in_entire_struct =
          Swap4BytesSigned( (UINT4)
              out->bytes_in_entire_struct);
      out->flags =
          Swap2BytesSigned( (UINT2) out->flags);
  }
  return out;
}


/*****************************************************************************
*                                                                            *
*  --------------------- extract_product_configuration --------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from an IRIS raw product
 * record and produce/return a pointer to a
 * product_configuration type structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If any failure occurs a pointer to null is returned
 * and error messages are written to stderr */
/* ======================================================================== */
pcf_s *extract_product_configuration(UINT1 *s1,
                                      _Bool target_is_big_endian) {
  pcf_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'pcf_s' structure in"
          " function 'extract_product_configuration'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  shd_s *shd_p = extract_structure_header(s1, target_is_big_endian);
  if( shd_p != NULL) {
      out->hdr = *shd_p; // copy structure
      RAVE_FREE(shd_p);  // free allocated space of struct
  }
  memcpy( &out->product_type_code, s1+12, 2);
  memcpy( &out->scheduling_code, s1+14, 2);
  memcpy( &out->seconds_to_skip_between_runs, s1+16, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->product_type_code =
          Swap2Bytes(out->product_type_code);
      out->scheduling_code =
          Swap2Bytes(out->scheduling_code);
      out->seconds_to_skip_between_runs =
          Swap4BytesSigned( (UINT4)
              out->seconds_to_skip_between_runs);
  }

  ymd_s *t1_p = extract_ymds_time(s1+20,
                                  target_is_big_endian);
  if( t1_p != NULL) {
      out->product_GenTime_UTC = *t1_p; // copy structure
      RAVE_FREE(t1_p);  // free allocated space of struct
  }
  ymd_s *t2_p = extract_ymds_time(s1+32,
                                  target_is_big_endian);
  if( t2_p != NULL) {
      out->IngestSweep_input_time_TZ = *t2_p; // copy structure
      RAVE_FREE(t2_p);  // free allocated space of struct
  }
  ymd_s *t3_p = extract_ymds_time(s1+44,
                                  target_is_big_endian);
  if( t3_p != NULL) {
      out->IngestFile_input_time_TZ = *t3_p; // copy structure
      RAVE_FREE(t3_p);  // free allocated space of struct
  }
  /*  44 + 12 byte time = 56 */
  memcpy( &out->spare_bytes, s1+56, 6);
  memcpy( &out->product_configfile_name, s1+62, 12);
  memcpy( &out->dataGen_task_name, s1+74, 12);
  memcpy( &out->flag_word, s1+86, 2);
  memcpy( &out->X_scale_cm_per_pixel, s1+88, 4);
  memcpy( &out->Y_scale_cm_per_pixel, s1+92, 4);
  memcpy( &out->Z_scale_cm_per_pixel, s1+96, 4);
  memcpy( &out->X_array_size, s1+100, 4);
  memcpy( &out->Y_array_size, s1+104, 4);
  memcpy( &out->Z_array_size, s1+108, 4);
  memcpy( &out->X_Radar_Location, s1+112, 4);
  memcpy( &out->Y_Radar_Location, s1+116, 4);
  memcpy( &out->Z_Radar_Location, s1+120, 4);
  memcpy( &out->Max_Range_in_cm, s1+124, 4);
  memcpy( &out->HydroClass, s1+128, 1);
  memcpy( &out->spare_byte, s1+129, 1);
  memcpy( &out->data_type_generated, s1+130, 2);
  memcpy( &out->name_of_projection, s1+132, 12);
  memcpy( &out->data_type_used_as_input, s1+144, 12);
  memcpy( &out->projection_type_code, s1+146, 1);
  memcpy( &out->spare_byte_2, s1+147, 1);
  memcpy( &out->radial_smoother_in_km_over_100, s1+148, 2);
  memcpy( &out->number_of_runs_this_product, s1+150, 2);
  memcpy( &out->Z_R_constant_thousandths, s1+152, 4);
  memcpy( &out->Z_R_exponent_thousandths, s1+156, 4);
  memcpy( &out->x_smoother_in_hundredths_of_km, s1+160, 2);
  memcpy( &out->y_smoother_in_hundredths_of_km, s1+162, 2);

  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->flag_word = Swap2Bytes(out->flag_word);
      out->X_scale_cm_per_pixel =
          Swap4BytesSigned( (UINT4)
              out->X_scale_cm_per_pixel);
      out->Y_scale_cm_per_pixel =
          Swap4BytesSigned( (UINT4)
              out->Y_scale_cm_per_pixel);
      out->Z_scale_cm_per_pixel =
          Swap4BytesSigned( (UINT4)
              out->Z_scale_cm_per_pixel);
      out->X_array_size = Swap4BytesSigned( (UINT4)
          out->X_array_size);
      out->Y_array_size = Swap4BytesSigned( (UINT4)
          out->Y_array_size);
      out->Z_array_size = Swap4BytesSigned( (UINT4)
          out->Z_array_size);
      out->X_Radar_Location = Swap4BytesSigned( (UINT4)
          out->X_Radar_Location);
      out->Y_Radar_Location = Swap4BytesSigned( (UINT4)
          out->Y_Radar_Location);
      out->Z_Radar_Location = Swap4BytesSigned( (UINT4)
          out->Z_Radar_Location);
      out->Max_Range_in_cm = Swap4BytesSigned( (UINT4)
          out->Max_Range_in_cm);
      out->data_type_generated = Swap2Bytes(out->data_type_generated);
      out->radial_smoother_in_km_over_100 =
          Swap2BytesSigned( (UINT2)
              out->radial_smoother_in_km_over_100);
      out->number_of_runs_this_product =
          Swap2BytesSigned( (UINT2) out->number_of_runs_this_product);
      out->Z_R_constant_thousandths =
          Swap4BytesSigned( (UINT4)
              out->Z_R_constant_thousandths);
      out->Z_R_exponent_thousandths =
          Swap4BytesSigned( (UINT4)
              out->Z_R_exponent_thousandths);
      out->x_smoother_in_hundredths_of_km =
          Swap2BytesSigned( (UINT2)
              out->x_smoother_in_hundredths_of_km);
      out->y_smoother_in_hundredths_of_km =
          Swap2BytesSigned( (UINT2)
              out->y_smoother_in_hundredths_of_km);
  }

  /* handle the different types of product specific info */
  if( out->product_type_code == PPI_type) {
    ppi_psi_struct *ppi_p = extract_psi_ppi(s1+164,
                                            target_is_big_endian);
    if( ppi_p != NULL) {
        out->product_specific_info.ppi = *ppi_p; // copy structure
        RAVE_FREE(ppi_p); // free allocated space of struct
    }
  }
  else if (out->product_type_code == RHI_type) {
     rhi_psi_struct *rhi_p = extract_psi_rhi(s1+164,
                                             target_is_big_endian);
     if( rhi_p != NULL) {
         out->product_specific_info.rhi = *rhi_p; // copy structure
         RAVE_FREE(rhi_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == CAPPI_type) {
     cappi_psi_struct *cappi_p = extract_psi_cappi(s1+164,
                                                   target_is_big_endian);
     if( cappi_p != NULL) {
         out->product_specific_info.cappi = *cappi_p; // copy structure
         RAVE_FREE(cappi_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == CROSS_type) {
     cross_psi_struct *cross_p = extract_psi_cross(s1+164,
                                                   target_is_big_endian);
     if( cross_p != NULL) {
         out->product_specific_info.cross = *cross_p; // copy structure
         RAVE_FREE(cross_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == TOPS_type) {
    top_psi_struct *top_p = extract_psi_tops(s1+164,
                                             target_is_big_endian);
    if( top_p != NULL) {
        out->product_specific_info.top = *top_p; // copy structure
        RAVE_FREE(top_p); // free allocated space of struct
    }
  }
  else if (out->product_type_code == TRACK_type) {
    track_psi_struct *track_p = extract_psi_track(s1+164,
                                                  target_is_big_endian);
    if( track_p != NULL) {
         out->product_specific_info.track = *track_p; // copy structure
         RAVE_FREE(track_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == RAIN1_type) {
     rain_psi_struct *rain1_p = extract_psi_rain(s1+164,
                                                 target_is_big_endian);
     if( rain1_p != NULL) {
         out->product_specific_info.rain = *rain1_p; // copy structure
         RAVE_FREE(rain1_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == RAINN_type) {
     rain_psi_struct *rainn_p  = extract_psi_rain(s1+164,
                                                  target_is_big_endian);
     if( rainn_p != NULL) {
         out->product_specific_info.rain = *rainn_p; // copy structure
         RAVE_FREE(rainn_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == VVP_type) {
     vvp_psi_struct *vvp_p  = extract_psi_vvp(s1+164,
                                              target_is_big_endian);
     if( vvp_p != NULL) {
         out->product_specific_info.vvp = *vvp_p; // copy structure
         RAVE_FREE(vvp_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == VIL_type) {
     vil_psi_struct *vil_p = extract_psi_vil(s1+164,
                                             target_is_big_endian);
     if( vil_p != NULL) {
         out->product_specific_info.vil = *vil_p; // copy structure
         RAVE_FREE(vil_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == SHEAR_type) {
     shear_psi_struct *shear_p = extract_psi_shear(s1+164,
                                                   target_is_big_endian);
     if( shear_p != NULL) {
         out->product_specific_info.shear = *shear_p; // copy structure
         RAVE_FREE(shear_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == WARN_type) {
     warn_psi_struct *warn_p = extract_psi_warn(s1+164,
                                                target_is_big_endian);
     if( warn_p != NULL) {
         out->product_specific_info.warn = *warn_p; // copy structure
         RAVE_FREE(warn_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == CATCH_type) {
     catch_psi_struct *catch_p = extract_psi_catch(s1+164,
                                                   target_is_big_endian);
     if( catch_p != NULL) {
         out->product_specific_info.catch = *catch_p; // copy structure
         RAVE_FREE(catch_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == RTI_type) {
     rti_psi_struct *rti_p = extract_psi_rti(s1+164,
                                             target_is_big_endian);
     if( rti_p != NULL) {
         out->product_specific_info.rti = *rti_p; // copy structure
         RAVE_FREE(rti_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == RAW_type) {
     raw_psi_struct *raw_p = extract_psi_raw(s1+164,
                                             target_is_big_endian);
     if( raw_p != NULL) {
        out->product_specific_info.raw = *raw_p; // copy structure
        RAVE_FREE(raw_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == MAX_type) {
     maximum_psi_struct *max_p = extract_psi_max(s1+164,
                                                 target_is_big_endian);
     if( max_p != NULL) {
         out->product_specific_info.max = *max_p; // copy structure
         RAVE_FREE(max_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == USER_type) {
     user_psi_struct *user_p = extract_psi_user(s1+164,
                                                target_is_big_endian);
     if( user_p != NULL) {
         out->product_specific_info.user = *user_p; // copy structure
         RAVE_FREE(user_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == WIND_type) {
     wind_psi_struct *wind_p  = extract_psi_wind(s1+164,
                                                 target_is_big_endian);
     if( wind_p != NULL) {
         out->product_specific_info.wind = *wind_p; // copy structure
         RAVE_FREE(wind_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == BEAM_type) {
     beam_psi_struct *beam_p = extract_psi_beam(s1+164,
                                                target_is_big_endian);
     if( beam_p != NULL) {
         out->product_specific_info.beam = *beam_p; // copy structure
         RAVE_FREE(beam_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == FCAST_type) {
     fcast_psi_struct *fcast_p  = extract_psi_fcast(s1+164,
                                                    target_is_big_endian);
     if( fcast_p != NULL) {
         out->product_specific_info.fcast = *fcast_p; // copy structure
         RAVE_FREE(fcast_p); // free allocated space of struct
     }
  }
  else if (out->product_type_code == TDWR_type) {
     tdwr_psi_struct *tdwr_p = extract_psi_tdwr(s1+164,
                                                target_is_big_endian);
     if( tdwr_p != NULL) {
         out->product_specific_info.tdwr = *tdwr_p; // copy structure
         RAVE_FREE(tdwr_p); // free allocated space of struct
      }
  }
  else if (out->product_type_code == SRI_type) {
     sri_psi_struct *sri_p = extract_psi_sri(s1+164,
                                             target_is_big_endian);
     if( sri_p != NULL) {
         out->product_specific_info.sri = *sri_p; // copy structure
         RAVE_FREE(sri_p); // free allocated space of struct
     }
  }
  else {
    memcpy( &out->product_specific_info.ipad, s1+164, PSI_SIZE );
  }

  /* now back to assigning regular variables */
  memcpy( &out->list_of_minor_task_suffixes,
      s1+164+PSI_SIZE, PCF_TASK_MINOR_SIZE);
  memcpy( &out->QPE_algorithm_name,
      s1+164+PSI_SIZE+PCF_TASK_MINOR_SIZE, PCF_QPE_ALGORITHM_SIZE);
  /* 
   * the last item is a structure 
   */
  csd_s *c_p;
  /* start location = 272, size =48 */
  c_p = extract_color_scale_def(
      s1+164+PSI_SIZE+PCF_TASK_MINOR_SIZE+PCF_QPE_ALGORITHM_SIZE,
      target_is_big_endian);
  if( c_p != NULL) {
      out->colors = *c_p; // copy structure
      RAVE_FREE(c_p); // free allocated space of struct
  }
  return out;
}


/*****************************************************************************
*                                                                            *
*  -------------------------- extract_product_end -------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from an IRIS raw product
 * record and produce/return a pointer to a
 * product_end type structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If any failure occurs a pointer to null is returned
 * and error messages are written to stderr */
/* ======================================================================== */
ped_s *extract_product_end(UINT1 *s1,
                            _Bool target_is_big_endian) {
  ped_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'ped_s' structure in"
          " function 'extract_product_end'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->site_name, s1, 16);
  memcpy( &out->IRIS_version_product_maker, s1+16, 8);
  memcpy( &out->IRIS_version_ingest_data, s1+24, 8);
  ymd_s *t1_p = NULL;
  t1_p = extract_ymds_time(s1+32,
                           target_is_big_endian);
  if( t1_p != NULL) {
      /* copy structure */
      out->time_of_oldest_input_ingest_file = *t1_p;
      /* free allocated memory of structure */
      RAVE_FREE(t1_p);
  }
  memcpy( &out->spare_bytes_1, s1+44, 28);
  memcpy( &out->minutes_LST_is_west_of_GMT, s1+72, 2);
  memcpy( &out->hardware_name_of_ingest_data_source,
      s1+74, 16);
  memcpy( &out->site_name_of_ingest_data_source, s1+90, 16);
  memcpy( &out->minutes_recorded_standard_time_is_west_of_GMT,
      s1+106, 2);
  memcpy( &out->latitude_of_center, s1+108, 4);
  memcpy( &out->longitude_of_center, s1+112, 4);
  memcpy( &out->signed_ground_height_relative_to_sea_level,
      s1+116, 2);
  memcpy( &out->height_of_radar_above_the_ground_in_meters,
      s1+118, 2);
  memcpy( &out->PRF_in_hertz, s1+120, 4);
  memcpy( &out->pulse_width_in_hundredths_of_microseconds,
      s1+124, 4);
  memcpy( &out->type_of_signal_processor_used, s1+128, 2);
  memcpy( &out->trigger_rate_scheme, s1+130, 2);
  memcpy( &out->number_of_samples_used, s1+132, 2);
  memcpy( &out->name_of_clutter_filter_file, s1+134, 12);
  memcpy( &out->number_of_linear_based_filter_for_the_first_bin,
     s1+146, 2);
  memcpy( &out->wavelength_in_hundredths_of_centimeters,
      s1+148, 4);
  memcpy( &out->truncation_height_in_cm_above_radar,
      s1+152, 4);
  memcpy( &out->range_of_the_first_bin_in_cm, s1+156, 4);
  memcpy( &out->range_of_the_last_bin_in_cm, s1+160, 4);
  memcpy( &out->number_of_output_bins, s1+164, 4);
  memcpy( &out->flag_word, s1+168, 2);
  memcpy( &out->number_of_ingest_or_product_files_used,
      s1+170, 2);
  memcpy( &out->type_of_polarization_used, s1+172, 2);
  memcpy( &out->IO_cal_value_horizontal_pol_in_hundredths_of_dBm,
      s1+174, 2);
  memcpy( &out->noise_at_calibration_horizontal_pol_in_hundredths_of_dBm,
      s1+176, 2);
  memcpy( &out->radar_constant_horizontal_pol_in_hundredths_of_dB,
      s1+178, 2);
  memcpy( &out->receiver_bandwidth_in_kHz, s1+180, 2);
  memcpy( &out->current_noise_level_horizontal_pol_in_hundredths_of_dBm,
      s1+182, 2);
  memcpy( &out->current_noise_level_vertical_pol_in_hundredths_of_dBm,
      s1+184, 2);
  memcpy( &out->LDR_offset_in_hundredths_dB, s1+186, 2);
  memcpy( &out->ZDR_offset_in_hundredths_dB, s1+188, 2);
  memcpy( &out->TFC_cal_flags, s1+190, 2);
  memcpy( &out->TFC_cal_flags2, s1+192, 2);
  memcpy( &out->spare_bytes_2, s1+194, 18);
  memcpy( &out->projection_angle_standard_parallel_1, s1+212, 4);
  memcpy( &out->projection_angle_standard_parallel_2, s1+216, 4);
  memcpy( &out->Equatorial_radius_of_earth_in_cm, s1+220, 4);
  memcpy( &out->one_over_flattening_in_millionths, s1+224, 4);
  memcpy( &out->fault_status_of_task, s1+228, 4);
  memcpy( &out->mask_of_input_sites_used_in_a_composite, s1+232, 4);
  memcpy( &out->number_of_log_based_filter_for_the_first_bin,
      s1+236, 2);
  memcpy( &out->nonzero_if_cluttermap_applied_to_the_ingest_data,
      s1+238, 2);
  memcpy( &out->latitude_of_projection_reference, s1+240, 4);
  memcpy( &out->longitude_of_projection_reference, s1+244, 4);
  memcpy( &out->product_sequence_number, s1+248, 2);
  memcpy( &out->spare_bytes_3, s1+250, 32);
  memcpy( &out->melting_level_in_meters, s1+282, 2);
  memcpy( &out->height_of_radar_in_meters, s1+284, 2);
  memcpy( &out->number_of_elements_in_product_results_array, s1+286, 2);
  memcpy( &out->mean_wind_speed, s1+288, 1);
  memcpy( &out->mean_wind_direction, s1+289, 1);
  memcpy( &out->spare_bytes_4, s1+290, 2);
  memcpy( &out->time_zone_name_of_recorded_data, s1+292, 8);
  memcpy( &out->offset_to_extended_time_header, s1+300,4);
  memcpy( &out->spare_bytes_5, s1+304, 4);

  /* swap some bytes if input file is big-endian */
  if( target_is_big_endian ) {
    out->minutes_LST_is_west_of_GMT =
        Swap2BytesSigned( (UINT2)
            out->minutes_LST_is_west_of_GMT);
    out->minutes_recorded_standard_time_is_west_of_GMT =
        Swap2BytesSigned( (UINT2)
            out->minutes_recorded_standard_time_is_west_of_GMT);
    out->latitude_of_center  = Swap4Bytes(out->latitude_of_center);
    out->longitude_of_center = Swap4Bytes(out->longitude_of_center);
    out->signed_ground_height_relative_to_sea_level =
        Swap2BytesSigned( (UINT2)
            out->signed_ground_height_relative_to_sea_level);
    out->height_of_radar_above_the_ground_in_meters =
        Swap2BytesSigned( (UINT2)
            out->height_of_radar_above_the_ground_in_meters);
    out->PRF_in_hertz = Swap4BytesSigned( (UINT4)
        out->PRF_in_hertz);
    out->pulse_width_in_hundredths_of_microseconds =
        Swap4BytesSigned( (UINT4)
            out->pulse_width_in_hundredths_of_microseconds);
    out->type_of_signal_processor_used =
        Swap2Bytes(out->type_of_signal_processor_used);
    out->trigger_rate_scheme = Swap2Bytes(out->trigger_rate_scheme);
    out->number_of_samples_used =
        Swap2BytesSigned( (UINT2)
            out->number_of_samples_used);
    out->number_of_linear_based_filter_for_the_first_bin =
        Swap2Bytes(out->number_of_linear_based_filter_for_the_first_bin);
    out->wavelength_in_hundredths_of_centimeters =
        Swap4BytesSigned( (UINT4) out->wavelength_in_hundredths_of_centimeters);
    out->truncation_height_in_cm_above_radar =
        Swap4BytesSigned( (UINT4) out->truncation_height_in_cm_above_radar);
    out->range_of_the_first_bin_in_cm =
        Swap4BytesSigned( (UINT4)
            out->range_of_the_first_bin_in_cm);
    out->range_of_the_last_bin_in_cm =
        Swap4BytesSigned( (UINT4)
            out->range_of_the_last_bin_in_cm);
    out->number_of_output_bins =
        Swap4BytesSigned( (UINT4)
            out->number_of_output_bins);
    out->flag_word = Swap2Bytes(out->flag_word);
    out->number_of_ingest_or_product_files_used =
        Swap2BytesSigned( (UINT2)
            out->number_of_ingest_or_product_files_used);
    out->type_of_polarization_used =
        Swap2Bytes(out->type_of_polarization_used);
    out->IO_cal_value_horizontal_pol_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->IO_cal_value_horizontal_pol_in_hundredths_of_dBm);
    out->noise_at_calibration_horizontal_pol_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->noise_at_calibration_horizontal_pol_in_hundredths_of_dBm);
    out->radar_constant_horizontal_pol_in_hundredths_of_dB =
        Swap2BytesSigned( (UINT2)
            out->radar_constant_horizontal_pol_in_hundredths_of_dB);
    out->receiver_bandwidth_in_kHz =
        Swap2Bytes(out->receiver_bandwidth_in_kHz);
    out->current_noise_level_horizontal_pol_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->current_noise_level_horizontal_pol_in_hundredths_of_dBm);
    out->current_noise_level_vertical_pol_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->current_noise_level_vertical_pol_in_hundredths_of_dBm);
    out->LDR_offset_in_hundredths_dB =
        Swap2BytesSigned( (UINT2)
            out->LDR_offset_in_hundredths_dB);
    out->ZDR_offset_in_hundredths_dB =
        Swap2BytesSigned( (UINT2)
            out->ZDR_offset_in_hundredths_dB);
    out->TFC_cal_flags =  Swap2Bytes(out->TFC_cal_flags);
    out->TFC_cal_flags2 =  Swap2Bytes(out->TFC_cal_flags2);
    out->projection_angle_standard_parallel_1 =
        Swap4Bytes(out->projection_angle_standard_parallel_1);
    out->projection_angle_standard_parallel_2 =
        Swap4Bytes(out->projection_angle_standard_parallel_2);
    out->Equatorial_radius_of_earth_in_cm =
        Swap4Bytes(out->Equatorial_radius_of_earth_in_cm);
    out->one_over_flattening_in_millionths =
        Swap4Bytes(out->one_over_flattening_in_millionths);
    out->fault_status_of_task =
        Swap4Bytes(out->fault_status_of_task);
    out->mask_of_input_sites_used_in_a_composite =
        Swap4Bytes(out->mask_of_input_sites_used_in_a_composite);
    out->number_of_log_based_filter_for_the_first_bin =
        Swap2Bytes(out->number_of_log_based_filter_for_the_first_bin);
    out->nonzero_if_cluttermap_applied_to_the_ingest_data =
        Swap2Bytes(out->nonzero_if_cluttermap_applied_to_the_ingest_data);
    out->latitude_of_projection_reference =
        Swap4Bytes(out->latitude_of_projection_reference);
    out->longitude_of_projection_reference =
        Swap4Bytes(out->longitude_of_projection_reference);
    out->product_sequence_number =
        Swap2BytesSigned( (UINT2)
            out->product_sequence_number);
    out->melting_level_in_meters =
        Swap2BytesSigned( (UINT2)
            out->melting_level_in_meters);
    out->height_of_radar_in_meters =
        Swap2BytesSigned( (UINT2)
            out->height_of_radar_in_meters);
    out->number_of_elements_in_product_results_array =
        Swap2BytesSigned( (UINT2)
            out->number_of_elements_in_product_results_array);
    out->offset_to_extended_time_header =
        Swap4Bytes(out->offset_to_extended_time_header);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------- extract_ingest_header ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * ingest_header type structure. The function is passed
 * a pointer to 'IRISbuf' structure.
 * If an allocation failure occurs, a pointer to
 * null is returned and error messages are printed    */
/* ======================================================================== */
ihd_s *extract_ingest_header(IRISbuf *IRISbuf_p,
                             _Bool target_is_big_endian) {
  /* declare some pointers to be used below */
  UINT1 *bufIRIS_p;
  bufIRIS_p = &IRISbuf_p->bufIRIS[0];
  UINT1 *ptr_s0, *ptr_s1, *ptr_s2, *ptr_s3, *ptr_s4;
  /*
   * prt_s0 points to beginning of the input data buffer,
   * which is the start of 'structure_header' data
   */
  ptr_s0 = bufIRIS_p;

  /* prt_s1 points to the start of 'ingest_configuration' data */
  ptr_s1 = ptr_s0 + STRUCT_HEADER_SIZE;

  /* prt_s2 points to the start of 'task_configuration' data */
  ptr_s2 = ptr_s1 + INGEST_CONFIGURATION_SIZE;

  /* prt_s3 points to the start of 732 bytes labeled 'Spare' */
  ptr_s3 = ptr_s2 + TASK_CONFIGURATION_SIZE;

  /* prt_s4 points to the start of 
   * 128 bytes labeled 'GPARM_from_DSP' 
   */
  ptr_s4 = ptr_s3 + 732;
  /* 
   * allocate a 'product_hdr' structure and call it 'out'
   */
  ihd_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if ( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'ihd_s' structure in"
          " function 'extract_ingest_header'.\n");
      return NULL;
  }
  /*
   * allocation went OK so fill the structure
   * each of the following functions return
   * a pointer to a structure
   */
  shd_s *shd_p;
  shd_p = extract_structure_header(ptr_s0, target_is_big_endian);
  icf_s *icf_p;
  icf_p = extract_ingest_configuration(ptr_s1, target_is_big_endian);
  tcf_s *tcf_p;
  tcf_p = extract_task_configuration(ptr_s2, target_is_big_endian);
  gpa_s *gpa_p;
  gpa_p = extract_gparm(ptr_s4, target_is_big_endian);
  /* pointers to structures get copied  */
  out->hdr = *shd_p;
  out->icf = *icf_p;
  out->tcf = *tcf_p;
  out->GParm = *gpa_p;
  /*
   * free the structures allocated in the called functions
   * (if the pointers to them are OK)
   */
  if( shd_p != NULL) RAVE_FREE(shd_p);
  if( icf_p != NULL) RAVE_FREE(icf_p);
  if( tcf_p != NULL) RAVE_FREE(tcf_p);
  if( gpa_p != NULL) RAVE_FREE(gpa_p);
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_ingest_configuration --------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * ingest_configuration type structure. The function is
 * passed a pointer to a location in a byte_array.
 * If allocation failure occurs a pointer to
 * null is returned and error messages are printed */
/* ======================================================================== */
icf_s *extract_ingest_configuration(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  UINT1 *ptr_r1;
  icf_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'icf_s' structure in"
          " function 'extract_ingest_configuration'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->name_of_file_on_disk, s1, 80);
  memcpy( &out->number_of_associated_disk_files_extant, s1+80, 2);
  memcpy( &out->number_of_sweeps_completed, s1+82, 2);
  memcpy( &out->total_size_of_all_files, s1+84, 4);
  ymd_s *ymd_p = NULL;
  ymd_p= extract_ymds_time(s1+88,
                           target_is_big_endian);
  if( ymd_p ) {
      out->time_that_volume_scan_was_started = *ymd_p; // copy structure
      RAVE_FREE(ymd_p); // free allocated memory of structure
  }
  memcpy( &out->twelve_spare_bytes, s1+88+YMDS_TIME_SIZE, 12);
  ptr_r1 = s1+88+YMDS_TIME_SIZE+12;
  memcpy( &out->number_of_bytes_in_ray_headers, ptr_r1, 2);
  memcpy( &out->number_of_bytes_in_extended_ray_headers, ptr_r1+2, 2);
  memcpy( &out->number_of_bytes_in_task_configuration_table, ptr_r1+4, 2);
  memcpy( &out->playback_version_number, ptr_r1+6, 2);
  memcpy( &out->four_spare_bytes, ptr_r1+8, 4);
  memcpy( &out->IRIS_version_number, ptr_r1+12, 8);
  memcpy( &out->ingest_hardware_name_of_site, ptr_r1+20, 16);
  memcpy( &out->minutes_west_of_GMT_of_LST, ptr_r1+36, 2);
  memcpy( &out->radar_site_name_from_setup_utility, ptr_r1+38, 16);
  memcpy( &out->minutes_west_of_GMT_recorded_time, ptr_r1+54, 2);
  memcpy( &out->latitude_of_radar, ptr_r1+56, 4);
  memcpy( &out->longitude_of_radar, ptr_r1+60, 4);
  memcpy( &out->height_of_ground_site_in_meters_above_sea_level, ptr_r1+64, 2);
  memcpy( &out->radar_height_in_meters_above_ground, ptr_r1+66, 2);
  memcpy( &out->resolution_as_rays_per_360_degree_sweep, ptr_r1+68, 2);
  memcpy( &out->index_of_first_ray, ptr_r1+70, 2);
  memcpy( &out->number_of_rays_in_sweep, ptr_r1+72, 2);
  memcpy( &out->bytes_in_each_gparam, ptr_r1+74, 2);
  memcpy( &out->altitude_of_radar_cm_above_sea_level, ptr_r1+76, 4);
  memcpy( &out->velocity_of_radar_in_cm_per_sec_east_north_up, ptr_r1+80, 12);
  memcpy( &out->antenna_offset_from_INU_in_cm_starboard_bow_up, ptr_r1+92, 12);
  memcpy( &out->fault_status, ptr_r1+104, 4);
  memcpy( &out->height_of_melting_level_above_sea_level_in_meters, ptr_r1+108, 2);
  memcpy( &out->two_spare_bytes, ptr_r1+110, 2);
  memcpy( &out->local_timezone_string, ptr_r1+112, 8);
  memcpy( &out->flags, ptr_r1+120, 4);
  memcpy( &out->config_name_in_the_dpolapp_conf_file, ptr_r1+124, 16);
  memcpy( &out->two_hundred_twenty_eight_spare_bytes, ptr_r1+140, 228);

  /* swap some bytes if the IRIS file is big-endian */
  if( target_is_big_endian ) {
    out->number_of_associated_disk_files_extant =
        Swap2BytesSigned( (UINT2)
            out->number_of_associated_disk_files_extant);
    out->number_of_sweeps_completed =
        Swap2BytesSigned( (UINT2)
            out->number_of_sweeps_completed);
    out->total_size_of_all_files =
        Swap4BytesSigned( (UINT4)
            out->total_size_of_all_files);
    out->number_of_bytes_in_ray_headers =
        Swap2BytesSigned( (UINT2)
            out->number_of_bytes_in_ray_headers);
    out->number_of_bytes_in_extended_ray_headers =
        Swap2BytesSigned( (UINT2)
            out->number_of_bytes_in_extended_ray_headers);
    out->number_of_bytes_in_task_configuration_table =
        Swap2BytesSigned( (UINT2)
            out->number_of_bytes_in_task_configuration_table);
    out->playback_version_number =
        Swap2BytesSigned( (UINT2)
            out->playback_version_number);
    out->minutes_west_of_GMT_of_LST =
        Swap2BytesSigned( (UINT2)
            out->minutes_west_of_GMT_of_LST);
    out->minutes_west_of_GMT_recorded_time =
        Swap2BytesSigned( (UINT2)
            out->minutes_west_of_GMT_recorded_time);
    out->latitude_of_radar = Swap4Bytes(out->latitude_of_radar);
    out->longitude_of_radar = Swap4Bytes(out->longitude_of_radar);
    out->height_of_ground_site_in_meters_above_sea_level =
        Swap2BytesSigned( (UINT2)
            out->height_of_ground_site_in_meters_above_sea_level);
    out->radar_height_in_meters_above_ground =
        Swap2BytesSigned( (UINT2)
            out->radar_height_in_meters_above_ground);
    out->resolution_as_rays_per_360_degree_sweep =
        Swap2Bytes(out->resolution_as_rays_per_360_degree_sweep);
    out->index_of_first_ray = Swap2Bytes(out->index_of_first_ray);
    out->number_of_rays_in_sweep =
        Swap2Bytes(out->number_of_rays_in_sweep);
    out->bytes_in_each_gparam = Swap2BytesSigned( (UINT2)
        out->bytes_in_each_gparam);
    out->altitude_of_radar_cm_above_sea_level =
        Swap4BytesSigned( (UINT4)
            out->altitude_of_radar_cm_above_sea_level);
    for(int i=0; i<3; i++) {
      out->velocity_of_radar_in_cm_per_sec_east_north_up[i] =
        Swap4BytesSigned( (UINT4)
            out->velocity_of_radar_in_cm_per_sec_east_north_up[i]);
      out->antenna_offset_from_INU_in_cm_starboard_bow_up[i] =
        Swap4BytesSigned( (UINT4)
            out->antenna_offset_from_INU_in_cm_starboard_bow_up[i]);
    }
    out->fault_status = Swap4Bytes(out->fault_status);
    out->height_of_melting_level_above_sea_level_in_meters =
        Swap2BytesSigned( (UINT2)
            out->height_of_melting_level_above_sea_level_in_meters);
    out->flags = Swap4Bytes(out->flags);
  }

  return out;
}

/*****************************************************************************
*                                                                            *
*  ----------------------- extract_task_configuration ---------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * task_configuration type structure. The function is
 * passe a pointer to location in a byte array.
 * If an allocation failure occurs, a pointer to
 * null is returned and error messages are printed */
/* ======================================================================== */
tcf_s *extract_task_configuration(UINT1 *s0,
                                   _Bool target_is_big_endian) {
  UINT1 *ptr_r0, *ptr_r1, *ptr_r2, *ptr_r3, *ptr_r4, *ptr_r5;
  UINT1 *ptr_r6, *ptr_r7, *ptr_r8;
  /* prt_r0 points to start of the 'structure_header' data */
  ptr_r0 = s0;
  
  /* prt_r1 points to the start of 'task_sched_info' data */
  ptr_r1 = ptr_r0 + STRUCT_HEADER_SIZE; // STRUCT_HEADER_SIZE = 12

  /* prt_r2 points to the start of 'task_dsp_info' data */
  ptr_r2 = ptr_r1 + TASK_SCHED_INFO_SIZE; // TASK_SCHED_INFO_SIZE = 120

  /* prt_r3 points to the start of 'task_calib_info' data */
  ptr_r3 = ptr_r2 + TASK_DSP_INFO_SIZE; // TASK_DSP_INFO_SIZE = 320

  /* prt_r4 points to the start of 'task_range_info' data */
  ptr_r4 = ptr_r3 + TASK_CALIB_INFO_SIZE; // TASK_CALIB_INFO_SIZE = 320

  /* prt_r5 points to the start of 'task_scan_info' data */
  ptr_r5 = ptr_r4 + TASK_RANGE_INFO_SIZE; // TASK_RANGE_INFO_SIZE = 160

  /* prt_r6 points to the start of 'task_misc_info' data */
  ptr_r6 = ptr_r5 + TASK_SCAN_INFO_SIZE; // TASK_SCAN_INFO_SIZE = 320

  /* prt_r7 points to the start of 'task_end_info' data */
  ptr_r7 = ptr_r6 + TASK_MISC_INFO_SIZE; // TASK_MISC_INFO_SIZE = 320

  /* prt_r8 points to the start of 'comments' data */
  ptr_r8 = ptr_r7 + TASK_CONF_END_SIZE; // TASK_CONF_END_SIZE = 320

  tcf_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tcf_s' structure in"
          " function 'extract_task_configuration'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  shd_s *shd_p = NULL;
  shd_p = extract_structure_header(ptr_r0,
                                   target_is_big_endian);
  if( shd_p != NULL ) {
      out->hdr = *shd_p; // copy structure
      RAVE_FREE(shd_p); // free allocated memory of structure
  }
  tschedi_s *tschedi_p = NULL;
  tschedi_p = extract_task_sched_info(ptr_r1,
                                      target_is_big_endian);
  if( tschedi_p != NULL ) {
      out->sch = *tschedi_p; // copy structure
      RAVE_FREE(tschedi_p); // free allocated memory of structure
  }
  tdi_s *tdi_p = NULL;
  tdi_p = extract_task_dsp_info(ptr_r2,
                                target_is_big_endian);
  if( tdi_p != NULL ) {
      out->dsp = *tdi_p; // copy structure
      RAVE_FREE(tdi_p); // free allocated memory of structure
  }
  tci_s *tci_p = NULL;
  tci_p = extract_task_calib_info(ptr_r3,
                                  target_is_big_endian);
  if( tci_p != NULL ) {
      out->cal = *tci_p; // copy structure
      RAVE_FREE(tci_p); // free allocated memory of structure
  }
  tri_s *tri_p = NULL;
  tri_p = extract_task_range_info(ptr_r4,
                                  target_is_big_endian);
  if( tri_p != NULL ) {
      out->rng = *tri_p; // copy structure
      RAVE_FREE(tri_p); // free allocated memory of structure
  }
  tscani_s *tscani_p = NULL;
  tscani_p = extract_task_scan_info(ptr_r5,
                                    target_is_big_endian);
  if( tscani_p != NULL ) {
      out->scan = *tscani_p; // copy structure
      RAVE_FREE(tscani_p); // free allocated memory of structure
  }
  tmi_s *tmi_p = NULL;
  tmi_p = extract_task_misc_info(ptr_r6,
                                 target_is_big_endian);
  if( tmi_p != NULL ) {
      out->misc = *tmi_p; // copy structure
      RAVE_FREE(tmi_p); // free allocated memory of structure
  }
  tei_s *tei_p = NULL;
  tei_p = extract_task_end_info(ptr_r7,
                                target_is_big_endian);
  if( tei_p != NULL ) {
    out->end = *tei_p; // copy structure
    RAVE_FREE(tei_p); // free allocated memory of structure
  }
  memcpy( &out->comnts, ptr_r8, TASK_COMNT_SIZE); // TASK_COMNT_SIZE = 720
  return out;
}

/*****************************************************************************
*                                                                            *
*  ----------------------------- extract_gparm ----------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * gparm type structure. The function is passed
 * a pointer to location inside a byte array.
 * If an allocation failure occurs, a pointer to
 * null is returned and error messages are printed */
/* ======================================================================== */
gpa_s *extract_gparm(UINT1 *s0,
                      _Bool target_is_big_endian) {
  /* find gparm struct in IRIS file dsp.h */
  gpa_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'gpa_s' structure in"
          " function 'extract_gparm'.\n");
      return NULL;
  }

  /* allocation went OK so fill the structure */
  memcpy( &out->irev_ser, s0, 2);
  memcpy( &out->ibin_out_num, s0+2, 2);
  memcpy( &out->iprt_mes, s0+4, 2);
  memcpy( &out->itaga, s0+6, 2);
  memcpy( &out->itagb, s0+8, 2);
  memcpy( &out->log_nse, s0+10, 2);
  memcpy( &out->i_nse_, s0+12, 2);
  memcpy( &out->q_nse_, s0+14, 2);
  memcpy( &out->istat_l, s0+16, 2);
  memcpy( &out->istat_i, s0+18, 2);
  memcpy( &out->idiag_a, s0+20, 2);
  memcpy( &out->idiag_b, s0+22, 2);
  memcpy( &out->isamp, s0+24, 2);
  memcpy( &out->itrg_cnt_a, s0+26, 2);
  memcpy( &out->itrg_cnt_b, s0+28, 2);
  memcpy( &out->iaqbins, s0+30, 2);
  memcpy( &out->iprbins, s0+32, 2);
  memcpy( &out->istat_i2, s0+34, 2);
  memcpy( &out->inse_rng, s0+36, 2);
  memcpy( &out->inse_prt, s0+38, 2);
  memcpy( &out->ipwmin_0, s0+40, 2);
  memcpy( &out->ipwmin_1, s0+42, 2);
  memcpy( &out->ipwmin_2, s0+44, 2);
  memcpy( &out->ipwmin_3, s0+46, 2);
  memcpy( &out->ipw_bits, s0+48, 2);
  memcpy( &out->ipw_now, s0+50, 2);
  memcpy( &out->iprt_gen, s0+52, 2);
  memcpy( &out->iprt_des, s0+54, 2);
  memcpy( &out->iprt_start, s0+56, 2);
  memcpy( &out->iprt_end, s0+58, 2);
  memcpy( &out->iflags, s0+60, 2);
  memcpy( &out->iz_slope, s0+62, 2);
  memcpy( &out->izns_thr, s0+64, 2);
  memcpy( &out->iccr_thr, s0+66, 2);
  memcpy( &out->isqi_thr, s0+68, 2);
  memcpy( &out->isig_thr, s0+70, 2);
  memcpy( &out->iz_calib, s0+72, 2);
  memcpy( &out->iqi_now, s0+74, 2);
  memcpy( &out->iz_now, s0+76, 2);
  memcpy( &out->ibin_avg, s0+78, 2);
  memcpy( &out->idiag_c, s0+80, 2);
  memcpy( &out->idiag_d, s0+82, 2);
  memcpy( &out->iproc_hdr0, s0+84, 2);
  memcpy( &out->isq_lo, s0+86, 2);
  memcpy( &out->isq_hi, s0+88, 2);
  memcpy( &out->qsq_lo, s0+90, 2);
  memcpy( &out->qsq_hi, s0+92, 2);
  memcpy( &out->zlin_noise, s0+94, 2);
  memcpy( &out->zlin_rms, s0+96, 2);
  memcpy( &out->inse_hv_ratio, s0+98, 2);
  memcpy( &out->iafclevel, s0+100, 2);
  memcpy( &out->intflt, s0+102, 2);
  memcpy( &out->intflt_p1, s0+104, 2);
  memcpy( &out->intflt_p2, s0+106, 2);
  memcpy( &out->istat_i3, s0+108, 2);
  memcpy( &out->itrigslew, s0+110, 2);
  memcpy( &out->iPolFlags, s0+112, 2);
  memcpy( &out->iMaskSpacingCM, s0+114, 2);
  memcpy( &out->istat_i4, s0+116, 2);
  memcpy( &out->unused_word_60, s0+118, 2);
  memcpy( &out->unused_word_61, s0+120, 2);
  memcpy( &out->unused_word_62, s0+122, 2);
  memcpy( &out->unused_word_63, s0+124, 2);
  memcpy( &out->unused_word_64, s0+126, 2);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->irev_ser =  Swap2Bytes(out->irev_ser);
    out->ibin_out_num = Swap2Bytes(out->ibin_out_num);
    out->iprt_mes = Swap2Bytes(out->iprt_mes);
    out->itaga = Swap2Bytes(out->itaga);
    out->itagb = Swap2Bytes(out->itagb);
    out->log_nse = Swap2Bytes(out->log_nse);
    out->i_nse_ = Swap2BytesSigned( (UINT2) out->i_nse_);
    out->q_nse_ = Swap2BytesSigned( (UINT2) out->q_nse_);
    out->istat_l = Swap2Bytes(out->istat_l);
    out->istat_i = Swap2Bytes(out->istat_i);
    out->idiag_a = Swap2Bytes(out->idiag_a);
    out->idiag_b = Swap2Bytes(out->idiag_b);
    out->isamp = Swap2Bytes(out->isamp);
    out->itrg_cnt_a = Swap2Bytes(out->itrg_cnt_a);
    out->itrg_cnt_b = Swap2Bytes(out->itrg_cnt_b);
    out->iaqbins = Swap2Bytes(out->iaqbins);
    out->iprbins = Swap2Bytes(out->iprbins);
    out->istat_i2 = Swap2Bytes(out->istat_i2);
    out->inse_rng = Swap2Bytes(out->inse_rng);
    out->inse_prt = Swap2Bytes(out->inse_prt);
    out->ipwmin_0 = Swap2Bytes(out->ipwmin_0);
    out->ipwmin_1 = Swap2Bytes(out->ipwmin_1);
    out->ipwmin_2 = Swap2Bytes(out->ipwmin_2);
    out->ipwmin_3 = Swap2Bytes(out->ipwmin_3);
    out->ipw_bits = Swap2Bytes(out->ipw_bits);
    out->ipw_now = Swap2Bytes(out->ipw_now);
    out->iprt_gen = Swap2Bytes(out->iprt_gen);
    out->iprt_des = Swap2Bytes(out->iprt_des);
    out->iprt_start = Swap2Bytes(out->iprt_start);
    out->iprt_end = Swap2Bytes(out->iprt_end);
    out->iflags = Swap2Bytes(out->iflags);
    out->iz_slope = Swap2BytesSigned( (UINT2) out->iz_slope);
    out->izns_thr = Swap2BytesSigned( (UINT2) out->izns_thr);
    out->iccr_thr = Swap2BytesSigned( (UINT2) out->iccr_thr);
    out->isqi_thr = Swap2Bytes(out->isqi_thr);
    out->isig_thr = Swap2BytesSigned( (UINT2) out->isig_thr);
    out->iz_calib = Swap2BytesSigned( (UINT2) out->iz_calib);
    out->iqi_now = Swap2Bytes(out->iqi_now);
    out->iz_now = Swap2Bytes(out->iz_now);
    out->ibin_avg = Swap2Bytes(out->ibin_avg);
    out->idiag_c = Swap2Bytes(out->idiag_c);
    out->idiag_d = Swap2Bytes(out->idiag_d);
    out->iproc_hdr0 = Swap2Bytes(out->iproc_hdr0);
    out->isq_lo = Swap2Bytes(out->isq_lo);
    out->isq_hi = Swap2BytesSigned( (UINT2) out->isq_hi);
    out->qsq_lo = Swap2Bytes(out->qsq_lo);
    out->qsq_hi = Swap2BytesSigned( (UINT2) out->qsq_hi);
    out->zlin_noise = Swap2BytesSigned( (UINT2) out->zlin_noise);
    out->zlin_rms = Swap2BytesSigned( (UINT2) out->zlin_rms);
    out->inse_hv_ratio = Swap2BytesSigned(
        (UINT2) out->inse_hv_ratio);
    out->iafclevel = Swap2BytesSigned( (UINT2) out->iafclevel);
    out->intflt = Swap2Bytes(out->intflt);
    out->intflt_p1 = Swap2BytesSigned( (UINT2) out->intflt_p1);
    out->intflt_p2 = Swap2BytesSigned( (UINT2) out->intflt_p2);
    out->istat_i3 = Swap2Bytes(out->istat_i3);
    out->itrigslew = Swap2BytesSigned( (UINT2) out->itrigslew);
    out->iPolFlags = Swap2Bytes(out->iPolFlags);
    out->iMaskSpacingCM = Swap2Bytes(out->iMaskSpacingCM);
    out->istat_i4 = Swap2Bytes(out->istat_i4);
    out->unused_word_60 = Swap2Bytes(out->unused_word_60);
    out->unused_word_61 = Swap2Bytes(out->unused_word_61);
    out->unused_word_62 = Swap2Bytes(out->unused_word_62);
    out->unused_word_63 = Swap2Bytes(out->unused_word_63);
    out->unused_word_64 = Swap2Bytes(out->unused_word_64);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_task_scan_info ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tscani_s *extract_task_scan_info(UINT1 *s1,
                                  _Bool target_is_big_endian) {
  tscani_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tscani_s' structure in"
          " function 'extract_task_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->antenna_scan_mode, s1, 2);
  memcpy( &out->angular_resolution_x1000, s1+2, 2);
  memcpy( &out->scan_speed, s1+4, 2);
  memcpy( &out->number_of_sweeps_to_perform, s1+6, 2);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->antenna_scan_mode = Swap2Bytes(out->antenna_scan_mode);
    out->angular_resolution_x1000 =
        Swap2BytesSigned( (UINT2)
            out->angular_resolution_x1000);
    out->scan_speed = Swap2Bytes(out->scan_speed);
    out->number_of_sweeps_to_perform =
        Swap2BytesSigned( (UINT2)
            out->number_of_sweeps_to_perform);
  }

  /*
   * note! Below we don't have anything to read for
   * 'exec' ==> antenna_scan_mode = 6 ;
   */
  if(out->antenna_scan_mode == 1 || out->antenna_scan_mode == 4) {
      /* PPI sector or PPI full */
      tpsi_s *tpsi_p = extract_task_ppi_scan_info(s1+8,
                                                  target_is_big_endian);
      if( tpsi_p != NULL) {
          out->u.ppi = *tpsi_p; // copy structure
          RAVE_FREE(tpsi_p);  // free allocated space of struct
      }
  }
  else if(out->antenna_scan_mode == 2 || out->antenna_scan_mode == 7) {
      /* RHI sector or RHI full */
      trsi_s *trsi_p = extract_task_rhi_scan_info(s1+8,
                                                  target_is_big_endian);
      if( trsi_p != NULL) {
          out->u.rhi = *trsi_p; // copy structure
          RAVE_FREE(trsi_p);  // free allocated space of struct
      }
  }
  else if(out->antenna_scan_mode == 3 ) {
      /* PPI manual or RHI manual */
      tmsi_s *tmsi_p = extract_task_manual_scan_info(s1+8,
                                                     target_is_big_endian);
      if( tmsi_p != NULL) {
          out->u.man = *tmsi_p;  // copy structure
          RAVE_FREE(tmsi_p);  // free allocated space of struct
       }
  }
  else if(out->antenna_scan_mode == 5 ) {
      /* File scan */
      tfsi_s *tfsi_p = extract_task_file_scan_info(s1+8,
                                                   target_is_big_endian);
      if( tfsi_p != NULL) {
          out->u.fil = *tfsi_p; // copy structure
          RAVE_FREE(tfsi_p);  // free allocated space of struct
      }
  }
  else if(out->antenna_scan_mode == 6 ) {
      tesi_s *tesi_p = extract_task_exec_scan_info(s1+8);
      if( tesi_p != NULL) {
          out->u.exec = *tesi_p; // copy structure
          RAVE_FREE(tesi_p);  // free allocated space of struct
      }
  }
  memcpy( &out->one_hundred_twelve_byte_spare, s1+208 ,112);
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_task_sched_info -----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_sched_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tschedi_s *extract_task_sched_info(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  tschedi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tschedi_s' structure in"
          " function 'extract_task_sched_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->start_time_seconds_within_day, s1, 4);
  memcpy( &out->stop_time_seconds_within_day, s1+4, 4);
  memcpy( &out->desired_skip_time_in_seconds, s1+8, 4);
  memcpy( &out->last_run_seconds_within_day, s1+12, 4);
  memcpy( &out->time_used_on_last_run_seconds, s1+16, 4);
  memcpy( &out->relative_day_of_last_run, s1+20, 4);
  memcpy( &out->iflag, s1+24, 2);
  memcpy( &out->ninety_four_bytes_spare, s1+26, 94);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
      out->start_time_seconds_within_day =
          Swap4BytesSigned( (UINT4)
              out->start_time_seconds_within_day);
      out->stop_time_seconds_within_day =
          Swap4BytesSigned( (UINT4)
              out->stop_time_seconds_within_day);
      out->desired_skip_time_in_seconds =
          Swap4BytesSigned( (UINT4)
              out->desired_skip_time_in_seconds);
      out->last_run_seconds_within_day =
          Swap4BytesSigned( (UINT4)
              out->last_run_seconds_within_day);
      out->time_used_on_last_run_seconds =
          Swap4BytesSigned( (UINT4)
              out->time_used_on_last_run_seconds);
      out->relative_day_of_last_run =
          Swap4BytesSigned( (UINT4)
              out->relative_day_of_last_run);
      out->iflag = Swap2Bytes(out->iflag);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------- extract_task_dsp_info ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_dsp_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tdi_s *extract_task_dsp_info(UINT1 *s1,
                              _Bool target_is_big_endian) {
  tdi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tdi_s' structure in"
          " function 'extract_task_dsp_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->dsp_major_mode, s1, 2);
  memcpy( &out->dsp_type, s1+2, 2);
  dsp_data_mask *mask1_p =
      extract_dsp_data_mask(s1+4, target_is_big_endian);
  if( mask1_p != NULL) {
      out->DataMask = *mask1_p; // copy structure
      RAVE_FREE(mask1_p); // free allocated space of struct
  }
  dsp_data_mask *mask2_p =
      extract_dsp_data_mask(s1+28, target_is_big_endian);
  if( mask2_p != NULL) {
      out->OriginalDataMask = *mask2_p; // copy structure
      RAVE_FREE(mask2_p); // free allocated space of struct
  }
  memcpy( &out->u, s1+52, TASK_DSP_MODE_SIZE); // copy union bytes 'as-is'
  memcpy( &out->fifty_two_spare_bytes, s1+84, 52);
  memcpy( &out->prf_in_hertz, s1+136, 4);
  memcpy( &out->pulse_width_in_hundredths_of_microseconds, s1+140, 4);
  memcpy( &out->multi_PRF_mode_flag, s1+144, 2);
  memcpy( &out->dual_PRF_delay, s1+146, 2);
  memcpy( &out->agc_feedback_code, s1+148, 2);
  memcpy( &out->sample_size, s1+150, 2);
  memcpy( &out->gain_control_flag, s1+152, 2);
  memcpy( &out->name_of_file_used_for_clutter_filter, s1+154, 12);
  memcpy( &out->clutter_filter_index, s1+166, 1);
  memcpy( &out->log_filter_first_bin, s1+167, 1);
  memcpy( &out->fixed_gain, s1+168, 2);
  memcpy( &out->gas_attenuation, s1+170, 2);
  memcpy( &out->flag_nonzero_if_clutter_map_used, s1+172, 2);
  memcpy( &out->xmt_phase_sequence, s1+174, 2);
  memcpy( &out->CfgHdr_Mask, s1+176, 4);
  memcpy( &out->flags_time_series_playback, s1+180, 2);
  memcpy( &out->two_spare_bytes, s1+182, 2);
  memcpy( &out->name_of_custom_ray_header, s1+184, 16);
  ecv_s *ecv_p;
  int m;
  for( m = 0 ; m < 6 ; m++ ) {
    ecv_p = extract_enum_convert(s1+200+m*4);
    if( ecv_p != NULL) {
        out->enums[m] = *ecv_p; // copy structure
        RAVE_FREE(ecv_p); // free allocated space of struct
    }
  }
  memcpy( &out->ninety_six_spare_bytes, s1+224, 96);

  /* swap some bytes if IRIS input file is bi-endian */
  if( target_is_big_endian ) {
    out->dsp_major_mode = Swap2Bytes(out->dsp_major_mode);
    out->dsp_type = Swap2Bytes(out->dsp_type);
    for(int i=0; i<16; i++)
      out->u.other.imisc[i] = Swap2BytesSigned( (UINT2)
          out->u.other.imisc[i]);
    out->prf_in_hertz = Swap4BytesSigned( (UINT4)
        out->prf_in_hertz);
    out->pulse_width_in_hundredths_of_microseconds =
        Swap4BytesSigned( (UINT4)
            out->pulse_width_in_hundredths_of_microseconds);
    out->multi_PRF_mode_flag = Swap2Bytes(out->multi_PRF_mode_flag);
    out->dual_PRF_delay = Swap2BytesSigned( (UINT2)
        out->dual_PRF_delay);
    out->agc_feedback_code = Swap2Bytes(out->agc_feedback_code);
    out->sample_size = Swap2BytesSigned( (UINT2)
        out->sample_size);
    out->gain_control_flag = Swap2Bytes(out->gain_control_flag);
    out->fixed_gain = Swap2BytesSigned( (UINT2)
        out->fixed_gain);
    out->gas_attenuation = Swap2Bytes(out->gas_attenuation);
    out->flag_nonzero_if_clutter_map_used =
        Swap2Bytes(out->flag_nonzero_if_clutter_map_used);
    out->xmt_phase_sequence = Swap2Bytes(out->xmt_phase_sequence);
    out->CfgHdr_Mask = Swap4Bytes(out->CfgHdr_Mask);
    out->flags_time_series_playback =
        Swap2Bytes(out->flags_time_series_playback);

  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_task_calib_info -----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_calib_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tci_s *extract_task_calib_info(UINT1 *s1,
                                _Bool target_is_big_endian) {
  tci_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tci_s' structure in"
          " function 'extract_task_calib_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->reflectivity_slope, s1, 2);
  /* the following are actual thresholds used */
  memcpy( &out->reflectivity_noise_threshold, s1+2, 2);
  memcpy( &out->clutter_correction_threshold, s1+4, 2);
  memcpy( &out->SQI_threshold, s1+6, 2);
  memcpy( &out->signal_power_thresholdr, s1+8, 2);
  memcpy( &out->pmi_threshold, s1+10, 2);
  memcpy( &out->six_bytes_spare, s1+12, 6);
  memcpy( &out->calibration_reflectivity, s1+18, 2);
  /* the following are threshold control flags for specific moment variables */
  memcpy( &out->flags_for_uncorrected_reflectivity, s1+20, 2);
  memcpy( &out->flags_for_corrected_reflectivity, s1+22, 2);
  memcpy( &out->flags_for_velocity, s1+24, 2);
  memcpy( &out->flags_for_width, s1+26, 2);
  memcpy( &out->flags_for_zdr, s1+28, 2);
  /* this next item is commented out in the iris_task.h file
   * and the number of spare bytes that followed has been incresed by 2
      memcpy( &out->flags_for_polarimetric_phase, s1+30, 2);
  */
  memcpy( &out->six_spare_bytes_2, s1+30, 6);
  /* miscellaneous processing flags */
  memcpy( &out->flags, s1+36, 2);
  memcpy( &out->two_spare_bytes, s1+38, 2);
  memcpy( &out->ldr_bias_in_dBx100, s1+40, 2);
  memcpy( &out->zdr_bias_in_dBx16, s1+42, 2);
  memcpy( &out->point_clutter_threshold_in_dBx100, s1+44, 2);
  memcpy( &out->point_clutter_bin_skip, s1+46, 2);
  /* should only use the lowest 4 bits of 16 bits in NEXRAD_point_clutter_bin_skip */
  memcpy( &out->I0_cal_value_Horiz_in_hundredths_of_dB, s1+48, 2);
  memcpy( &out->I0_cal_value_Vert_in_hundredths_of_dB, s1+50, 2);
  memcpy( &out->noise_at_calibration_Horiz_in_hundredths_of_dBm, s1+52, 2);
  memcpy( &out->noise_at_calibration_Vert_in_hundredths_of_dBm, s1+54, 2);
  memcpy( &out->radar_constant_Horiz_in_hundredths_of_dB, s1+56, 2);
  memcpy( &out->radar_constant_Vert_in_hundredths_of_dB, s1+58, 2);
  memcpy( &out->receiver_bandwidth_in_kHz, s1+60, 2);
  /* more miscellaneous processing flags */

  memcpy( &out->flags2, s1+62, 2);
  memcpy( &out->uncorrected_reflectivity_tcfMask, s1+64, 2);
  memcpy( &out->corrected_reflectivity_tcfMask, s1+66, 2);
  memcpy( &out->velocity_tcfMask, s1+68, 2);
  memcpy( &out->width_tcfMask, s1+70, 2);
  memcpy( &out->zdr_tcfMask, s1+72, 2);
  /* this next item is commented out in the iris_task.h file
   * and the number of spare bytes that followed has been incresed by 2
      memcpy( &out->polarimetric_phas_tcfMask, s1+74, 2);
  */
  memcpy( &out->two_hundred_fourty_six_spare_bytes, s1+74, 246);

  /* swap some bytes if the IRIS input file is big-endian */
  if ( target_is_big_endian ) {
    out->reflectivity_slope = Swap2BytesSigned( (UINT2)
        out->reflectivity_slope);
    out->reflectivity_noise_threshold =
        Swap2BytesSigned( (UINT2)
            out->reflectivity_noise_threshold);
    out->clutter_correction_threshold =
        Swap2BytesSigned( (UINT2)
            out->clutter_correction_threshold);
    out->SQI_threshold =  Swap2BytesSigned( (UINT2)
        out->SQI_threshold);
    out->signal_power_thresholdr =
        Swap2BytesSigned( (UINT2)
            out->signal_power_thresholdr);
    out->pmi_threshold = Swap2BytesSigned( (UINT2)
        out->pmi_threshold);
    out->calibration_reflectivity =
        Swap2BytesSigned( (UINT2)
            out->calibration_reflectivity);
    out->flags_for_uncorrected_reflectivity =
        Swap2Bytes(out->flags_for_uncorrected_reflectivity);
    out->flags_for_corrected_reflectivity =
        Swap2Bytes(out->flags_for_corrected_reflectivity);
    out->flags_for_velocity =
        Swap2Bytes(out->flags_for_velocity);
    out->flags_for_width =
        Swap2Bytes(out->flags_for_width);
    out->flags_for_zdr = Swap2Bytes(out->flags_for_zdr);
    out->flags = Swap2Bytes(out->flags);
    out->ldr_bias_in_dBx100 = Swap2BytesSigned( (UINT2)
        out->ldr_bias_in_dBx100);
    out->zdr_bias_in_dBx16 = Swap2BytesSigned( (UINT2)
        out->zdr_bias_in_dBx16);
    out->point_clutter_threshold_in_dBx100 =
        Swap2BytesSigned( (UINT2)
            out->point_clutter_threshold_in_dBx100);
    out->point_clutter_bin_skip =
        Swap2Bytes(out->point_clutter_bin_skip);

    out->I0_cal_value_Horiz_in_hundredths_of_dB =
        Swap2BytesSigned( (UINT2)
            out->I0_cal_value_Horiz_in_hundredths_of_dB);
    out->I0_cal_value_Vert_in_hundredths_of_dB =
        Swap2BytesSigned( (UINT2)
            out->I0_cal_value_Vert_in_hundredths_of_dB);
    out->noise_at_calibration_Horiz_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->noise_at_calibration_Horiz_in_hundredths_of_dBm);
    out->noise_at_calibration_Vert_in_hundredths_of_dBm =
        Swap2BytesSigned( (UINT2)
            out->noise_at_calibration_Vert_in_hundredths_of_dBm);
    out->radar_constant_Horiz_in_hundredths_of_dB =
        Swap2BytesSigned( (UINT2)
            out->radar_constant_Horiz_in_hundredths_of_dB);
    out->radar_constant_Vert_in_hundredths_of_dB =
        Swap2BytesSigned( (UINT2)
            out->radar_constant_Vert_in_hundredths_of_dB);
    out->receiver_bandwidth_in_kHz =
        Swap2Bytes(out->receiver_bandwidth_in_kHz);
    out->flags2 = Swap2Bytes(out->flags2);
    out->uncorrected_reflectivity_tcfMask =
        Swap2Bytes(out->uncorrected_reflectivity_tcfMask);
    out->corrected_reflectivity_tcfMask =
        Swap2Bytes(out->corrected_reflectivity_tcfMask);
    out->velocity_tcfMask = Swap2Bytes(out->velocity_tcfMask);
    out->width_tcfMask = Swap2Bytes(out->width_tcfMask);
    out->zdr_tcfMask = Swap2Bytes(out->zdr_tcfMask);
  }
  /* should only use the lowest 4 bits of 16 bits in
   * point_clutter_bin_skip and now the byte order should
   * be little-endian, so get the first nibble         */
  UINT2 lowNybble=0, binSkip=0;
  binSkip=out->point_clutter_bin_skip;
  /* bitwise AND of binSkip and 0000 0000 0000 1111 to get the low nybble */
  lowNybble = binSkip & 15;
  out->point_clutter_bin_skip = lowNybble;
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_task_misc_info ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_misc_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tmi_s *extract_task_misc_info(UINT1 *s1,
                               _Bool target_is_big_endian) {
  tmi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tmi_s' structure in"
          " function 'extract_task_misc_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->radar_wavelength_in_cm_x100, s1, 4);
  memcpy( &out->serial_number_of_transmitter, s1+4, 16);
  memcpy( &out->transmit_power_in_watts, s1+20, 4);
  memcpy( &out->iflags, s1+24, 2);
  memcpy( &out->type_of_polarization, s1+26, 2);
  memcpy( &out->truncation_height_in_cm_above_radar, s1+28, 4);
  memcpy( &out->eighteen_bytes_reserved, s1+32, 18);
  memcpy( &out->twelve_bytes_spare, s1+50, 12);
  memcpy( &out->number_of_bytes_of_comments_entered, s1+62, 2);
  memcpy( &out->horizontal_beam_width, s1+64, 4);
  memcpy( &out->vertical_beam_width, s1+68, 4);
  for(int k = 0; k < 10 ; k++) {
      memcpy( &out->iUser[k], s1+72+k*4, 4);
  }
  memcpy( &out->two_hundred_eight_bytes_spare, s1+112, 208);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->radar_wavelength_in_cm_x100 =
        Swap4BytesSigned( (UINT4)
            out->radar_wavelength_in_cm_x100);
    out->transmit_power_in_watts =
        Swap4BytesSigned( (UINT4)
            out->transmit_power_in_watts);
    out->iflags = Swap2Bytes(out->iflags);
    out->type_of_polarization = Swap2Bytes(out->type_of_polarization);
    out->truncation_height_in_cm_above_radar =
        Swap4BytesSigned( (UINT4)
            out->truncation_height_in_cm_above_radar);
    out->number_of_bytes_of_comments_entered =
        Swap2BytesSigned( (UINT2)
            out->number_of_bytes_of_comments_entered);
    out->horizontal_beam_width =
        Swap4Bytes(out->horizontal_beam_width);
    out->vertical_beam_width = Swap4Bytes(out->vertical_beam_width);
    for(int k = 0; k < 10 ; k++)
      out->iUser[k] = Swap4Bytes(out->iUser[k]);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ extract_task_range_info -----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_range_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tri_s *extract_task_range_info(UINT1 *s1,
                                _Bool target_is_big_endian) {
  tri_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tri_s' structure in"
          " function 'extract_task_range_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->range_of_first_bin_in_cm, s1, 4);
  memcpy( &out->range_of_last_bin_in_cm, s1+4, 4);
  memcpy( &out->number_of_input_range_bins, s1+8, 2);
  memcpy( &out->number_of_output_range_bins, s1+10, 2);
  memcpy( &out->step_between_input_bins_in_cm, s1+12, 4);
  memcpy( &out->step_between_output_bins_in_cm, s1+16, 4);
  memcpy( &out->variation_of_range_bin_spacing_flag, s1+20, 2);
  memcpy( &out->averaging_of_range_bin_spacing, s1+22, 2);
  memcpy( &out->smoothing_of_range_bin_spacing, s1+24, 2);
  memcpy( &out->one_hundred_thirty_four_bytes_spare, s1+22, 134);

  /* Swap some bytes if the IRIS input file is bi-endian */
  if( target_is_big_endian ) {
    out->range_of_first_bin_in_cm =
        Swap4BytesSigned( (UINT4)
            out->range_of_first_bin_in_cm);
    out->range_of_last_bin_in_cm =
        Swap4BytesSigned( (UINT4)
            out->range_of_last_bin_in_cm);
    out->number_of_input_range_bins =
        Swap2BytesSigned( (UINT2)
            out->number_of_input_range_bins);
    out->number_of_output_range_bins =
        Swap2BytesSigned( (UINT2)
            out->number_of_output_range_bins);
    out->step_between_input_bins_in_cm =
        Swap4BytesSigned( (UINT4)
            out->step_between_input_bins_in_cm);
    out->step_between_output_bins_in_cm =
        Swap4BytesSigned( (UINT4)
            out->step_between_output_bins_in_cm);
    out->variation_of_range_bin_spacing_flag =
        Swap2Bytes(out->variation_of_range_bin_spacing_flag);
    out->averaging_of_range_bin_spacing =
        Swap2BytesSigned( (UINT2)
            out->averaging_of_range_bin_spacing);
    out->smoothing_of_range_bin_spacing =
        Swap2BytesSigned( (UINT2) out->smoothing_of_range_bin_spacing);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------- extract_task_end_info ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_end_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr  */
/* ======================================================================== */
tei_s *extract_task_end_info(UINT1 *s1,
                              _Bool target_is_big_endian) {
  ymd_s *ymd_p = NULL;
  tei_s *out = NULL;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tei_s' structure in"
          " function 'extract_task_end_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->task_major_number, s1, 2);
  memcpy( &out->task_minor_number, s1+2, 2);
  memcpy( &out->name_of_task_configuration_file, s1+4, 12);
  memcpy( &out->eighty_byte_task_description, s1+16, 80);
  memcpy( &out->number_of_tasks_in_this_hybrid_set, s1+96, 4);
  memcpy( &out->task_state, s1+100, 2);
  memcpy( &out->two_spare_bytes, s1+102,2);
  ymd_p = extract_ymds_time(s1+104, target_is_big_endian);
  if( ymd_p != NULL) {
      out->task_time = *ymd_p; // copy structure
      RAVE_FREE(ymd_p);  // free allocated space of struct
  }
  memcpy( &out->two_hundred_four_bytes_spare,s1+116, 204);

  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->task_major_number = Swap2BytesSigned( (UINT2)
        out->task_major_number);
    out->task_minor_number = Swap2BytesSigned( (UINT2)
        out->task_minor_number);
    out->number_of_tasks_in_this_hybrid_set =
        Swap4BytesSigned( (UINT4)
            out->number_of_tasks_in_this_hybrid_set);
    out->task_state = Swap2Bytes(out->task_state);
  }

  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_task_ppi_scan_info ----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_ppi_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tpsi_s *extract_task_ppi_scan_info(UINT1 *s1,
                                    _Bool target_is_big_endian) {
  tpsi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tcpsi_s' structure in"
          " function 'extract_task_ppi_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->left_azimuthal_angle_limit, s1, 2);
  memcpy( &out->right_azimuthal_angle_limit, s1+2, 2);
  for(int l = 0; l < MAX_SWEEPS ; l++ ) {
      memcpy( &out->list_of_elevation_angles_to_scan[l], s1+4+l*2, 2);
  }
  memcpy( &out->one_hundred_fifteen_bytes_spare, s1+84, 115);
  memcpy( &out->iStartEnd, s1+199, 1);

  /* swap some bytes if the target is big-endian */
  if( target_is_big_endian ) {
    out->left_azimuthal_angle_limit =
        Swap2Bytes(out->left_azimuthal_angle_limit);
    out->right_azimuthal_angle_limit =
        Swap2Bytes(out->right_azimuthal_angle_limit);
    for(int l = 0; l < MAX_SWEEPS ; l++ )
        out->list_of_elevation_angles_to_scan[l] =
            Swap2Bytes(out->list_of_elevation_angles_to_scan[l]);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_task_rhi_scan_info ----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_rhi_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
trsi_s *extract_task_rhi_scan_info(UINT1 *s1,
                                    _Bool target_is_big_endian) {
  trsi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'trsi_s' structure in"
          " function 'extract_task_rhi_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->Lower_elevation_angle_limit, s1, 2);
  memcpy( &out->Upper_elevation_angle_limit, s1+2, 2);
  for(int l = 0; l < MAX_SWEEPS ; l++ ) {
      memcpy( &out->list_of_azimuth_angles_to_scan[l], s1+4+l*2, 2);
  }
  memcpy( &out->one_hundred_fifteen_bytes_spare, s1+84, 115);
  memcpy( &out->iStartEnd, s1+199, 1);

  /* swap some bytes if the target is big-endian */
  if( target_is_big_endian ) {
    out->Lower_elevation_angle_limit =
        Swap2Bytes(out->Lower_elevation_angle_limit);
    out->Upper_elevation_angle_limit =
        Swap2Bytes(out->Upper_elevation_angle_limit);
    for(int l = 0; l < MAX_SWEEPS ; l++ )
        out->list_of_azimuth_angles_to_scan[l] =
            Swap2Bytes(out->list_of_azimuth_angles_to_scan[l]);
  }

  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------- extract_task_manual_scan_info --------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_manual_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tmsi_s *extract_task_manual_scan_info(UINT1 *s1,
                                       _Bool target_is_big_endian) {
  tmsi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tmsi_s' structure in"
          " function 'extract_task_manual_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 2);
  memcpy( &out->two_spare_bytes, s1+2, 2);
  memcpy( &out->first_azimuth_angle, s1+4, 4);
  memcpy( &out->first_elevation_angle, s1+8, 4);
  memcpy( &out->ipad_end, s1+12, 188);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->flags = Swap2Bytes(out->flags);
    out->first_azimuth_angle = Swap4Bytes(out->first_azimuth_angle);
    out->first_elevation_angle = Swap4Bytes(out->first_elevation_angle);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_task_file_scan_info ---------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_file_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tfsi_s *extract_task_file_scan_info(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  tfsi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tfsi_s' structure in"
          " function 'extract_task_file_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->first_azimuth_angle, s1, 2);
  memcpy( &out->first_elevation_angle, s1+2, 2);
  memcpy( &out->file_name_for_antenna_control, s1+4, 12);
  memcpy( &out->one_hundred_eighty_four_bytes_spare, s1+16,184);
  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
      out->first_azimuth_angle =
          Swap2Bytes(out->first_azimuth_angle);
      out->first_elevation_angle =
          Swap2Bytes(out->first_elevation_angle);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_task_exec_scan_info ---------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a task_exec_scan_info structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
tesi_s *extract_task_exec_scan_info(UINT1 *s1) {
  tesi_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tesi_s' structure in"
          " function 'extract_task_exec_scan_info'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->sCommand, s1, 160);
  memcpy( &out->fourty_bytes_spare, s1+160, 40);
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------- extract_dsp_data_mask ------------------------  *
*                                                                            *
*****************************************************************************/
/* ======================================================================== */
/* A function to copy data from an IRIS raw product
 * record to a dsp_data_mask structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
dsp_data_mask *extract_dsp_data_mask(UINT1 *s1,
                                      _Bool target_is_big_endian) {
  dsp_data_mask *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'dsp_data_mask' structure in"
          " function 'extract_dsp_data_mask'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->dWord0, s1, 4);
  memcpy( &out->iXhdrType, s1+4, 4);
  memcpy( &out->dWord1, s1+8, 4);
  memcpy( &out->dWord2, s1+12, 4);
  memcpy( &out->dWord3, s1+16, 4);
  memcpy( &out->dWord4, s1+20, 4);
  /* swap some bytes if IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->dWord0 = Swap4Bytes(out->dWord0);
    out->iXhdrType = Swap4Bytes(out->iXhdrType);
    out->dWord1 = Swap4Bytes(out->dWord1);
    out->dWord2 = Swap4Bytes(out->dWord2);
    out->dWord3 = Swap4Bytes(out->dWord3);
    out->dWord4 = Swap4Bytes(out->dWord4);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------------- getabuf -----------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to transfer bytes from a previously
 * opened input file to a byte array. The byte array
 * is completely filled unless we hit end of file.
 * The byte array itself and the number of bytes
 * transferred are members of a structure that is
 * returned.  This function returns a pointer to the
 * structure and that pointer is NULL if we are unable
 * to allocate the structure  */
/* ======================================================================== */
IRISbuf *getabuf(FILE *fp, UINT2 bytes2Copy ) {
   /* allocate an 'IRISbuf' structure and call it 'out' */
   IRISbuf *out;
   out = RAVE_MALLOC( sizeof *out );
   if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'IRISbuf' structure in"
          " function 'getabuf'.\n");
      return NULL;
   }
   out->bytesCopied = 0;
   out->errorInd = 0;
   out->numberSkipped = 0;
   /* fill the output structure */
   int b1;
   for(out->bytesCopied = 0; out->bytesCopied < bytes2Copy;) {
      if( ( b1 = fgetc(fp) ) == EOF ) { /* Was there an error on reading? */
         if( feof(fp) ) {
            out->errorInd = 2;
            clearerr(fp);  /* clear file's error or EOF flag */
            return out;
         }
         if( ferror(fp) ) {
            /* tell the client that there was an error reading before eof */
            Iris_printf("Error while reading input file.\n");
            out->errorInd = 1;
            return out;
         }
      }
      /* b1 is an int, so cast it as a signed 1-byte integer */
      out->bufIRIS[out->bytesCopied] = (UINT1) b1;
      out->bytesCopied++;
   }
   return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_ymds_time --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a ymds_time structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
ymd_s *extract_ymds_time(UINT1 *s1,
                          _Bool target_is_big_endian) {
  ymd_s *out = NULL;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'ymd' structure in"
          " function 'extract_ymds_time'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->seconds_since_midnight, s1, 4);
  memcpy( &out->milliseconds_and_UTC_DST_indication, s1+4, 2);
  memcpy( &out->year, s1+6, 2);
  memcpy( &out->month, s1+8, 2);
  memcpy( &out->day, s1+10, 2);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->seconds_since_midnight =
          Swap4Bytes(out->seconds_since_midnight);
      out->milliseconds_and_UTC_DST_indication =
          Swap2Bytes(out->milliseconds_and_UTC_DST_indication);
      out->year = Swap2Bytes(out->year);
      out->month = Swap2Bytes(out->month);
      out->day = Swap2Bytes(out->day);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ Swap2BytesSigned ------------------------------  *
*                                                                            *
*****************************************************************************/
/* Use this function to swap bytes of signed short integers, and use the macro
 * 'Swap2Bytes' for swapping the bytes of unsigned short integers           */
SINT2 Swap2BytesSigned( UINT2 shortIn) {
  UINT2 u2short = Swap2Bytes(shortIn);
  SINT2 s2Short = (SINT2) u2short;
  return s2Short;
}

/*****************************************************************************
*                                                                            *
*  ------------------------ Swap4BytesSigned ------------------------------  *
*                                                                            *
*****************************************************************************/
/* Use this function to swap bytes of signed 4-byte integers, and use the macro
 * 'Swap4Bytes' for swapping the bytes of unsigned 4-byte integers           */
SINT4 Swap4BytesSigned( UINT4 intIn) {
  UINT4 u4int = Swap4Bytes(intIn);
  SINT4 s4int = (SINT4) u4int;
  return s4int;
}

/*****************************************************************************
*                                                                            *
*  -------------------------- extract_enum_convert ------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a enum_convert structure and return
 * a pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error message is written to stderr */
/* ======================================================================== */
ecv_s *extract_enum_convert(UINT1 *s1) {
  ecv_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'ecv_s' structure in"
          " function 'extract_enum_convert'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->id_of_active_echo_classifier, s1, 1);
  memcpy( &out->bit_offset_of_the_enum_segment, s1+1, 1);
  memcpy( &out->length_of_the_enum_segment_in_bits, s1+2, 1);
  return out;
}

/* ================================================== */
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * ray_header type structure. The function is
 * passed a pointer to location inside a byte array.
 * If an allocation failure occurs, a pointer to null
 * is returned and an error message is printed        */
rhd_s *extract_ray_header(UINT1 *ptr_s0) {
  /*
   * allocate an 'ingest_data_header' structure and
   * call it 'out'
   */
  rhd_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
    Iris_printf(
            "Error! Unable to allocate 'rhd_s' structure in"
            " function 'extract_ray_header'.\n");
    return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->azimuth_angle_at_beginning_of_ray, ptr_s0, 2);
  memcpy( &out->elevation_angle_at_beginning_of_ray, ptr_s0+2, 2);
  memcpy( &out->azimuth_angle_at_end_of_ray, ptr_s0+4, 2);
  memcpy( &out->elevation_angle_at_end_of_ray, ptr_s0+6, 2);
  memcpy( &out->actual_number_of_bins_in_ray, ptr_s0+8, 2);
  memcpy( &out->time_in_seconds_from_start_of_sweep, ptr_s0+10, 2);
  
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_ppi ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a ppi_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
ppi_psi_struct *extract_psi_ppi(UINT1 *s1,
                                _Bool target_is_big_endian) {
  ppi_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'ppi_psi_struct' structure in"
          " function 'extract_psi_ppi'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->elevation_angle, s1, 2);
  memcpy( &out->two_spare_bytes, s1+2, 2);
  memcpy( &out->max_range_in_cm, s1+4, 4);
  memcpy( &out->max_height_above_ref_in_cm, s1+8, 4);
  memcpy( &out->ipad_end, s1+12, PSI_SIZE-12);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->elevation_angle =
          Swap2Bytes(out->elevation_angle);
      out->max_range_in_cm =
          Swap4BytesSigned( (UINT4) out->max_range_in_cm);
      out->max_height_above_ref_in_cm =
          Swap4BytesSigned( (UINT4) out->max_height_above_ref_in_cm);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_rhi ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a rhi_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
rhi_psi_struct *extract_psi_rhi(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  rhi_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'rhi_psi_struct' structure in"
          " function 'extract_psi_rhi'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->azimuth_angle, s1, 2);

  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->azimuth_angle = Swap2Bytes(out->azimuth_angle);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_cappi --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a cappi_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
cappi_psi_struct *extract_psi_cappi(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  cappi_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'cappi_psi_struct' structure in"
          " function 'extract_psi_cappi'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->shear_flags, s1, 4);
  memcpy( &out->cappi_height_cm_above_ref, s1+4, 4);
  memcpy( &out->flags, s1+8, 2);
  memcpy( &out->azimuth_smoothing_for_shear, s1+10, 2);
  memcpy( &out->shear_correction_name, s1+12, 12);
  memcpy( &out->max_age_of_shear_correction_in_seconds, s1+24, 4);
  memcpy( &out->ipad_end, s1+28, PSI_SIZE-28);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->shear_flags = Swap4Bytes(out->shear_flags);
    out->cappi_height_cm_above_ref =
        Swap4BytesSigned( (SINT4) out->cappi_height_cm_above_ref);
    out->flags = Swap2Bytes(out->flags);
    out->azimuth_smoothing_for_shear =
        Swap2Bytes(out->azimuth_smoothing_for_shear);
    out->max_age_of_shear_correction_in_seconds =
        Swap4Bytes(out->max_age_of_shear_correction_in_seconds);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_cross --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a cross_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
cross_psi_struct *extract_psi_cross(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  cross_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'cross_psi_struct' structure in"
          " function 'extract_psi_cross'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->azimuth_angle_of_left2right_line_on_picture, s1, 2);
  memcpy( &out->flags, s1+2, 2);
  memcpy( &out->eight_spare_bytes, s1+4, 8);
  memcpy( &out->east_coord_of_center_in_cm, s1+12, 4);
  memcpy( &out->north_coord_of_center_in_cm, s1+16, 4);
  memcpy( &out->name_of_data_cube_file, s1+20, 12);
  memcpy( &out->user_miscellaneous, s1+32, PSI_SIZE-32);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->azimuth_angle_of_left2right_line_on_picture =
        Swap2Bytes(out->azimuth_angle_of_left2right_line_on_picture);
    out->flags = Swap2Bytes(out->flags);
    out->east_coord_of_center_in_cm =
        Swap4BytesSigned( (UINT4) out->east_coord_of_center_in_cm);
    out->north_coord_of_center_in_cm =
        Swap4BytesSigned( (UINT4) out->north_coord_of_center_in_cm);
    for(UINT2 j=0; j < (PSI_SIZE-32)/4; j++)
      out->user_miscellaneous[j] =
        Swap4BytesSigned( (UINT4) out->user_miscellaneous[j]);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_tops ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a top_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
top_psi_struct *extract_psi_tops(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  top_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'top_psi_struct' structure in"
          " function 'extract_psi_tops'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 4);
  memcpy( &out->z_threshold_in_sixteenths_of_dBz, s1+4, 2);

  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
      out->flags = Swap4Bytes(out->flags);
      out->z_threshold_in_sixteenths_of_dBz =
          Swap2Bytes(out->z_threshold_in_sixteenths_of_dBz);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  -------------------------- extract_psi_track ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a track_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
track_psi_struct *extract_psi_track(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  track_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'track_psi_struct' structure in"
          " function 'extract_psi_track'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->centroid_area_threshold_in_square_meters, s1, 4);
  memcpy( &out->threshold_level_for_centroid, s1+4, 4);
  memcpy( &out->protected_area_mask, s1+8, 4);
  memcpy( &out->maximum_forecast_time_in_seconds, s1+12, 4);
  memcpy( &out->maximum_age_between_products_for_motion_calc, s1+16, 4);
  memcpy( &out->maximum_motion_allowed_in_mm_per_second, s1+20, 4);
  memcpy( &out->flag_word, s1+24, 4);
  memcpy( &out->maximum_span_in_seconds_of_track_points_in_file, s1+28, 4);
  memcpy( &out->input_product_type, s1+32, 4);
  memcpy( &out->input_product_name, s1+36, 12);
  memcpy( &out->point_connecting_error_allowance, s1+48, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->centroid_area_threshold_in_square_meters =
        Swap4BytesSigned( (UINT4)
            out->centroid_area_threshold_in_square_meters);
    out->threshold_level_for_centroid =
        Swap4BytesSigned( (UINT4)
            out->threshold_level_for_centroid);
    out->protected_area_mask =
        Swap4Bytes(out->protected_area_mask);
    out->maximum_forecast_time_in_seconds =
        Swap4BytesSigned( (UINT4)
            out->maximum_forecast_time_in_seconds);
    out->maximum_age_between_products_for_motion_calc =
        Swap4Bytes(out->maximum_age_between_products_for_motion_calc);
    out->maximum_motion_allowed_in_mm_per_second =
        Swap4BytesSigned( (UINT4)
            out->maximum_motion_allowed_in_mm_per_second);
    out->flag_word = Swap4Bytes(out->flag_word);
    out->maximum_span_in_seconds_of_track_points_in_file =
        Swap4BytesSigned( (UINT4)
           out->maximum_span_in_seconds_of_track_points_in_file);
    out->input_product_type =
        Swap4Bytes(out->input_product_type);
    out->point_connecting_error_allowance =
        Swap4BytesSigned( (UINT4)
            out->point_connecting_error_allowance);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_rain ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a rain_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
rain_psi_struct *extract_psi_rain(UINT1 *s1,
                                  _Bool target_is_big_endian) {
  rain_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'rain_psi_struct' structure in"
          " function 'extract_psi_rain'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->minimum_Z_to_accumulate, s1, 4);
  memcpy( &out->average_gage_correction_factor, s1+4, 2);
  memcpy( &out->seconds_of_accumulation, s1+6, 2);
  memcpy( &out->flag_word, s1+8, 2);
  memcpy( &out->number_of_hours_to_accumulate, s1+10, 2);
  memcpy( &out->name_of_input_product_to_use, s1+12, 12);
  memcpy( &out->span_in_seconds_of_the_input_files, s1+24, 4);
  memcpy( &out->ipad_end,s1+28,PSI_SIZE-28);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->minimum_Z_to_accumulate =
        Swap4Bytes(out->minimum_Z_to_accumulate);
    out->average_gage_correction_factor =
        Swap2Bytes(out->average_gage_correction_factor);
    out->seconds_of_accumulation =
        Swap2Bytes(out->seconds_of_accumulation);
    out->flag_word = Swap2Bytes(out->flag_word);
    out->number_of_hours_to_accumulate =
        Swap2BytesSigned( (UINT2) out->number_of_hours_to_accumulate);
    out->span_in_seconds_of_the_input_files =
        Swap4Bytes(out->span_in_seconds_of_the_input_files);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_vvp ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a vvp_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
vvp_psi_struct *extract_psi_vvp(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  vvp_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'vvp_psi_struct' structure in"
          " function 'extract_psi_vvp'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->minimum_range_to_process_in_cm, s1, 4);
  memcpy( &out->maximum_range_to_process_in_cm, s1+4, 4);
  memcpy( &out->minimum_height_above_reference_to_process_in_cm, s1+8, 4);
  memcpy( &out->maximum_height_above_reference_to_process_in_cm, s1+14, 4);
  memcpy( &out->number_of_height_intervals_to_process, s1+16, 4);
  memcpy( &out->target_number_of_bins_per_interval, s1+20, 4);
  memcpy( &out->wind_parameters_to_compute, s1+24, 4);
  memcpy( &out->minimum_radial_velocity_in_cm_per_seconds, s1+28, 4);
  memcpy( &out->maximum_horizontal_velocity_error_to_accept, s1+32, 4);
  memcpy( &out->minimum_sample_size, s1+36, 4);
  memcpy( &out->minimum_horizontal_velocity_to_accept, s1+40, 4);
  memcpy( &out->maximum_horizontal_velocity_to_accept, s1+44, 4);
  memcpy( &out->maximum_mean_reflectivity_to_accept, s1+48, 4);
  memcpy( &out->maximum_vertical_velocity_to_accept, s1+52, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->minimum_range_to_process_in_cm =
        Swap4BytesSigned( (UINT4) out->minimum_range_to_process_in_cm);
    out->maximum_range_to_process_in_cm =
        Swap4BytesSigned( (UINT4) out->maximum_range_to_process_in_cm);
    out->minimum_height_above_reference_to_process_in_cm =
        Swap4BytesSigned( (UINT4)
            out->minimum_height_above_reference_to_process_in_cm);
    out->maximum_height_above_reference_to_process_in_cm =
        Swap4BytesSigned( (UINT4)
            out->maximum_height_above_reference_to_process_in_cm);
    out->number_of_height_intervals_to_process =
        Swap4BytesSigned( (UINT4)
            out->number_of_height_intervals_to_process);
    out->target_number_of_bins_per_interval =
        Swap4BytesSigned( (UINT4)
            out->target_number_of_bins_per_interval);
    out->wind_parameters_to_compute =
        Swap4Bytes(out->wind_parameters_to_compute);
    out->minimum_radial_velocity_in_cm_per_seconds =
        Swap4Bytes(out->minimum_radial_velocity_in_cm_per_seconds);
    out->maximum_horizontal_velocity_error_to_accept =
        Swap4Bytes(out->maximum_horizontal_velocity_error_to_accept);
    out->minimum_sample_size =
        Swap4Bytes(out->minimum_sample_size);
    out->minimum_horizontal_velocity_to_accept =
        Swap4Bytes(out->minimum_horizontal_velocity_to_accept);
    out->maximum_horizontal_velocity_to_accept =
        Swap4Bytes(out->maximum_horizontal_velocity_to_accept);
    out->maximum_mean_reflectivity_to_accept =
        Swap4Bytes(out->maximum_mean_reflectivity_to_accept);
    out->maximum_vertical_velocity_to_accept =
        Swap4Bytes(out->maximum_vertical_velocity_to_accept);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_vil ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a vil_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
vil_psi_struct *extract_psi_vil(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  vil_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'vil_psi_struct' structure in"
          " function 'extract_psi_vil'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->bottom_of_height_interval_in_cm, s1+4, 4);
  memcpy( &out->top_of_height_interval_in_cm, s1+8, 4);

  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->bottom_of_height_interval_in_cm =
        Swap4BytesSigned( (UINT4)
            out->bottom_of_height_interval_in_cm);
    out->top_of_height_interval_in_cm =
        Swap4BytesSigned( (UINT4)
            out->top_of_height_interval_in_cm);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_shear --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a shear_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
shear_psi_struct *extract_psi_shear(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  shear_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'shear_psi_struct' structure in"
          " function 'extract_psi_shear'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->azimuth_smoothing_angle, s1, 4);
  memcpy( &out->elevation_angle, s1+4, 2);
  memcpy( &out->flag_word, s1+6, 4);
  memcpy( &out->two_spare_bytes, s1+10, 2);
  memcpy( &out->name_of_VVP_product_to_use, s1+12, 12);
  memcpy( &out->maximum_age_of_VVP_to_use_in_secs, s1+24, 4);
  memcpy( &out->ipad_end, s1+28, PSI_SIZE-28);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->azimuth_smoothing_angle =
        Swap4Bytes(out->azimuth_smoothing_angle);
    out->elevation_angle = Swap2Bytes(out->elevation_angle);
    out->flag_word = Swap4Bytes(out->flag_word);
    out->maximum_age_of_VVP_to_use_in_secs =
        Swap4Bytes(out->maximum_age_of_VVP_to_use_in_secs);

  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_warn --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a warn_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
warn_psi_struct *extract_psi_warn(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  warn_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'warn_psi_struct' structure in"
          " function 'extract_psi_warn'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->centroid_area_threshold_in_square_meters, s1, 4);
  UINT1 *s2 = s1+4;
  for (int i = 0; i < WARN_MAX_INPUTS; i++)
    memcpy( &out->threshold_levels_in_hundredths[i], s2+i*4, 4);
  s2 = s1 + 4 + WARN_MAX_INPUTS*4;
  for (int ii = 0; ii < WARN_MAX_INPUTS; ii++)
    memcpy( &out->data_valid_times_in_seconds[ii], s2+ii*2, 2);
  s2 = s1 + 4 + WARN_MAX_INPUTS*6 + 2;
  memcpy( &out->symbol_to_display, s2, 12);
  s2 = s2 + 12;
  for (int j = 0; j < WARN_MAX_INPUTS; j++)
    memcpy( &out->names_of_product_files[j][0], s2+j*12, 12);
  s2 = s2 + WARN_MAX_INPUTS*12;
  for (int jj = 0; jj < WARN_MAX_INPUTS; jj++)
    memcpy( &out->product_types_used_as_input[jj], s2+jj, 1);
  s2 = s2 + WARN_MAX_INPUTS;
  memcpy( &out->control_flags, s2, 1);
  memcpy( &out->protected_area_bit_flags, s2+1, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->centroid_area_threshold_in_square_meters =
        Swap4BytesSigned( (UINT4)
            out->centroid_area_threshold_in_square_meters);
    for (int i = 0; i < WARN_MAX_INPUTS; i++)
      out->threshold_levels_in_hundredths[i] =
          Swap4BytesSigned( (UINT4)
              out->threshold_levels_in_hundredths[i]);
    for (int ii = 0; ii < WARN_MAX_INPUTS; ii++)
      out->data_valid_times_in_seconds[ii] =
          Swap2BytesSigned( (UINT4)
              out->data_valid_times_in_seconds[ii]);
    out->protected_area_bit_flags =
        Swap4Bytes(out->protected_area_bit_flags);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_catch --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a catch_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
catch_psi_struct *extract_psi_catch(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  catch_psi_struct *out;
  out = RAVE_MALLOC(sizeof *out);
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'catch_psi_struct' structure in"
          " function 'extract_psi_catch'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 4);
  memcpy( &out->hours_of_accumulation, s1+4, 4);
  memcpy( &out->threshold_offset_in_thousandths_or_mm, s1+8, 4);
  memcpy( &out->threshold_fraction_in_thousandths, s1+12, 4);
  memcpy( &out->name_of_RAIN1_product, s1+16, 12);
  memcpy( &out->name_of_catchment_file_to_use, s1+28, 16);
  memcpy( &out->seconds_of_accumulation, s1+44, 4);
  memcpy( &out->min_Z_RAIN1, s1+48, 4);
  memcpy( &out->span_in_seconds_RAIN1, s1+52, 4);
  memcpy( &out->ave_Gage_correction_factor, s1+56, 4);
  memcpy( &out->ipad_end, s1+60, PSI_SIZE-60);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->flags = Swap4Bytes(out->flags);
    out->hours_of_accumulation = Swap4Bytes(out->hours_of_accumulation);
    out->threshold_offset_in_thousandths_or_mm =
        Swap4BytesSigned( (UINT4)
            out->threshold_offset_in_thousandths_or_mm);
    out->seconds_of_accumulation =
        Swap4Bytes(out->seconds_of_accumulation);
    out->min_Z_RAIN1 = Swap4Bytes(out->min_Z_RAIN1);
    out->span_in_seconds_RAIN1 =
        Swap4Bytes(out->span_in_seconds_RAIN1);
    out->ave_Gage_correction_factor =
        Swap4Bytes(out->ave_Gage_correction_factor);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_rti ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a rti_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
rti_psi_struct *extract_psi_rti(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  rti_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'rti_psi_struct' structure in"
          " function 'extract_psi_rti'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->nominal_sweep_angle, s1, 4);
  memcpy( &out->starting_time_offset_from_sweep_time_in_ms, s1+4, 4);
  memcpy( &out->ending_time_offset, s1+8, 4);
  memcpy( &out->azimuth_angle_of_first_ray_in_file, s1+12, 4);
  memcpy( &out->elevation_angle_of_first_ray_in_file, s1+16, 4);
  memcpy( &out->ipad_end, s1+20, PSI_SIZE-20);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->nominal_sweep_angle =
        Swap4Bytes(out->nominal_sweep_angle);
    out->starting_time_offset_from_sweep_time_in_ms =
        Swap4Bytes(out->starting_time_offset_from_sweep_time_in_ms);
    out->ending_time_offset =
        Swap4Bytes(out->ending_time_offset);
    out->azimuth_angle_of_first_ray_in_file =
        Swap4Bytes(out->azimuth_angle_of_first_ray_in_file);
    out->elevation_angle_of_first_ray_in_file =
        Swap4Bytes(out->elevation_angle_of_first_ray_in_file);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_raw ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a raw_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
raw_psi_struct *extract_psi_raw(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  raw_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'raw_psi_struct' structure in"
          " function 'extract_psi_raw'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->data_type_mask_word_0, s1, 4);
  memcpy( &out->range_of_last_bin_in_cm, s1+4, 4);
  memcpy( &out->format_conversion_flag, s1+8, 4);
  memcpy( &out->flag_word, s1+12, 4);
  memcpy( &out->sweep_number_if_separate_files, s1+16, 4);
  memcpy( &out->xhdr_type, s1+20, 4);
  memcpy( &out->data_type_mask_1, s1+24, 4);
  memcpy( &out->data_type_mask_2, s1+28, 4);
  memcpy( &out->data_type_mask_3, s1+32, 4);
  memcpy( &out->data_type_mask_4, s1+36, 4);
  memcpy( &out->playback_version, s1+40, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->data_type_mask_word_0 =
        Swap4Bytes(out->data_type_mask_word_0);
    out->range_of_last_bin_in_cm =
        Swap4BytesSigned( (UINT4)
            out->range_of_last_bin_in_cm);
    out->format_conversion_flag =
        Swap4Bytes(out->format_conversion_flag);
    out->flag_word = Swap4Bytes(out->flag_word);
    out->sweep_number_if_separate_files =
        Swap4BytesSigned( (UINT4)
            out->sweep_number_if_separate_files);
    out->xhdr_type = Swap4Bytes(out->xhdr_type);
    out->data_type_mask_1 = Swap4Bytes(out->data_type_mask_1);
    out->data_type_mask_2 = Swap4Bytes(out->data_type_mask_2);
    out->data_type_mask_3 = Swap4Bytes(out->data_type_mask_3);
    out->data_type_mask_4 = Swap4Bytes(out->data_type_mask_4);
    out->playback_version = Swap4Bytes(out->playback_version);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_max ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a maximum_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
maximum_psi_struct *extract_psi_max(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  maximum_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'maximum_psi_struct' structure in"
          " function 'extract_psi_max'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->four_spare_bytes, s1, 4);
  memcpy( &out->bottom_of_interval_in_cm, s1+4, 4);
  memcpy( &out->top_of_interval_in_cm, s1+8, 4);
  memcpy( &out->number_of_pixels_in_side_panels, s1+12, 4);
  memcpy( &out->horizontal_smoother_in_side_panels, s1+16, 2);
  memcpy( &out->vertical_smoother_in_side_panels, s1+18, 2);
  memcpy( &out->ipad_end, s1+20, PSI_SIZE-20);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->bottom_of_interval_in_cm =
        Swap4BytesSigned( (UINT4) out->bottom_of_interval_in_cm);
    out->top_of_interval_in_cm =
        Swap4BytesSigned( (UINT4) out->top_of_interval_in_cm);
    out->number_of_pixels_in_side_panels =
        Swap4BytesSigned( (UINT4)
            out->number_of_pixels_in_side_panels);
    out->horizontal_smoother_in_side_panels =
        Swap2BytesSigned( (UINT2)
            out->horizontal_smoother_in_side_panels);
    out->vertical_smoother_in_side_panels =
        Swap2BytesSigned( (UINT2)
            out->vertical_smoother_in_side_panels);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_sline --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a sline_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
sline_psi_struct *extract_psi_sline(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  sline_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'sline_psi_struct' structure in"
          " function 'extract_psi_sline'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->area_in_square_meters, s1, 4);
  memcpy( &out->shear_threshold_cm_per_sec_per_km, s1+4, 4);
  memcpy( &out->bit_flags_to_choose_protected_areas, s1+8, 4);
  memcpy( &out->maximum_forecast_time_in_seconds, s1+12, 4);
  memcpy( &out->maximum_age_between_products_for_motion_calc, s1+16, 4);
  memcpy( &out->maximum_velocity_allowed_in_motion, s1+20, 4);
  memcpy( &out->flag_word, s1+24, 4);
  memcpy( &out->azimuthal_smoothing_angle, s1+28, 4);
  memcpy( &out->elevation_angle, s1+32, 4);
  memcpy( &out->elevation_angle_2, s1+36, 4);
  memcpy( &out->name_of_VVP_task, s1+40, 12);
  memcpy( &out->maximum_age_of_VVP_in_seconds, s1+52, 4);
  memcpy( &out->curve_fit_standard_deviation_threshold_in_cm, s1+56, 4);
  memcpy( &out->min_length_of_sline_in_tenths_of_km, s1+60, 4);
  memcpy( &out->ipad_end, s1+64, PSI_SIZE-64);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->area_in_square_meters =
        Swap4BytesSigned( (UINT4)
            out->area_in_square_meters);
    out->shear_threshold_cm_per_sec_per_km =
        Swap4BytesSigned( (UINT4)
            out->shear_threshold_cm_per_sec_per_km);
    out->bit_flags_to_choose_protected_areas =
        Swap4Bytes(out->bit_flags_to_choose_protected_areas);
    out->maximum_forecast_time_in_seconds =
        Swap4BytesSigned( (UINT4)
            out->maximum_forecast_time_in_seconds);
    out->maximum_age_between_products_for_motion_calc =
        Swap4Bytes(out->maximum_age_between_products_for_motion_calc);
    out->maximum_velocity_allowed_in_motion =
        Swap4BytesSigned( (UINT4)
            out->maximum_velocity_allowed_in_motion);
    out->flag_word = Swap4Bytes(out->flag_word);
    out->azimuthal_smoothing_angle =
        Swap4Bytes(out->azimuthal_smoothing_angle);
    out->elevation_angle = Swap4Bytes(out->elevation_angle);
    out->elevation_angle_2 = Swap4Bytes(out->elevation_angle_2);
    out->maximum_age_of_VVP_in_seconds =
        Swap4Bytes(out->maximum_age_of_VVP_in_seconds);
    out->curve_fit_standard_deviation_threshold_in_cm =
        Swap4BytesSigned( (UINT4)
            out->curve_fit_standard_deviation_threshold_in_cm);
    out->min_length_of_sline_in_tenths_of_km =
        Swap4Bytes(out->min_length_of_sline_in_tenths_of_km);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_wind ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a wind_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
wind_psi_struct *extract_psi_wind(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  wind_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out);
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'wind_psi_struct' structure in"
          " function 'extract_psi_wind'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->minimum_height_in_cm, s1, 4);
  memcpy( &out->maximum_height_in_cm, s1+4, 4);
  memcpy( &out->minimum_range_in_cm, s1+8, 4);
  memcpy( &out->maximum_range_in_cm, s1+12, 4);
  memcpy( &out->number_of_points_in_range, s1+16, 4);
  memcpy( &out->number_of_points_in_azimuth, s1+20, 4);
  memcpy( &out->sector_length_in_cm, s1+24, 4);
  memcpy( &out->sector_width_angle, s1+28, 4);
  memcpy( &out->flag_word, s1+32, 4);
  memcpy( &out->wind_parameters_mask_of_included_VVP, s1+36, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->minimum_height_in_cm =
        Swap4BytesSigned( (UINT4)
            out->minimum_height_in_cm);
    out->maximum_height_in_cm =
        Swap4BytesSigned( (UINT4)
            out->maximum_height_in_cm);
    out->minimum_range_in_cm =
        Swap4BytesSigned( (UINT4)
            out->minimum_range_in_cm);
    out->maximum_range_in_cm =
        Swap4BytesSigned( (UINT4)
            out->maximum_range_in_cm);
    out->number_of_points_in_range =
        Swap4BytesSigned( (UINT4)
            out->number_of_points_in_range);
    out->number_of_points_in_azimuth =
         Swap4BytesSigned( (UINT4)
             out->number_of_points_in_azimuth);
    out->sector_length_in_cm =
        Swap4BytesSigned( (UINT4)
            out->sector_length_in_cm);
    out->sector_width_angle =
        Swap4Bytes(out->sector_width_angle);
    out->flag_word = Swap4Bytes(out->flag_word);
    out->wind_parameters_mask_of_included_VVP =
        Swap4Bytes(out->wind_parameters_mask_of_included_VVP);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_beam ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a beam_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
beam_psi_struct *extract_psi_beam(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  beam_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'beam_psi_struct' structure in"
          " function 'extract_psi_beam'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->minimum_range_in_cm, s1, 4);
  memcpy( &out->maximum_range_in_cm, s1+4, 4);
  memcpy( &out->left_azimuth, s1+8, 4);
  memcpy( &out->right_azimuth, s1+12, 4);
  memcpy( &out->lower_elevation, s1+16, 4);
  memcpy( &out->upper_elevation, s1+20, 4);
  memcpy( &out->azimuth_smoothing, s1+24, 4);
  memcpy( &out->elevation_smoothing, s1+28, 4);
  memcpy( &out->azimuth_of_sun_at_start, s1+32, 4);
  memcpy( &out->elevation_of_sun_at_start, s1+36, 4);
  memcpy( &out->azimuth_of_sun_at_end, s1+40, 4);
  memcpy( &out->elevation_of_sun_at_end, s1+44, 4);
  memcpy( &out->ipad_end, s1+48, PSI_SIZE-48);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->minimum_range_in_cm = Swap4Bytes(out->minimum_range_in_cm);
    out->maximum_range_in_cm = Swap4Bytes(out->maximum_range_in_cm);
    out->left_azimuth = Swap4Bytes(out->left_azimuth);
    out->right_azimuth = Swap4Bytes(out->right_azimuth);
    out->lower_elevation = Swap4Bytes(out->lower_elevation);
    out->upper_elevation = Swap4Bytes(out->upper_elevation);
    out->azimuth_smoothing = Swap4Bytes(out->azimuth_smoothing);
    out->elevation_smoothing = Swap4Bytes(out->elevation_smoothing);
    out->azimuth_of_sun_at_start =
        Swap4Bytes(out->azimuth_of_sun_at_start);
    out->elevation_of_sun_at_start =
        Swap4Bytes(out->elevation_of_sun_at_start);
    out->azimuth_of_sun_at_end =
        Swap4Bytes(out->azimuth_of_sun_at_end);
    out->elevation_of_sun_at_end =
        Swap4Bytes(out->elevation_of_sun_at_end);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  --------------------------- extract_psi_fcast --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a fcast_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
fcast_psi_struct *extract_psi_fcast(UINT1 *s1,
                                     _Bool target_is_big_endian) {
  fcast_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'fcast_psi_struct' structure in"
          " function 'extract_psi_fcast'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->correlation_threshold, s1, 4);
  memcpy( &out->data_threshold, s1+4, 4);
  memcpy( &out->mean_speed_in_cm_per_hour, s1+8, 4);
  memcpy( &out->direction_of_mean_speed, s1+12, 4);
  memcpy( &out->maximum_time_between_products_in_seconds, s1+16, 4);
  memcpy( &out->maximum_allowable_velocity_in_cm_per_seconds, s1+20, 4);
  memcpy( &out->flags, s1+24, 4);
  memcpy( &out->desired_output_resolution_in_cm, s1+28, 4);
  memcpy( &out->type_of_input_product, s1+32, 4);
  memcpy( &out->name_of_input_product, s1+36, 12);
  memcpy( &out->ipad_end, s1+48, PSI_SIZE-48);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->correlation_threshold =
        Swap4Bytes(out->correlation_threshold);
    out->data_threshold = Swap4BytesSigned( (UINT4)
        out->data_threshold);
    out->mean_speed_in_cm_per_hour =
        Swap4BytesSigned( (UINT4)
            out->mean_speed_in_cm_per_hour);
    out->direction_of_mean_speed =
        Swap4Bytes(out->direction_of_mean_speed);
    out->maximum_time_between_products_in_seconds =
        Swap4Bytes(out->maximum_time_between_products_in_seconds);
    out->maximum_allowable_velocity_in_cm_per_seconds =
        Swap4BytesSigned( (UINT4)
            out->maximum_allowable_velocity_in_cm_per_seconds);
    out->flags = Swap4Bytes(out->flags);
    out->desired_output_resolution_in_cm =
        Swap4BytesSigned( (UINT4)
            out->desired_output_resolution_in_cm);
    out->type_of_input_product =
        Swap4Bytes(out->type_of_input_product);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_tdwr --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a tdwr_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
tdwr_psi_struct *extract_psi_tdwr(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  tdwr_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'tdwr_psi_struct' structure in"
          " function 'extract_psi_tdwr'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 4);
  memcpy( &out->maximum_range_in_cm, s1+4, 4);
  memcpy( &out->source_ID, s1+8, 4);
  memcpy( &out->center_field_wind_direction, s1+12, 3);
  memcpy( &out->center_field_wind_speed, s1+16, 2);
  memcpy( &out->center_field_gust_speed, s1+18, 2);
  memcpy( &out->mask_of_protected_areas_checked, s1+20, 4);
  memcpy( &out->number_of_centroids_in_file, s1+28, 4);
  memcpy( &out->number_of_shear_lines_in_file, s1+32, 4);
  memcpy( &out->forecast_time_in_seconds, s1+36, 4);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->flags = Swap4Bytes(out->flags);
    out->maximum_range_in_cm =
        Swap4Bytes(out->maximum_range_in_cm);
    out->mask_of_protected_areas_checked =
        Swap4Bytes(out->mask_of_protected_areas_checked);
    out->number_of_centroids_in_file =
        Swap4Bytes(out->number_of_centroids_in_file);
    out->number_of_shear_lines_in_file =
        Swap4Bytes(out->number_of_shear_lines_in_file);
    out->forecast_time_in_seconds =
        Swap4BytesSigned( (UINT4)
            out->forecast_time_in_seconds);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_user --------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a user_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
user_psi_struct *extract_psi_user(UINT1 *s1,
                                   _Bool target_is_big_endian) {
  user_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'user_psi_struct' structure in"
          " function 'extract_psi_user'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  for(int i = 0; i < PSI_SIZE/4 ; i++) {
      memcpy( &out->imisc[i], s1+i*4, 4);
  }
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    for(int i = 0; i < PSI_SIZE/4 ; i++)
      out->imisc[i] = Swap4BytesSigned( (UINT4) out->imisc[i]);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------------- extract_psi_sri ---------------------------  *
*                                                                            *
*****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a sri_psi_struct structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
sri_psi_struct *extract_psi_sri(UINT1 *s1,
                                 _Bool target_is_big_endian) {
  sri_psi_struct *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'sri_psi_struct' structure in"
          " function 'extract_psi_sri'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 4);
  memcpy( &out->total_number_of_bins_inserted, s1+4, 4);
  memcpy( &out->number_of_bins_with_data, s1+8, 4);
  memcpy( &out->number_of_data_bins_profile_corrected, s1+12, 4);
  memcpy( &out->surface_height_in_meters, s1+16, 2);
  memcpy( &out->maximum_height_in_meters, s1+18, 2);
  memcpy( &out->melting_height_in_meters, s1+20, 2);
  memcpy( &out->melting_level_thickness_in_m, s1+22, 2);
  memcpy( &out->melting_level_intensity, s1+24, 2);
  memcpy( &out->gradient_above_melting_per_100dB_per_km, s1+26, 2);
  memcpy( &out->gradient_below_melting_per_100dB_per_km, s1+28, 2);
  memcpy( &out->convective_check_height_in_meters, s1+30, 2);
  memcpy( &out->convective_check_level, s1+32, 2);
  memcpy( &out->ipad_end, s1+34, PSI_SIZE-34);
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->flags = Swap4Bytes(out->flags);
    out->total_number_of_bins_inserted =
        Swap4BytesSigned( (UINT4)
            out->total_number_of_bins_inserted);
    out->number_of_bins_with_data =
        Swap4BytesSigned( (UINT4)
            out->number_of_bins_with_data);
    out->number_of_data_bins_profile_corrected =
        Swap4BytesSigned( (UINT4)
            out->number_of_data_bins_profile_corrected);
    out->surface_height_in_meters =
        Swap2BytesSigned( (UINT2)
            out->surface_height_in_meters);
    out->maximum_height_in_meters =
        Swap2BytesSigned( (UINT2)
            out->maximum_height_in_meters);
    out->melting_height_in_meters =
        Swap2BytesSigned( (UINT2)
            out->melting_height_in_meters);
    out->melting_level_thickness_in_m =
        Swap2BytesSigned( (UINT2)
            out->melting_level_thickness_in_m);
    out->melting_level_intensity =
        Swap2BytesSigned( (UINT2)
            out->melting_level_intensity);
    out->gradient_above_melting_per_100dB_per_km =
        Swap2BytesSigned( (UINT2)
            out->gradient_above_melting_per_100dB_per_km);
    out->gradient_below_melting_per_100dB_per_km =
        Swap2BytesSigned( (UINT2)
            out->gradient_below_melting_per_100dB_per_km);
    out->convective_check_height_in_meters =
        Swap2BytesSigned( (UINT2)
            out->convective_check_height_in_meters);
    out->convective_check_level =
        Swap2BytesSigned( (UINT2)
            out->convective_check_level);
  }
  return out;
}

/* ================================================== */
/* A function to copy data from an IRIS raw product
 * record to a raw_prod_bhdr structure and return
 * a pointer to the structure. This function is
 * passed a pointer to an 'IRISbuf' structure.
 * If allocation failure occurs, a pointer to null is
 * returned and an error message is written to stderr  */
rpb_s *extract_raw_prod_bhdr(IRISbuf *IRISbuf_p,
                             _Bool target_is_big_endian) {
  UINT1 *s0;
  s0 = &IRISbuf_p->bufIRIS[0];
  rpb_s *out= NULL;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
    Iris_printf(
            "Error! Unable to allocate 'rpb_s' structure in"
            " function 'extract_raw_product_bhdr'.\n");
    return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->record_number, s0, 2);
  memcpy( &out->sweep_number, s0+2, 2);
  memcpy( &out->offset_of_first_ray_in_record, s0+4, 4);
  memcpy( &out->ray_number_within_sweep, s0+6, 2);
  memcpy( &out->flags, s0+8, 2);
  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->record_number = Swap2BytesSigned( (UINT2) out->record_number);
    out->sweep_number = Swap2BytesSigned( (UINT2) out->sweep_number);
    out->offset_of_first_ray_in_record = 
       Swap2BytesSigned( (UINT2) out->offset_of_first_ray_in_record);
    out->ray_number_within_sweep = 
      Swap2BytesSigned( (UINT2) out->ray_number_within_sweep);
    out->flags = Swap2Bytes(out->flags);
  }
  return out;
}

/*****************************************************************************
*                                                                            *
*  ---------------------- extract_ingest_data_header ----------------------  *
*                                                                            *
*****************************************************************************/
/* A function to extract data from a IRIS raw product
 * record and produce/return a pointer to an
 * ingest_data_header type structure. The function is
 * passed a pointer to an 'IRISbuf' structure.
 * If an allocation failure occurs, a pointer to null
 * is returned and an error message is printed  */
/* ======================================================================== */
idh_s *extract_ingest_data_header(IRISbuf *IRISbuf_p,
                                  UINT2 offset,
                                  _Bool target_is_big_endian) {
  /* declare some pointers to be used below */
  UINT1 *ptr_s0, *ptr_s1, *ptr_s2, *ptr_s3;
  /* 
   * prt_s0 points to beginning of the input data buffer
   * Note! void bufIRIS pointer cast to pointer to UINT1 here
   */
  UINT1 *bufIRIS_p = NULL;
  /* simplify by setting a pointer to the input array of bytes */
  bufIRIS_p = &IRISbuf_p->bufIRIS[0];
  ptr_s0 = bufIRIS_p;
  /* 
   * prt_s1 points to the start of 'ingest_data_Header'
   * which where a 'structure_header' structure begins
   */
  ptr_s1 = ptr_s0 + offset;
  /* 
   * prt_s2 points to the start of 'ymds_time' structure
   * that follows the structure header data
   */
  ptr_s2 = ptr_s1 + STRUCT_HEADER_SIZE;
  /* 
   * prt_s3 points to the start of of the regular data
   * that follows the ymds_time data
   */
  ptr_s3 = ptr_s2 + YMDS_TIME_SIZE;
  /* 
   * allocate an 'ingest_data_header' structure and call it 'out'
   */
  idh_s *out = NULL;
  out = RAVE_MALLOC( sizeof *out );
  if ( !out ) {
      Iris_printf(
          "Error! Unable to allocate 'idh_s' structure in"
          " function 'extract_ingest_data_header'.\n");
      return NULL;
  }
  /* allocation went OK so fill the structure */
  shd_s *shd_p = NULL;
  shd_p = extract_structure_header(ptr_s1,
                                   target_is_big_endian);
  if( shd_p ) {
     out->hdr = *shd_p; // copy structure
     RAVE_FREE(shd_p); // free allocated memory of structure
  }
  ymd_s *ymd_p = NULL;
  ymd_p = extract_ymds_time(ptr_s2,
                            target_is_big_endian);
  if( ymd_p ) {
       out->sweep_start_time = *ymd_p; // copy structure
       RAVE_FREE(ymd_p); // free allocated memory of structure
  }
  memcpy( &out->sweep_number, ptr_s3, 2);
  memcpy( &out->resolution_as_rays_per_360_degree_sweep, ptr_s3+2, 2);
  memcpy( &out->index_of_first_ray, ptr_s3+4, 2);
  memcpy( &out->number_of_rays_in_sweep, ptr_s3+6, 2);
  memcpy( &out->rays_written, ptr_s3+8, 2);
  memcpy( &out->fixed_angle_of_sweep, ptr_s3+10, 2);
  memcpy( &out->number_of_bits_per_bin, ptr_s3+12, 2);
  memcpy( &out->data_type, ptr_s3+14, 2);

  /* swap some bytes if the IRIS input file is big-endian */
  if( target_is_big_endian ) {
    out->sweep_number = Swap2BytesSigned( (UINT2) out->sweep_number);
    out->resolution_as_rays_per_360_degree_sweep =
        Swap2BytesSigned( (UINT2)
            out->resolution_as_rays_per_360_degree_sweep);
    out->index_of_first_ray = Swap2BytesSigned( (UINT2)
        out->index_of_first_ray);
    out->number_of_rays_in_sweep =
        Swap2BytesSigned( (UINT2) out->number_of_rays_in_sweep);
    out->rays_written = Swap2BytesSigned( (UINT2) out->rays_written);
    out->fixed_angle_of_sweep = Swap2Bytes(out->fixed_angle_of_sweep);
    out->number_of_bits_per_bin =
        Swap2BytesSigned( (UINT2) out->number_of_bits_per_bin);
    out->data_type = Swap2Bytes(out->data_type);
  }
  return out;
}

/*****************************************************************************
 *                                                                            *
 *  ------------------------ extract_color_scale_def -----------------------  *
 *                                                                            *
 *****************************************************************************/
/* A function to copy data from an IRIS raw product
 * record to a color_scale_def structure and return a
 * pointer to the structure. This function is
 * passed a pointer to the start of pertinent data
 * within the input buffer.
 * If allocation failure occurs a pointer to null is
 * returned and an error messages is written to stderr */
/* ======================================================================== */
csd_s *extract_color_scale_def(UINT1 *s1, _Bool target_is_big_endian) {
  csd_s *out;
  out = RAVE_MALLOC( sizeof *out );
  if( !out ) {
    Iris_printf(
            "Error! Unable to allocate 'csd' structure in"
            " function 'extract_color_scale_def'.\n");
    return NULL;
  }
  /* allocation went OK so fill the structure */
  memcpy( &out->flags, s1, 4);
  memcpy( &out->starting_level, s1+4, 4);
  memcpy( &out->level_step, s1+8, 4);
  memcpy( &out->number_of_colors_in_scale, s1+12, 2);
  memcpy( &out->set_number_and_color_scale_number, s1+14, 2);
  memcpy( &out->starting_values_for_variable_levels, s1+16, 2*16);
  
  /* if input file is big_endian then swap bytes */
  if( target_is_big_endian) {
    out->flags = Swap4Bytes(out->flags);
    out->starting_level = Swap4BytesSigned( (UINT4)
    out->starting_level);
    out->level_step = Swap4BytesSigned( (UINT4)
    out->level_step);
    out->number_of_colors_in_scale =
    Swap2BytesSigned( (UINT2)
    out->number_of_colors_in_scale);
    out->set_number_and_color_scale_number =
    Swap2Bytes(out->set_number_and_color_scale_number);
    for(int i=0;i<16;i++)
      out->starting_values_for_variable_levels[i] =
      Swap2Bytes(out->starting_values_for_variable_levels[i]);
  }
  return out;
}

/*****************************************************************************
 *                                                                            *
 *  ----------------------- deep_copy_product_header -----------------------  *
 *                                                                            *
 *****************************************************************************/
/* A function to copy data from one product header structure to another.
 * The pointer to the 'from' structure is the first argument.
 * The pointer to the 'to' structure is a sub-structure of the second argument.
 * The function returns nothing, but it populates the 'to' structure as a side 
 * effect.
 * A product header structure holds 3 structures:
 *  struct structure_header hdr;          // Generic Header       12 bytes
 *  struct product_configuration pcf;     // Product Config Info 320 bytes
 *  struct product_end end;               // Product End Info    308 bytes
 * 
 * These pointers to heap allocated stuctures will just make one pointer
 * alias another so a deep copy is required.
 * 
 * ======================================================================== */
void deep_copy_product_header(phd_s *from, file_element_s **file_element_pp) {
  /* 
   * the structure_header structure 
   */
  phd_s *to = (*file_element_pp)->product_header_p;
  to->hdr.bytes_in_entire_struct = from->hdr.bytes_in_entire_struct;
  to->hdr.flags = from->hdr.flags;
  to->hdr.format_version_number = from->hdr.format_version_number;
  to->hdr.structure_identifier = from->hdr.structure_identifier;
  /*
   * the product_configuration structure
   */
  to->pcf.colors.flags = from->pcf.colors.flags;
  to->pcf.colors.level_step = from->pcf.colors.level_step;
  to->pcf.colors.number_of_colors_in_scale = 
    from->pcf.colors.number_of_colors_in_scale;
  to->pcf.colors.set_number_and_color_scale_number = 
    from->pcf.colors.set_number_and_color_scale_number;
  to->pcf.colors.starting_level = from->pcf.colors.starting_level;
  for(int k=0; k < 16; k++) {
    to->pcf.colors.starting_values_for_variable_levels[k] = 
      from->pcf.colors.starting_values_for_variable_levels[k];
  }
  to->pcf.data_type_generated = from->pcf.data_type_generated;
  to->pcf.data_type_used_as_input = from->pcf.data_type_used_as_input;
  for(int k=0; k < 12; k++) {    
    to->pcf.dataGen_task_name[k] = from->pcf.dataGen_task_name[k];
  }
  to->pcf.flag_word = from->pcf.flag_word;
  to->pcf.hdr.bytes_in_entire_struct = from->pcf.hdr.bytes_in_entire_struct;
  to->pcf.hdr.flags = from->pcf.hdr.flags;
  to->pcf.hdr.format_version_number = from->pcf.hdr.format_version_number;
  to->pcf.hdr.structure_identifier = from->pcf.hdr.structure_identifier;
  to->pcf.HydroClass = from->pcf.HydroClass;
  to->pcf.IngestFile_input_time_TZ.day = from->pcf.IngestFile_input_time_TZ.day;
  to->pcf.IngestFile_input_time_TZ.milliseconds_and_UTC_DST_indication = 
    from->pcf.IngestFile_input_time_TZ.milliseconds_and_UTC_DST_indication;
  to->pcf.IngestFile_input_time_TZ.month = from->pcf.IngestFile_input_time_TZ.month;
  to->pcf.IngestFile_input_time_TZ.year = from->pcf.IngestFile_input_time_TZ.year;
  to->pcf.IngestFile_input_time_TZ.seconds_since_midnight = 
    from->pcf.IngestFile_input_time_TZ.seconds_since_midnight;
  to->pcf.IngestSweep_input_time_TZ.day = from->pcf.IngestSweep_input_time_TZ.day;
  to->pcf.IngestSweep_input_time_TZ.milliseconds_and_UTC_DST_indication = 
    from->pcf.IngestSweep_input_time_TZ.milliseconds_and_UTC_DST_indication;
  to->pcf.IngestSweep_input_time_TZ.month = from->pcf.IngestSweep_input_time_TZ.month;
  to->pcf.IngestSweep_input_time_TZ.year = from->pcf.IngestSweep_input_time_TZ.year;
  to->pcf.IngestSweep_input_time_TZ.seconds_since_midnight = 
    from->pcf.IngestSweep_input_time_TZ.seconds_since_midnight;
  for(int k=0; k < PCF_TASK_MINOR_SIZE; k++) {
   to->pcf.list_of_minor_task_suffixes[k] = from->pcf.list_of_minor_task_suffixes[k];
  }
  to->pcf.Max_Range_in_cm = from->pcf.Max_Range_in_cm;
  for(int k=0; k < 12; k++) { 
    to->pcf.name_of_projection[k] = from->pcf.name_of_projection[k];
  }
  to->pcf.number_of_runs_this_product = from->pcf.number_of_runs_this_product;
  for(int k=0; k < 12; k++) { 
    to->pcf.product_configfile_name[k] = from->pcf.product_configfile_name[k];
  }
  to->pcf.product_GenTime_UTC.day = from->pcf.product_GenTime_UTC.day;
  to->pcf.product_GenTime_UTC.milliseconds_and_UTC_DST_indication = 
    from->pcf.product_GenTime_UTC.milliseconds_and_UTC_DST_indication;
  to->pcf.product_GenTime_UTC.month = from->pcf.product_GenTime_UTC.month;
  to->pcf.product_GenTime_UTC.year = from->pcf.product_GenTime_UTC.year;
  to->pcf.product_GenTime_UTC.seconds_since_midnight = 
    from->pcf.product_GenTime_UTC.seconds_since_midnight;
  /* 
   * Note! Although product_specific_info is a union of 80 byte structures
   * it should be able to be copied by assignment since there are no pointers
   * or arrays in these structures.
   */
  to->pcf.product_specific_info = from->pcf.product_specific_info;
  to->pcf.product_type_code = from->pcf.product_type_code;
  to->pcf.projection_type_code = from->pcf.projection_type_code;
  for(int k=0; k < PCF_QPE_ALGORITHM_SIZE; k++) { 
    to->pcf.QPE_algorithm_name[k] = from->pcf.QPE_algorithm_name[k];
  }
  to->pcf.radial_smoother_in_km_over_100 = 
    from->pcf.radial_smoother_in_km_over_100;
  to->pcf.scheduling_code = from->pcf.scheduling_code;
  to->pcf.seconds_to_skip_between_runs = 
    from->pcf.seconds_to_skip_between_runs;
  to->pcf.X_array_size = from->pcf.X_array_size;
  to->pcf.X_Radar_Location = from->pcf.X_Radar_Location;
  to->pcf.X_scale_cm_per_pixel = from->pcf.Y_scale_cm_per_pixel;
  to->pcf.x_smoother_in_hundredths_of_km = 
    from->pcf.x_smoother_in_hundredths_of_km;
  to->pcf.Y_array_size = from->pcf.Y_array_size;
  to->pcf.Y_Radar_Location = from->pcf.Y_Radar_Location;
  to->pcf.Y_scale_cm_per_pixel = from->pcf.Y_scale_cm_per_pixel;
  to->pcf.y_smoother_in_hundredths_of_km = 
    from->pcf.y_smoother_in_hundredths_of_km;
  to->pcf.Z_array_size = from->pcf.Z_array_size;
  to->pcf.Z_R_constant_thousandths = from->pcf.Z_R_constant_thousandths;
  to->pcf.Z_R_exponent_thousandths = from->pcf.Z_R_exponent_thousandths;
  to->pcf.Z_Radar_Location = from->pcf.Z_Radar_Location;
  to->pcf.Z_scale_cm_per_pixel = from->pcf.Z_scale_cm_per_pixel;
  /*
   * the product_end structure
   */
  to->end.current_noise_level_horizontal_pol_in_hundredths_of_dBm =
    from->end.current_noise_level_horizontal_pol_in_hundredths_of_dBm;
  to->end.current_noise_level_vertical_pol_in_hundredths_of_dBm =
    from->end.current_noise_level_vertical_pol_in_hundredths_of_dBm;
  to->end.Equatorial_radius_of_earth_in_cm = 
    from->end.Equatorial_radius_of_earth_in_cm;
  to->end.fault_status_of_task = from->end.fault_status_of_task;
  to->end.flag_word = from->end.flag_word;
  for(int k=0; k < 16; k++) { 
    to->end.hardware_name_of_ingest_data_source[k] = 
     from->end.hardware_name_of_ingest_data_source[k];
  }
  to->end.height_of_radar_above_the_ground_in_meters = 
    from->end.height_of_radar_above_the_ground_in_meters;
  to->end.height_of_radar_in_meters = from->end.height_of_radar_in_meters;
  to->end.IO_cal_value_horizontal_pol_in_hundredths_of_dBm = 
    from->end.IO_cal_value_horizontal_pol_in_hundredths_of_dBm;
  for(int k=0; k < 8; k++) { 
    to->end.IRIS_version_ingest_data[k] = 
      from->end.IRIS_version_ingest_data[k];
  }
  for(int k=0; k < 8; k++) { 
    to->end.IRIS_version_product_maker[k] = 
      from->end.IRIS_version_product_maker[k];
  }
  to->end.latitude_of_center = from->end.latitude_of_center;
  to->end.latitude_of_projection_reference = 
    from->end.latitude_of_projection_reference;
  to->end.LDR_offset_in_hundredths_dB = from->end.LDR_offset_in_hundredths_dB;
  to->end.longitude_of_center = from->end.longitude_of_center;
  to->end.longitude_of_projection_reference = 
    from->end.longitude_of_projection_reference;
  to->end.mask_of_input_sites_used_in_a_composite = 
    from->end.mask_of_input_sites_used_in_a_composite;
  to->end.mean_wind_direction = from->end.mean_wind_direction;
  to->end.mean_wind_speed = from->end.mean_wind_speed;
  to->end.melting_level_in_meters = from->end.melting_level_in_meters;
  to->end.minutes_LST_is_west_of_GMT = from->end.minutes_LST_is_west_of_GMT;
  to->end.minutes_recorded_standard_time_is_west_of_GMT = 
    from->end.minutes_recorded_standard_time_is_west_of_GMT;
  for(int k=0; k < 12; k++) { 
    to->end.name_of_clutter_filter_file[k] = 
      from->end.name_of_clutter_filter_file[k];
  }
  to->end.noise_at_calibration_horizontal_pol_in_hundredths_of_dBm = 
    from->end.noise_at_calibration_horizontal_pol_in_hundredths_of_dBm;
  to->end.nonzero_if_cluttermap_applied_to_the_ingest_data = 
    from->end.nonzero_if_cluttermap_applied_to_the_ingest_data;
  to->end.number_of_elements_in_product_results_array = 
    from->end.number_of_elements_in_product_results_array;
  to->end.number_of_ingest_or_product_files_used = 
    from->end.number_of_ingest_or_product_files_used;
  to->end.number_of_linear_based_filter_for_the_first_bin = 
    from->end.number_of_linear_based_filter_for_the_first_bin;
  to->end.number_of_log_based_filter_for_the_first_bin = 
    from->end.number_of_log_based_filter_for_the_first_bin;
  to->end.number_of_output_bins = from->end.number_of_output_bins;
  to->end.number_of_samples_used = from->end.number_of_samples_used;
  to->end.offset_to_extended_time_header = 
    from->end.offset_to_extended_time_header;
  to->end.one_over_flattening_in_millionths = 
    from->end.one_over_flattening_in_millionths;
  to->end.PRF_in_hertz = from->end.PRF_in_hertz;
  to->end.product_sequence_number = from->end.product_sequence_number;
  to->end.projection_angle_standard_parallel_1 = 
    from->end.projection_angle_standard_parallel_1;
  to->end.projection_angle_standard_parallel_2 = 
    from->end.projection_angle_standard_parallel_2;
  to->end.pulse_width_in_hundredths_of_microseconds = 
    from->end.pulse_width_in_hundredths_of_microseconds;
  to->end.radar_constant_horizontal_pol_in_hundredths_of_dB = 
    from->end.radar_constant_horizontal_pol_in_hundredths_of_dB;
  to->end.range_of_the_first_bin_in_cm = 
    from->end.range_of_the_first_bin_in_cm;
  to->end.range_of_the_last_bin_in_cm = 
    from->end.range_of_the_last_bin_in_cm;
  to->end.receiver_bandwidth_in_kHz = from->end.receiver_bandwidth_in_kHz;
  to->end.signed_ground_height_relative_to_sea_level = 
    from->end.signed_ground_height_relative_to_sea_level;
  for(int k=0; k < 16; k++) { 
    to->end.site_name[k] = from->end.site_name[k];
  }
  for(int k=0; k < 16; k++) { 
    to->end.site_name_of_ingest_data_source[k] = 
      from->end.site_name_of_ingest_data_source[k];
  }
  to->end.TFC_cal_flags = from->end.TFC_cal_flags;
  to->end.TFC_cal_flags2 = from->end.TFC_cal_flags2;
  to->end.time_of_oldest_input_ingest_file.day = 
    from->end.time_of_oldest_input_ingest_file.day;
  to->end.time_of_oldest_input_ingest_file.milliseconds_and_UTC_DST_indication = 
    from->end.time_of_oldest_input_ingest_file.milliseconds_and_UTC_DST_indication;
  to->end.time_of_oldest_input_ingest_file.month = 
    from->end.time_of_oldest_input_ingest_file.month;
  to->end.time_of_oldest_input_ingest_file.seconds_since_midnight = 
    from->end.time_of_oldest_input_ingest_file.seconds_since_midnight;
  to->end.time_of_oldest_input_ingest_file.year = 
    from->end.time_of_oldest_input_ingest_file.year;
  for(int k=0; k < 8; k++) { 
      to->end.time_zone_name_of_recorded_data[k] = 
        from->end.time_zone_name_of_recorded_data[k];
  }
  to->end.trigger_rate_scheme = from->end.trigger_rate_scheme;
  to->end.truncation_height_in_cm_above_radar = 
    from->end.truncation_height_in_cm_above_radar;
  to->end.type_of_polarization_used = from->end.type_of_polarization_used;
  to->end.type_of_signal_processor_used = 
    from->end.type_of_signal_processor_used;
  to->end.wavelength_in_hundredths_of_centimeters = 
    from->end.wavelength_in_hundredths_of_centimeters;
  to->end.ZDR_offset_in_hundredths_dB = from->end.ZDR_offset_in_hundredths_dB;
  return;
}

/*****************************************************************************
 *                                                                            *
 *  ----------------------- deep_copy_ingest_header -----------------------  *
 *                                                                            *
 *****************************************************************************/
/* A function to copy data from one injest header structure to another.
 * The pointer to the 'from' structure is the first argument.
 * The pointer to the 'to' structure is a sub-structure of the second argument.
 * The function returns nothing, but it populates the 'to' structure as a side 
 * effect.
 * An ingest header structure holds:
 *  struct structure_header hdr;       // Generic Header       12 bytes
 *  struct ingest_configuration icf;   // Ingest Config Info  480 bytes
 *  struct task_configuration tcf;     // Task Config        2612 bytes
 *  char spare[732];
 *  struct gparm GParm;                //                      128 bytes
 *                         // Read from the RVP just after configuration
 *  char reserved[920];
 * 
 * These pointers to heap-allocated-stuctures will just make one pointer
 * alias another so a deep copy is required.
 * 
 * ======================================================================== */
void deep_copy_ingest_header(ihd_s *from, file_element_s **file_element_pp) {
  /* 
   * the structure_header structure 
   */
  ihd_s *to = (*file_element_pp)->ingest_header_p;
  to->hdr.bytes_in_entire_struct = from->hdr.bytes_in_entire_struct;
  to->hdr.flags = from->hdr.flags;
  to->hdr.format_version_number = from->hdr.format_version_number;
  to->hdr.structure_identifier = from->hdr.structure_identifier;
  /* 
   * ingest_configuration 
   */
  to->icf.altitude_of_radar_cm_above_sea_level = 
    from->icf.altitude_of_radar_cm_above_sea_level;
  for(int k=0; k < 3; k++) { 
    to->icf.antenna_offset_from_INU_in_cm_starboard_bow_up[k] = 
      from->icf.antenna_offset_from_INU_in_cm_starboard_bow_up[k];
  }
  to->icf.bytes_in_each_gparam = from->icf.bytes_in_each_gparam;
  for(int k=0; k < 16; k++) { 
    to->icf.config_name_in_the_dpolapp_conf_file[k] = 
      from->icf.config_name_in_the_dpolapp_conf_file[k];
  }
  to->icf.fault_status = from->icf.fault_status;
  to->icf.flags = from->icf.flags;
  to->icf.height_of_ground_site_in_meters_above_sea_level = 
    from->icf.height_of_ground_site_in_meters_above_sea_level;
  to->icf.height_of_melting_level_above_sea_level_in_meters = 
    from->icf.height_of_melting_level_above_sea_level_in_meters;
  to->icf.index_of_first_ray = from->icf.index_of_first_ray;
  for(int k=0; k < 16; k++) {
    to->icf.ingest_hardware_name_of_site[k] = 
      from->icf.ingest_hardware_name_of_site[k];
  }
  for(int k=0; k < 8; k++) {
    to->icf.IRIS_version_number[k] = from->icf.IRIS_version_number[k];
  }
  to->icf.latitude_of_radar = from->icf.latitude_of_radar;
  to->icf.longitude_of_radar = from->icf.longitude_of_radar;
  for(int k=0; k < 8; k++) {
    to->icf.local_timezone_string[k] = from->icf.local_timezone_string[k];
  }
  to->icf.minutes_west_of_GMT_of_LST = 
    from->icf.minutes_west_of_GMT_of_LST;
  to->icf.minutes_west_of_GMT_recorded_time = 
    from->icf.minutes_west_of_GMT_recorded_time;
  for(int k=0; k < 80; k++) {
    to->icf.name_of_file_on_disk[k] = from->icf.name_of_file_on_disk[k];
  }
  to->icf.number_of_associated_disk_files_extant = 
    from->icf.number_of_associated_disk_files_extant;
  to->icf.number_of_bytes_in_extended_ray_headers = 
    from->icf.number_of_bytes_in_extended_ray_headers;
  to->icf.number_of_rays_in_sweep = from->icf.number_of_rays_in_sweep;
  to->icf.number_of_sweeps_completed = 
    from->icf.number_of_sweeps_completed;
  to->icf.playback_version_number = from->icf.playback_version_number;
  to->icf.radar_height_in_meters_above_ground = 
    from->icf.radar_height_in_meters_above_ground;
  for(int k=0; k < 16; k++) {
    to->icf.radar_site_name_from_setup_utility[k] = 
      from->icf.radar_site_name_from_setup_utility[k];
  }
  to->icf.resolution_as_rays_per_360_degree_sweep = 
    from->icf.resolution_as_rays_per_360_degree_sweep;
  to->icf.time_that_volume_scan_was_started.day = 
    from->icf.time_that_volume_scan_was_started.day;
  to->icf.time_that_volume_scan_was_started.
    milliseconds_and_UTC_DST_indication = 
    from->icf.time_that_volume_scan_was_started.
    milliseconds_and_UTC_DST_indication;
  to->icf.time_that_volume_scan_was_started.month = 
    from->icf.time_that_volume_scan_was_started.month;
  to->icf.time_that_volume_scan_was_started.
    seconds_since_midnight = 
    from->icf.time_that_volume_scan_was_started.
    seconds_since_midnight;
  to->icf.time_that_volume_scan_was_started.year = 
    from->icf.time_that_volume_scan_was_started.year;
  to->icf.total_size_of_all_files = from->icf.total_size_of_all_files;
  for(int k=0; k < 3; k++) { 
    to->icf.velocity_of_radar_in_cm_per_sec_east_north_up[k] = 
      from->icf.velocity_of_radar_in_cm_per_sec_east_north_up[k];
  }
  /* 
   * task_configuration 
   */
  to->tcf.hdr = from->tcf.hdr; /* copy whole structure */
  to->tcf.sch = from->tcf.sch; /* copy whole structure */
  to->tcf.dsp.agc_feedback_code = from->tcf.dsp.agc_feedback_code;
  to->tcf.dsp.CfgHdr_Mask = from->tcf.dsp.CfgHdr_Mask;
  to->tcf.dsp.clutter_filter_index = from->tcf.dsp.clutter_filter_index;
  to->tcf.dsp.DataMask = from->tcf.dsp.DataMask; /* copy whole structure */
  to->tcf.dsp.dsp_major_mode = from->tcf.dsp.dsp_major_mode;
  to->tcf.dsp.dsp_type = from->tcf.dsp.dsp_type;
  to->tcf.dsp.dual_PRF_delay = from->tcf.dsp.dual_PRF_delay;
  for(int k=0; k < 6; k++) {
    to->tcf.dsp.enums[k] = from->tcf.dsp.enums[k];
  }
  to->tcf.dsp.fixed_gain = from->tcf.dsp.fixed_gain;
  to->tcf.dsp.flag_nonzero_if_clutter_map_used = 
    from->tcf.dsp.flag_nonzero_if_clutter_map_used;
  to->tcf.dsp.flags_time_series_playback = 
    from->tcf.dsp.flags_time_series_playback;
  to->tcf.dsp.gain_control_flag = from->tcf.dsp.gain_control_flag;
  to->tcf.dsp.gas_attenuation = from->tcf.dsp.gas_attenuation;
  to->tcf.dsp.log_filter_first_bin = 
    from->tcf.dsp.log_filter_first_bin;
  to->tcf.dsp.multi_PRF_mode_flag = from->tcf.dsp.multi_PRF_mode_flag;
  for(int k=0; k < 16; k++) {
    to->tcf.dsp.name_of_custom_ray_header[k] = 
      from->tcf.dsp.name_of_custom_ray_header[k];
  }
  for(int k=0; k < 12; k++) {
    to->tcf.dsp.name_of_file_used_for_clutter_filter[k] = 
    from->tcf.dsp.name_of_file_used_for_clutter_filter[k];
  }
  to->tcf.dsp.OriginalDataMask = from->tcf.dsp.OriginalDataMask;/* copy whole structure */
  to->tcf.dsp.prf_in_hertz = from->tcf.dsp.prf_in_hertz;
  to->tcf.dsp.pulse_width_in_hundredths_of_microseconds = 
    from->tcf.dsp.pulse_width_in_hundredths_of_microseconds;
  to->tcf.dsp.sample_size = from->tcf.dsp.sample_size;
  to->tcf.dsp.u = from->tcf.dsp.u; /* copy whole unuin of structures */
  to->tcf.dsp.xmt_phase_sequence = from->tcf.dsp.xmt_phase_sequence;
  to->tcf.cal = from->tcf.cal; /* copy whole structure */
  to->tcf.rng = from->tcf.rng; /* copy whole structure */
  to->tcf.scan = from->tcf.scan; /* copy whole structure */
  to->tcf.misc = from->tcf.misc; /* copy whole structure */
  for(int k=0; k < 80; k++) {
    to->tcf.end.eighty_byte_task_description[k] = 
      from->tcf.end.eighty_byte_task_description[k];
  }
  for(int k=0; k < 12; k++) {
    to->tcf.end.name_of_task_configuration_file[k] = 
      from->tcf.end.name_of_task_configuration_file[k];
  }
  to->tcf.end.number_of_tasks_in_this_hybrid_set = 
    from->tcf.end.number_of_tasks_in_this_hybrid_set;
  to->tcf.end.task_major_number = from->tcf.end.task_major_number;
  to->tcf.end.task_minor_number = from->tcf.end.task_minor_number;
  to->tcf.end.task_state = from->tcf.end.task_state;
  to->tcf.end.task_time.day = from->tcf.end.task_time.day;
  to->tcf.end.task_time.milliseconds_and_UTC_DST_indication = 
    from->tcf.end.task_time.milliseconds_and_UTC_DST_indication;
  to->tcf.end.task_time.month = from->tcf.end.task_time.month;
  to->tcf.end.task_time.seconds_since_midnight = 
    from->tcf.end.task_time.seconds_since_midnight;
  to->tcf.end.task_time.year = from->tcf.end.task_time.year;
  /* 
   * gparm 
   */
  to->GParm = from->GParm; /* copy whole structure */
  return;
}

