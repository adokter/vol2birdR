# Tests the Vol2BirdConfig class and all the various options that are available.
#
#library(vol2birdR)

#library(testthat)

test_that("elevMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$elevMin, 0.0, tolerance = 0.0001)
  a$elevMin<-45.0
  expect_equal(a$elevMin, 45.0, tolerance = 0.0001)
})

test_that("elevMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$elevMax, 90.0, tolerance = 0.0001)
  a$elevMax<-45.0
  expect_equal(a$elevMax, 45.0, tolerance = 0.0001)
})

test_that("dbzType",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$dbzType, "DBZH")
  a$dbzType<-"VRADH"
  expect_equal(a$dbzType, "VRADH")
})

test_that("azimMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$azimMin, 0.0, tolerance = 0.0001)
  a$azimMin<-45.0
  expect_equal(a$azimMin, 45.0, tolerance = 0.0001)
})

test_that("azimMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$azimMax, 360.0, tolerance = 0.0001)
  a$azimMax<-180.0
  expect_equal(a$azimMax, 180.0, tolerance = 0.0001)
})

test_that("layerThickness",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$layerThickness, 200.0, tolerance = 0.0001)
  a$layerThickness<-150.0
  expect_equal(a$layerThickness, 150.0, tolerance = 0.0001)
})

test_that("nLayers",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$nLayers, 25)
  a$nLayers<-20
  expect_equal(a$nLayers, 20)
})

test_that("rangeMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$rangeMin, 5000.0, tolerance = 0.0001)
  a$rangeMin<-4000.0
  expect_equal(a$rangeMin, 4000.0, tolerance = 0.0001)
})

test_that("rangeMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$rangeMax, 35000.0, tolerance = 0.0001)
  a$rangeMax<-30000.0
  expect_equal(a$rangeMax, 30000.0, tolerance = 0.0001)
})

test_that("radarWavelength",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$radarWavelength, 5.3, tolerance = 0.0001)
  a$radarWavelength<-5.5
  expect_equal(a$radarWavelength, 5.5, tolerance = 0.0001)
})

test_that("useClutterMap",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$useClutterMap, FALSE)
  a$useClutterMap<-TRUE
  expect_equal(a$useClutterMap, TRUE)
})

test_that("clutterValueMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$clutterValueMin, 0.1, tolerance = 0.0001)
  a$clutterValueMin<-0.2
  expect_equal(a$clutterValueMin, 0.2, tolerance = 0.0001)
})

test_that("clutterMap",{
  a<-Vol2BirdConfig$new()
  expect_equal("", a$clutterMap)
  a$clutterMap<-"/this/location/file.xxx"
  expect_equal(a$clutterMap, "/this/location/file.xxx")
})

test_that("printDbz",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printDbz, FALSE)
  a$printDbz<-TRUE
  expect_equal(a$printDbz, TRUE)
})

test_that("printDealias",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printDealias, FALSE)
  a$printDealias<-TRUE
  expect_equal(a$printDealias, TRUE)
})

test_that("printVrad",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printVrad, FALSE)
  a$printVrad<-TRUE
  expect_equal(a$printVrad, TRUE)
})

test_that("printRhohv",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printRhohv, FALSE)
  a$printRhohv<-TRUE
  expect_equal(a$printRhohv, TRUE)
})

test_that("printTex",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printTex, FALSE)
  a$printTex<-TRUE
  expect_equal(a$printTex, TRUE)
})

test_that("printCell",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printCell, FALSE)
  a$printCell<-TRUE
  expect_equal(a$printCell, TRUE)
})

test_that("printCellProp",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printCellProp, FALSE)
  a$printCellProp<-TRUE
  expect_equal(a$printCellProp, TRUE)
})

test_that("printClut",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printClut, FALSE)
  a$printClut<-TRUE
  expect_equal(a$printClut, TRUE)
})

test_that("printOptions",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printOptions, FALSE)
  a$printOptions<-TRUE
  expect_equal(a$printOptions, TRUE)
})

test_that("printProfileVar",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printProfileVar, FALSE)
  a$printProfileVar<-TRUE
  expect_equal(a$printProfileVar, TRUE)
})

