#include <Rcpp.h>
#include <memory>
#include <string.h>

extern "C" {
#include "libvol2bird/libvol2bird.h"
#include "libvol2bird/constants.h"
}

using namespace Rcpp;
#include "vol2birdR_types.h"

//' @export PolarVolume
class PolarVolume {
private:
  PolarVolume_t* _polarvolume;
public:
  PolarVolume() {
    _polarvolume = (PolarVolume_t*)RAVE_OBJECT_NEW(&PolarVolume_TYPE);
    if (_polarvolume == NULL) {
      throw Rcpp::exception(std::string("Could not create internal polar volume instance").c_str());
    }
  }
  PolarVolume(PolarVolume_t* polarvolume) {
    _polarvolume = (PolarVolume_t*)RAVE_OBJECT_COPY(polarvolume);
  }
  PolarVolume(const PolarVolume& c) {
    _polarvolume = (PolarVolume_t*)RAVE_OBJECT_COPY(c._polarvolume);
  }
  virtual ~PolarVolume() {
    RAVE_OBJECT_RELEASE(_polarvolume);
  }
  PolarVolume_t* get() {
    return _polarvolume;
  }
  int getNumberOfScans() {
    return PolarVolume_getNumberOfScans(_polarvolume);
  }
};
//RCPP_EXPOSED_CLASS(PolarVolume)

//' @export RaveIO
class RaveIO {
private:
  RaveIO_t* _raveio;
  PolarVolume* _polarvolume;	
public:
  RaveIO() {
    _raveio=(RaveIO_t*)RAVE_OBJECT_NEW(&RaveIO_TYPE);
    _polarvolume=NULL;
  }
  RaveIO(std::string filename, bool lazyLoading=FALSE) {
    _polarvolume=NULL;  
    _raveio=RaveIO_open(filename.c_str(), lazyLoading?1:0,NULL);
    if (_raveio == NULL) {
      throw Rcpp::exception(std::string("Could not open file").c_str());
    }
  }
  virtual ~RaveIO() {
    RAVE_OBJECT_RELEASE(_raveio);
  }
  
  PolarVolume object() {
    if (_polarvolume == NULL) {
      RaveCoreObject* object = (RaveCoreObject*)RaveIO_getObject(_raveio);
      if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
        _polarvolume = new PolarVolume((PolarVolume_t*)object);
      } else {
        throw Rcpp::exception(std::string("Could not return object").c_str());
      }
    }
    return *_polarvolume;
  }
      
  
  /*
  Rcpp::XPtr<PolarVolume> object() {
    std::cout << "In object" << std::endl;
    RaveCoreObject* object = (RaveCoreObject*)RaveIO_getObject(_raveio);
    if (RAVE_OBJECT_CHECK_TYPE(object, &PolarVolume_TYPE)) {
      if (_polarvolume != NULL) {
        delete _polarvolume;
      }
      _polarvolume = new PolarVolume((PolarVolume_t*)object);
      Rcpp::XPtr<PolarVolume> ptr(new PolarVolume((PolarVolume_t*)object), true);
      return ptr;
      //return std::shared_ptr<PolarVolume>(new PolarVolume((PolarVolume_t*)object));
    }
    throw Rcpp::exception(std::string("Could not return object").c_str());
  }
  */
  
  CharacterVector filename() {
    return CharacterVector::create(Rcpp::String(RaveIO_getFilename(_raveio)));
  }
};

