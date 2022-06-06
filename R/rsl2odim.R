#' Convert a NEXRAD polar volume file to an ODIM polar volume file
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
#' @param pvolfile_out Character. File name. When provided, writes a polar
#'   volume (`pvol`) file in the ODIM HDF5 format to disk. Useful for converting
#'   RSL formats to ODIM, and for adding MistNet segmentation output.
#' @param config optional configuration object of class `Rcpp_Vol2BirdConfig`,
#' typically output from \link{vol2bird_config}
#' @param verbose logical. When TRUE print profile output to console.
#' 
#' @seealso
#' * [vol2bird_config()]
#' @export
rsl2odim <- function(file, vpfile="", pvolfile_out="", config, verbose=TRUE){
  for (filename in file) {
    assert_that(file.exists(filename))
  }
  if (!are_equal(vpfile, "")) {
    assert_that(is.writeable(dirname(vpfile)))
  }
  if(missing(config)){
    config <- vol2bird_config()
  }
  assert_that(is.flag(verbose))
  
  if(config$useMistNet){
    assert_that(mistnet_exists(),msg="mistnet installation not found, install with `install_mistnet()`")
  }
  
  assert_that(inherits(config,"Rcpp_Vol2BirdConfig"))
  
  # make a copy of the configuration object
  # necessary to accommodate parallel processes, since the processor might change the
  # configuration object based on the input file characteristics
  config_copy <- vol2bird_config(config)
  
  processor<-Vol2Bird$new()
  processor$verbose <- verbose
  processor$rsl2odim(path.expand(file), config_copy, path.expand(pvolfile_out))
}
