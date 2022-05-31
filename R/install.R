#' The default branch
#' @keywords internal
branch <- "main"

#' Contains a list of mistnet libraries for the various OS:s
#' @keywords internal
install_config <- list(
  "1.10.2" = list(
    "cpu" = list(
      "darwin" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.10.2.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "96ebbf1e2e44f30ee80bf3c8e4a31e15"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-cpu.zip", branch)
      ),
      "darwin-arm64" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.10.2.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "96ebbf1e2e44f30ee80bf3c8e4a31e15"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-arm64-cpu.zip", branch)
      ),
      "windows" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.10.2%2Bcpu.zip",
          path = "libtorch/",
          filter = ".dll",
          md5hash = "c49ddfd07ba65e0ff4a54e041ed22c42"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Windows-cpu.zip", branch)
      ),
      "linux" = list(
        "libtorch" = list(
          path = "libtorch/",
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip",
          md5hash = "99d16043865716f5e38a8d15480b61c6"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Linux-cpu.zip", branch)
      )
    )
  )
)

#' Returns the path of the mistnet libraries for specified version
#' @param version The Mistnet version checked for
#' @return the path to the libraries
#' @keywords internal
install_path <- function(version = "1.0") {
  path <- Sys.getenv("MISTNET_HOME")
  if (nzchar(path)) {
    normalizePath(path, mustWork = FALSE)
  } else {
    normalizePath(file.path(system.file("", package = "vol2birdR")), mustWork = FALSE)
  }
}

#' Returns the torch installation path.
#' @export
torch_install_path <- function() {
  install_path()
}

#' Checks if the torch and mistnet library has been installed or not.
#' Returns TRUE if both torch and mistnet libraries can be found, otherwise FALSE
#' @export
install_exists <- function() {
  if (!dir.exists(install_path())) {
    return(FALSE)
  }

  if (!length(list.files(file.path(install_path(), "lib"), "torch")) > 0) {
    return(FALSE)
  }

  if (!length(list.files(file.path(install_path(), "lib"), "mistnet")) > 0) {
    return(FALSE)
  }

  TRUE
}

#' Verifies if torch is installed
#'
#' @export
torch_is_installed <- function() {
  install_exists()
}

#' Returns the path of the mistnet libraries for specified version
#' @param library_name The name of the library searched for, either libmistnet or libtorch
#' @param install_path The location where to look for the libraries
#' @return if anything could be located or not
#' @keywords internal
lib_installed <- function(library_name, install_path) {
  x <- list.files(file.path(install_path, "lib"))

  if (library_name == "libmistnet") {
    any(grepl("mistnet", x))
  } else if (library_name == "libtorch") {
    any(grepl("torch", x))
  }
}

#' Installs the library
#' @param library_name The name of the library searched for, either libmistnet or libtorch
#' @param library_url Where to fetch the library
#' @param install_path Where to put the library
#' @param source_path If library should be fetched from somewhere else
#' @param filter Not used
#' @param md5hash MD5 check
#' @param inst_path inst path
#' @keywords internal
#' @return if anything could be located or not
mistnet_install_lib <- function(library_name, library_url,
                                install_path, source_path, filter, md5hash,
                                inst_path) {
  library_extension <- paste0(".", tools::file_ext(library_url))
  temp_file <- tempfile(fileext = library_extension)
  temp_path <- tempfile()

  utils::download.file(library_url, temp_file)
  on.exit(try(unlink(temp_file)))

  if (!is.null(md5hash) && is.character(md5hash) && length(md5hash) == 1) {
    hash <- tools::md5sum(temp_file)
    if (hash != md5hash) {
      stop(
        "The file downloaded from '", library_url,
        "' does not match the expected md5 hash '",
        md5hash, "'. The observed hash is '", hash,
        "'. Due to security reasons the installation is stopped."
      )
    }
  }

  uncompress <- if (identical(library_extension, "tgz")) utils::untar else utils::unzip

  uncompress(temp_file, exdir = temp_path)
  
  #if (!file.exists(file.path(install_path, inst_path))){
  #  dir.create(file.path(install_path, inst_path))
  #}
  
  file.copy(
    from = dir(file.path(temp_path, source_path), full.names = TRUE),
    to = file.path(install_path, inst_path),
    recursive = TRUE
  )
}

#' Returns the system name
#' @keywords internal
install_os <- function() {
  tolower(Sys.info()[["sysname"]])
}

