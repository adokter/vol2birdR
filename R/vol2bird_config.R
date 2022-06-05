#' Create a vol2bird configuration object
#' 
#' Creates a vol2bird configuration object of class `Vol2BirdConfig`
#'
#' @return an object of class `Vol2BirdConfig`
#' 
#' @export
#' 
#' @seealso
#' * [vol2bird()]
#' 
#' @importFrom utils capture.output
#'
#' @examples
#' # create a configuration object
#' config <- vol2bird_config()
#' # list the the configuration elements:
#' config
#' # change the maximum range included in the profile generation to 40 km:
#' config$rangeMax <- 40000
#

vol2bird_config <- function(){
  output=Vol2BirdConfig$new()
  output$mistNetPath <- file.path(find.package("vol2birdR"), "data", "mistnet_nexrad.pt")
  output
}

# change the default print method for Rcpp_Vol2BirdConfig class
setMethod(f = "show",
          signature = "Rcpp_Vol2BirdConfig",
          definition = function(object){
            str_capture=capture.output(str(object))
            # remove first and last two lines of str output, and add header:
            cat(c("vol2bird configuration:",str_capture[2:(length(str_capture)-2)]),sep="\n")
          })
