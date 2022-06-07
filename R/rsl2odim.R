#' Convert a NEXRAD polar volume file to an ODIM polar volume file
#'
#' @inheritParams vol2bird
#' 
#' @seealso
#' * [vol2bird_config()]
#' @export
rsl2odim <- function(file, pvolfile_out="", config, verbose=TRUE){
  for (filename in file) {
    assert_that(file.exists(filename))
  }
  if (!are_equal(pvolfile_out, "")) {
    assert_that(is.writeable(dirname(pvolfile_out)))
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
