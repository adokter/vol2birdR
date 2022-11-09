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
  constants_header_file <- system.file("libvol2bird","constants.h",package="vol2birdR")
  assert_that(file.exists(constants_header_file))
  numeric_version(strsplit(grep("VERSION ",readLines(constants_header_file),value=TRUE),"\"")[[1]][2])
}
