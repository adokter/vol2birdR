#' Create a vol2bird configuration instance
#' 
#' Creates a vol2bird configuration instance of class `Rcpp_Vol2BirdConfig`
#'
#' @return an object of class `Rcpp_Vol2BirdConfig`
#' 
#' @details
#' All processing options for [vol2bird()] are set using a configuration instance of class `Rcpp_Vol2BirdConfig`
#' In some cases it might be necessary to copy and modify configuration instance, for example
#' when processing polar volume files with different settings.
#' In these cases you can't copy the instance like:
#' ```R
#' config<-vol2bird_config()
#' extra_config<-config
#' ```
#' In the above example, the `config` and `extra_config` instances will both refer to the same object.
#' (copy by reference). To avoid this (and make a copy by value), use:
#' ```R
#' config<-vol2bird_config()
#' # create a copy identical to object config:
#' extra_config<-vol2bird_config(config)
#' ```
#' @export
#' 
#' @seealso
#' * [vol2bird()]
#'
#' @examples
#' # create a configuration object
#' config <- vol2bird_config()
#' # list the the configuration elements:
#' config
#' # change the maximum range included in the profile generation to 40 km:
#' config$rangeMax <- 40000
#' # make a copy of the configuration object:
#' config_copy <- vol2bird_config(config)
vol2bird_config <- function(config){
  if(missing(config)){
    output=Vol2BirdConfig$new()
  }
  else{
    assert_that(inherits(config,"Rcpp_Vol2BirdConfig"))
    output=Vol2BirdConfig$new(config)
  }
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
