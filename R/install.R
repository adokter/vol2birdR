#' The default branch
#' @keywords internal
branch <- "main"
supported_pytorch_versions=c("1.10.2", "1.12.1")

#' Contains a list of 'MistNet' libraries for the various OS's
#' @keywords internal
install_config <- list(
  "1.12.1" = list(
    # Future version is 1.12.1. Need it to build for macOS ARM
    "cpu" = list(
      "darwin" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.12.1.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "bc05ee56d9134e5a36b97b949adf956a"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-cpu.zip", branch)
      ),
      "darwin-arm64" = list(
        "libtorch" = list(
          url = "https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/libtorch-macos-arm64-volbird-1.12.1.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "d2779f95bcd5527da3233382e45f6e3f"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-arm64-cpu.zip", branch)
      ),
      "windows" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.12.1%2Bcpu.zip",
          path = "libtorch/",
          filter = ".dll",
          md5hash = "18f46c55560cefe628f8c57b324a0a5c"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Windows-cpu.zip", branch)
      ),
      "linux" = list(
        "libtorch" = list(
          path = "libtorch/",
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.12.1%2Bcpu.zip",
          md5hash = "cfbc46b318c1e94efa359a98d4047ec8"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Linux-cpu.zip", branch)
      )
    )
  ),
  "1.10.2" = list(
    "cpu" = list(
      "darwin" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.10.2.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "96ebbf1e2e44f30ee80bf3c8e4a31e15"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-cpu_1_10_2.zip", branch)
      ),
      "darwin-arm64" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.10.2.zip",
          path = "libtorch/",
          filter = ".dylib",
          md5hash = "96ebbf1e2e44f30ee80bf3c8e4a31e15"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/macOS-arm64-cpu_1_10_2.zip", branch)
      ),
      "windows" = list(
        "libtorch" = list(
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-win-shared-with-deps-1.10.2%2Bcpu.zip",
          path = "libtorch/",
          filter = ".dll",
          md5hash = "c49ddfd07ba65e0ff4a54e041ed22c42"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Windows-cpu_1_10_2.zip", branch)
      ),
      "linux" = list(
        "libtorch" = list(
          path = "libtorch/",
          url = "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip",
          md5hash = "99d16043865716f5e38a8d15480b61c6"
        ),
        "libmistnet" = sprintf("https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/%s/latest/Linux-cpu_1_10_2.zip", branch)
      )
    )
  )
)

#' Returns the path of the 'MistNet' libraries for specified version
#' @param version The 'MistNet' version checked for
#' @return the path to the libraries
#' @keywords internal
install_path <- function(version = "1.0") {
  path <- Sys.getenv("MISTNET_HOME")
  if (nzchar(path)) {
    normalizePath(path, mustWork = FALSE)
  } else {
    base::normalizePath(tools::R_user_dir("vol2birdR", "data"), winslash = "/", mustWork = FALSE)
  }
}

#' Returns the 'LibTorch' installation path.
#'
#' Returns the directory where the LibTorch library has been downloaded
#' @export
#'
#' @return a character path
#'
#' @examples
#' torch_install_path()
torch_install_path <- function() {
  install_path()
}

