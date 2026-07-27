# List of installation files to download

List the 'LibTorch' and 'MistNet' files to download as local files in
order to proceed with
[`install_mistnet_from_file()`](https://adriaandokter.com/vol2bird/reference/install_mistnet_from_file.md).

## Usage

``` r
get_install_urls(version = "1.10.2", type = install_type(version = version))
```

## Arguments

- version:

  The 'LibTorch' version to install.

- type:

  The installation type for 'LibTorch'. Valid value is currently
  `"cpu"`.

## Value

a named list with character urls

## See also

- [`install_mistnet_from_file()`](https://adriaandokter.com/vol2bird/reference/install_mistnet_from_file.md)
