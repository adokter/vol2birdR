test_that("example polar volume file exists",{
  pvolfile <- system.file("extdata", "volume.h5", package = "vol2birdR")
  expect_true(file.exists(pvolfile))
})

# prepare test file names
tmpdir <- tempdir()
pvolfile_in <- paste(tmpdir, "pvol.h5", sep = "/")
pvolfile_out <- paste(tmpdir,"pvol_out.h5", sep = "/")
vpfile1 <- paste(tmpdir, "vp1.h5", sep = "/")
vpfile2 <- paste(tmpdir, "vp2.h5", sep = "/")

test_that("vol2bird writes output files", {
  skip_if_no_temp_access()
  expect_true(file.copy(system.file("extdata", "volume.h5", package = "vol2birdR"), pvolfile_in, overwrite = TRUE))
  conf <- vol2bird_config()
  expect_s4_class(conf,"Rcpp_Vol2BirdConfig")
  # suppress dealiasing warning messages:
  conf$maxNyquistDealias = 1
  vol2bird(file = pvolfile_in, config = conf, vpfile = vpfile1, verbose = FALSE)
  expect_true(file.exists(vpfile1))
  expect_false(file.exists(pvolfile_out))
  vol2bird(file = pvolfile_in, config = conf, vpfile = vpfile1, pvolfile_out = pvolfile_out, verbose = FALSE)
  expect_true(file.exists(pvolfile_out))
})

test_that("vol2bird parses configurations", {
  # check that config object is updated
  conf1 <- vol2bird_config()
  # suppress dealiasing warning messages:
  conf1$maxNyquistDealias = 1
  conf2 <- vol2bird_config(conf1)
  vol2bird(file = pvolfile_in, config = conf1, update_config = FALSE, verbose = FALSE)
  expect_true(are_equal(conf1,conf2))
  vol2bird(file = pvolfile_in, config = conf1, update_config = TRUE, verbose = FALSE)
  expect_false(are_equal(conf1,conf2))
  # check that profile file with more altitude layers is larger
  conf2 <- vol2bird_config(conf1)
  conf1$nLayers=25
  conf2$nLayers=50
  output1 <- capture.output(vol2bird(file = pvolfile_in, config = conf1, verbose = TRUE))
  output2 <- capture.output(vol2bird(file = pvolfile_in, config = conf2, verbose = TRUE))
  expect_lt(length(output1), length(output2))
})
