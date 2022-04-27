#include <Rcpp.h>
#include <memory>
#include <librave.h>
#include <libmistnet/mistnet.h>
#include <thread>

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

std::thread::id main_thread_id() noexcept {
  static const auto tid = std::this_thread::get_id();
  return tid;
}

// [[Rcpp::export]]
void cpp_vol2bird_namespace__store_main_thread_id() {
  // called upon package load to remember the thread ID of the main thread
  main_thread_id();
}

void Vol2Bird_Rprintf(const char* msg) {
  Rprintf("%s", msg);
}

// [[Rcpp::export]]
void cpp_vol2bird_initialize() {
  // called upon package load to remember the thread ID of the main thread
  HL_init();
  vol2bird_set_printf(Vol2Bird_Rprintf);
}

void call_r_gc() {
  Rcpp::Function r_gc("gc");
  r_gc(Rcpp::Named("full") = false);
  R_RunPendingFinalizers();
}

// [[Rcpp::export]]
void cpp_mistnet_init(std::string path) {
  std::string error;
  if (!mistnetInit(path, &error))
    Rcpp::stop(error);
}
