#' @useDynLib torchpkg
#' @importFrom Rcpp sourceCpp
NULL

globalVariables(c("..", "self", "private", "N"))

.generator_null <- NULL
.compilation_unit <- NULL

.onAttach <- function(libname, pkgname) {
}

.onLoad <- function(libname, pkgname) {
  cpp_vol2bird_namespace__store_main_thread_id()
  cpp_vol2bird_initialize()
  cpp_vol2bird_set_wsr88d_site_location(file.path(find.package(pkgname), "librsl", "wsr88d_locations.dat"))
  
  install_success <- TRUE
   
  if (install_exists() && install_success) {
    # in case init fails we will just have disabled mistnet and run without it..
    tryCatch(
      {
        mistnet_start()
      },
      error = function(e) {
      }
    )
  }
}

.onUnload <- function(libpath) {

}

#' Bullets
release_bullets <- function() {
  c(
    "Create the cran/ branch and update the branch variable",
    "Uncomment the indicated line in the .RBuildignore file"
  )
}
