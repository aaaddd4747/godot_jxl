#pragma once

#define GQOI_MAJOR 2
#define GQOI_MINOR 2
#define GQOI_PATCH 1
#define GQOI_VERSION ((GQOI_MAJOR << (8 * 3)) + (GQOI_MINOR << (8 * 2)) + (GQOI_PATCH << (8 * 1)))

#define _GQOI_VERSION_STR_TEXT(text) #text
#define _GQOI_VERSION_STR(major, minor, patch) (_GQOI_VERSION_STR_TEXT(major) "." _GQOI_VERSION_STR_TEXT(minor) "." _GQOI_VERSION_STR_TEXT(patch))
#define GQOI_VERSION_STR _GQOI_VERSION_STR(GQOI_MAJOR, GQOI_MINOR, GQOI_PATCH)

#define GJXL_MAJOR 0
#define GJXL_MINOR 0
#define GJXL_PATCH 0
#define GJXL_VERSION ((GJXL_MAJOR << (8 * 3)) + (GJXL_MINOR << (8 * 2)) + (GJXL_PATCH << (8 * 1)))

#define _GJXL_VERSION_STR_TEXT(text) #text
#define _GJXL_VERSION_STR(major, minor, patch) (_GJXL_VERSION_STR_TEXT(major) "." _GJXL_VERSION_STR_TEXT(minor) "." _GJXL_VERSION_STR_TEXT(patch))
#define GJXL_VERSION_STR _GJXL_VERSION_STR(GJXL_MAJOR, GJXL_MINOR, GJXL_PATCH)
