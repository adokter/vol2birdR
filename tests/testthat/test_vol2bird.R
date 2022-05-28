# Tests the Vol2Bird class 
#
#library(vol2birdR)

#library(testthat)

test_that("verbose",{
  classUnderTest<-Vol2Bird$new()
  expect_equal(classUnderTest$verbose, FALSE)
  classUnderTest$verbose<-TRUE
  expect_equal(classUnderTest$verbose, TRUE)
})

