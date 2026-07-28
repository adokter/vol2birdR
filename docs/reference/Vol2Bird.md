# Calculate a vertical profile (`vp`) from a polar volume (`pvol`) file

Calculates a vertical profile of biological scatterers (`vp`) from a
polar volume (`pvol`) file using the algorithm
[vol2bird](https://github.com/adokter/vol2bird/) (Dokter et al. 2011
[doi:10.1098/rsif.2010.0116](https://doi.org/10.1098/rsif.2010.0116) ).

## Usage

``` r
vol2bird(
  file,
  config,
  vpfile = "",
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

- vpfile:

  Character. File name. When provided with .csv extension, writes a
  vertical profile in [VPTS CSV format](https://aloftdata.eu/vpts-csv/).
  Provided with another or no extension, writes a vertical profile in
  the ODIM HDF5 format to disk.

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
  `vol2bird()` is used in loops like
  [`lapply()`](https://rdrr.io/r/base/lapply.html) or in parallel
  processes.

## Value

No value returned, creates a file specified by `file` argument

## See also

- [`vol2bird_config()`](https://adriaandokter.com/vol2bird/reference/vol2bird_config.md)

## Examples

``` r
# Locate the polar volume example file
pvolfile <- system.file("extdata", "volume.h5", package = "vol2birdR")

# Create a configuration instance:
conf <- vol2bird_config()

# Define output filename (VPTS csv format)
output_file_csv <- paste0(tempdir(), "/vp.csv")

# Calculate the profile (VPTS csv output):
vol2bird(file = pvolfile, config = conf, vpfile = output_file_csv)
#> Running vol2birdSetUp
#> Warning: radial velocities will be dealiased...
#> # vol2bird Vertical Profile of Birds (VPB)
#> # source: WMO:02606,RAD:SE50,PLC:Angelholm,NOD:seang,ORG:82,CTY:643,CMT:Swedish radar
#> # polar volume input: /private/var/folders/85/mhhbjmj50wnb0_g0kntpzrw80000gr/T/RtmptUMjo8/temp_libpath25f276725783/vol2birdR/extdata/volume.h5
#> # date   time HGHT    u      v       w     ff    dd  sd_vvp gap dbz     eta   dens   DBZH   n   n_dbz n_all n_dbz_all
#> 20151018 1800    0     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800  200    NaN    NaN     NaN   NaN   NaN   3.27 T   2.88  674.1  61.28   4.87   536   817  4351 10602
#> 20151018 1800  400  -6.47 -13.41    6.55 14.89 205.8   3.22 F   5.54 1245.2 113.20   8.23  2116  4214  5304 17421
#> 20151018 1800  600  -6.79 -13.10  -16.88 14.75 207.4   3.32 F   4.69 1024.3  93.12   4.44  1869  4085  3144  8098
#> 20151018 1800  800  -6.56 -11.58  -12.92 13.30 209.5   3.90 F   0.45  385.7  35.06   1.34  1155  3897  2043  7801
#> 20151018 1800 1000  -5.07  -9.50   30.60 10.77 208.1   4.07 F  -1.94  222.1  20.19  -0.49   744  4370  1380  7829
#> 20151018 1800 1200  -5.49  -8.52   22.43 10.14 212.8   4.84 F  -1.86  226.3  20.57  -1.10   446  2276   747  3206
#> 20151018 1800 1400  -7.58  -9.40   57.81 12.07 218.9   4.80 F  -2.23  208.0  18.91  -1.10   198  1694   483  2851
#> 20151018 1800 1600  -3.69  -9.78   35.81 10.46 200.7   4.51 F  -2.50  195.3  17.75  10.83   161  2045   315  3213
#> 20151018 1800 1800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0

# Define output filename (ODIM h5 format)
output_file_h5 <- paste0(tempdir(), "/vp.h5")

# Calculate the profile (ODIM h5 output):
vol2bird(file = pvolfile, config = conf, vpfile = output_file_h5)
#> Running vol2birdSetUp
#> Warning: radial velocities will be dealiased...
#> # vol2bird Vertical Profile of Birds (VPB)
#> # source: WMO:02606,RAD:SE50,PLC:Angelholm,NOD:seang,ORG:82,CTY:643,CMT:Swedish radar
#> # polar volume input: /private/var/folders/85/mhhbjmj50wnb0_g0kntpzrw80000gr/T/RtmptUMjo8/temp_libpath25f276725783/vol2birdR/extdata/volume.h5
#> # date   time HGHT    u      v       w     ff    dd  sd_vvp gap dbz     eta   dens   DBZH   n   n_dbz n_all n_dbz_all
#> 20151018 1800    0     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800  200    NaN    NaN     NaN   NaN   NaN   3.27 T   2.88  674.1  61.28   4.87   536   817  4351 10602
#> 20151018 1800  400  -6.47 -13.41    6.55 14.89 205.8   3.22 F   5.54 1245.2 113.20   8.23  2116  4214  5304 17421
#> 20151018 1800  600  -6.79 -13.10  -16.88 14.75 207.4   3.32 F   4.69 1024.3  93.12   4.44  1869  4085  3144  8098
#> 20151018 1800  800  -6.56 -11.58  -12.92 13.30 209.5   3.90 F   0.45  385.7  35.06   1.34  1155  3897  2043  7801
#> 20151018 1800 1000  -5.07  -9.50   30.60 10.77 208.1   4.07 F  -1.94  222.1  20.19  -0.49   744  4370  1380  7829
#> 20151018 1800 1200  -5.49  -8.52   22.43 10.14 212.8   4.84 F  -1.86  226.3  20.57  -1.10   446  2276   747  3206
#> 20151018 1800 1400  -7.58  -9.40   57.81 12.07 218.9   4.80 F  -2.23  208.0  18.91  -1.10   198  1694   483  2851
#> 20151018 1800 1600  -3.69  -9.78   35.81 10.46 200.7   4.51 F  -2.50  195.3  17.75  10.83   161  2045   315  3213
#> 20151018 1800 1800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 2800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 3800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4000     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4200     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4400     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4600     na     na      na    na    na     na T     na     na     na     na     0     0     0     0
#> 20151018 1800 4800     na     na      na    na    na     na T     na     na     na     na     0     0     0     0

# clean up
unlink(c(output_file_csv, output_file_h5))
```
