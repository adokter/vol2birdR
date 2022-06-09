## vol2birdR 0.1.0
This version is the initial release of this package

## Test environments
* local OS X install, R 4.2.0
* local Ubuntu 20.04 LTS, 18.04 LTS, R 4.2.0
* local Windows install, R 4.2.0
* remote R-devel https://builder.r-hub.io.
* remote R-devel Windows https://win-builder.r-project.org/

## R CMD check results
There were no ERRORs or WARNINGs. 

We have the following NOTES:
- GNU make is a SystemRequirements.

## check_rhub() results
There were no ERRORs or WARNINGs. 

We have the following 3 NOTES:

❯ checking CRAN incoming feasibility ... NOTE
  
  New submission
  
  Found the following (possibly) invalid URLs:
    URL: https://doi.org/10.1111/2041-210X.13280
      From: man/vol2birdR-package.Rd
      Status: 503
      Message: Service Unavailable

❯ checking installed package size ... NOTE
    installed size is 10.9Mb
    sub-directories of 1Mb or more:
      libs   9.8Mb

❯ checking for GNU extensions in Makefiles ... NOTE
  GNU make is a SystemRequirements.

- installed size > 5Mb due to system dependency libraries being installed.
- The uploaded .tar.gz package size is < 1Mb
- We verified the https://doi.org/10.1111/2041-210X.13280 link is accessible

## check_win_devel() results:
There were no ERRORs or WARNINGs.

We have one NOTE:

❯ checking CRAN incoming feasibility ... NOTE

  New submission

  Found the following (possibly) invalid URLs:
    URL: https://doi.org/10.1111/2041-210X.13280
      From: man/vol2birdR-package.Rd
      Status: 503
      Message: Service Unavailable

We verified this link is accessible

## Downstream dependencies
There are currently no downstream dependencies for this package
