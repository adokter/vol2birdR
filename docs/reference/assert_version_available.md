# Validates a version/platform combination

Errors if the requested 'LibTorch' version has no build for the current
platform.

## Usage

``` r
assert_version_available(version, platform = install_platform())
```

## Arguments

- version:

  the requested 'LibTorch' version

- platform:

  the platform key to check, defaults to the current platform

## Value

invisibly `TRUE`, called for its side effect of erroring
