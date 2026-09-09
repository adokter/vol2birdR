# Retrieve or set the nexrad location file

Retrieves and sets the path of the RSL nexrad location file

## Usage

``` r
nexrad_station_file(file)
```

## Arguments

- file:

  A string containing the path of a location file. Do not specify to
  retrieve path of current location file.

## Value

The path of the nexrad location file

## Details

The RSL library stores the locations and names of NEXRAD stations in a
fixed-width text file. This function retrieves the path of the location
file, and allows one to set the path to a different location file.

## Examples

``` r
# return current location file
nexrad_station_file()
#> [1] "/private/var/folders/85/mhhbjmj50wnb0_g0kntpzrw80000gr/T/Rtmp256Sp8/temp_libpath4a7740dfe62d/vol2birdR/librsl/wsr88d_locations.dat"
# store nexrad station file path
file_path <- nexrad_station_file()
# set station location file
nexrad_station_file(file_path)
#> [1] "/private/var/folders/85/mhhbjmj50wnb0_g0kntpzrw80000gr/T/Rtmp256Sp8/temp_libpath4a7740dfe62d/vol2birdR/librsl/wsr88d_locations.dat"
```
