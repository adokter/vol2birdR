# Check if MistNet installation is complete

Checks if the 'LibTorch' and 'MistNet' libraries have been installed,
and that the mistnet model file has been downloaded.

## Usage

``` r
mistnet_installed(path, verbose = FALSE)
```

## Arguments

- path:

  Optional non-default file path to check for the mistnet model file.

- verbose:

  When TRUE print informative messages on missing library and model
  files.

## Value

TRUE if the 'LibTorch' and 'MistNet' libraries can be found and the and
the MistNet model file can be located, otherwise FALSE.

## See also

- [`mistnet_exists()`](https://adriaandokter.com/vol2bird/reference/mistnet_exists.md)

- [`install_mistnet()`](https://adriaandokter.com/vol2bird/reference/install_mistnet.md)

- [`install_mistnet_model()`](https://adriaandokter.com/vol2bird/reference/install_mistnet_model.md)
