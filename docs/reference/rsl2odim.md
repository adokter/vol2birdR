# Convert a NEXRAD polar volume file to an ODIM polar volume file

Convert a NEXRAD polar volume file to an ODIM polar volume file

## Usage

``` r
rsl2odim(
  file,
  config,
  pvolfile_out = "",
  verbose = TRUE,
  update_config = FALSE
)
```

## Arguments

- file:

  Character (vector). Either a path to a single radar polar volume
  (`pvol`) file containing multiple scans/sweeps, or multiple paths to
  scan files containing a single scan/sweep. The file data format should
  be either 1)
  [ODIM](https://github.com/adokter/vol2bird/blob/master/doc/OPERA2014_O4_ODIM_H5-v2.2.pdf)
  format, which is the implementation of the OPERA data information
  model in the [HDF5](https://www.hdfgroup.org/solutions/hdf5/)
  format, 2) NEXRAD format supported by the ['RSL'
  library](https://trmm-fc.gsfc.nasa.gov/trmm_gv/software/rsl/) or 3)
  Vaisala IRIS (IRIS RAW) format. IRIS format is not available on CRAN,
  see vol2birdR development version on Github.

- config:

  optional configuration object of class `Rcpp_Vol2BirdConfig`,
  typically output from
  [vol2bird_config](https://adriaandokter.com/vol2bird/reference/vol2bird_config.md)

- pvolfile_out:

  Character. File name. When provided, writes a polar volume (`pvol`)
  file in the ODIM HDF5 format to disk. Useful for converting 'RSL'
  formats to ODIM, and for adding 'MistNet' segmentation output.

- verbose:

  logical. When TRUE print profile output to console.

- update_config:

  logical. When TRUE processing options that are determined based on
  input file characteristics are returned and updated in the object
  specified by the `config` argument. Do not set to `TRUE` when
  [`vol2bird()`](https://adriaandokter.com/vol2bird/reference/vol2bird.md)
  is used in loops like [`lapply()`](https://rdrr.io/r/base/lapply.html)
  or in parallel processes.

## Value

No value returned, creates a file specified by `pvolfile_out` argument.

## See also

- [`vol2bird_config()`](https://adriaandokter.com/vol2bird/reference/vol2bird_config.md)

## Examples

``` r
# \donttest{
# define filenames
nexrad_file <- paste0(tempdir(),"/KBGM20221001_000243_V06")
odim_file <- paste0(tempdir(),"/KBGM20221001_000243_V06.h5")
# download NEXRAD file:
source <- "https://unidata-nexrad-level2.s3.amazonaws.com/2022/10/01/KBGM/KBGM20221001_000243_V06"
download.file(source, destfile = nexrad_file, mode="wb")
# convert NEXRAD file to ODIM hdf5 format:
rsl2odim(nexrad_file, pvolfile_out = odim_file)
#> Filename = /var/folders/85/mhhbjmj50wnb0_g0kntpzrw80000gr/T//Rtmp53T9Yq/KBGM20221001_000243_V06, callid = KBGM
#> Reading RSL polar volume with nominal time 20221001-000243, source: RAD:KBGM,PLC:BINGHAMTON,state:NY,radar_name:KBGM
# clean up
file.remove(nexrad_file)
#> [1] TRUE
file.remove(odim_file)
#> [1] TRUE
# }
```