#' Installs the mistnet libraries
#' @param version version to install
#' @param type what type of libraries to be installed
#' @param install_path Where libraries should be installed
#' @param install_config the library config
#' @keywords internal
mistnet_install_libs <- function(version, type, install_path, install_config) {
  current_os <- install_os()

  if (!version %in% names(install_config)) {
    stop(
      "Version ", version, " is not available, available versions: ",
      paste(names(install_config), collapse = ", ")
    )
  }

  if (!type %in% names(install_config[[version]])) {
    stop("The ", type, " installation type is currently unsupported.")
  }

  if (current_os == "darwin") {
    if (tolower(Sys.info()[["machine"]]) == "arm64") {
      current_os <- "darwin-arm64"
    }
  }

  if (!current_os %in% names(install_config[[version]][[type]])) {
    stop("The ", current_os, " operating system is currently unsupported.")
  }

  install_info <- install_config[[version]][[type]][[current_os]]

  for (library_name in names(install_info)) {
    if (lib_installed(library_name, install_path)) {
      next
    }

    library_info <- install_info[[library_name]]
    
    if (!is.list(library_info)) {
      library_info <- list(url = library_info, filter = "", path = "", inst_path = "lib")
    }
    if (is.null(library_info$filter)) library_info$filter <- ""
    if (is.null(library_info$inst_path)) library_info$inst_path <- ""

    mistnet_install_lib(
      library_name = library_name,
      library_url = library_info$url,
      install_path = install_path,
      source_path = library_info$path,
      filter = function(e) grepl(library_info$filter, e),
      md5hash = library_info$md5hash,
      inst_path = library_info$inst_path
    )
  }

  invisible(install_path)
}

#' Install Mistnet
#'
#' Installs Mistnet and its dependencies.
#'
#' @param version The Mistnet version to install.
#' @param reinstall Re-install Mistnet even if its already installed?
#' @param path Optional path to install or check for an already existing installation.
#' @param timeout Optional timeout in seconds for large file download.
#' @param ... other optional arguments (like \code{`load`} for manual installation).
#'
#' @details
#'
#' When using \code{path} to install in a specific location, make sure the \code{MISTNET_HOME} environment
#' variable is set to this same path to reuse this installation. The \code{TORCH_INSTALL} environment
#' variable can be set to \code{0} to prevent auto-installing torch and \code{TORCH_LOAD} set to \code{0}
#' to avoid loading dependencies automatically. These environment variables are meant for advanced use
#' cases and troubleshooting only.
#' When timeout error occurs during library archive download, or length of downloaded files differ from
#' reported length, an increase of the \code{timeout} value should help.
#'
#' @export
install_mistnet <- function(version = "1.10.2", reinstall = FALSE,
                          path = install_path(), timeout = 360, ...) {
  if (reinstall) {
    unlink(path, recursive = TRUE)
  }

  if (!dir.exists(path)) {
    ok <- dir.create(path, showWarnings = FALSE, recursive = TRUE)
    if (!ok) {
      rlang::abort(c(
        "Failed creating directory",
        paste("Check that you can write to: ", path)
      ))
    }
  }

  # check for write permission
  if (file.access(path, 2) < 0) {
    rlang::abort(c(
      "No write permissions to install mistnet.",
      paste("Check that you can write to:", path),
      "Or set the MISTNET_HOME env var to a path with write permissions."
    ))
  }

  if (!is.null(list(...)$install_config) && is.list(list(...)$install_config)) {
    install_config <- list(...)$install_config
  }

  withr::with_options(
    list(timeout = timeout),
    mistnet_install_libs(version, "cpu", path, install_config)
  )

  # reinitialize mistnet, might happen if installation fails on load and manual install is required
  if (!identical(list(...)$load, FALSE)) {
    mistnet_start(reload = TRUE)
  }
}

#' Install Mistnet from files
#'
#' Installs Torch and its dependencies from files.
#'
#' @param version The Torch version to install.
#' @param libtorch The installation archive file to use for Torch. Shall be a \code{"file://"} URL scheme.
#' @param libmistnet The installation archive file to use for Mistnet. Shall be a \code{"file://"} URL scheme.
#' @param ... other parameters to be passed to \code{"install_torch()"}
#'
#' @details
#'
#' When \code{"install_mistnet()"} initiated download is not possible, but installation archive files are
#' present on local filesystem, \code{"install_torch_from_file()"} can be used as a workaround to installation issue.
#' \code{"libtorch"} is the archive containing all torch modules, and \code{"libmistnet"} is the C interface to libtorch
#' that is used for the R package. Both are highly dependent, and should be checked through \code{"get_install_libs_url()"}
#'
#'
#' @export
install_mistnet_from_file <- function(version = "1.10.2", libtorch, libmistnet, ...) {
  stopifnot(inherits(url(libtorch), "file"))
  stopifnot(inherits(url(mistnet), "file"))

  install_config[[version]][["cpu"]][[install_os()]][["libtorch"]][["url"]] <- libtorch
  install_config[[version]][["cpu"]][[install_os()]][["libmistnet"]] <- libmistnet

  install_mistnet(version = version, type = "cpu", install_config = install_config, ...)
}

#' List of files to download
#'
#' List the Torch and mistnet files to download as local files in order to proceed with install_torch_from_file().
#'
#' @param version The Torch version to install.
#' @param type The installation type for Torch. Valid values are \code{"cpu"} or the 'CUDA' version.
#'
#'
#' @export
get_install_libs_url <- function(version = "1.10.2", type = install_type(version = version)) {
  libtorch <- install_config[[version]][[type]][[install_os()]][["libtorch"]][["url"]]
  libmistnet <- install_config[[version]][[type]][[install_os()]][["libmistnet"]]
  
  list(libtorch = libtorch, libmistnet = libmistnet)
}
