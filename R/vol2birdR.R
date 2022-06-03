#' @details
#' To get started, see:
#'
#' \itemize{
#'   \item Dokter et al. (2016) \doi{https://doi.org/10.1098/rsif.2010.0116}: a
#'   paper describing the profiling algorithmp.
#'   \item \href{https://adriaandokter.com/vol2bird}{vol2bird C code documentation}: 
#'   an overview of the algorithm structure.
#' }
#'
#' @keywords internal
#'
#' @import assertthat
#' @importFrom utils str
#' @importFrom rlang abort
#' @import Rcpp
#' @export RaveIO
#' @export PolarVolume
#' @export Vol2BirdConfig
#' @export Vol2Bird
"_PACKAGE"

loadModule("RaveIO",TRUE)
loadModule("PolarVolume",TRUE)
loadModule("Vol2Bird",TRUE)
loadModule("Vol2BirdConfig",TRUE)
