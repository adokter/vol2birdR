# Install 'MistNet' libraries from files

Installs 'LibTorch' and 'MistNet' dependencies from files.

## Usage

``` r
install_mistnet_from_file(
  version = "1.12.1",
  libtorch,
  libmistnet,
  mistnet_model = NULL,
  ...
)
```

## Arguments

- version:

  The 'LibTorch' version to install.

- libtorch:

  The installation archive file to use for 'LibTorch'. Shall be a
  `"file://"` URL scheme.

- libmistnet:

  The installation archive file to use for 'MistNet'. Shall be a
  `"file://"` URL scheme.

- mistnet_model:

  The installation archive file to use for the model. Shall be a
  `"file://"` URL scheme. Is optional!

- ...:

  other parameters to be passed to `install_torch()`

## Value

a list with character urls

## Details

When
[`install_mistnet()`](https://adriaandokter.com/vol2bird/reference/install_mistnet.md)
initiated download is not possible, but installation archive files are
present on local filesystem, `install_mistnet_from_file()` can be used
as a workaround to installation issues. `"libtorch"` is the archive
containing all 'LibTorch' modules, and `"libmistnet"` is the 'C'
interface to 'LibTorch' that is used for the 'R' package. Both are
highly platform dependent, and should be checked through
[`get_install_urls()`](https://adriaandokter.com/vol2bird/reference/get_install_urls.md)

    > get_install_urls()
    $libtorch
    [1] "https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip"

    $libmistnet
    [1] "https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/Linux-cpu.zip"

    $mistnet_model
    [1] "http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt"

In a terminal, download above zip-files.

    %> mkdir /tmp/myfiles
    %> cd /tmp/myfiles
    %> wget https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-1.10.2%2Bcpu.zip
    %> wget https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/Linux-cpu.zip
    %> wget http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt

Then in R, type:

    > install_mistnet_from_file(libtorch="file:///tmp/myfiles/libtorch-cxx11-abi-shared-with-deps-1.10.2+cpu.zip",
         libmistnet="file:///tmp/myfiles/Linux-cpu.zip",
         mistnet_model="file:///tmp/myfiles/mistnet_nexrad.pt")

## See also

- [`install_mistnet()`](https://adriaandokter.com/vol2bird/reference/install_mistnet.md)

## Examples

``` r
# get paths to files to be downloaded
get_install_urls()
#> $libtorch
#> [1] "https://download.pytorch.org/libtorch/cpu/libtorch-macos-1.10.2.zip"
#> 
#> $libmistnet
#> [1] "https://s3.amazonaws.com/vol2bird-builds/vol2birdr/refs/heads/main/latest/macOS-cpu_1_10_2.zip"
#> 
#> $mistnet_model
#> [1] "http://mistnet.s3.amazonaws.com/mistnet_nexrad.pt"
#> 
# download the files to a directory on disk, e.g. to /tmp/myfile,
# then install with:
if (FALSE) { # \dontrun{
install_mistnet_from_file(
     libtorch="file:///tmp/myfiles/libtorch-cxx11-abi-shared-with-deps-1.10.2+cpu.zip",
     libmistnet="file:///tmp/myfiles/Linux-cpu.zip",
     mistnet_model="file:///tmp/myfiles/mistnet_nexrad.pt")
} # }
```
