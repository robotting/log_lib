#pragma once

// Static linking: no import/export decoration.
#if defined(LOG_LIB_STATIC)
#  define LOG_LIB_API

// Windows (MSVC / MinGW / Cygwin)
#elif defined(_WIN32) || defined(__CYGWIN__)
// MinGW / GCC on Windows
#  if defined(__MINGW32__) || (defined(__GNUC__) && !defined(_MSC_VER))
#    if defined(LOG_LIB_BUILD_SHARED)
#      define LOG_LIB_API __attribute__((dllexport))
#    elif defined(LOG_LIB_SHARED)
#      define LOG_LIB_API __attribute__((dllimport))
#    else
#      define LOG_LIB_API
#    endif
// MSVC, clang-cl
#  else
#    if defined(LOG_LIB_BUILD_SHARED)
#      define LOG_LIB_API __declspec(dllexport)
#    elif defined(LOG_LIB_SHARED)
#      define LOG_LIB_API __declspec(dllimport)
#    else
#      define LOG_LIB_API
#    endif
#  endif

// macOS, Linux, and other Unix-like targets (Mach-O / ELF).
// Only the library build needs an export attribute; consumers do not use
// dllimport — LOG_LIB_SHARED leaves LOG_LIB_API empty here by design.
#elif defined(__APPLE__) || defined(__linux__) || \
      (defined(__unix__) && !defined(__CYGWIN__))
#  if defined(LOG_LIB_BUILD_SHARED)
#    define LOG_LIB_API __attribute__((visibility("default")))
#  else
#    define LOG_LIB_API
#  endif

#else
#  define LOG_LIB_API
#endif
