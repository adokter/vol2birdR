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

  # Path where the DLL should be
  dll_path <- file.path(install_path(), "lib")
  cat("Attempting to load MistNet from:", dll_path, "\n")

  # Try-catch block to capture any errors during initialization
  tryCatch({
    cpp_mistnet_init(dll_path)
    .globals$mistnet_started <- TRUE
    cat("MistNet successfully initialized.\n")
  }, error = function(e) {
    cat("Error during MistNet initialization: ", e$message, "\n")
  }, warning = function(w) {
    cat("Warning during MistNet initialization: ", w$message, "\n")
  })
}
