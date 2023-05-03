#' @details
#' To get started, see:
#'
#' \itemize{
#'   \item Dokter et al. (2016) \doi{https://doi.org/10.1098/rsif.2010.0116}: a
#'   paper describing the profiling algorithm.
#'   \item \href{https://adriaandokter.com/vol2bird/}{'vol2bird' 'C' code documentation}: 
#'   an overview of the algorithm structure.
#'   \item Lin T-Y et al. (2019) \doi{https://doi.org/10.1111/2041-210X.13280}: a
#'   paper describing the 'MistNet' model for rain segmentation.
#' }
#'
#' @keywords internal
#'
#' @import methods
#' @useDynLib vol2birdR, .registration = TRUE
#' @import assertthat
#' @import pkgbuild
#' @importFrom utils str
#' @importFrom rlang abort
#' @importFrom utils capture.output
#' @import Rcpp
"_PACKAGE"

loadModule("RaveIO",TRUE)
loadModule("PolarVolume",TRUE)
loadModule("Vol2Bird",TRUE)
loadModule("Vol2BirdConfig",TRUE)
