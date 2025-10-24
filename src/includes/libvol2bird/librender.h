#include "cartesian.h"
#include "polarvolume.h"

Cartesian_t* polarVolumeToCartesian(PolarVolume_t* pvol, long dim, long res, double init);

double distance2height(double distance,double elev);

double distance2range(double distance,double elev);

double range2distance(double range,double elev);

double range2height(double range,double elev);

double beamProfile(double height, double elev, double range, double antenna, double beamAngle);

double*** init3DTensor(int dim1, int dim2, int dim3, double init);

float**** create4DTensor(float *array, int dim1, int dim2, int dim3, int dim4);

int polarVolumeTo3DTensor(PolarVolume_t* pvol, double ****tensor, int dim, long res, int nParam);

int fill3DTensor(double ***tensor, RaveObjectList_t* list, int dim1, int dim2, int dim3);

float* flatten3DTensor(double ***tensor, int dim1, int dim2, int dim3);

void free3DTensor(double ***tensor, int dim1, int dim2);

void free4DTensor(float ****tensor, int dim1, int dim2, int dim3);

#ifdef MISTNET
int run_mistnet(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size);
#endif

PolarScan_t* PolarVolume_getScanClosestToElevation_vol2bird(PolarVolume_t* volume, double elev);

int addTensorToPolarVolume(PolarVolume_t* pvol, float ****tensor, int dim1, int dim2, int dim3, int dim4, long res);

int addClassificationToPolarVolume(PolarVolume_t* pvol, float ****tensor, int dim1, int dim2, int dim3, int dim4, long res);
