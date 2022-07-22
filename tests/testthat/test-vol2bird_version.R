test_that("numeric version is returned",{
  expect_s3_class(vol2bird_version(), "numeric_version")
})
