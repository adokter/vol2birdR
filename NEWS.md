# vol2birdR 1.0.4
* reduce loading messages for mistnet (#72)
* update linking for Rtools44 compatibility (#73, #74)

# vol2birdR 1.0.3
* Refactored conditional linking of rtools43 dependencies (#60)
* Removed unused param from documentation in cpp_vol2bird_version()
* Made write binary mode explicit for Windows Server compatibility (#57)
* fix a rare segfault on specific nexrad files with missing velocity data (#61)

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
