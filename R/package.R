globalVariables(c("..", "self", "private", "N"))

.generator_null <- NULL
.compilation_unit <- NULL

.onAttach <- function(libname, pkgname) {
}

.onLoad <- function(libname, pkgname) {

  # required environment variable on Mac for parallel use of libtorch:
  osname<-tolower(Sys.info()[["sysname"]])
  if (osname == "darwin" && Sys.getenv("OMP_NUM_THREADS") == "") {
    Sys.setenv(OMP_NUM_THREADS = parallel::detectCores())
  }

  # set PROJ_LIB path for pre-compiled binaries for access to proj library data
  # proj subdirectory contains a copy of proj library data directory (`pkg-config --variable=datadir proj`)
  if (Sys.getenv("PROJ_LIB") == ""){
     if(file.exists(prj<-system.file("proj", package = "vol2birdR")[1])){
        Sys.setenv("PROJ_LIB" = prj)
     }
  }

  cpp_vol2bird_namespace__store_main_thread_id()
  cpp_vol2bird_initialize()
  cpp_vol2bird_set_wsr88d_site_location(file.path(find.package(pkgname), "librsl", "wsr88d_locations.dat"))
  
  install_success <- TRUE
   
  if (mistnet_exists() && install_success) {
    # in case init fails we will just have disabled mistnet and run without it..
    tryCatch(
      {
        mistnet_start(verbose = FALSE)
      },
      error = function(e) {
      }
    )
  }
}

.onUnload <- function(libpath) {

}
