#' Create a 'vol2bird' configuration instance
#'
#' Creates or copies a 'vol2bird' configuration instance of class `Rcpp_Vol2BirdConfig`
#'
#' @param config a configuration instance to be copied.
#'
#' @return an object of class `Rcpp_Vol2BirdConfig`
#'
#' @details
#' ## Copying configuration instances
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
#'
#' ## User configuration options
#' The `Rcpp_Vol2BirdConfig` class object sets the following 'vol2bird' processing options:
#' * `azimMax`: Numeric. The minimum azimuth (0-360 degrees) used for constructing the bird density profile
#' * `azimMin`: Numeric. The maximum azimuth (0-360 degrees) used for constructing the bird density profile
#' * `birdRadarCrossSection`: Numeric. Radar cross section in cm^2
#' * `clutterMap`: Character. clutter map path and filename
#' * `clutterValueMin`: Numeric. sample volumes in the static cluttermap with a value above
#' this threshold will be considered clutter-contaminated. Default 0.1
#' * `dbzType`: Character. Reflectivity factor quantity to use. Default `DBZH`
#' * `dualPol`: Logical. Whether to use dual-pol moments for filtering meteorological echoes. Default `TRUE`
#' * `elevMax`: Numeric. The minimum scan elevation in degrees used for constructing the bird density profile
#' * `elevMin`: Numeric. The maximum scan elevation in degrees used for constructing the bird density profile
#' * `layerThickness`: Numeric. The width/thickness of an altitude layer in m. Default 200
#' * `mistNetPath`: Character. Path of 'MistNet' segmentation model in pytorch (.pt) format
#' * `nLayers`: Integer. The number of layers in an altitude profile. Default 25
#' * `radarWavelength`: Numeric. The radar wavelength in cm to assume when unavailable as an attribute in the input file. Default 5.3
#' * `rangeMax`: Numeric. The maximum range in m used for constructing the bird density profile. Default 35000
#' * `rangeMin`: Numeric. The minimum range in m used for constructing the bird density profile. Default 5000
#' * `rhohvThresMin`: Numeric. Correlation coefficients higher than this threshold will be classified as precipitation. Default 0.95
#' * `singlePol`: Logical. Whether to use single-pol moments for filtering meteorological echoes. Default `TRUE`
#' * `stdDevMinBird`: Numeric. VVP Radial velocity standard deviation threshold. Default 2 m/s.
#' * `useClutterMap`: Logical. Whether to use a static clutter map. Default `FALSE`
#' * `useMistNet`: Logical. Whether to use the 'MistNet' segmentation model. Default `FALSE`.
#'
#' ## Advanced configuration options
#' Changing these settings is rarely needed.
#' * `cellEtaMin`:  Numeric. Maximum mean reflectivity in cm^2/km^3 for cells containing birds
#' * `cellStdDevMax`: Numeric. When analyzing precipitation cells, only cells for which the stddev of
#' vrad (aka the texture) is less than cellStdDevMax are considered in the rest of the analysis
#' * `dbzThresMin`: Numeric. Minimum reflectivity factor of a gate to be considered for inclusion in a weather cell. Default 0 dBZ
#' * `dealiasRecycle`: Logical. Whether we should dealias all data once (default `TRUE`), or dealias for each profile individually (`FALSE`)
#' * `dealiasVrad`: Logical. Whether we should dealias the radial velocities. Default `TRUE`.
#' * `etaMax`: Numeric. Maximum reflectivity in cm^2/km^3 for single gates containing birds. Default 36000
#' * `exportBirdProfileAsJSONVar`: Logical. Deprecated, do not use. Default `FALSE`
#' * `fitVrad`: Logical. Whether or not to fit a model to the observed vrad. Default `TRUE`
#' * `maxNyquistDealias`: Numeric. When all scans have nyquist velocity higher than this value, dealiasing is suppressed. Default 25 m/s.
#' * `minNyquist`: Numeric. Scans with Nyquist velocity lower than this value are excluded. Default 5 m/s.
#' * `mistNetElevs`: Numeric vector of length 5. Elevations to use in Cartesian projection for 'MistNet'. Default `c(0.5, 1.5, 2.5, 3.5, 4.5)`
#' * `mistNetElevsOnly`: Logical. When `TRUE` (default), use only the specified elevation scans for 'MistNet' to calculate profile, otherwise use all available elevation scans
#' * `requireVrad`: Logical. For a range gate to contribute it should have a valid radial velocity. Default `FALSE`
#' * `resample`: Logical. Whether to resample the input polar volume. Downsampling speeds up the calculation. Default `FALSE`
#' * `resampleNbins`: Numeric. Resampled number of range bins. Ignored when `resample` is `FALSE`. Default 100
#' * `resampleNrays`: Numeric. Resampled number of azimuth bins. Ignored when `resample` is `FALSE`. Default 360
#' * `resampleRscale`: Numeric. Resampled range gate length in m. Ignored when `resample` is `FALSE`. Default 500 m.

