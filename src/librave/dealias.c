/* --------------------------------------------------------------------
Copyright (C) 2011 Swedish Meteorological and Hydrological Institute, SMHI

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

/** Function for dealiasing weather radar winds.
 * @file
 * @author Gunther Haase, SMHI
 * @date 2013-02-06
 * @date 2018-04-11
 * - If the number of valid velocity pixels on a circle with radius (range bin)*rscale is below a threshold, no dealiasing is performed.
 * - Data representation of VRADH/V is no longer hard-coded.
 */

#include "dealias.h"

double max_vector (double *a, int n) {
  int i;
  double max = -32000;
  for (i=0; i<n; i++) {
    if (*(a+i) > max) max = *(a+i);
  }
  return max;
}


double min_vector (double *a, int n) {
  int i;
  double min = 32000;
  for (i=0; i<n; i++) {
    if (*(a+i) < min) min = *(a+i);
  }
  return min;
}

int dealiased_by_quantity(PolarScan_t* scan, const char* quantity) {
  PolarScanParam_t* param = NULL;
  RaveAttribute_t* attr = NULL;
  int ret = 0;
  int retda = 0;
  char* da;

  if (PolarScan_hasParameter(scan, quantity)) {
    param = PolarScan_getParameter(scan, quantity);
    attr = PolarScanParam_getAttribute(param, "how/dealiased");
    if (attr != NULL) {
      retda = RaveAttribute_getString(attr, &da);
      if (retda) {
        if (!strncmp(da, "True", (size_t)4)) {
          ret = 1;
        }
      }
    }
  }
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(param);
  return ret;
}

int dealiased(PolarScan_t* scan) {
  return dealiased_by_quantity(scan, "VRAD");
}

