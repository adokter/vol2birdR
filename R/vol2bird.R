#' Calculate a vertical profile (`vp`) from a polar volume (`pvol`) file
#'
#' Calculates a vertical profile of biological scatterers (`vp`) from a polar
#' volume (`pvol`) file using the algorithm
#' [vol2bird](https://github.com/adokter/vol2bird/) (Dokter et al.
#' 2011 \doi{10.1098/rsif.2010.0116}).
#'
#' @param file Character (vector). Either a path to a single radar polar volume
#'   (`pvol`) file containing multiple scans/sweeps, or multiple paths to scan
#'   files containing a single scan/sweep. Or a single `pvol` object. The file data format should be either 1)
#'   [ODIM](https://github.com/adokter/vol2bird/blob/master/doc/OPERA2014_O4_ODIM_H5-v2.2.pdf)
#'    format, which is the implementation of the OPERA data information model in
#'   the [HDF5](https://support.hdfgroup.org/HDF5/) format, 2) a format
#'   supported by the [RSL
#'   library](https://trmm-fc.gsfc.nasa.gov/trmm_gv/software/rsl/) or 3) Vaisala
#'   IRIS (IRIS RAW) format.
#' @param vpfile Character. File name. When provided, writes a vertical profile
#'   file (`vpfile`) in the ODIM HDF5 format to disk.
#' @param pvolfile_out Character. File name. When provided, writes a polar
#'   volume (`pvol`) file in the ODIM HDF5 format to disk. Useful for converting
#'   RSL formats to ODIM, and for adding MistNet segmentation output.
#' @param config optional configuration object of class `Rcpp_Vol2BirdConfig`,
#' typically output from \link{vol2bird_config}
#' 
#' @seealso
#' * [vol2bird_config()]
vol2bird <- function(file, vpfile="", pvolfile_out="", config){
  for (filename in file) {
    assert_that(file.exists(filename))
  }
  if (!are_equal(vpfile, "")) {
    assert_that(is.writeable(dirname(vpfile)))
  }
  if(missing(config)){
    config <- Vol2BirdConfig$new()
  }
  assert_that(inherits(config,"Rcpp_Vol2BirdConfig"))
  
  processor<-Vol2Bird$new()
  processor$process(path.expand(file), config, vpfile, pvolfile_out)
}
