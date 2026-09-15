# vol2birdR 1.3.2
* fixes build and compiler warnings as requested by CRAN
* make sure we use HDF5 1.12 API even when build system has hdf5>=2
* upgrade to most recent pytorch library
* fixes a configuration failure on Debian and Ubuntu with HDF52.x, 
 where the core HDF5 library is named `libhdf5_serial` rather than `libhdf5` (#162).