#' ## Algorithm constants
#' Changing any of these constants is not recommended
#' * `constant_absVDifMax`: Numeric. After fitting the radial velocity data, throw out any VRAD observations that
#' are more than absVDifMax away from the fitted value as outliers. Default 10
#' * `constant_areaCellMin`: Numeric. When analyzing cells, areaCellMin determines the minimum size
#' of a cell to be considered in the rest of the analysis. in km^2. Default 0.5
#' * `constant_cellClutterFractionMax`: Cells with clutter fractions above this value are likely not birds. Default 0.5
#' * `constant_chisqMin`: Minimum standard deviation of the VVP fit. Default 1e-05
#' * `constant_fringeDist`: Each identified weather cell is grown by a distance equal to 'fringeDist' using a region-growing approach. Default 5000
#' * `constant_nAzimNeighborhood`: vrad's texture is calculated based on the local neighborhood. The neighborhood size in the azimuth direction is equal to this value. Default 3
#' * `constant_nBinsGap`: When determining whether there are enough vrad observations in each direction, use nBinsGap sectors. Default 8
#' * `constant_nCountMin`: The minimum number of neighbors for the texture value to be considered valid, as used in calcTexture(). Default 4
#' * `constant_nNeighborsMin`: the minimum number of direct neighbors with dbz value above dbzThresMin as used in findWeatherCells(). Default 5
#' * `constant_nObsGapMin`: there should be at least this many vrad observations in each sector. Default 5
#' * `constant_nPointsIncludedMin`: when calculating the altitude-layer averaged dbz, there should be at least this many valid data points. Default 25
#' * `constant_nRangNeighborhood`: vrad's texture is calculated based on the local neighborhood. The neighborhood size in the range direction is equal to this value. Default 3
#' * `constant_refracIndex`: Refractive index of the scatterers. Default equal to water 0.964
#' * `constant_vradMin`: When analyzing cells, radial velocities lower than vradMin are treated as clutter. Default 1 m/s.
#'
#' ## Debug printing options
#' Enable these printing options only for debugging purposes
#' in a terminal, since large amounts of data will be dumped into the console.
#' * `printCell`: Logical. Print precipitation cell data to stderr. Default `FALSE`
#' * `printCellProp`: Logical. Print precipitation cell properties to stderr. Default `FALSE`
#' * `printClut`: Logical. Print clutter data to stderr. Default `FALSE`
#' * `printDbz`: Logical. Print reflectivity factor data to stderr. Default `FALSE`
#' * `printDealias`: Logical. `FALSE`
#' * `printOptions`: Logical. Print options to stderr. Default `FALSE`
#' * `printPointsArray`: Logical. Print the 'points' array to stderr. Default `FALSE`
#' * `printProfileVar`: Logical. Print profile data to stderr. Default `FALSE`
#' * `printRhohv`: Logical. Print correlation coefficient data to stderr. Default `FALSE`
#' * `printTex`: Logical. Print radial velocity texture data to stderr. Default `FALSE`
#' * `printVrad`: Logical. Print radial velocity data to stderr. Default `FALSE`
#'
#' @export
#'
#' @seealso
#' * [vol2bird()]
#'
#' @examples
#' # create a configuration instance
#' config <- vol2bird_config()
#' # list the the configuration elements:
#' config
#' # change the maximum range included in the profile generation to 40 km:
#' config$rangeMax <- 40000
#' # make a copy of the configuration instance:
#' config_copy <- vol2bird_config(config)
vol2bird_config <- function(config){
  if(missing(config)){
    output=Vol2BirdConfig$new()
    mistnet_model_path <- file.path(base::normalizePath(tools::R_user_dir("vol2birdR", "data"), winslash = "/", mustWork = FALSE),"data","mistnet_nexrad.pt")
    if(file.exists(mistnet_model_path)){
      output$mistNetPath <-  mistnet_model_path
    }
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
            str_capture <- capture.output(str(object))
            # hide constant options
            str_capture <- str_capture[!grepl("constant_",str_capture)]
            # hide print options
            str_capture <- str_capture[!grepl("print",str_capture)]
            # remove first and last two lines of str output, and add header:
            cat(c("'vol2bird' configuration:",str_capture[2:(length(str_capture)-2)]),sep="\n")
          })