//' @export Vol2BirdConfig
class Vol2BirdConfig {
private:
  vol2bird_t _alldata;
  void initialize_config(vol2bird_t* alldata) {
    //strcpy(alldata->misc.filename_pvol, filein.c_str());
    alldata->options.elevMin = 0.0;
    alldata->options.elevMax = 45.0;
    strcpy(alldata->options.dbzType, "DBZH");
    alldata->options.azimMax = 360.0;
    alldata->options.azimMin = 0.0;
    alldata->options.layerThickness = 200.0;
    alldata->options.nLayers = 25;
    alldata->options.rangeMax = 35000.0;
    alldata->options.rangeMin = 5000.0;
    alldata->options.elevMax = 90.0;
    alldata->options.elevMin = 0.0;
    alldata->options.radarWavelength = 5.3;
    alldata->options.useClutterMap = FALSE;
    alldata->options.clutterValueMin = 0.1;
    strcpy(alldata->options.clutterMap,"");
    alldata->options.printDbz = FALSE;
    alldata->options.printDealias = FALSE;
    alldata->options.printVrad = FALSE;
    alldata->options.printRhohv = FALSE;
    alldata->options.printTex = FALSE;
    alldata->options.printCell = FALSE;
    alldata->options.printCellProp = FALSE;
    alldata->options.printClut = FALSE;
    alldata->options.printOptions = FALSE;
    alldata->options.printProfileVar = FALSE;
    alldata->options.printPointsArray = FALSE;
    alldata->options.fitVrad = TRUE;
    alldata->options.exportBirdProfileAsJSONVar = FALSE;
    alldata->options.minNyquist = 5.0;
    alldata->options.maxNyquistDealias = 25.0;
    alldata->options.birdRadarCrossSection = 11.0;
    alldata->options.cellStdDevMax = 0.0; // cfg_getfloat(*cfg,"STDEV_CELL");
    alldata->options.stdDevMinBird = 2.0;
    alldata->options.etaMax = 36000.0;
    alldata->options.cellEtaMin = 11500.0;
    strcpy(alldata->options.dbzType,"DBZH");
    alldata->options.requireVrad = FALSE;
    alldata->options.dealiasVrad = FALSE;
    alldata->options.dealiasRecycle = FALSE;
    alldata->options.dualPol = TRUE;
    alldata->options.singlePol = TRUE;
    alldata->options.dbzThresMin = 0.0;
    alldata->options.rhohvThresMin = 0.95;
    alldata->options.resample = FALSE;
    alldata->options.resampleRscale = 500.0;
    alldata->options.resampleNbins = 100;
    alldata->options.resampleNrays = 360;
    alldata->options.mistNetNElevs = 5;
    alldata->options.mistNetElevs[0] = 0.5;
    alldata->options.mistNetElevs[1] = 1.5;
    alldata->options.mistNetElevs[2] = 2.5;
    alldata->options.mistNetElevs[3] = 3.5;
    alldata->options.mistNetElevs[4] = 4.5;
    alldata->options.mistNetElevsOnly = TRUE;
    alldata->options.useMistNet = TRUE;
    strcpy(alldata->options.mistNetPath,"/opt/vol2bird/etc/mistnet_nexrad.pt");


    // ------------------------------------------------------------- //
    //              vol2bird options from constants.h                //
    // ------------------------------------------------------------- //

    alldata->constants.areaCellMin = AREACELL;
    alldata->constants.cellClutterFractionMax = CLUTPERCCELL;
    alldata->constants.chisqMin = CHISQMIN;
    alldata->constants.fringeDist = FRINGEDIST;
    alldata->constants.nBinsGap = NBINSGAP;
    alldata->constants.nPointsIncludedMin = NDBZMIN;
    alldata->constants.nNeighborsMin = NEIGHBORS;
    alldata->constants.nObsGapMin = NOBSGAPMIN;
    alldata->constants.nAzimNeighborhood = NTEXBINAZIM;
    alldata->constants.nRangNeighborhood = NTEXBINRANG;
    alldata->constants.nCountMin = NTEXMIN;
    alldata->constants.refracIndex = REFRACTIVE_INDEX_OF_WATER;
    alldata->constants.absVDifMax = VDIFMAX;
    alldata->constants.vradMin = VRADMIN;

    // ------------------------------------------------------------- //
    //       some other variables, derived from user options         //
    // ------------------------------------------------------------- //

    alldata->misc.rCellMax = alldata->options.rangeMax + RCELLMAX_OFFSET;
    alldata->misc.nDims = 2;
    alldata->misc.nParsFitted = 3;

    // the following settings depend on wavelength, will be set in Vol2birdSetup
    alldata->misc.dbzFactor = NAN;
    alldata->misc.dbzMax = NAN;
    alldata->misc.cellDbzMin = NAN;

    alldata->misc.loadConfigSuccessful = TRUE;
  }  
  
public:
  Vol2BirdConfig() {
    initialize_config(&_alldata);
  }
  