#' Checks if the 'LibTorch' and 'MistNet' libraries have been installed or not.
#' @return TRUE if both 'LibTorch' and 'MistNet' libraries can be found, otherwise FALSE
#' @export
#' @seealso
#' * [mistnet_installed()]
#' * [install_mistnet()]
mistnet_exists <- function() {
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

#' Check if MistNet installation is complete
#'
#' Checks if the 'LibTorch' and 'MistNet' libraries have been installed, and
#' that the mistnet model file has been downloaded.
#' @param path Optional non-default file path to check for the mistnet model file.
#' @param verbose When TRUE print informative messages on missing library and model files.
#' @return TRUE if the 'LibTorch' and 'MistNet' libraries can be found and the
#' and the MistNet model file can be located, otherwise FALSE.
#' @export
#' @seealso
#' * [mistnet_exists()]
#' * [install_mistnet()]
#' * [install_mistnet_model()]
mistnet_installed <- function(path, verbose = FALSE) {
  if(missing(path)){
     path <- file.path(vol2birdR::torch_install_path(),"data","mistnet_nexrad.pt")
  }
  else{
     assert_that(is.character(path))
  }

  if(!mistnet_exists()){
     if(verbose) message("torch and/or mistnet libraries not installed, see `install_mistnet()`")
     return(FALSE)
  }
  if(!file.exists(path)){
     if(verbose) message("mistnet model file not found, see `install_mistnet_model()`")
     return(FALSE) 
  }

  TRUE
}


#' Returns the path of the 'MistNet' libraries for specified version
#' @param library_name The name of the library searched for, either 'libmistnet' or 'LibTorch'
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
#' @param library_name The name of the library searched for, either 'libmistnet' or 'libtorch'
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

  utils::download.file(library_url, temp_file, mode="wb")
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

#' Installs the 'MistNet' libraries
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

#' Returns the LibTorch install type
#' @keywords internal
install_type <- function(version) {
  return("cpu")
}

#' Install 'MistNet' model file
#'
#' Installs the 'MistNet' model file in 'PyTorch' format
#'
#' @param reinstall Re-install the model even if its already installed
#' @param path Optional path to install or check for an already existing installation.
#' @param timeout Optional timeout in seconds for large file download.
#' @param from_url From where the 'MistNet' model file should be downloaded.
#' @param method The download method to use, see \link[utils]{download.file}
#' @param ... other optional arguments (like \code{`load`} for manual installation).
#'
#' @return No value returned, this function downloads a file
#'
#' @details
#' Download and install the 'MistNet' model file. By default the library is downloaded to
#' data/mistnet_nexrad.pt in the 'tools::R_user_dir("vol2birdR", "data")' directory.
#'
#' Alternatively, the model file can be downloaded to a different location, which has the
#' advantage that it doesn't have to be redownloaded after a reinstall of 'vol2birdR'.
#'
#' 'vol2birdR' will automatically detect the model file if it is downloaded to
#' `/opt/vol2bird/etc/mistnet_nexrad.pt`, which can be done as follows
#' ```R
#' install_mistnet_model(path="/opt/vol2bird/etc/mistnet_nexrad.pt")
#' ```
#' @examples
#' \donttest{
#' install_mistnet_model()
#' }
#'
#' @export
install_mistnet_model <- function(reinstall=FALSE, path = file.path(torch_install_path(),"data","mistnet_nexrad.pt"), timeout = 1800, from_url="http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt", method="libcurl", ...)
{
  if (!dir.exists(dirname(path))) {
    if(!dir.create(dirname(path), recursive=TRUE)){
      stop("cannot create directory")
    }
  }

  if (reinstall) {
    if (file.exists(path)) {
      unlink(path)
    }
  }

  if (file.exists(path)) {
    return(TRUE)
  }

  temp_file <- tempfile(fileext = ".pt")

  withr::with_options(
    list(timeout = timeout),
    utils::download.file(from_url, temp_file, method=method, mode="wb")
  )
  on.exit(try(unlink(temp_file)))

  file.copy(
    from = temp_file,
    to = path
  )

  return(TRUE)
}

#' Install 'MistNet' libraries
#'
#' Installs libraries and dependencies for using 'MistNet'.
#'
#' @param version The 'LibTorch' version to install.
#' @param reinstall Re-install 'MistNet' even if its already installed?
#' @param path Optional path to install or check for an already existing installation.
#' @param timeout Optional timeout in seconds for large file download.
#' @param ... other optional arguments (like \code{`load`} for manual installation).
#'
#' @return no value returned. Installs libraries into the package
#'
#' @details
#' By default libraries are installed in the 'tools::R_user_dir("vol2birdR", "data")' directory.
#'
#' When using \code{path} to install in a specific location, make sure the \code{MISTNET_HOME} environment
#' variable is set to this same path to reuse this installation.
#'
#' The \code{TORCH_INSTALL} environment
#' variable can be set to \code{0} to prevent auto-installing 'LibTorch and \code{TORCH_LOAD} set to \code{0}
#' to avoid loading dependencies automatically. These environment variables are meant for advanced use
#' cases and troubleshooting only.
#'
#' When timeout error occurs during library archive download, or length of downloaded files differ from
#' reported length, an increase of the \code{timeout} value should help.
#'
#' @export
#'
#' @examples
#' \donttest{
#' install_mistnet()
#' }
#'
#' @seealso
#' * [install_mistnet_from_file()]
install_mistnet <- function(version = "1.12.1", reinstall = FALSE, path = install_path(), timeout = 360, ...) {
  message("Starting installation of MistNet...\n")
  assert_that(version %in% supported_pytorch_versions,
              msg = paste("version should be",paste(supported_pytorch_versions, collapse = " or ")))
  assert_that(is.flag(reinstall))
  assert_that(is.number(timeout))

  if (reinstall) {
    message("Reinstall flag set. Unlinking the existing path...\n")
    unlink(path, recursive = TRUE)
  }

  if (!dir.exists(path)) {
    message("Creating directory for MistNet installation...\n")
    ok <- dir.create(path, showWarnings = FALSE, recursive = TRUE)
    if (!ok) {
      rlang::abort(c("Failed creating directory", paste("Check that you can write to: ", path)))
    }
  }

  # check for write permission
  message("Checking write permissions...\n")
  if (file.access(path, 2) < 0) {
    rlang::abort(c("No write permissions to install mistnet.",
                   paste("Check that you can write to:", path),
                   "Or set the MISTNET_HOME env var to a path with write permissions."))
  }

  if (!is.null(list(...)$install_config) && is.list(list(...)$install_config)) {
    install_config <- list(...)$install_config
  }

  message("Installing MistNet libraries...\n")
  withr::with_options(list(timeout = timeout), mistnet_install_libs(version, "cpu", path, install_config))

  message("Initializing MistNet...\n")
  if (!identical(list(...)$load, FALSE)) {
    mistnet_start(reload = TRUE, verbose = FALSE)
    message("MistNet initialized successfully.\n")
  }

  message("MistNet installation complete.\n")
}


#' Install 'MistNet' libraries from files
#'
#' Installs 'LibTorch' and 'MistNet' dependencies from files.
#'
#' @param version The 'LibTorch' version to install.
#' @param libtorch The installation archive file to use for 'LibTorch'. Shall be a \code{"file://"} URL scheme.
#' @param libmistnet The installation archive file to use for 'MistNet'. Shall be a \code{"file://"} URL scheme.
#' @param mistnet_model The installation archive file to use for the model. Shall be a \code{"file://"} URL scheme. Is optional!
#' @param ... other parameters to be passed to `install_torch()`
#'
#' @return a list with character urls
#'
#' @details
#'
#' When [install_mistnet()] initiated download is not possible, but installation archive files are
#' present on local filesystem, [install_mistnet_from_file()] can be used as a workaround to installation issues.
#' \code{"libtorch"} is the archive containing all 'LibTorch' modules, and \code{"libmistnet"} is the 'C' interface to 'LibTorch'
#' that is used for the 'R' package. Both are highly platform dependent, and should be checked through [get_install_urls()]
#'
#' ```R
#' > get_install_urls()
#' $libtorch
#' [1] "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip"
#'
#' $libmistnet
#' [1] "https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/Linux-cpu.zip"
#'
#' $mistnet_model
#' [1] "http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt"
#' ```
#'
#' In a terminal, download above zip-files.
#' ```R
#' %> mkdir /tmp/myfiles
#' %> cd /tmp/myfiles
#' %> wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip
#' %> wget https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/Linux-cpu.zip
#' %> wget http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt
#' ```
#' Then in R, type:
#' ```R
#' > install_mistnet_from_file(libtorch="file:///tmp/myfiles/libtorch-cxx11-abi-shared-with-deps-1.10.2+cpu.zip",
#'      libmistnet="file:///tmp/myfiles/Linux-cpu.zip",
#'      mistnet_model="file:///tmp/myfiles/mistnet_nexrad.pt")
#' ```
#' @export
#'
#' @examples
#' # get paths to files to be downloaded
#' get_install_urls()
#' # download the files to a directory on disk, e.g. to /tmp/myfile,
#' # then install with:
#' \dontrun{
#' install_mistnet_from_file(
#'      libtorch="file:///tmp/myfiles/libtorch-cxx11-abi-shared-with-deps-1.10.2+cpu.zip",
#'      libmistnet="file:///tmp/myfiles/Linux-cpu.zip",
#'      mistnet_model="file:///tmp/myfiles/mistnet_nexrad.pt")
#' }
#'
#' @seealso
#' * [install_mistnet()]
install_mistnet_from_file <- function(version = "1.12.1", libtorch, libmistnet, mistnet_model=NULL, ...) {
  assert_that(version %in% supported_pytorch_versions,
              msg = paste("version should be",paste(supported_pytorch_versions, collapse = " or ")))

  assert_that(inherits(url(libtorch), "file"))
  assert_that(inherits(url(libmistnet), "file"))

  install_config[[version]][["cpu"]][[install_os()]][["libtorch"]][["url"]] <- libtorch
  install_config[[version]][["cpu"]][[install_os()]][["libmistnet"]] <- libmistnet

  install_mistnet(version = version, type = "cpu", install_config = install_config, ...)

  if (!is.null(mistnet_model)) {
    install_mistnet_model(from_url=mistnet_model)
  }
}

#' List of installation files to download
#'
#' List the 'LibTorch' and 'MistNet' files to download as local files
#' in order to proceed with [install_mistnet_from_file()].
#'
#' @param version The 'LibTorch' version to install.
#' @param type The installation type for 'LibTorch'. Valid value is currently \code{"cpu"}.
#'
#' @export
#'
#' @return a named list with character urls
#'
#' @seealso
#' * [install_mistnet_from_file()]
get_install_urls <- function(version = "1.10.2", type = install_type(version = version)) {
  assert_that(version %in% supported_pytorch_versions)
  libtorch <- install_config[[version]][[type]][[install_os()]][["libtorch"]][["url"]]
  libmistnet <- install_config[[version]][[type]][[install_os()]][["libmistnet"]]
  mistnet_model <- "http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt"
  list(libtorch = libtorch, libmistnet = libmistnet, mistnet_model = mistnet_model)
}
