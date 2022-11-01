#' Loads a volume vol2bird-style meaning that a number of different file formats
#' are tried and eventually loaded.
#' @param file name of file to be loaded
#' @return The loaded volume as a Rcpp_PolarVolume that can be passed around
#' @export
load_volume <- function(file){
  processor<-Vol2Bird$new()
  vol<-processor$load_volume(file)
  return(vol) 
}