PolarScanParam_t* create_dealiased_parameter(PolarScan_t* scan, const char* quantity, const char* newquantity)
{
  PolarScanParam_t *result = NULL, *clone = NULL, *param = NULL;
  RaveAttribute_t* attr = NULL;
  RaveAttribute_t* dattr = RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_t* htattr = NULL;

  int nbins, nrays, i, j, n, m, ib, ir, eind, vo_valid;
  double gain, offset, nodata, undetect, NI, val, vm, min1, esum, u1, v1, min2, dmy, vmin_vo, vmax_vo, vmin_vd, vmax_vd;

  int *vrad_nodata = NULL, *vrad_undetect = NULL;
  double *x = NULL, *y = NULL, *vo = NULL, *vd = NULL, *uh = NULL, *vh = NULL, *e = NULL, *xt = NULL, *yt = NULL, *vt1 = NULL, *dv = NULL, *v = NULL;

  nbins = PolarScan_getNbins(scan);
  nrays = PolarScan_getNrays(scan);

  if (PolarScan_hasParameter(scan, quantity)) {
    param = PolarScan_getParameter(scan, quantity);
    if (param == NULL) {
      RAVE_ERROR1("Failed to get parameter %s", quantity);
      goto done;
    }
    clone = RAVE_OBJECT_CLONE(param);
    if (clone == NULL) {
      RAVE_ERROR1("Failed to get clone parameter %s", quantity);
      goto done;
    }
  } else {
    RAVE_INFO1("Scan has no suitable parameter %s", quantity);
    goto done;
  }

  PolarScanParam_setQuantity(clone, newquantity);

  gain = PolarScanParam_getGain(clone);
  offset = PolarScanParam_getOffset(clone);
  nodata = PolarScanParam_getNodata(clone);
  undetect = PolarScanParam_getUndetect(clone);
  attr = PolarScan_getAttribute(scan, "how/NI");  /* only location? */
  if (attr != NULL) {
    RaveAttribute_getDouble(attr, &NI);
  } else {
    NI = fabs(offset);
  }
  // number of rows
  m = floor (VAF/NI*VMAX);
  // number of columns
  n = NF;

  vrad_nodata = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(int));
  vrad_undetect = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(int));
  x = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
  y = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
  vo = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
  vd = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
  uh = RAVE_CALLOC ((size_t)(m*n), sizeof(double));
  vh = RAVE_CALLOC ((size_t)(m*n), sizeof(double));
  e = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
  xt = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
  yt = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
  vt1 = RAVE_CALLOC ((size_t)nrays, sizeof(double));
  dv = RAVE_CALLOC ((size_t)(MVA+1), sizeof(double));
  v = RAVE_CALLOC ((size_t)(MVA+1)*nrays, sizeof(double));

  if (vrad_nodata == NULL || vrad_undetect == NULL || x == NULL || y == NULL ||
      vo == NULL || vd == NULL || uh == NULL || vh == NULL || e == NULL ||
      xt == NULL || yt == NULL || vt1 == NULL || dv == NULL || v == NULL) {
    RAVE_ERROR0("Memory allocation error");
    goto done;
  }

  // read and re-arrange data (ray -> bin)
  for (ir=0; ir<nrays; ir++) {
    for (ib=0; ib<nbins; ib++) {
      PolarScanParam_getValue(clone, ib, ir, &val);
      if (val==nodata) *(vrad_nodata+ir+ib*nrays) = 1;
      if (val==undetect) *(vrad_undetect+ir+ib*nrays) = 1;
      if ((val!=nodata) && (val!=undetect)) *(vo+ir+ib*nrays) = offset+gain*val;
      else {
        *(vo+ir+ib*nrays) = NAN;
        *(vd+ir+ib*nrays) = NAN;
      }

      // map measured data to 3D
      *(x+ir+ib*nrays) = NI/M_PI * cos(*(vo+ir+ib*nrays)*M_PI/NI);
      *(y+ir+ib*nrays) = NI/M_PI * sin(*(vo+ir+ib*nrays)*M_PI/NI);
    }
  }

  for (i=0; i<n; i++) {
    for (j=0; j<m; j++) {
      *(uh+i*m+j) = NI/VAF*(j+1) * sin(2*M_PI/NF*i);
      *(vh+i*m+j) = NI/VAF*(j+1) * cos(2*M_PI/NF*i);
    }
  }

  for (ir=0; ir<nrays; ir++) {
    for (i=0; i<n; i++) {
      for (j=0; j<m; j++) {
        vm = *(uh+i*m+j) * sin(360./nrays*ir*DEG2RAD) +
             *(vh+i*m+j) * cos(360./nrays*ir*DEG2RAD);
        *(xt+i*m+j+ir*m*n) = NI/M_PI * cos(vm*M_PI/NI);
        *(yt+i*m+j+ir*m*n) = NI/M_PI * sin(vm*M_PI/NI);
      }
    }
  }

  for (ib=0; ib<nbins; ib++) {
    for (ir=0; ir<nrays; ir++) {
      for (i=0; i<m*n; i++) {
        *(e+i+ir*m*n) = fabs(*(xt+i+ir*m*n)-*(x+ir+ib*nrays)) +
                        fabs(*(yt+i+ir*m*n)-*(y+ir+ib*nrays));
      }
    }

    min1 = 1e32;
    eind = 0;
    u1 = 0;
    v1 = 0;
    for (i=0; i<m*n; i++) {
      esum = 0;
      for (ir=0; ir<nrays; ir++) {
        if (!isnan(*(e+i+ir*m*n))) {
          esum = esum + *(e+i+ir*m*n);
        }
      }
      if (esum<min1) {
        min1 = esum;
        eind = i;
      }
      u1 = *(uh+eind);
      v1 = *(vh+eind);
    }

    for (ir=0; ir<nrays; ir++) {
      *(vt1+ir) = u1*sin(360./nrays*ir*DEG2RAD) + v1*cos(360./nrays*ir*DEG2RAD);
    }

    for (i=0; i<MVA+1; i++) {
      *(dv+i) = NI*(2*i-MVA);
    }

    for (ir=0; ir<nrays; ir++) {
      for (i=0; i<MVA+1; i++) {
        *(v+i+ir*(MVA+1)) = *(dv+i);
      }
    }

    vo_valid=0;
    for (ir=0; ir<nrays; ir++) {
      min2 = 1e32;
      dmy = 0;
      for (i=0; i<MVA+1; i++) {
        dmy = fabs(*(v+i+ir*(MVA+1))-(*(vt1+ir)-*(vo+ir+ib*nrays)));
        if ((dmy<min2) && (!isnan(dmy))) {
          *(vd+ir+ib*nrays) = *(vo+ir+ib*nrays) + *(dv+i);
          min2 = dmy;
        }
      }
      // Number of valid velocity pixels on a circle with radius ib*rscale
      if (!isnan(*(vo+ir+ib*nrays))) {
        vo_valid = vo_valid+1;
      }
    }
    // If the number of valid velocity pixels on a circle with radius ib*rscale is below a threshold, no dealiasing is performed.
    if (vo_valid<FRAY*nrays) {
      for (ir=0; ir<nrays; ir++) {
        *(vd+ir+ib*nrays) = *(vo+ir+ib*nrays);
      }
    }
  }

  // Data representation of VRADH/V
  RaveDataType datatype = PolarScanParam_getDataType(param);
  int typesize = get_ravetype_size(datatype);
  int nbitval = pow(2,typesize*8);
  // Maximum and minimum observed/dealiased velocities
  vmax_vo = max_vector(vo, nrays*nbins);
  vmin_vo = min_vector(vo, nrays*nbins);
  vmax_vd = max_vector(vd, nrays*nbins);
  vmin_vd = min_vector(vd, nrays*nbins);
  // Rescale dealiased velocities if they are outside NI
  if (vmin_vd<vmin_vo || vmax_vd>vmax_vo) {
    gain = (vmax_vd-vmin_vd)/(nbitval-3);
    offset = vmin_vd-gain-EPSILON;
  }
  PolarScanParam_setOffset (clone, offset);
  PolarScanParam_setGain (clone, gain);

  for (ir=0 ; ir<nrays ; ir++) {
    for (ib=0; ib<nbins; ib++) {
      *(vd+ir+ib*nrays) = (*(vd+ir+ib*nrays)-offset)/gain;
      if (*(vrad_nodata+ir+ib*nrays)) *(vd+ir+ib*nrays) = nodata;
      if (*(vrad_undetect+ir+ib*nrays)) *(vd+ir+ib*nrays) = undetect;

      PolarScanParam_setValue (clone, ib, ir, *(vd+ir+ib*nrays));
    }
  }

  RaveAttribute_setName(dattr, "how/dealiased");
  RaveAttribute_setString(dattr, "True");
  PolarScanParam_addAttribute(clone, dattr);

  // We don't report if we get any kind of memory error etc...
  htattr = RaveAttributeHelp_createString("how/task", "se.smhi.detector.dealias");
  if (htattr != NULL) {
    PolarScanParam_addAttribute(clone, htattr);
  }

  result = RAVE_OBJECT_COPY(clone);
