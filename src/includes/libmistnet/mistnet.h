/*
 * Original idea and part of the code originates from https://github.com/mlverse/torch after some mail exchange
 * with Daniel Falbel (Copyright).
 * Original licensed with MIT. https://github.com/mlverse/torch/blob/main/LICENSE.md
*/
#ifndef MISTNET_H
#define MISTNET_H

#ifndef _WIN32
#include <dlfcn.h>
#else
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif

#ifndef HOST_API
#define HOST_API inline
#endif

#ifdef MISTNET_BUILD

#define MISTNET_PTR
#define MISTNET_HEADERS_ONLY
#ifdef _WIN32
#define MISTNET_API extern "C" __declspec(dllexport)
#endif

#else // MISTNET_BUILD

#define MISTNET_PTR *

#endif

#ifndef MISTNET_API
#ifdef MISTNET_HEADERS_ONLY
#define MISTNET_API extern
#else
#define MISTNET_API
#endif
#endif

#ifndef MISTNET_HOST_HANDLER
void mistnet_host_handler();
#define MISTNET_HOST_HANDLER mistnet_host_handler();
#endif

#ifndef MISTNET_CHECK_LOADED
extern bool mistnet_loaded;
void check_mistnet_loaded();
#define MISTNET_CHECK_LOADED check_mistnet_loaded();
#endif

#ifdef __cplusplus
extern "C"
{
#endif
MISTNET_API int (MISTNET_PTR _mistnet_run_mistnet)(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size);
HOST_API int mistnet_run_mistnet(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size) 
{
  MISTNET_CHECK_LOADED
  return _mistnet_run_mistnet(tensor_in, tensor_out, model_path, tensor_size);
  MISTNET_HOST_HANDLER;
}

int run_mistnet(float* tensor_in, float** tensor_out, const char* model_path, int tensor_size)
{
  MISTNET_CHECK_LOADED
  return _mistnet_run_mistnet(tensor_in, tensor_out, model_path, tensor_size);
  MISTNET_HOST_HANDLER;
}

int check_mistnet_loaded_c(void);

#ifdef __cplusplus
}
#endif

#ifndef MISTNET_HEADERS_ONLY

#include <string>

inline const char *pathSeparator()
{
#ifdef _WIN32
  return "\\";
#else
  return "/";
#endif
}

inline const char *libraryName()
{
#ifdef __APPLE__
  return "libmistnet.dylib";
#else
#ifdef _WIN32
  return "mistnet.dll";
#else
  return "libmistnet.so";
#endif
#endif
}
void *pLibrary = NULL;

#define LOAD_SYMBOL(name)                                         \
  if (!mistnetLoadSymbol(pLibrary, #name, (void **)&name, pError))  \
    return false;

void mistnetLoadError(std::string *pError)
{
#ifdef _WIN32
  LPVOID lpMsgBuf;
  DWORD dw = ::GetLastError();

  DWORD length = ::FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      dw,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPTSTR)&lpMsgBuf,
      0, NULL);

  if (length != 0)
  {
    std::string msg((LPTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
    pError->assign(msg);
  }
  else
  {
    pError->assign("Unknown error");
  }
#else
  const char *msg = ::dlerror();
  if (msg != NULL)
    pError->assign(msg);
  else
    pError->assign("Unknown error");
#endif
}

bool mistnetLoadLibrary(const std::string &libPath, std::string *pError)
{
  pLibrary = NULL;

  char lastLibChar = libPath.at(libPath.size() - 1);
  std::string separator = (lastLibChar == '/' || lastLibChar == '\\') ? "" : pathSeparator();
  std::string libFile = libPath + separator + libraryName();

#ifdef _WIN32

  typedef DLL_DIRECTORY_COOKIE(WINAPI * PAddDllDirectory)(PCWSTR);
  HMODULE hKernel = ::GetModuleHandle("kernel32.dll");

  if (hKernel == NULL)
  {
    mistnetLoadError(pError);
    *pError = "Get Kernel - " + *pError;
    return false;
  }

  PAddDllDirectory add_dll_directory = (PAddDllDirectory)::GetProcAddress(hKernel, "AddDllDirectory");

  if (add_dll_directory != NULL)
  {
    std::wstring libPathWStr = std::wstring(libPath.begin(), libPath.end());
    DLL_DIRECTORY_COOKIE cookie = add_dll_directory(libPathWStr.c_str());

    if (cookie == NULL)
    {
      mistnetLoadError(pError);
      *pError = "Add Dll Directory - " + *pError;
      return false;
    }
  }

  pLibrary = (void *)::LoadLibraryEx(libFile.c_str(), NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#else
  pLibrary = ::dlopen(libFile.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
  if (pLibrary == NULL)
  {
    mistnetLoadError(pError);
    *pError = libFile + " - " + *pError;
    return false;
  }
  else
  {
    return true;
  }
}

bool mistnetLoadSymbol(void *pLib, const std::string &name, void **ppSymbol, std::string *pError)
{
  *ppSymbol = NULL;
#ifdef _WIN32
  *ppSymbol = (void *)::GetProcAddress((HINSTANCE)pLib, name.c_str());
#else
  *ppSymbol = ::dlsym(pLib, name.c_str());
#endif
  if (*ppSymbol == NULL)
  {
    mistnetLoadError(pError);
    *pError = name + " - " + *pError;
    return false;
  }
  else
  {
    return true;
  }
}

bool mistnetCloseLibrary(void *pLib, std::string *pError)
{
#ifdef _WIN32
  if (!::FreeLibrary((HMODULE)pLib))
#else
  if (::dlclose(pLib) != 0)
#endif
  {
    mistnetLoadError(pError);
    return false;
  }
  else
  {
    return true;
  }
}

bool mistnetInit(const std::string &libPath, std::string *pError)
{
  if (!mistnetLoadLibrary(libPath, pError))
    return false;
  mistnet_loaded = true;
  LOAD_SYMBOL(_mistnet_run_mistnet);

  return true;
}

#else

extern bool mistnetInit(const std::string &libPath, std::string *pError);
extern bool mistnetCloseLibrary(void *pLib, std::string *pError);

#endif

#endif
