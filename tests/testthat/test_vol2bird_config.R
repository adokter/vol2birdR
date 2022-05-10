# Tests the Vol2BirdConfig class and all the various options that are available.
#
library(vol2birdR)

#library(testthat)

test_that("elevMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(0.0, a$elevMin)
  a$elevMin<-45.0
  expect_equal(45.0, a$elevMin)
})

test_that("elevMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(90.0, a$elevMax)
  a$elevMax<-45.0
  #expect_equal(45.0, a$elevMax)
})