done:
  RAVE_FREE(vrad_nodata);
  RAVE_FREE(vrad_undetect);
  RAVE_FREE(x);
  RAVE_FREE(y);
  RAVE_FREE(vo);
  RAVE_FREE(vd);
  RAVE_FREE(uh);
  RAVE_FREE(vh);
  RAVE_FREE(e);
  RAVE_FREE(xt);
  RAVE_FREE(yt);
  RAVE_FREE(vt1);
  RAVE_FREE(dv);
  RAVE_FREE(v);
  RAVE_OBJECT_RELEASE(param);
  RAVE_OBJECT_RELEASE(attr);
  RAVE_OBJECT_RELEASE(clone);
  RAVE_OBJECT_RELEASE(htattr);
  RAVE_OBJECT_RELEASE(dattr);
  return result;
}

int dealias_scan_by_quantity(PolarScan_t* scan, const char* quantity, double emax)
{
  PolarScanParam_t* param = NULL;
  RaveAttribute_t* attr = NULL;
  RaveAttribute_t* dattr = RAVE_OBJECT_NEW(&RaveAttribute_TYPE);
  RaveAttribute_t* htattr = NULL;

  int nbins, nrays, i, j, n, m, ib, ir, eind, vo_valid;
  int retval = 0;
  double elangle, gain, offset, nodata, undetect, NI, val, vm, min1, esum, u1, v1, min2, dmy, vmin_vo, vmax_vo, vmin_vd, vmax_vd;

  nbins = PolarScan_getNbins(scan);
  nrays = PolarScan_getNrays(scan);
  elangle = PolarScan_getElangle(scan);

  if ( (PolarScan_hasParameter(scan, quantity)) && (!dealiased_by_quantity(scan, quantity)) ) {
    if (elangle*RAD2DEG<=emax) {
      param = PolarScan_getParameter(scan, quantity);
      gain = PolarScanParam_getGain(param);
      offset = PolarScanParam_getOffset(param);
      nodata = PolarScanParam_getNodata(param);
      undetect = PolarScanParam_getUndetect(param);
      attr = PolarScan_getAttribute(scan, "how/NI");  /* only location? */
      if (attr != NULL) {
        RaveAttribute_getDouble(attr, &NI);
      } else {
        NI = fabs(offset);
      }
      // number of rows
      m = floor (VAF/NI*VMAX);
      // number of columns
      n = NF;

      int *vrad_nodata = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(int));
      int *vrad_undetect = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(int));
      double *x = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
      double *y = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
      double *vo = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
      double *vd = RAVE_CALLOC ((size_t)nrays*nbins, sizeof(double));
      double *uh = RAVE_CALLOC ((size_t)(m*n), sizeof(double));
      double *vh = RAVE_CALLOC ((size_t)(m*n), sizeof(double));
      double *e = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
      double *xt = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
      double *yt = RAVE_CALLOC ((size_t)(m*n*nrays), sizeof(double));
      double *vt1 = RAVE_CALLOC ((size_t)nrays, sizeof(double));
      double *dv = RAVE_CALLOC ((size_t)(MVA+1), sizeof(double));
      double *v = RAVE_CALLOC ((size_t)(MVA+1)*nrays, sizeof(double));

      // read and re-arrange data (ray -> bin)
      for (ir=0; ir<nrays; ir++) {
        for (ib=0; ib<nbins; ib++) {
          PolarScanParam_getValue(param, ib, ir, &val);
          if (val==nodata) *(vrad_nodata+ir+ib*nrays) = 1;
          if (val==undetect) *(vrad_undetect+ir+ib*nrays) = 1;
          if ((val!=nodata) && (val!=undetect)) *(vo+ir+ib*nrays) = offset+gain*val;
          else {
            *(vo+ir+ib*nrays) = NAN;
            *(vd+ir+ib*nrays) = NAN;
          }

          // map measured data to 3D
          *(x+ir+ib*nrays) = NI/M_PI * cos(*(vo+ir+ib*nrays)*M_PI/NI);
          *(y+ir+ib*nrays) = NI/M_PI * sin(*(vo+ir+ib*nrays)*M_PI/NI);
        }
      }

      for (i=0; i<n; i++) {
        for (j=0; j<m; j++) {
          *(uh+i*m+j) = NI/VAF*(j+1) * sin(2*M_PI/NF*i);
          *(vh+i*m+j) = NI/VAF*(j+1) * cos(2*M_PI/NF*i);
        }
      }

      for (ir=0; ir<nrays; ir++) {
        for (i=0; i<n; i++) {
          for (j=0; j<m; j++) {
            vm = *(uh+i*m+j) * sin(360./nrays*ir*DEG2RAD) +
                 *(vh+i*m+j) * cos(360./nrays*ir*DEG2RAD);
            *(xt+i*m+j+ir*m*n) = NI/M_PI * cos(vm*M_PI/NI);
            *(yt+i*m+j+ir*m*n) = NI/M_PI * sin(vm*M_PI/NI);
          }
        }
      }

      for (ib=0; ib<nbins; ib++) {
        for (ir=0; ir<nrays; ir++) {
          for (i=0; i<m*n; i++) {
            *(e+i+ir*m*n) = fabs(*(xt+i+ir*m*n)-*(x+ir+ib*nrays)) +
                            fabs(*(yt+i+ir*m*n)-*(y+ir+ib*nrays));
          }
        }

        min1 = 1e32;
        eind = 0;
        u1 = 0;
        v1 = 0;
        for (i=0; i<m*n; i++) {
          esum = 0;
          for (ir=0; ir<nrays; ir++) {
            if (!isnan(*(e+i+ir*m*n))) {
              esum = esum + *(e+i+ir*m*n);
            }
          }
          if (esum<min1) {
            min1 = esum;
            eind = i;
          }
          u1 = *(uh+eind);
          v1 = *(vh+eind);
        }

        for (ir=0; ir<nrays; ir++) {
          *(vt1+ir) = u1*sin(360./nrays*ir*DEG2RAD) + v1*cos(360./nrays*ir*DEG2RAD);
        }

        for (i=0; i<MVA+1; i++) {
          *(dv+i) = NI*(2*i-MVA);
        }

        for (ir=0; ir<nrays; ir++) {
          for (i=0; i<MVA+1; i++) {
            *(v+i+ir*(MVA+1)) = *(dv+i);
          }
        }

        vo_valid=0;
        for (ir=0; ir<nrays; ir++) {
          min2 = 1e32;
          dmy = 0;
          for (i=0; i<MVA+1; i++) {
            dmy = fabs(*(v+i+ir*(MVA+1))-(*(vt1+ir)-*(vo+ir+ib*nrays)));
            if ((dmy<min2) && (!isnan(dmy))) {
              *(vd+ir+ib*nrays) = *(vo+ir+ib*nrays) + *(dv+i);
              min2 = dmy;
            }
          }
          // Number of valid velocity pixels on a circle with radius ib*rscale
          if (!isnan(*(vo+ir+ib*nrays))) {
            vo_valid = vo_valid+1;
          }
        }
        // If the number of valid velocity pixels on a circle with radius ib*rscale is below a threshold, no dealiasing is performed.
        if (vo_valid<FRAY*nrays) {
          for (ir=0; ir<nrays; ir++) {
            *(vd+ir+ib*nrays) = *(vo+ir+ib*nrays);
          }
        }
      }

      // Data representation of VRADH/V
      RaveDataType datatype = PolarScanParam_getDataType(param);
      int typesize = get_ravetype_size(datatype);
      int nbitval = pow(2,typesize*8);
      // Maximum and minimum observed/dealiased velocities
      vmax_vo = max_vector(vo, nrays*nbins);
      vmin_vo = min_vector(vo, nrays*nbins);
      vmax_vd = max_vector(vd, nrays*nbins);
      vmin_vd = min_vector(vd, nrays*nbins);
      // Rescale dealiased velocities if they are outside NI
      if (vmin_vd<vmin_vo || vmax_vd>vmax_vo) {
        gain = (vmax_vd-vmin_vd)/(nbitval-3);
        offset = vmin_vd-gain-EPSILON;
      }
      PolarScanParam_setOffset (param, offset);
      PolarScanParam_setGain (param, gain);

      for (ir=0 ; ir<nrays ; ir++) {
        for (ib=0; ib<nbins; ib++) {
          *(vd+ir+ib*nrays) = (*(vd+ir+ib*nrays)-offset)/gain;
          if (*(vrad_nodata+ir+ib*nrays)) *(vd+ir+ib*nrays) = nodata;
          if (*(vrad_undetect+ir+ib*nrays)) *(vd+ir+ib*nrays) = undetect;

          PolarScanParam_setValue (param, ib, ir, *(vd+ir+ib*nrays));
        }
      }

      RaveAttribute_setName(dattr, "how/dealiased");
      RaveAttribute_setString(dattr, "True");
      PolarScanParam_addAttribute(param, dattr);

      // We don't report if we get any kind of memory error etc...
      htattr = RaveAttributeHelp_createString("how/task", "se.smhi.detector.dealias");
      if (htattr != NULL) {
        PolarScanParam_addAttribute(param, htattr);
      }

      RAVE_FREE(vrad_nodata);
      RAVE_FREE(vrad_undetect);
      RAVE_FREE(x);
      RAVE_FREE(y);
      RAVE_FREE(vo);
      RAVE_FREE(vd);
      RAVE_FREE(uh);
      RAVE_FREE(vh);
      RAVE_FREE(e);
      RAVE_FREE(xt);
      RAVE_FREE(yt);
      RAVE_FREE(vt1);
      RAVE_FREE(dv);
      RAVE_FREE(v);
      RAVE_OBJECT_RELEASE(param);
      RAVE_OBJECT_RELEASE(attr);
    }
    retval = 1;

  } else {
    retval = 0;  /* No quantity or already dealiased */
  }
  RAVE_OBJECT_RELEASE(htattr);
  RAVE_OBJECT_RELEASE(dattr);
  return retval;
}

int dealias_scan(PolarScan_t* scan) {
  return dealias_scan_by_quantity(scan, "VRAD", EMAX);
}

int dealias_pvol_by_quantity(PolarVolume_t* inobj, const char* quantity, double emax)
{
  PolarScan_t* scan = NULL;
  int is, nscans;
  int retval = 0;  /* Using this feels a bit artificial */

  nscans = PolarVolume_getNumberOfScans(inobj);

  for (is=0; is < nscans; is++) {
    scan = PolarVolume_getScan(inobj, is);
    retval = dealias_scan_by_quantity(scan, quantity, emax);
    RAVE_OBJECT_RELEASE(scan);
  }

  return retval;

}

int dealias_pvol(PolarVolume_t* inobj) {
  return dealias_pvol_by_quantity(inobj, "VRAD", EMAX);
}
