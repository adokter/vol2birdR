# Changelog

## vol2birdR 1.3.1

- No changes for users, fixes compiler warnings as requested by CRAN
  ([\#160](https://github.com/adokter/vol2birdR/issues/160)).

## vol2birdR 1.3.0

CRAN release: 2026-06-09

### New Features

- Add functionality to create profiles relative to ground level and
  antenna level (in addition to pre-existing default: sea level)
  ([\#138](https://github.com/adokter/vol2birdR/issues/138),
  [\#142](https://github.com/adokter/vol2birdR/issues/142)).

- Add configuration options to
  [`vol2bird_config()`](https://adriaandokter.com/vol2bird/reference/vol2bird_config.md): (1)
  `groundHeightParam` to specify the scan parameter containing digital
  elevation information for ground level profiles. (2) `heightReference`
  to specify the reference height (`sea`, `ground` or `antenna`)
  ([\#138](https://github.com/adokter/vol2birdR/issues/138),
  [\#142](https://github.com/adokter/vol2birdR/issues/142)).

- Add `height_reference` as an output column to csv profile output,
  adopting VPTS CSV v1.1 format
  ([\#154](https://github.com/adokter/vol2birdR/issues/154)).

- Add TDWR radar station info
  ([\#104](https://github.com/adokter/vol2birdR/issues/104)). Caution:
  wavelength of TDWR stations not parsing correctly yet
  ([\#155](https://github.com/adokter/vol2birdR/issues/155)).

- Align radar locations and antenna heights with latest metadata
  provided by NCEI
  ([\#144](https://github.com/adokter/vol2birdR/issues/144)).

- add missing KHDC radar station info
  ([\#148](https://github.com/adokter/vol2birdR/issues/148)).

- Distinguish between stdout and stderr messages
  ([\#142](https://github.com/adokter/vol2birdR/issues/142), 67e021f).

### Bugfixes

- fix beam width attribute in polar volume object
  ([\#153](https://github.com/adokter/vol2birdR/issues/153)).

- Fixes a rare segfault identified on Mac when reading a corrupted
  NEXRAD file
  ([\#102](https://github.com/adokter/vol2birdR/issues/102)).

- Improve messaging when reading ill-formatted ODIM hdf5 file
  ([\#136](https://github.com/adokter/vol2birdR/issues/136)).

- Abort graciously when encountering invalid ray indices in legacy
  NEXRAD files
  ([\#147](https://github.com/adokter/vol2birdR/issues/147)).

- Fix console print formatting
  ([\#139](https://github.com/adokter/vol2birdR/issues/139)).

- Bump to latest versions of Github runner images for continuous
  integration of tests
  ([\#150](https://github.com/adokter/vol2birdR/issues/150),
  [\#151](https://github.com/adokter/vol2birdR/issues/151)).

## vol2birdR 1.2.1

CRAN release: 2025-09-02

- Fixes a bucket link to `unidata-nexard-level2` in the documentation
  ([\#129](https://github.com/adokter/vol2birdR/issues/129)).

## vol2birdR 1.2.0

CRAN release: 2025-09-02

- Fixes a windows bug in the handling of temporary files that caused
  `rls2odim()` and
  [`vol2bird()`](https://adriaandokter.com/vol2bird/reference/vol2bird.md)
  to become unresponsive after reading a corrupted file
  ([\#114](https://github.com/adokter/vol2birdR/issues/114)).

- Update NEXRAD bucket to `unidata-nexrad-level2`
  ([\#129](https://github.com/adokter/vol2birdR/issues/129)).

- Upgrade to windows-2022 github runner to build mistnet library
  ([\#130](https://github.com/adokter/vol2birdR/issues/130)).

## vol2birdR 1.1.1

CRAN release: 2025-06-25

- Fixes a bug when using a cluttermap in
  [`vol2bird_config()`](https://adriaandokter.com/vol2bird/reference/vol2bird_config.md).
  Cluttermaps are now decoded by taking into account the the gain and
  offset attributes
  ([\#122](https://github.com/adokter/vol2birdR/issues/122)).

## vol2birdR 1.1.0

CRAN release: 2025-06-13

- Change to Rcpp messages for warnings and messages by vol2birdR, to
  allow message suppression in R
  ([\#115](https://github.com/adokter/vol2birdR/issues/115),
  [\#116](https://github.com/adokter/vol2birdR/issues/116)).

## vol2birdR 1.0.9

CRAN release: 2025-03-28

- Bugfix for rounding error in seconds of timestamp written in VPTS CSV
  files for NEXRAD data
  ([\#112](https://github.com/adokter/vol2birdR/issues/112)).

## vol2birdR 1.0.8

CRAN release: 2025-03-27

- Removes a stray DEBUG warning message that isn’t a true warning
  ([\#103](https://github.com/adokter/vol2birdR/issues/103)).
- Add timestamp seconds to VPTS CSV output
  ([\#105](https://github.com/adokter/vol2birdR/issues/105)).
- Add
  [`mistnet_installed()`](https://adriaandokter.com/vol2bird/reference/mistnet_installed.md)
  function to test if mistnet installation is complete
  ([\#109](https://github.com/adokter/vol2birdR/issues/109)).

## vol2birdR 1.0.7

CRAN release: 2025-02-21

- Fixes a segfault that occurred for mistnet runs on data with missing
  parameters ([\#29](https://github.com/adokter/vol2birdR/issues/29)).
- Reinstalls no longer require a redownload of mistnet library files by
  using persistent cache
  ([\#94](https://github.com/adokter/vol2birdR/issues/94)).
- Upgrade mistnet library build runners to macos-latest and
  ubuntu-latest
  ([\#66](https://github.com/adokter/vol2birdR/issues/66),#69).

## vol2birdR 1.0.6

CRAN release: 2025-02-07

- Fix warning message `pj_obj_create: Cannot find proj.db`
  ([\#50](https://github.com/adokter/vol2birdR/issues/50))
- Added an automatic mistnet library build for M1 macs
  ([\#82](https://github.com/adokter/vol2birdR/issues/82))

## vol2birdR 1.0.5

CRAN release: 2024-09-27

Addresses several build warning messages as required by CRAN

- Fixed broken hdf5 info link (c628cae)
- Suppressed warnings associated with Rtools43 for Windows builds
  (aee7ca9)
- Added overflow check for nCells (e6bd1bd)
- Explicitly specified C++17 standard in Windows build (aee7ca9)

## vol2birdR 1.0.4

CRAN release: 2024-08-22

- Reduced loading messages for mistnet
  ([\#72](https://github.com/adokter/vol2birdR/issues/72))
- Updated linking for Rtools44 compatibility
  ([\#73](https://github.com/adokter/vol2birdR/issues/73),
  [\#74](https://github.com/adokter/vol2birdR/issues/74))

## vol2birdR 1.0.3

CRAN release: 2024-07-13

- Refactored conditional linking of rtools43 dependencies
  ([\#60](https://github.com/adokter/vol2birdR/issues/60))
- Removed unused param from documentation in cpp_vol2bird_version()
- Made write binary mode explicit for Windows Server compatibility
  ([\#57](https://github.com/adokter/vol2birdR/issues/57))
- Fixed a rare segfault on specific nexrad files with missing velocity
  data ([\#61](https://github.com/adokter/vol2birdR/issues/61))

## vol2birdR 1.0.2

CRAN release: 2024-01-19

- Refactored linking of lsharpyuv according to rtools43 instructions
  ([\#54](https://github.com/adokter/vol2birdR/issues/54),#55,#56)
- Changed package maintainer

## vol2birdR 1.0.1

CRAN release: 2023-05-19

- Added pkg-config as a dependency on linux systems and M1 Mac
- Added check_proj to m4 files which checks for the PROJ library
- Corrected minor typos

## vol2birdR 1.0.0

- Initial CRAN release. All issues included in this release can be found
  [here](https://github.com/adokter/vol2birdR/milestone/1?closed=1).

## vol2birdR 0.2.2

- New macros for locating system libraries.
- Fixes for valgrind additional warnings
- Adding information on contributors and copyright holders to
  DESCRIPTION file

## vol2birdR 0.2.1

Changes to how system libraries are located to fix failing build on M1
Mac on CRAN

## vol2birdR 0.2.0

CRAN release: 2022-11-16

Initial CRAN reviewed version of new package
