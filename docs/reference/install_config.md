# Contains a list of 'MistNet' libraries for the various OS's

Note that not every 'LibTorch' version is available for every platform,
so this list is deliberately ragged: version `"2.1.2"` carries only an
Intel macOS entry (it is the last release 'PyTorch' shipped an x86_64
macOS build for), while `"2.14.0"` covers every platform except Intel
macOS. Use
[`supported_pytorch_versions_for_platform`](https://adriaandokter.com/vol2bird/reference/supported_pytorch_versions_for_platform.md)
rather than assuming a version is installable everywhere.

## Usage

``` r
install_config
```

## Details

The urls here must stay in step with the version table in
`mistnet/CMakeLists.txt`, which downloads 'LibTorch' to compile
`libmistnet` in CI. A mismatch yields a successful build followed by a
load-time failure, which is why the `libmistnet` artifacts are version
suffixed.