  double get_elevMin() { return _alldata.options.elevMin; }
  void set_elevMin(double e) { _alldata.options.elevMin = e;}
  double get_elevMax() { return _alldata.options.elevMax; }
  void set_elevMax(double e) { _alldata.options.elevMax = e;}
  std::string get_dbzType() { return std::string(_alldata.options.dbzType); }
  void set_dbzType(std::string v) { strcpy(_alldata.options.dbzType, v.c_str());}
};

//' @export Vol2Bird
class Vol2Bird {
private:
public:
  Vol2Bird() {
  }
  virtual ~Vol2Bird() {
  }

  void setup_conf(vol2bird_t* alldata) {
    //strcpy(alldata->misc.filename_pvol, filein.c_str());
    //alldata.misc.loadConfigSuccessful = TRUE;
    //alldata.options.elevMin = 0.0;
    //alldata.options.elevMax = 45.0;
    //strcpy(alldata.options.dbzType, "DBZH");

    alldata->options.azimMax = 360.0;
    alldata->options.azimMin = 0.0;
    alldata->options.layerThickness = 200.0;
    alldata->options.nLayers = 25;
    alldata->options.rangeMax = 35000.0;
    alldata->options.rangeMin = 5000.0;
    alldata->options.elevMax = 90.0;
    alldata->options.elevMin = 0.0;
    alldata->options.radarWavelength = 5.3;
    alldata->options.useClutterMap = FALSE;
    alldata->options.clutterValueMin = 0.1;
    strcpy(alldata->options.clutterMap,"");
    alldata->options.printDbz = FALSE;
    alldata->options.printDealias = FALSE;
    alldata->options.printVrad = FALSE;
    alldata->options.printRhohv = FALSE;
    alldata->options.printTex = FALSE;
    alldata->options.printCell = FALSE;
    alldata->options.printCellProp = FALSE;
    alldata->options.printClut = FALSE;
    alldata->options.printOptions = FALSE;
    alldata->options.printProfileVar = FALSE;
    alldata->options.printPointsArray = FALSE;
    alldata->options.fitVrad = TRUE;
    alldata->options.exportBirdProfileAsJSONVar = FALSE;
    alldata->options.minNyquist = 5.0;
    alldata->options.maxNyquistDealias = 25.0;
    alldata->options.birdRadarCrossSection = 11.0;
    alldata->options.cellStdDevMax = 0.0; // cfg_getfloat(*cfg,"STDEV_CELL");
    alldata->options.stdDevMinBird = 2.0;
    alldata->options.etaMax = 36000.0;
    alldata->options.cellEtaMin = 11500.0;
    strcpy(alldata->options.dbzType,"DBZH");
    alldata->options.requireVrad = FALSE;
    alldata->options.dealiasVrad = FALSE;
    alldata->options.dealiasRecycle = FALSE;
    alldata->options.dualPol = TRUE;
    alldata->options.singlePol = TRUE;
    alldata->options.dbzThresMin = 0.0;
    alldata->options.rhohvThresMin = 0.95;
    alldata->options.resample = FALSE;
    alldata->options.resampleRscale = 500.0;
    alldata->options.resampleNbins = 100;
    alldata->options.resampleNrays = 360;
    alldata->options.mistNetNElevs = 5;
    alldata->options.mistNetElevs[0] = 0.5;
    alldata->options.mistNetElevs[1] = 1.5;
    alldata->options.mistNetElevs[2] = 2.5;
    alldata->options.mistNetElevs[3] = 3.5;
    alldata->options.mistNetElevs[4] = 4.5;
    alldata->options.mistNetElevsOnly = TRUE;
    alldata->options.useMistNet = TRUE;
    strcpy(alldata->options.mistNetPath,"/opt/vol2bird/etc/mistnet_nexrad.pt");


    // ------------------------------------------------------------- //
    //              vol2bird options from constants.h                //
    // ------------------------------------------------------------- //

    alldata->constants.areaCellMin = AREACELL;
    alldata->constants.cellClutterFractionMax = CLUTPERCCELL;
    alldata->constants.chisqMin = CHISQMIN;
    alldata->constants.fringeDist = FRINGEDIST;
    alldata->constants.nBinsGap = NBINSGAP;
    alldata->constants.nPointsIncludedMin = NDBZMIN;
    alldata->constants.nNeighborsMin = NEIGHBORS;
    alldata->constants.nObsGapMin = NOBSGAPMIN;
    alldata->constants.nAzimNeighborhood = NTEXBINAZIM;
    alldata->constants.nRangNeighborhood = NTEXBINRANG;
    alldata->constants.nCountMin = NTEXMIN;
    alldata->constants.refracIndex = REFRACTIVE_INDEX_OF_WATER;
    alldata->constants.absVDifMax = VDIFMAX;
    alldata->constants.vradMin = VRADMIN;

    // ------------------------------------------------------------- //
    //       some other variables, derived from user options         //
    // ------------------------------------------------------------- //

    alldata->misc.rCellMax = alldata->options.rangeMax + RCELLMAX_OFFSET;
    alldata->misc.nDims = 2;
    alldata->misc.nParsFitted = 3;

    // the following settings depend on wavelength, will be set in Vol2birdSetup
    alldata->misc.dbzFactor = NAN;
    alldata->misc.dbzMax = NAN;
    alldata->misc.cellDbzMin = NAN;

    alldata->misc.loadConfigSuccessful = TRUE;

  }

