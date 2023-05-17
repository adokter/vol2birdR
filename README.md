
<!-- README.md is generated from README.Rmd. Please edit that file -->
<!-- badges: start -->

[![CRAN
status](https://www.r-pkg.org/badges/version/vol2birdR)](https://cran.r-project.org/package=vol2birdR)
[![R-CMD-check](https://github.com/adokter/vol2birdR/workflows/R-CMD-check/badge.svg)](https://github.com/adokter/vol2birdR/actions)

<!-- badges: end -->

# vol2birdR

‘**vol2birdR**’ is an ‘R’ package for the ‘vol2bird’ algorithm for
calculating vertical profiles of birds and other biological scatterers
from weather radar data.

It also provides an ‘R’ interface to the ‘MistNet’ convolutional neural
network for precipitation segmentation, installing PyTorch libraries and
model.

‘**vol2birdR**’ can be used as a stand-alone package, but we recommend
[bioRad](https://adriaandokter.com/bioRad/) as the primary user
interface, with ‘**vol2birdR**’ acting as a dependency of
[bioRad](https://adriaandokter.com/bioRad/).

# Install

‘**vol2birdR**’ is available for all major platforms (Linux, OS X and
Windows).

For OS X and Linux the GNU Scientific Library (GSL), PROJ and HDF5
libraries need to be installed as system libraries prior to installation
of ‘**vol2birdR**’:

<details>
<summary>
Additional information when installing the dependencies on macOS
</summary>

Since the installation process requires the [Homebrew](https://brew.sh/)
package manager you will have to install it. Open a terminal and issue
the following command:

      /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

When the installation has completed it will print out some additional
information that is essential to follow.

    ==> Next steps:
    - Run these two commands in your terminal to add Homebrew to your PATH:
        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> /Users/anders/.zprofile
        eval "$(/opt/homebrew/bin/brew shellenv)"
    - Run brew help to get started
    - Further documentation:
        https://docs.brew.sh

You need to ensure that you follow the above two commands. The first one
will add the necessary environment variables to your user

        echo 'eval "$(/opt/homebrew/bin/brew shellenv)"' >> /Users/anders/.zprofile

The second command will ensure that you get the necessary environment
variables into the terminal where you ran the installation process of
Homebrew.

        eval "$(/opt/homebrew/bin/brew shellenv)"

</details>

| System                                      | Command                                                                      |
|:--------------------------------------------|:-----------------------------------------------------------------------------|
| **OS X (using Homebrew)**                   | `brew install hdf5 proj gsl pkg-config`                                      |
| **Debian-based systems (including Ubuntu)** | `sudo apt-get install libhdf5-dev libproj-dev gsl-bin libgsl-dev pkg-config` |
| **Systems supporting yum and RPMs**         | `sudo yum install hdf5-devel proj-devel gsl gsl-devel pkgconfig`             |

Next, you can install the released version of ‘vol2birdR’ from
[CRAN](https://CRAN.R-project.org) with:

``` r
install.packages("vol2birdR")
```

Alternatively, you can install the latest development version from
[GitHub](https://github.com/adokter/bioRad) with:

``` r
# install.packages("devtools")
devtools::install_github("adokter/vol2birdR")
```

Then load the package with:

``` r
library(vol2birdR)
```

### MistNet installation

MistNet is a deep convolution neural net for segmenting out
precipitation from radar data, see Lin et al. 2019. To use MistNet,
follow the following additional installation steps in R:

    # STEP 1: install additional libraries for using MistNet:
    library(vol2birdR)
    install_mistnet()

After completing this step, the following command should evaluate to
`TRUE`:

    mistnet_exists()

Next, download the mistnet model. Note that the model file is large,
over 500Mb.

    # STEP 2: download mistnet model:
    install_mistnet_model()

See
[vignette](https://adriaandokter.com/vol2birdR/articles/vol2birdR.html)
for additional installation information

## References:

Citation for ‘vol2bird’ algorithm:

-   [**Bird migration flight altitudes studied by a network of
    operational weather
    radars**](https://doi.org/10.1098/rsif.2010.0116) Dokter AM, Liechti
    F, Stark H, Delobbe L, Tabary P, Holleman I J. R. Soc. Interface,
    **8**, 30–43, 2011, DOI
    [10.1098/rsif.2010.0116](https://doi.org/10.1098/rsif.2010.0116)

Paper describing recent algorithm extensions and the bioRad package:

-   [**bioRad: biological analysis and visualization of weather radar
    data**](https://doi.org/10.1111/ecog.04028) Dokter AM, Desmet P,
    Spaaks JH, van Hoey S, Veen L, Verlinden L, Nilsson C, Haase G,
    Leijnse H, Farnsworth A, Bouten W, Shamoun-Baranes J. Ecography,
    **42**, 852-860, 2019, DOI
    [10.1111/ecog.04028](https://doi.org/10.1111/ecog.04028)

‘vol2bird’ implements dealiasing using the torus mapping method by Haase
and Landelius:

-   [**Dealiasing of Doppler radar velocities using a torus
    mapping**](https://doi.org/10.1175/1520-0426(2004)021%3C1566:DODRVU%3E2.0.CO;2)
    Haase G, Landelius T. Journal of Atmospheric and Oceanic Technology
    **21**, 1566-1573, 2004, DOI
    [10.1175/1520-0426(2004)021\<1566:DODRVU\>2.0.CO;2](https://doi.org/10.1175/1520-0426(2004)021%3C1566:DODRVU%3E2.0.CO;2)

Use the following citation for the ‘MistNet’ rain segmentation model:

-   [**MistNet: Measuring historical bird migration in the US using
    archived weather radar data and convolutional neural
    networks.**](https://doi.org/10.1111/2041-210X.13280) Lin T-Y,
    Winner K, Bernstein G, Mittal A, Dokter AM, Horton KG, Nilsson C,
    Van Doren BM, Farnsworth A, La Sorte FA, Maji S, Sheldon D. Methods
    in Ecology and Evolution, **10**, 1908-1922, 2019, DOI
    [10.1111/2041-210X.13280](https://doi.org/10.1111/2041-210X.13280)
