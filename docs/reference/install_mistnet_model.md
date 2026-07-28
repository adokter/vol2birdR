# Install 'MistNet' model file

Installs the 'MistNet' model file in 'PyTorch' format

## Usage

``` r
install_mistnet_model(
  reinstall = FALSE,
  path = file.path(torch_install_path(), "data", "mistnet_nexrad.pt"),
  timeout = 1800,
  from_url = "http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt",
  method = "libcurl",
  ...
)
```

## Arguments

- reinstall:

  Re-install the model even if its already installed

- path:

  Optional path to install or check for an already existing
  installation.

- timeout:

  Optional timeout in seconds for large file download.

- from_url:

  From where the 'MistNet' model file should be downloaded.

- method:

  The download method to use, see
  [download.file](https://rdrr.io/r/utils/download.file.html)

- ...:

  other optional arguments (like `` `load` `` for manual installation).

## Value

No value returned, this function downloads a file

## Details

Download and install the 'MistNet' model file. By default the library is
downloaded to data/mistnet_nexrad.pt in the
'tools::R_user_dir("vol2birdR", "data")' directory.

Alternatively, the model file can be downloaded to a different location,
which has the advantage that it doesn't have to be redownloaded after a
reinstall of 'vol2birdR'.

'vol2birdR' will automatically detect the model file if it is downloaded
to `/opt/vol2bird/etc/mistnet_nexrad.pt`, which can be done as follows

    install_mistnet_model(path="/opt/vol2bird/etc/mistnet_nexrad.pt")

## Examples

``` r
# \donttest{
install_mistnet_model()
#> [1] TRUE
# }
```
