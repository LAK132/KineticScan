#ifndef SCANNER_MAIN_HPP
#define SCANNER_MAIN_HPP

#include <lak/system/architecture.hpp>

#include "kineticscan_git.hpp"

#define APP_VERSION GIT_TAG "-" GIT_HASH
#define APP_NAME    "KineticScan " STRINGIFY(LAK_ARCH) " " APP_VERSION

#endif
