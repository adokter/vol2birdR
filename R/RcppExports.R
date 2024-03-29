# Generated by using Rcpp::compileAttributes() -> do not edit by hand
# Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#' @name PolarVolume-class
#' @title PolarVolume
#' @description The polar volume object as defined by RAVE.
#' @keywords internal
NULL

#' @name RaveIO-class
#' @title RaveIO routines
#' @description Provides I/O routines using the rave framework
#' @keywords internal
NULL

#' @name Vol2BirdConfig-class
#' @title Vol2Bird configuration
#' @description The vol2bird configuration used during processing
#' @keywords internal
#' @seealso [vol2bird_config()]
NULL

#' @name Vol2Bird-class
#' @title Vol2Bird processor class
#' @description The vol2bird processing class.
#' Provides methods for processing polar volumes/scans.
#' @keywords internal
#' @seealso [vol2bird()]
NULL

#' @rdname PolarVolume-class
#' @name Rcpp_PolarVolume-class
#' @title Rcpp_PolarVolume-class
#' @description The Rcpp PolarVolume class
#' A polar volume
NULL

#' @rdname RaveIO-class
#' @name Rcpp_RaveIO-class
#' @title Rcpp_RaveIO-class
#' @description The Rcpp RaveIO class
#' Used when loading and saving objects
NULL

#' @rdname Vol2BirdConfig-class
#' @name Rcpp_Vol2BirdConfig-class
#' @title Rcpp_Vol2BirdConfig-class
#' @description The Rcpp vol2bird configuration class.
#' Configuration instance used when processing data.
NULL

#' @rdname Vol2Bird-class
#' @name Rcpp_Vol2Bird-class
#' @title Rcpp_Vol2Bird-class
#' @description The Rcpp vol2bird processing class.
NULL

#' Sets the main thread id
#'
#' @keywords internal
cpp_vol2bird_namespace__store_main_thread_id <- function() {
    invisible(.Call(`_vol2birdR_cpp_vol2bird_namespace__store_main_thread_id`))
}

#' Initializes the vol2birdR library
#'
#' @keywords internal
cpp_vol2bird_initialize <- function() {
    invisible(.Call(`_vol2birdR_cpp_vol2bird_initialize`))
}

#' Sets the wsr88d site location file
#'
#' @param loc location of file
#' @keywords internal
cpp_vol2bird_set_wsr88d_site_location <- function(loc) {
    invisible(.Call(`_vol2birdR_cpp_vol2bird_set_wsr88d_site_location`, loc))
}

#' Returns the wsr88d site location file
#'
#' @return location of site location file
#' @keywords internal
cpp_vol2bird_get_wsr88d_site_location <- function() {
    .Call(`_vol2birdR_cpp_vol2bird_get_wsr88d_site_location`)
}

#' Initializes the mistnet shared library pointed to by the path
#'
#' @keywords internal
#' @param path The shared library
cpp_vol2bird_version <- function() {
    .Call(`_vol2birdR_cpp_vol2bird_version`)
}

#' Initializes the mistnet shared library pointed to by the path
#'
#' @keywords internal
#' @param path The shared library
cpp_mistnet_init <- function(path) {
    invisible(.Call(`_vol2birdR_cpp_mistnet_init`, path))
}

#' The software has to be compiled with -DRAVE_MEMORY_DEBUG and without -DNO_RAVE_PRINTF.
#' Manual handling for now.
#' @keywords internal
cpp_printMemory <- function() {
    invisible(.Call(`_vol2birdR_cpp_printMemory`))
}

