/**
 * Transformation routines for going between projections
 * @file
 * @author Anders Henja (Swedish Meteorological and Hydrological Institute, SMHI)
 * @date 2009-08-05
 */
#include "rave_transform.h"
#include "raveutil.h"
#include "limits.h"
#include "float.h"
#include "rave_alloc.h"
#include <math.h>
#include <stdio.h>
#include "projection.h"

/*@{ Private functions */
/**
 * Returns x^3 - 2x^2 + 1
 * @param[in] x the value
 * @return the result
 */
static double cubf01(double x)
{
  double y = (x * x * x - 2* x * x + 1);
  return y;
}

/**
 * Returns - x^3 + 5x^2 - 8x + 4
 * @param[in] x the value
 * @return the result
 */
static double cubf12(double x)
{
  double y = (-x * x * x + 5* x * x - 8* x + 4);
  return y;
}

/**
 * If |x| < 1 => Returns @ref cubf01.
 * If |x| <= 2 => Returns @ref cubf12.
 * Otherwise returns 0.
 * @param[in] x the value
 * @return the result
 */
static double cubf2(double x)
{
  if (x < 0)
    x = -x; /* No sign */
  if (x < 1)
    return cubf01(x);
  if (x <= 2)
    return cubf12(x);
  return 0;
}
/*@} End of Private functions */

/*@{ Interface functions */