test_that("printPointsArray",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$printPointsArray, FALSE)
  a$printPointsArray<-TRUE
  expect_equal(a$printPointsArray, TRUE)
})

test_that("fitVrad",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$fitVrad, TRUE)
  a$fitVrad<-FALSE
  expect_equal(a$fitVrad, FALSE)
})

test_that("exportBirdProfileAsJSONVar",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$exportBirdProfileAsJSONVar, FALSE)
  a$exportBirdProfileAsJSONVar<-TRUE
  expect_equal(a$exportBirdProfileAsJSONVar, TRUE)
})

test_that("minNyquist",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$minNyquist, 5.0, tolerance = 0.0001)
  a$minNyquist<-4.0
  expect_equal(a$minNyquist, 4.0, tolerance = 0.0001)
})

test_that("maxNyquistDealias",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$maxNyquistDealias, 25.0, tolerance = 0.0001)
  a$maxNyquistDealias<-4.0
  expect_equal(a$maxNyquistDealias, 4.0, tolerance = 0.0001)
})

test_that("birdRadarCrossSection",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$birdRadarCrossSection, 11.0, tolerance = 0.0001)
  a$birdRadarCrossSection<-4.0
  expect_equal(a$birdRadarCrossSection, 4.0, tolerance = 0.0001)
})

test_that("cellStdDevMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$cellStdDevMax, 5.0, tolerance = 0.0001)
  a$cellStdDevMax<-4.0
  expect_equal(a$cellStdDevMax, 4.0, tolerance = 0.0001)
})

test_that("stdDevMinBird",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$stdDevMinBird, 2.0, tolerance = 0.0001)
  a$stdDevMinBird<-4.0
  expect_equal(a$stdDevMinBird, 4.0, tolerance = 0.0001)
})

test_that("etaMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$etaMax, 36000.0, tolerance = 0.0001)
  a$etaMax<-4.0
  expect_equal(a$etaMax, 4.0, tolerance = 0.0001)
})

test_that("cellEtaMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$cellEtaMin, 11500.0, tolerance = 0.0001)
  a$cellEtaMin<-4000.0
  expect_equal(a$cellEtaMin, 4000.0, tolerance = 0.0001)
})

test_that("requireVrad",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$requireVrad, FALSE)
  a$requireVrad<-TRUE
  expect_equal(a$requireVrad, TRUE)
})

test_that("dealiasVrad",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$dealiasVrad, TRUE)
  a$dealiasVrad<-FALSE
  expect_equal(a$dealiasVrad, FALSE)
})

test_that("dealiasRecycle",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$dealiasRecycle, TRUE)
  a$dealiasRecycle<-FALSE
  expect_equal(a$dealiasRecycle, FALSE)
})

test_that("dualPol",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$dualPol, TRUE)
  a$dualPol<-FALSE
  expect_equal(a$dualPol, FALSE)
})

test_that("singlePol",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$singlePol, TRUE)
  a$singlePol<-FALSE
  expect_equal(a$singlePol, FALSE)
})

test_that("dbzThresMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$dbzThresMin, 0.0, tolerance = 0.0001)
  a$dbzThresMin<-4.0
  expect_equal(a$dbzThresMin, 4.0, tolerance = 0.0001)
})

test_that("rhohvThresMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$rhohvThresMin, 0.95, tolerance = 0.0001)
  a$rhohvThresMin<-4.0
  expect_equal(a$rhohvThresMin, 4.0, tolerance = 0.0001)
})

test_that("resample",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$resample, FALSE)
  a$resample<-TRUE
  expect_equal(a$resample, TRUE)
})

test_that("resampleRscale",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$resampleRscale, 500.0, tolerance = 0.0001)
  a$resampleRscale<-400.0
  expect_equal(a$resampleRscale, 400.0, tolerance = 0.0001)
})

test_that("resampleNbins",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$resampleNbins, 100)
  a$resampleNbins<-101
  expect_equal(a$resampleNbins, 101)
})

test_that("resampleNrays",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$resampleNrays, 360)
  a$resampleNrays<-420
  expect_equal(a$resampleNrays, 420)
})

