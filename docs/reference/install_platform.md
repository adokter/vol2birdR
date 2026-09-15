# Returns the platform key used to index `install_config`

Unlike
[`install_os`](https://adriaandokter.com/vol2bird/reference/install_os.md)
this distinguishes Intel from Apple Silicon macOS, which are separate
entries in `install_config` because 'PyTorch' ships different archives
for them (and, from 2.2 onwards, none at all for Intel).

## Usage

``` r
install_platform()
```

## Value

one of `"linux"`, `"windows"`, `"darwin"` or `"darwin-arm64"`
