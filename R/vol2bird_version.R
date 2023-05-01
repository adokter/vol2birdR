#' Return 'vol2bird' version
#'
#' Return version of the 'vol2bird' algorithm
#'
#' @return an object of class \link{numeric_version}
#'
#' @export
#' @examples
#' # check installed 'vol2bird' version:
#' vol2bird_version()
vol2bird_version <- function(){
  numeric_version(cpp_vol2bird_version())
}