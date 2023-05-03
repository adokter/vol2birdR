#' Retrieve or set the nexrad location file
#'
#' Retrieves and sets the path of the RSL nexrad location file
#' @details The RSL library stores the locations and names of NEXRAD stations in a fixed-width
#' text file. This function retrieves the path of the location file, and allows one to set
#' the path to a different location file.
#'
#' @param file A string containing the path of a location file. Do not specify to retrieve path of current location file.
#' @return The path of the nexrad location file
#' @examples
#' # return current location file
#' nexrad_station_file()
#' # store nexrad station file path
#' file_path <- nexrad_station_file()
#' # set station location file
#' nexrad_station_file(file_path)
#'
#' @export
nexrad_station_file <- function(file){
  if(missing(file)){
    return(cpp_vol2bird_get_wsr88d_site_location())
  }
  assert_that(file.exists(file))
  data=tryCatch(utils::read.delim(file),error=function(e) NULL)
  assert_that(is.data.frame(data),msg=paste0("file '",file,"' is not a fixed-width data table"))
  assert_that(ncol(data)==11,msg=sprintf("expecting a file with 11 columns, found %i",ncol(data)))
  cpp_vol2bird_set_wsr88d_site_location(file)
  return(file)
}
