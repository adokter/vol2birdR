# Install 'MistNet' libraries

Installs libraries and dependencies for using 'MistNet'.

## Usage

``` r
install_mistnet(
  version = "1.12.1",
  reinstall = FALSE,
  path = install_path(),
  timeout = 360,
  ...
)
```

## Arguments

- version:

  The 'LibTorch' version to install.

- reinstall:

  Re-install 'MistNet' even if its already installed?

- path:

  Optional path to install or check for an already existing
  installation.

- timeout:

  Optional timeout in seconds for large file download.

- ...:

  other optional arguments (like `` `load` `` for manual installation).

## Value

no value returned. Installs libraries into the package

## Details

By default libraries are installed in the
'tools::R_user_dir("vol2birdR", "data")' directory.

When using `path` to install in a specific location, make sure the
`MISTNET_HOME` environment variable is set to this same path to reuse
this installation.

The `TORCH_INSTALL` environment variable can be set to `0` to prevent
auto-installing 'LibTorch and `TORCH_LOAD` set to `0` to avoid loading
dependencies automatically. These environment variables are meant for
advanced use cases and troubleshooting only.

When timeout error occurs during library archive download, or length of
downloaded files differ from reported length, an increase of the
`timeout` value should help.

## See also

- [`install_mistnet_from_file()`](https://adriaandokter.com/vol2bird/reference/install_mistnet_from_file.md)

## Examples

``` r
# \donttest{
install_mistnet()
#> Starting installation of MistNet...
#> Checking write permissions...
#> Installing MistNet libraries...
#> Initializing MistNet...
#> MistNet initialized successfully.
#> MistNet installation complete.
# }
```
