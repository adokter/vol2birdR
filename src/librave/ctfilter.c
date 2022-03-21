/* --------------------------------------------------------------------
Copyright (C) 2014 Swedish Meteorological and Hydrological Institute, SMHI

This is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with HLHDF.  If not, see <http://www.gnu.org/licenses/>.
------------------------------------------------------------------------*/

/** Functionality for identifying and removing residual non-precipitation
 * echoes in Cartesian radar products, using the standard SAF-NWC cloud-top
 * product from EUMETCAST in HDF5.
 * @file
 * @author Daniel Michelson, SMHI
 * @date 2014-03-25
 */

#include "ctfilter.h"


int ctFilter(Cartesian_t* product, Cartesian_t* ct) {
   CartesianParam_t* param = NULL;
   RaveField_t* qfield = RAVE_OBJECT_NEW(&RaveField_TYPE);
   RaveAttribute_t* attr = NULL;
   RaveValueType rvt_p = RaveValueType_NODATA;
   /*RaveValueType rvt_c = RaveValueType_NODATA;*/
   Projection_t* pproj = NULL;
   Projection_t* cproj = NULL;
   const char* quantity;
   double pval, cval, undetect;
   int ret = 0;
   int xp, yp, xc, yc;
   double xsurfp, ysurfp, xsurfc, ysurfc;
   long xsizep, ysizep, xsizec, ysizec;

   xsizep = Cartesian_getXSize(product);
   ysizep = Cartesian_getYSize(product);
   xsizec = Cartesian_getXSize(ct);
   ysizec = Cartesian_getYSize(ct);
   pproj = Cartesian_getProjection(product);
   cproj = Cartesian_getProjection(ct);

   undetect = Cartesian_getUndetect(product);

   if (!RaveField_createData(qfield, xsizep, ysizep, RaveDataType_UCHAR)) {
     RAVE_ERROR0("CTFILTER: Could not initialize quality field");
     goto done;
   }
   attr = RaveAttributeHelp_createString("how/task","se.smhi.quality.ctfilter");
   if (!RaveField_addAttribute(qfield, attr)) {
     RAVE_ERROR0("CTFILTER: Could not set how/task");
     goto done;
   }

   /* Default parameter should have been selected already for both product
    and ct */
   for (yp=0; yp<ysizep; yp++) {
     for (xp=0; xp<xsizep; xp++) {
       rvt_p = Cartesian_getValue(product, xp, yp, &pval);

       if (rvt_p == RaveValueType_DATA) {
         xsurfp = Cartesian_getLocationX(product, xp);
         ysurfp = Cartesian_getLocationY(product, yp);

          /* Do we need the Z coordinate? */
         if (!Projection_transformx(pproj, cproj, xsurfp, ysurfp, 0.0,
             &xsurfc, &ysurfc, NULL)) {
           RAVE_ERROR0("CTFILTER: Error navigating data");
           goto done;
          }
         xc = Cartesian_getIndexX(ct, xsurfc);
         yc = Cartesian_getIndexY(ct, ysurfc);

         /* Out of bounds check */
         if ( (0<xc && xc<xsizec) && (0<yc && yc<ysizec) ) {
           /*rvt_c = */Cartesian_getValue(ct, xc, yc, &cval);

           /* Same code as used in NORDRAD2.
		          Probabilities of rain from Dybbroe et al. 2005 for AVHRR,
		          although we are using MSG */
           switch ((int)cval) {
           case 1:   /* Cloud-free land: <2.6% */
           case 2:   /* Cloud-free sea: <2.6% */
           case 3:   /* Snow/ice contaminated land: <2.6% */
           case 4:   /* Snow/ice contaminated sea: <2.6% */
             // case 5:   /* Very low cumuliform cloud: 2.1% */
             // case 6:   /* Very low stratiform cloud: 2.1% */
             // case 15:  /* Very thin cirrus cloud: 4.9% (bad in winter) */
             // case 19:  /* Fractional or sub-pixel cloud: 3.5% */
             Cartesian_setValue(product, xp, yp, undetect);
             RaveField_setValue(qfield, xp, yp, pval);
             break;
           default:
             break;
           }
         }
       }
     }
  }

  quantity = Cartesian_getDefaultParameter(product);
  param = Cartesian_getParameter(product, quantity);
  if (!CartesianParam_addQualityField(param, qfield)) {
    goto done;
  }

  ret = 1;
  done:
  RAVE_OBJECT_RELEASE(qfield);
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(pproj);
  RAVE_OBJECT_RELEASE(cproj);
  return ret;
}
