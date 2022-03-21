.globals <- new.env(parent = emptyenv())
.globals$mistnet_started <- FALSE

mistnet_default <- function() {
  "1.0.0"
}

mistnet_start <- function(version = mistnet_default(), reload = FALSE) {
  if (!install_exists()) {
    stop("Mistnet is disabled.")
  }

  if (.globals$mistnet_started && !reload) {
    return()
  }

  cpp_mistnet_init(file.path(install_path(), "lib"))

  .globals$mistnet_started <- TRUE
}