test_that("mistNetElevs",{
  a<-Vol2BirdConfig$new()
  expect_equal(c(0.5,1.5,2.5,3.5,4.5), a$mistNetElevs, tolerance = 0.0001)
  a$mistNetElevs<-c(0.7,1.5,2.5,3.5,4.5)
  expect_equal(c(0.7,1.5,2.5,3.5,4.5), a$mistNetElevs, tolerance = 0.0001)
})

test_that("mistNetElevsOnly",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$mistNetElevsOnly, TRUE)
  a$mistNetElevsOnly<-FALSE
  expect_equal(a$mistNetElevsOnly, FALSE)
})

test_that("useMistNet",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$useMistNet, FALSE)
  a$useMistNet<-TRUE
  expect_equal(a$useMistNet, TRUE)
})

test_that("mistNetPath",{
  a<-Vol2BirdConfig$new()
  expect_equal("/opt/vol2bird/etc/mistnet_nexrad.pt", a$mistNetPath)
  a$mistNetPath<-"/this/location/file.pt"
  expect_equal(a$mistNetPath, "/this/location/file.pt")
})

test_that("constant_areaCellMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_areaCellMin, 0.5, tolerance = 0.0001)
  a$constant_areaCellMin<-0.6
  expect_equal(a$constant_areaCellMin, 0.6, tolerance = 0.0001)
})

test_that("constant_cellClutterFractionMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_cellClutterFractionMax, 0.5, tolerance = 0.0001)
  a$constant_cellClutterFractionMax<-0.6
  expect_equal(a$constant_cellClutterFractionMax, 0.6, tolerance = 0.0001)
})

test_that("constant_chisqMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_chisqMin, 1e-5, tolerance = 0.0000001)
  a$constant_chisqMin<-2e-5
  expect_equal(a$constant_chisqMin, 2e-5, tolerance = 0.0000001)
})

test_that("constant_fringeDist",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_fringeDist, 5000.0, tolerance = 0.0001)
  a$constant_fringeDist<-6000.0
  expect_equal(a$constant_fringeDist, 6000.0, tolerance = 0.0001)
})

test_that("constant_nBinsGap",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nBinsGap, 8)
  a$constant_nBinsGap<-9
  expect_equal(a$constant_nBinsGap, 9)
})

test_that("constant_nPointsIncludedMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nPointsIncludedMin, 25)
  a$constant_nPointsIncludedMin<-26
  expect_equal(a$constant_nPointsIncludedMin, 26)
})

test_that("constant_nNeighborsMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nNeighborsMin, 5)
  a$constant_nNeighborsMin<-6
  expect_equal(a$constant_nNeighborsMin, 6)
})

test_that("constant_nObsGapMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nObsGapMin, 5)
  a$constant_nObsGapMin<-6
  expect_equal(a$constant_nObsGapMin, 6)
})

test_that("constant_nAzimNeighborhood",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nAzimNeighborhood, 3)
  a$constant_nAzimNeighborhood<-4
  expect_equal(a$constant_nAzimNeighborhood, 4)
})

test_that("constant_nRangNeighborhood",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nRangNeighborhood, 3)
  a$constant_nRangNeighborhood<-4
  expect_equal(a$constant_nRangNeighborhood, 4)
})

test_that("constant_nCountMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_nCountMin, 4)
  a$constant_nCountMin<-5
  expect_equal(a$constant_nCountMin, 5)
})

test_that("constant_refracIndex",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_refracIndex, 0.964, tolerance=0.0001)
  a$constant_refracIndex<-1.964
  expect_equal(a$constant_refracIndex, 1.964, tolerance=0.0001)
})

test_that("constant_absVDifMax",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_absVDifMax, 10.0, tolerance=0.0001)
  a$constant_absVDifMax<-11.0
  expect_equal(a$constant_absVDifMax, 11.0, tolerance=0.0001)
})

test_that("constant_vradMin",{
  a<-Vol2BirdConfig$new()
  expect_equal(a$constant_vradMin, 1.0, tolerance=0.0001)
  a$constant_vradMin<-1.1
  expect_equal(a$constant_vradMin, 1.1, tolerance=0.0001)
})
