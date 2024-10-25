#' Calculate a vertical profile (`vp`) from a polar volume (`pvol`) file
#'
#' Calculates a vertical profile of biological scatterers (`vp`) from a polar
#' volume (`pvol`) file using the algorithm
#' [vol2bird](https://github.com/adokter/vol2bird/) (Dokter et al.
#' 2011 \doi{10.1098/rsif.2010.0116}).
#'
#' @param file Character (vector). Either a path to a single radar polar volume
#'   (`pvol`) file containing multiple scans/sweeps, or multiple paths to scan
#'   files containing a single scan/sweep. The file data format should be either 1)
#'   [ODIM](https://github.com/adokter/vol2bird/blob/master/doc/OPERA2014_O4_ODIM_H5-v2.2.pdf)
#'    format, which is the implementation of the OPERA data information model in
#'   the [HDF5](https://www.hdfgroup.org/solutions/hdf5/) format, 2) NEXRAD format
#'   supported by the ['RSL'
#'   library](https://trmm-fc.gsfc.nasa.gov/trmm_gv/software/rsl/) or 3) Vaisala
#'   IRIS (IRIS RAW) format. IRIS format is not available on CRAN, see
#'   vol2birdR development version on Github.
#' @param config optional configuration object of class `Rcpp_Vol2BirdConfig`,
#' typically output from \link{vol2bird_config}
#' @param vpfile Character. File name. When provided with .csv extension, writes a vertical profile
#' in [VPTS CSV format](https://aloftdata.eu/vpts-csv/). Provided with another or no extension,
#'  writes a vertical profile in the ODIM HDF5 format to disk.
#' @param pvolfile_out Character. File name. When provided, writes a polar
#'   volume (`pvol`) file in the ODIM HDF5 format to disk. Useful for converting
#'   'RSL' formats to ODIM, and for adding 'MistNet' segmentation output.
#' @param verbose logical. When TRUE print profile output to console.
#' @param update_config logical. When TRUE processing options that are determined based on
#' input file characteristics are returned and updated in the object specified by the `config`
#' argument. Do not set to `TRUE` when `vol2bird()` is used in loops like `lapply()` or in parallel processes.
#'
#' @return No value returned, creates a file specified by `file` argument
#'
#' @examples
#' # Locate the polar volume example file
#' pvolfile <- system.file("extdata", "volume.h5", package = "vol2birdR")
#'
#' # Create a configuration instance:
#' conf <- vol2bird_config()
#'
#' # Define output file
#' output_file <- paste0(tempdir(), "/vp.h5")
#'
#' # Calculate the profile:
#' vol2bird(file = pvolfile, config = conf, vpfile = output_file)
#'
#' @seealso
#' * [vol2bird_config()]
#' @export
vol2bird <- function(file, config, vpfile="", pvolfile_out="", verbose=TRUE, update_config=FALSE){
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
  assert_that(is.flag(update_config))

  assert_that(inherits(config,"Rcpp_Vol2BirdConfig"))

  if(config$useMistNet){
    assert_that(mistnet_exists(),msg="'MistNet' installation not found, install with `install_mistnet()`")
    assert_that(file.exists(config$mistNetPath),msg="'MistNet' model file not found, point `mistNetPath` option to valid 'MistNet' file or download the model with `install_mistnet_model()`")
  }

  # make a copy of the configuration object for parallel processes.
  # The processor might change the configuration object based on the
  # input file characteristics
  if(update_config){
    config_instance <- config
  }
  else{
    config_instance <- vol2bird_config(config)
  }

  processor<-Vol2Bird$new()
  processor$verbose <- verbose
  processor$process(path.expand(file), config_instance, path.expand(vpfile), path.expand(pvolfile_out))
}
