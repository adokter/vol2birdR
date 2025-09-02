# vol2birdR 1.1.1.9000 (development version)
* Fixes a windows bug in the handling of temporary files that caused `rls2odim()` and `vol2bird()` to become unresponsive after reading a corrupted file (#114).

* Update NEXRAD bucket to `unidata-nexrad-level2` (#208).

* Upgrade to windows-2022 github runner to build mistnet library (#130).

# vol2birdR 1.1.1
* Fixes a bug when using a cluttermap in `vol2bird_config()`. Cluttermaps are now decoded by taking into account the the gain and offset attributes (#122).

# vol2birdR 1.1.0
* Change to Rcpp messages for warnings and messages by vol2birdR, to allow message suppression in R (#115, #116).

# vol2birdR 1.0.9
* Bugfix for rounding error in seconds of timestamp written in VPTS CSV files for NEXRAD data (#112).

# vol2birdR 1.0.8
* Removes a stray DEBUG warning message that isn't a true warning (#103).
* Add timestamp seconds to VPTS CSV output (#105).
* Add `mistnet_installed()` function to test if mistnet installation is complete (#109).

# vol2birdR 1.0.7
* Fixes a segfault that occurred for mistnet runs on data with missing parameters (#29).
* Reinstalls no longer require a redownload of mistnet library files by using persistent cache (#94).
* Upgrade mistnet library build runners to macos-latest and ubuntu-latest (#66,#69).

# vol2birdR 1.0.6
* Fix warning message `pj_obj_create: Cannot find proj.db` (#50)
* Added an automatic mistnet library build for M1 macs (#82)

# vol2birdR 1.0.5
Addresses several build warning messages as required by CRAN

* Fixed broken hdf5 info link (c628cae)
* Suppressed warnings associated with Rtools43 for Windows builds (aee7ca9)
* Added overflow check for nCells (e6bd1bd)
* Explicitly specified C++17 standard in Windows build (aee7ca9)

# vol2birdR 1.0.4
* Reduced loading messages for mistnet (#72)
* Updated linking for Rtools44 compatibility (#73, #74)

# vol2birdR 1.0.3
* Refactored conditional linking of rtools43 dependencies (#60)
* Removed unused param from documentation in cpp_vol2bird_version()
* Made write binary mode explicit for Windows Server compatibility (#57)
* Fixed a rare segfault on specific nexrad files with missing velocity data (#61)

# vol2birdR 1.0.2
* Refactored linking of lsharpyuv according to rtools43 instructions (#54,#55,#56)
* Changed package maintainer

# vol2birdR 1.0.1
* Added pkg-config as a dependency on linux systems and M1 Mac
* Added check_proj to m4 files which checks for the PROJ library
* Corrected minor typos

# vol2birdR 1.0.0
* Initial CRAN release. All issues included in this release can be found [here](https://github.com/adokter/vol2birdR/milestone/1?closed=1).

# vol2birdR 0.2.2
* New macros for locating system libraries.
* Fixes for valgrind additional warnings
* Adding information on contributors and copyright holders to DESCRIPTION file

# vol2birdR 0.2.1
Changes to how system libraries are located to fix failing build on M1 Mac on CRAN

# vol2birdR 0.2.0
Initial CRAN reviewed version of new package
