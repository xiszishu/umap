project(FITS)

add_library(qfits SHARED IMPORTED)
set_target_properties(qfits PROPERTIES
  IMPORTED_LOCATION "$ENV{HOME}/qfits/lib/libqfits.so.0.0.0"
  INTERFACE_INCLUDE_DIRECTORIES "$ENV{HOME}/qfits/include"
)

FIND_PACKAGE( OpenMP REQUIRED )
if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
    add_executable(FITS pixel_loader.cpp)

    target_link_libraries(FITS libumap_static)
    target_link_libraries(FITS libumaptest_static)
    target_link_libraries(FITS qfits)

    include_directories( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_BINARY_DIR}/include )

    install(TARGETS FITS
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib/static
            RUNTIME DESTINATION bin )
else()
    message("Skpping median, OpenMP required")
endif()