## vol2birdR 0.2.1
Changes into how system dependencies are located, to address a failing build on CRAN's m1mac as a result of the proj system library not being found. Now first trying pkg-config on all architectures. If it can't find pkg-config also try try homebrew on mac. Using that information, we first try to build with proj.4 support. If that doesn't work we try proj.6
