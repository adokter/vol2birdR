# Lists the 'LibTorch' versions installable on the current platform

Not every supported 'LibTorch' version is available for every platform:
Intel macOS stops at 2.1.2 because that is the last release 'PyTorch'
built for it, while 2.14.0 is available everywhere else.

## Usage

``` r
supported_pytorch_versions_for_platform(platform = install_platform())
```

## Arguments

- platform:

  the platform key to check, defaults to the current platform

## Value

a character vector of versions, in the order declared by
`supported_pytorch_versions`