  int run(const std::string& filein) {
	vol2bird_t alldata;
	PolarVolume_t* volume = NULL;
	char* filenames[50];
	setup_conf(&alldata);
	filenames[0] = (char*)filein.c_str();  // We are pointing directly in memory here, might cause crashes..
	int nr_files = 1;

	volume = vol2birdGetVolume(filenames, nr_files, 1000000, 1);

	int initSuccessful = vol2birdSetUp(volume, &alldata) == 0;

    return initSuccessful;
  }
};

RCPP_EXPOSED_CLASS_NODECL(PolarVolume)
RCPP_MODULE(PolarVolume) {
  class_<PolarVolume>("PolarVolume")
  .constructor()
  .method("getNumberOfScans", &PolarVolume::getNumberOfScans)
  ;
}
//RCPP_EXPOSED_AS(PolarVolume)

RCPP_EXPOSED_CLASS_NODECL(RaveIO)
RCPP_MODULE(RaveIO) {
  class_<RaveIO>("RaveIO")
  .constructor()
  .constructor<std::string>()
  .method("object", &RaveIO::object)
  .method("filename", &RaveIO::filename)
  ;
}
//RCPP_EXPOSED_AS(RaveIO)

RCPP_EXPOSED_CLASS_NODECL(Vol2BirdConfig)
RCPP_MODULE(Vol2BirdConfig) {
  class_<Vol2BirdConfig>("Vol2BirdConfig")
  .constructor()
  .property("elevMin", &Vol2BirdConfig::get_elevMin, &Vol2BirdConfig::set_elevMin)
  .property("elevMax", &Vol2BirdConfig::get_elevMax, &Vol2BirdConfig::set_elevMax)
  .property("dbzType", &Vol2BirdConfig::get_dbzType, &Vol2BirdConfig::set_dbzType)
  ;
}
//RCPP_EXPOSED_AS(Vol2BirdConfig)
  
RCPP_EXPOSED_CLASS_NODECL(Vol2Bird)
RCPP_MODULE(Vol2Bird) {
  class_<Vol2Bird>("Vol2Bird")
  .constructor()
  .method("run", &Vol2Bird::run)
  ;
}
//RCPP_EXPOSED_AS(Vol2Bird)



