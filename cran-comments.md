# vol2birdR 1.0.0
* Addressing copyright issues after initial CRAN review in Nov 2022
* New macros for locating proj system libraries. Previous ax_lib_proj.m4 has been replaced by
  the simplified and self-written check_proj.m4 script.
* Compiled code has been removed from inst and moved to src.
* All header files have been moved to src/includes
* Support for the IRIS data format has been removed entirely, which contained a previously
  unnoticed code snipped copyrighted by Vaisala. We are in contact with Vaisala for approval in 
  a future release.
* Carefully reviewed all files of the package and declared copyright holders and authors in
  DESCRIPTION file and inst/COPYRIGHT file
# vol2birdR 1.0.1
* Added pkg-config as a dependency on linux systems and M1 Mac
* Added check_proj to m4 files which checks for the PROJ library
# vol2birdR 1.0.2
* Added pkg-config as a dependency on linux systems and M1 Mac
* Added check_proj to m4 files which checks for the PROJ library