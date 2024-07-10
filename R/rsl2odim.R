
#' Check file read permissions
#' 
#' @param file_path The path to the file to check.
#' @return TRUE if file has read permissions, otherwise throws an error.
#' @importFrom assertthat assert_that
#' @keywords internal
#' @noRd
check_file_access <- function(file_path) {
  assert_that(file.access(file_path, 4) == 0, msg = paste("Error: No read permission for the file:", file_path))
  return(TRUE)
}

#' Convert a NEXRAD polar volume file to an ODIM polar volume file
#'
#' @inheritParams vol2bird
#'
#' @return No value returned, creates a file specified by `pvolfile_out` argument.
#'
#' @seealso
#' * [vol2bird_config()]
#' @importFrom assertthat assert_that
#' @export
#' @examples
#' \donttest{
#' # define filenames
#' nexrad_file <- paste0(tempdir(),"/KBGM20221001_000243_V06")
#' odim_file <- paste0(tempdir(),"/KBGM20221001_000243_V06.h5")
#' # download NEXRAD file:
#' download.file("https://noaa-nexrad-level2.s3.amazonaws.com/2022/10/01/KBGM/KBGM20221001_000243_V06",
#' destfile = nexrad_file, mode="wb")
#' # convert NEXRAD file to ODIM hdf5 format:
#' rsl2odim(nexrad_file, pvolfile_out = odim_file)
#' # clean up
#' file.remove(nexrad_file)
#' file.remove(odim_file)
#' }
rsl2odim <- function(file, config, pvolfile_out="", verbose=TRUE, update_config=FALSE){
  for (filename in file) {
    if (!file.exists(filename)) {
      warning("Error: File does not exist: ", filename)
      }
      if (!check_file_access(filename)) {
        warning("File access check failed for: ", filename)
    }
  }
  if (!are_equal(pvolfile_out, "")) {
    assert_that(is.writeable(dirname(pvolfile_out)))
  }
  if(missing(config)){
    config <- vol2bird_config()
  }
  assert_that(is.flag(verbose))
  assert_that(is.flag(update_config))

  if(config$useMistNet){
    assert_that(mistnet_exists(),msg="mistnet installation not found, install with `install_mistnet()`")
    assert_that(file.exists(config$mistNetPath),msg="mistnet model file not found, point `mistNetPath` option to valid mistnet file or download the model with `install_mistnet_model()`")
  }

  assert_that(inherits(config,"Rcpp_Vol2BirdConfig"))

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
  processor$rsl2odim(path.expand(file), config_instance, path.expand(pvolfile_out))
}



