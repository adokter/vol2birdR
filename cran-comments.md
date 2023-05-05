# vol2birdR 1.0.0
* New release attempt, addressing copyright issues after initial CRAN review
* New macros for locating proj system libraries. Previous ax_lib_proj.m4 has been replaced by
  simplified check_proj.m4 script. Copyright for this script lies with the package authors.
* As requested by reviewer, compiled code has been removed from inst and moved to src.
* All header files have been moved to src/includes
* Support for IRIS data format has been removed entirely, which was copyrighted by Vaisala
  We are in contact with Vaisala for approval for using the formerly included piece of coded.
* 
* Adding information on contributors and copyright holders to DESCRIPTION file, using comments
