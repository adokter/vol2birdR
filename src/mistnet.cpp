#include <Rcpp.h>
#include <memory>
#include <librave.h>
#include <libmistnet/mistnet.h>

//  #include <thread>

using namespace Rcpp;

void mistnet_host_handler()
{
//  if (mistnetLastError() != NULL) {
//    std::string last = mistnetLastError();
//    mistnetLastErrorClear();
//
//    std::string error_msg = translate_error_message(std::string(last.c_str()));
//
//    throw Rcpp::exception(error_msg.c_str());
//  }
}

bool mistnet_loaded = false;
void check_mistnet_loaded ()
{
  if (!mistnet_loaded)
  {
    throw std::runtime_error("Mistnet is not loaded. Please use `install_mistnet()` to install additional dependencies.");
  }
}

extern "C" {
int check_mistnet_loaded_c(void)
{
  if (mistnet_loaded) {
    return 1;
  }
  return 0;
}
}

//std::thread::id main_thread_id() noexcept {
//  static const auto tid = std::this_thread::get_id();
//  return tid;
//}

//' Sets the main thread id
//'
//' @keywords internal
// [[Rcpp::export]]
void cpp_vol2bird_namespace__store_main_thread_id() {
  // called upon package load to remember the thread ID of the main thread

  // AHE: Temporary removed. Requires C++ 11.
  // main_thread_id();
}

void Vol2Bird_Rprintf(const char* msg) {
  Rprintf("%s", msg);
}

void RSL_Rprintf(const char* msg) {
  Rprintf("%s", msg);
}

void IRIS2ODIM_Rprintf(const char* msg) {
  Rprintf("%s", msg);
}

extern "C" {
void RSL_set_printfun(void(*printfun)(const char*));
void Iris_set_printf(void(*printfun)(const char*));
}

//' Initializes the vol2birdR library
//'
//' @keywords internal
// [[Rcpp::export]]
void cpp_vol2bird_initialize() {
  // called upon package load to remember the thread ID of the main thread
  HL_init();
  vol2bird_set_printf(Vol2Bird_Rprintf);
  vol2bird_set_err_printf(Vol2Bird_Rprintf);
  Iris_set_printf(IRIS2ODIM_Rprintf);
  RSL_set_printfun(RSL_Rprintf);
}

extern "C" {
void wsr88d_set_site_info_file(const char* pth);
const char* wsr88d_get_site_info_file(void);
}

//' Sets the wsr88d site location file
//'
//' @param loc location of file
//' @keywords internal
// [[Rcpp::export]]
void cpp_vol2bird_set_wsr88d_site_location(std::string loc)
{
  wsr88d_set_site_info_file(loc.c_str());
}

//' Returns the wsr88d site location file
//'
//' @return location of site location file
//' @keywords internal
// [[Rcpp::export]]
std::string cpp_vol2bird_get_wsr88d_site_location()
{
  return std::string(wsr88d_get_site_info_file());
}


void call_r_gc() {
  Rcpp::Function r_gc("gc");
  r_gc(Rcpp::Named("full") = false);
  R_RunPendingFinalizers();
}

//' Initializes the mistnet shared library pointed to by the path
//'
//' @keywords internal
//' @param path The shared library
// [[Rcpp::export]]
void cpp_mistnet_init(std::string path) {
  std::string error;
  if (!mistnetInit(path, &error))
    Rcpp::stop(error);
}

extern "C" {
void RaveCoreObject_printStatistics(void);
void rave_alloc_print_statistics(void);
}

//' The software has to be compiled with -DRAVE_MEMORY_DEBUG and without -DNO_RAVE_PRINTF.
//' Manual handling for now.
// [[Rcpp::export]]
void cpp_printMemory() {
  RaveCoreObject_printStatistics();
  rave_alloc_print_statistics();
}
