set(LASZIP_API_SOURCES
    laszip_api.h
    laszip_api.c
)

LASZIP_ADD_LIBRARY(${LASZIP_API_LIB_NAME} ${LASZIP_API_SOURCES})
LASZIP_ADD_INCLUDES("laszip" "${CMAKE_CURRENT_SOURCE_DIR}/laszip_api.h" "${CMAKE_SOURCE_DIR}/include/laszip/laszip_common.h" ${LASZIP_API_VERSION_H})

target_include_directories(${LASZIP_BASE_LIB_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/vendor/LASzip-3.5.0/include/laszip)
set_target_properties(${LASZIP_BASE_LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
