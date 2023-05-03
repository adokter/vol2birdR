#' Skip test if tempdir() not accessible
#'
#' Some function tests require tempdir() to be writeable in package vol2birdR.
#' This helper function allows to skip a test if tempdir() is not accessible
#' Inspired by <https://testthat.r-lib.org/articles/skipping.html#helpers>.
#'
#' @keywords internal
skip_if_no_temp_access <- function() {
  tmpdir <- tempdir()
  if (is.writeable(tmpdir) && is.readable(tmpdir)) {
    return(invisible(TRUE))
  }
  testthat::skip("tempdir() not accessible")
}