double get_array_item_2d(
  unsigned char* in_a, int x, int y, RaveDataType type, int stride_xsize)
{
  double ret = 0;
  switch (type) {
  case RaveDataType_CHAR: {
    char *a = (char *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_UCHAR: {
    unsigned char *a = (unsigned char *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_SHORT: {
    short *a = (short *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_USHORT: {
    unsigned short *a = (unsigned short *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_INT: {
    int *a = (int *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_UINT: {
    unsigned int *a = (unsigned int *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_LONG: {
    long *a = (long *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_ULONG: {
    unsigned long *a = (unsigned long *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_FLOAT: {
    float *a = (float *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  case RaveDataType_DOUBLE: {
    double *a = (double *) in_a;
    ret = a[y * stride_xsize + x];
    break;
  }
  default:
    /* Ignored for now => 0.0 */
    printf("get_array_item_2d: Unsupported type: '%d'\n", type);
  }
  return ret;
}

void set_array_item_2d(
  unsigned char* out_a, int x, int y, double v, RaveDataType type, int stride_xsize)
{
  switch (type) {
  case RaveDataType_CHAR: {
    char *a = (char *) out_a;
    int c = mytrunc(v);

    if (c < -128)
      c = -128;
    if (c > 127)
      c = 127;
    a[y * stride_xsize + x] = c;
    break;
  }
  case RaveDataType_UCHAR: {
    unsigned char *a = (unsigned char *) out_a;
    unsigned char c;

    if (v < 0) /* Oops: Not allowed!*/
      v = 0;
    if (v > 255)
      v = 255;
    c = mytrunc(v);
    a[y * stride_xsize + x] = c;
    break;
  }
  case RaveDataType_SHORT: {
    short *a = (short *) out_a;
    int c = mytrunc(v);
    if (c < SHRT_MIN) /* Oops: Not allowed!*/
      c = SHRT_MIN;
    if (c > SHRT_MAX)
      c = SHRT_MAX;
    a[y * stride_xsize + x] = c;
    break;
  }
  case RaveDataType_USHORT: {
    unsigned short *a = (unsigned short *) out_a;
    if (v < 0) /* Oops: Not allowed!*/
      v = 0;
    if (v > USHRT_MAX)
      v = USHRT_MAX;
    a[y * stride_xsize + x] = (unsigned short)v;
    break;
  }
  case RaveDataType_INT: {
    int *a = (int *) out_a;
    int c;
    if (v > INT_MAX)
      v = INT_MAX;
    if (v < INT_MIN)
      v = INT_MIN;
    c = mytrunc(v);
    a[y * stride_xsize + x] = c;
    break;
  }
  case RaveDataType_UINT: {
    unsigned int *a = (unsigned int *) out_a;
    if (v < 0)
      v = 0;
    if (v > UINT_MAX)
      v = UINT_MAX;
    a[y * stride_xsize + x] = (unsigned int)v;
    break;
  }
  case RaveDataType_LONG: {
    long *a = (long *) out_a;
    long c;
    if (v > LONG_MAX)
      v = LONG_MAX;
    if (v < LONG_MIN)
      v = LONG_MIN;
    c = v; /* Should work on 64bit boxes after above preparations. */
    a[y * stride_xsize + x] = c;
    break;
  }
  case RaveDataType_ULONG: {
    unsigned long *a = (unsigned long *) out_a;
    if (v < 0)
      v = 0;
    if (v > ULONG_MAX)
      v = ULONG_MAX;
    a[y * stride_xsize + x] = (unsigned long)v;
    break;
  }
  case RaveDataType_FLOAT: {
    float *a = (float *) out_a;
    if (v > FLT_MAX)
      v = FLT_MAX;
    if (v < FLT_MIN)
      v = FLT_MIN;
    a[y * stride_xsize + x] = v;
    break;
  }
  case RaveDataType_DOUBLE: {
    double *a = (double *) out_a;
    a[y * stride_xsize + x] = v;
    break;
  }
  default:
    /* Ignored for now => 0.0 */
    printf("set_array_item_2d: Unsupported type: '%d'\n", type);
  }
}

double get_array_item_3d(RavePolarVolume* pvol, int e, int r, int a)
{
  return get_array_item_2d((unsigned char*) pvol->fields[e].src, r, a,
                           pvol->fields[e].type, pvol->fields[e].stride);
}

void set_array_item_3d(RavePolarVolume* pvol, int e, int r, int a, double v)
{
  set_array_item_2d((unsigned char*) pvol->fields[e].src, r, a, v,
                    pvol->fields[e].type, pvol->fields[e].stride);
}

TransformWeight* init_tw(int noi)
{
  TransformWeight* v;

  v = RAVE_MALLOC(sizeof(TransformWeight));

  if (!v)
    return NULL;

  if (noi > 0) {
    v->nodata = 0.0;
    v->noecho = 0.0;
    v->weights = NULL;
    v->weights = RAVE_MALLOC(sizeof(RaveWeight2D) * noi);
    v->weightsn = noi;
    v->total_wsum = 0.0;
    v->scale_weights = 1;
  } else {
    v->nodata = 0.0;
    v->noecho = 0.0;
    v->weights = NULL;
    v->weightsn = 0;
    v->total_wsum = 0.0;
    v->scale_weights = 1;
  }
  return v;
}

void free_tw(TransformWeight* v)
{
  if (v) {
    if (v->weights) {
      RAVE_FREE(v->weights);
    }
    RAVE_FREE(v);
  }
}

static int internal_transform_proj(PJ* p1, PJ* p2, double* x, double* y)
{
#ifdef USE_PROJ4_API
  double tx=*x;
  double ty=*y;
  pj_transform(p1, p2, 1, 1, &tx, &ty, NULL);
  *x=tx;
  *y=ty;
  return 1;
#else
  PJ* pj = NULL;
  PJ_CONTEXT* context = NULL;
  PJ_COORD inc,outc;

  inc.uv.u = *x;
  inc.uv.v = *y;

  context = proj_context_create();
  if (context == NULL) {
    fprintf(stderr, "Failed to create context");
    return 0;
  }
  proj_log_level(context, Projection_getDebugLevel());
  pj = proj_create_crs_to_crs(context, proj_pj_info(p1).definition, proj_pj_info(p2).definition, NULL);
  if (pj == NULL) {
    fprintf(stderr, "Failed to create crs pj");
    proj_context_destroy(context);
    return 0;
  }

  outc = proj_trans(pj, PJ_FWD, inc);
  *x = outc.uv.u;
  *y = outc.uv.v;
  proj_destroy(pj);
  proj_context_destroy(context);
  return 1;
#endif
}

TransformWeight* get_nearest_weights_2d(
  int x, int y, UV here_s, RaveTransform2D* tw)
{
  int gx, gy;
  TransformWeight* retw = NULL;
  double vv;

  double tx = here_s.u;
  double ty = here_s.v;

  /* transform between projections */
  if (!internal_transform_proj(tw->outpj, tw->inpj, &tx, &ty)) {
    return NULL;
  }

  /* trunc to nearest pixel */
  gx = mytrunc((tx - tw->inUL.u) / tw->inxscale);
  gy = mytrunc((tw->inUL.v - ty) / tw->inyscale);

  if ((gx >= 0 && gx < tw->inxmax) && (gy >= 0 && gy < tw->inymax)) {
    vv = get_array_item_2d(tw->data, gx, gy, tw->type, tw->stride_xsize);
    if (vv == tw->nodata) {
      return NULL;
    }
    retw = init_tw(1);
    retw->nodata = tw->nodata;
    retw->noecho = tw->noecho;
    retw->weights[0].weight = 1.0;
    retw->weights[0].value = vv;
    retw->total_wsum = 1.0;
    retw->scale_weights = 0;
    retw->weights[0].x = gx;
    retw->weights[0].y = gy;
  }
  return retw;
}

TransformWeight* get_bilinear_weights_2d(
  int x, int y, UV here_s, RaveTransform2D* tw)
{
  int gx, gy, tx, ty, ox, oy;
  int nhits = 0;
  double exactx, exacty, diffx, diffy;
  TransformWeight* retw = init_tw(4);
  double cx = here_s.u;
  double cy = here_s.v;

  retw->scale_weights = 0;
  retw->nodata = tw->nodata;
  retw->noecho = tw->noecho;

  /* transform between projections */
  if (!internal_transform_proj(tw->outpj, tw->inpj, &cx, &cy)) {
    return NULL;
  }

  /* trunc to nearest pixel */
  exactx = (cx - tw->inUL.u) / tw->inxscale;
  exacty = (tw->inUL.v - cy) / tw->inyscale;
  gx = mytrunc(exactx);
  gy = mytrunc(exacty);
  diffx = exactx - gx;
  diffy = exacty - gy;

  /* Just multiply distance x by y */
  retw->weights[0].weight = (1 - diffx) * (1 - diffy);
  retw->weights[1].weight = (1 - diffx) * diffy;
  retw->weights[2].weight = diffx * (1 - diffy);
  retw->weights[3].weight = diffx * diffy;

  /* Look around */
  for (ox = 0; ox < 2; ox++) {
    tx = gx + ox;
    for (oy = 0; oy < 2; oy++) {
      int idx = ox * 2 + oy; /* For array cache */
      double item;

      ty = gy + oy;

      retw->weights[idx].x = tx;
      retw->weights[idx].y = ty;

      if ((tx >= 0 && tx < tw->inxmax) && (ty >= 0 && ty < tw->inymax)) {
        item = get_array_item_2d(tw->data, tx, ty, tw->type, tw->stride_xsize);
        if (item != tw->nodata) {
          retw->weights[idx].value = item;
          nhits++;
          retw->total_wsum += retw->weights[idx].weight;
        } else {
          retw->weights[idx].value = tw->nodata;
        }
      } else {
        retw->weights[idx].value = tw->nodata;
      }
    }
  }

  if (!nhits) {
    free_tw(retw);
    retw = NULL;
  }

  return retw;
}

TransformWeight* get_cubic_weights_2d(int x, int y, UV here_s,
  RaveTransform2D *tw)
{
  int gx, gy, tx, ty, ox, oy;
  int nhits = 0;
  double exactx, exacty, diffx, diffy;
  TransformWeight* retw = init_tw(16);
  double cx = here_s.u;
  double cy = here_s.v;

  retw->scale_weights = 0;
  retw->nodata = tw->nodata;
  retw->noecho = tw->noecho;

  /* transform between projections */
  if (!internal_transform_proj(tw->outpj, tw->inpj, &cx, &cy)) {
    return NULL;
  }

  /* trunc to nearest pixel */
  exactx = (cx - tw->inUL.u) / tw->inxscale;
  exacty = (tw->inUL.v - cy) / tw->inyscale;
  gx = mytrunc(exactx);
  gy = mytrunc(exacty);
  diffx = exactx - gx;
  diffy = exacty - gy;

  /* Look around */
  for (ox = -1; ox < 3; ox++) {
    tx = gx + ox;
    for (oy = -1; oy < 3; oy++) {
      int idx = (ox + 1) * 4 + (oy + 1); /* For array cache */
      double offx, offy, item;

      ty = gy + oy;
      offx = ox - diffx;
      offy = oy - diffy;
      retw->weights[idx].weight = cubf2(offx) * cubf2(offy);
      retw->weights[idx].x = tx;
      retw->weights[idx].y = ty;

      if ((tx >= 0 && tx < tw->inxmax) && (ty >= 0 && ty < tw->inymax)) {
        item = get_array_item_2d(tw->data, tx, ty, tw->type, tw->stride_xsize);

        if (item != tw->nodata) {
          nhits++;
          retw->weights[idx].value = item;
          retw->total_wsum += retw->weights[idx].weight;
        } else {
          retw->weights[idx].value = tw->nodata;
        }
      } else {
        retw->weights[idx].value = tw->nodata;
      }
    }
  }

  if (!nhits) {
    free_tw(retw);
    retw = NULL;
  }

  return retw;
}

TransformWeight* get_cressman_weights_2d(int x, int y, UV here_s,
  RaveTransform2D *tw)
{
  int ox, oy, offx, offy, minx, miny, maxx, maxy, nvals;
  UV there_s, near_s, corner_s, cressmin, cressmax;
  int idx = 0;
  double item;
  TransformWeight* retw;
  there_s.u = here_s.u;
  there_s.v = here_s.v;

  /* transform between projections */
  if (!internal_transform_proj(tw->outpj, tw->inpj, &there_s.u, &there_s.v)) {
    return NULL;
  }

  /* Now identify "worst cases" for the area: */
  cressmin = cressmax = there_s;
  for (offx = -1; offx <= 1; offx += 2) {
    for (offy = -1; offy <= 1; offy += 2) {
      near_s.u = here_s.u + offx * tw->R;
      near_s.v = here_s.v + offy * tw->R;
#ifdef USE_PROJ4_API
      {
        UV near = pj_inv(near_s, tw->outpj);
        corner_s = pj_fwd(near, tw->inpj);
      }
#else
      {
        PJ_COORD inpc, outpc, outcorner;
        inpc.uv = near_s;
        outpc = proj_trans(tw->outpj, PJ_INV, inpc);
        outcorner = proj_trans(tw->inpj, PJ_FWD, outpc);
        corner_s = outcorner.uv;
      }
#endif
      if (corner_s.u < cressmin.u)
        cressmin.u = corner_s.u;
      if (corner_s.v < cressmin.v)
        cressmin.v = corner_s.v;
      if (corner_s.u > cressmax.u)
        cressmax.u = corner_s.u;
      if (corner_s.v > cressmax.v)
        cressmax.v = corner_s.v;
    }
  }

  /* Find index bounding box: */
  minx = floor((cressmin.u - tw->inUL.u) / tw->inxscale);
  miny = floor((tw->inUL.v - cressmax.v) / tw->inyscale);
  maxx = ceil((cressmax.u - tw->inUL.u) / tw->inxscale);
  maxy = ceil((tw->inUL.v - cressmin.v) / tw->inyscale);
  nvals = (maxx - minx + 1) * (maxy - miny + 1);

  retw = init_tw(nvals);

  retw->nodata = tw->nodata;
  retw->noecho = tw->noecho;

  /* Look around */
  for (ox = minx; ox <= maxx; ox++) {
    double xmeters = (ox * tw->inxscale + tw->inUL.u) - there_s.u;
    for (oy = miny; oy < maxy; oy++) {
      double ymeters = (tw->inUL.v - oy * tw->inyscale) - there_s.v;
      double r = sqrt(ymeters * ymeters + xmeters * xmeters);

      if (r <= tw->R) { /* Compute weight */
        if ((ox >= 0 && ox < tw->inxmax) && (oy >= 0 && oy < tw->inymax)) {

          item = get_array_item_2d(tw->data, ox, oy, tw->type, tw->stride_xsize);

          if (item != tw->nodata) {
            retw->weights[idx].value = item;
            retw->weights[idx].x = ox;
            retw->weights[idx].y = oy;
            retw->weights[idx].distance = r;

            if (tw->method == CRESSMAN)
              retw->weights[idx].weight = (tw->R * tw->R - r * r) / (tw->R
                  * tw->R + r * r);
            else if (tw->method == UNIFORM)
              retw->weights[idx].weight = 1L;
            else
              retw->weights[idx].weight = 1.0 - r / tw->R;

            retw->total_wsum += retw->weights[idx].weight;
            idx++;
          }
        }
      }
    }
  }

  retw->weightsn = idx;

  return retw;
}

TransformWeight* get_weights_2d(int x, int y, UV here_s, RaveTransform2D* tw)
{
  if (tw->method == CRESSMAN || tw->method == UNIFORM || tw->method == INVERSE) {
    return get_cressman_weights_2d(x, y, here_s, tw);
  } else if (tw->method == CUBIC) {
    return get_cubic_weights_2d(x, y, here_s, tw);
  } else if (tw->method == BILINEAR) {
    return get_bilinear_weights_2d(x, y, here_s, tw);
  } else if (tw->method == NEAREST) {
    return get_nearest_weights_2d(x, y, here_s, tw);
  } else {
    printf("UNSUPPORTED INTERPOLATION METHOD %d\n", tw->method);
    return NULL;
  }
}

double compute_weights_2d(TransformWeight *tw)
{
  double v = 0.0;
  int i;
  if (tw->scale_weights && tw->total_wsum == 0.0) {
    printf("total weight sum was zero, handle this outside\n");
    return 0.0;
  }

  for (i = 0; i < tw->weightsn; i++) {
    if (tw->weights[i].value != tw->nodata) {
      if (tw->scale_weights)
        v += tw->weights[i].value * tw->weights[i].weight / tw->total_wsum;
      else
        v += tw->weights[i].value * tw->weights[i].weight;
    }
  }

  return v;
}

/*@} End of Interface functions */
