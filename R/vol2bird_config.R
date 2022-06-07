#' Create a vol2bird configuration instance
#' 
#' Creates a vol2bird configuration instance of class `Rcpp_Vol2BirdConfig`
#'
#' @param config a configuration object to be copied.
#'
#' @return an object of class `Rcpp_Vol2BirdConfig`
#' 
#' @details
#' ## configuration options
#' The class `Rcpp_Vol2BirdConfig` class object accesses the following vol2bird processing options:
#' * `azimMax`: the minimum azimuth [0-360 degrees] used for constructing the bird density profile
#' * `azimMin`: the maximum azimuth [0-360 degrees] used for constructing the bird density profile
#' * `birdRadarCrossSection`: DOCUMENTATION TO BE FINISHED! num 11
#' * `cellEtaMin`: num 11500
#' * `cellStdDevMax`: num 5
#' * `clutterMap`: chr ""
#' * `clutterValueMin`: num 0.1
#' * `constant_absVDifMax`: num 10
#' * `constant_areaCellMin`: num 0.5
#' * `constant_cellClutterFractionMax`: num 0.5
#' * `constant_chisqMin`: num 1e-05
#' * `constant_fringeDist`: num 5000
#' * `constant_nAzimNeighborhood`: int 3
#' * `constant_nBinsGap`: int 8
#' * `constant_nCountMin`: int 4
#' * `constant_nNeighborsMin`: int 5
#' * `constant_nObsGapMin`: int 5
#' * `constant_nPointsIncludedMin`: int 25
#' * `constant_nRangNeighborhood`: int 3
#' * `constant_refracIndex`: num 0.964
#' * `constant_vradMin`: num 1
#' * `dbzThresMin`: num 0
#' * `dbzType`: chr "DBZH"
#' * `dealiasRecycle`: logi TRUE
#' * `dealiasVrad`: logi TRUE
#' * `dualPol`: logi TRUE
#' * `elevMax`: num 90
#' * `elevMin`: num 0
#' * `etaMax`: num 36000
#' * `exportBirdProfileAsJSONVar`: logi FALSE
#' * `fitVrad`: logi TRUE
#' * `layerThickness`: num 200
#' * `maxNyquistDealias`: num 25
#' * `minNyquist`: num 5
#' * `misc_cellDbzMin`: num NaN
#' * `misc_dbzFactor`: num NaN
#' * `misc_dbzMax`: num NaN
#' * `mistNetElevs`: num [1`:5] 0.5 1.5 2.5 3.5 4.5
#' * `mistNetElevsOnly`: logi TRUE
#' * `mistNetPath`: chr "/Library/Frameworks/R.framework/Versions/4.1/Resources/library/vol2birdR/data/mistnet_nexrad.pt"
#' * `nLayers`: int 25
#' * `printCell`: logi FALSE
#' * `printCellProp`: logi FALSE
#' * `printClut`: logi FALSE
#' * `printDbz`: logi FALSE
#' * `printDealias`: logi FALSE
#' * `printOptions`: logi FALSE
#' * `printPointsArray`: logi FALSE
#' * `printProfileVar`: logi FALSE
#' * `printRhohv`: logi FALSE
#' * `printTex`: logi FALSE
#' * `printVrad`: logi FALSE
#' * `radarWavelength`: num 5.3
#' * `rangeMax`: num 35000
#' * `rangeMin`: num 5000
#' * `requireVrad`: logi FALSE
#' * `resample`: logi FALSE
#' * `resampleNbins`: int 100
#' * `resampleNrays`: int 360
#' * `resampleRscale`: num 500
#' * `rhohvThresMin`: num 0.95
#' * `singlePol`: logi TRUE
#' * `stdDevMinBird`: num 2
#' * `useClutterMap`: logi FALSE
#' * `useMistNet`: logi FALSE
#' 
#' ## copying configuration instances
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
    output$mistNetPath <- file.path(find.package("vol2birdR"), "data", "mistnet_nexrad.pt")
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
