.globals <- new.env(parent = emptyenv())
.globals$mistnet_started <- FALSE

#' The default 'MistNet' version
#' @return the default 'MistNet' version
#' @keywords internal
mistnet_default <- function() {
  "1.0.0"
}

#' Initialize the 'MistNet' system if enabled.
#' @param version version of 'MistNet' library
#' @param reload if 'MistNet' library should be reloaded or not, default FALSE.
#' @keywords internal
mistnet_start <- function(version = mistnet_default(), reload = FALSE) {
  if (!mistnet_exists()) {
    stop("'MistNet' is disabled.")
  }

  if (.globals$mistnet_started && !reload) {
    return()
  }

  cpp_mistnet_init(file.path(install_path(), "lib"))

  .globals$mistnet_started <- TRUE
}
