#define FONTSTASH_IMPLEMENTATION
#define FONS_USE_FREETYPE

#if defined(_MSC_VER)
#pragma warning(disable : 4996)
#pragma warning(disable : 4018)
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif
#include "fontstash.h"
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include <sokol_gfx.h>
#include <sokol_log.h>

#if defined(_MSC_VER)
#pragma warning(disable : 4244)
#endif
#include "sokol_gp.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"