cmake_minimum_required(VERSION 3.16)

project(qr_scanner)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/W4>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/permissive->)
add_compile_options($<$<CXX_COMPILER_ID:GNU>:-Wall>)
add_compile_options($<$<CXX_COMPILER_ID:GNU>:-pedantic>)
add_compile_options($<$<CXX_COMPILER_ID:GNU>:-Wextra>)

if(MSVC)
    option(MULTIPROCESS_BUILD "Enables the /MP compile flag for Visual C++." ON)
    if(MULTIPROCESS_BUILD)
        set(MULTIPROCESS_LIMIT_BUILD_PROCESSES 0 CACHE STRING
            "If set to a positive number, this will be used as an argument to /MP.")
        if(NOT MULTIPROCESS_LIMIT_BUILD_PROCESSES)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${MULTIPROCESS_LIMIT_BUILD_PROCESSES}")
        endif()
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Qt5 REQUIRED Widgets Multimedia CONFIG HINTS $ENV{QT5_ROOT}/msvc2017_64 $ENV{QT5_ROOT}/msvc2019_64)
find_package(gbBase REQUIRED)
find_package(fmt REQUIRED)

find_library(QZXING_LIBRARY_DEBUG QZXing2 HINTS ${PROJECT_SOURCE_DIR}/qzxing_prebuilt/lib/Debug)
find_library(QZXING_LIBRARY_RELEASE QZXing2 HINTS ${PROJECT_SOURCE_DIR}/qzxing_prebuilt/lib/Release)
set(QZXING_LIBRARIES debug ${QZXING_LIBRARY_DEBUG} optimized ${QZXING_LIBRARY_RELEASE})
find_path(QZXING_INCLUDE_DIR NAMES QZXing.h HINTS ${PROJECT_SOURCE_DIR}/qzxing_prebuilt/include)
add_library(qzxing INTERFACE)
target_include_directories(qzxing INTERFACE ${QZXING_INCLUDE_DIR})
target_link_libraries(qzxing INTERFACE ${QZXING_LIBRARIES})

find_library(ESCAPI_LIBRARY_DEBUG escapi HINTS ${PROJECT_SOURCE_DIR}/escapi/bin/x64/Debug)
find_library(ESCAPI_LIBRARY_RELEASE escapi HINTS ${PROJECT_SOURCE_DIR}/escapi/bin/x64/Release)
set(ESCAPI_LIBRARIES debug ${ESCAPI_LIBRARY_DEBUG} optimized ${ESCAPI_LIBRARY_RELEASE})
find_path(ESCAPI_INCLUDE_DIR NAMES escapi.h HINTS ${PROJECT_SOURCE_DIR}/escapi/common)
add_library(escapi INTERFACE)
target_include_directories(escapi INTERFACE ${ESCAPI_INCLUDE_DIR})
target_link_libraries(escapi INTERFACE ${ESCAPI_LIBRARIES})
target_sources(escapi INTERFACE ${ESCAPI_INCLUDE_DIR}/escapi.cpp)
source_group(escapi FILES ${ESCAPI_INCLUDE_DIR}/escapi.cpp)


set(QR_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(QR_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)

set(QR_SOURCE_FILES
    ${QR_SOURCE_DIR}/main.cpp
    ${QR_SOURCE_DIR}/detect_qr.cpp
)

set(QR_HEADER_FILES
    ${QR_SOURCE_DIR}/detect_qr.hpp
)

set(QR_UI_DIR ${PROJECT_SOURCE_DIR}/src/ui)
set(QR_QT_SOURCE_FILES
    ${QR_UI_DIR}/camera_select.cpp
    ${QR_UI_DIR}/capture_feed.cpp
)
set(QR_QT_HEADER_FILES
)
set(QR_QT_MOC_HEADER_FILES
    ${QR_UI_DIR}/camera_select.hpp
    ${QR_UI_DIR}/capture_feed.hpp
)
qt5_wrap_cpp(QR_QT_MOC_SOURCE_FILES ${QR_QT_MOC_HEADER_FILES})
source_group(moc FILES
    ${QR_QT_MOC_SOURCE_FILES}
)

add_executable(qr_scanner WIN32
    ${QR_SOURCE_FILES}
    ${QR_HEADER_FILES}
    ${QR_QT_SOURCE_FILES}
    ${QR_QT_HEADER_FILES}
    ${QR_QT_MOC_HEADER_FILES}
    ${QR_QT_MOC_SOURCE_FILES}
)
target_include_directories(qr_scanner PUBLIC ${QR_INCLUDE_DIR})
target_link_libraries(qr_scanner PUBLIC escapi qzxing Qt5::Widgets Ghulbus::gbBase fmt::fmt)
if(WIN32)
    #target_link_libraries(qr_scanner PUBLIC Qt5::WinMain)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT qr_scanner)


if(WIN32)
    function(getPDBForDLL DLL_PATH OUT_VAR)
        get_filename_component(dll_dir ${DLL_PATH} DIRECTORY)
        get_filename_component(dll_we ${DLL_PATH} NAME_WE)
        set(${OUT_VAR} "${dll_dir}/${dll_we}.pdb" PARENT_SCOPE)
    endfunction()


    function(getQt5Dlls QT_TARGET OUT_VAR)
        unset(DLLS)
        get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_DEBUG)
        list(APPEND DLLS ${tmp})
        getPDBForDLL(${tmp} tmp_pdb)
        if (EXISTS ${tmp_pdb})
            list(APPEND DLLS ${tmp_pdb})
        endif()
        get_property(tmp TARGET ${QT_TARGET} PROPERTY IMPORTED_LOCATION_RELEASE)
        list(APPEND DLLS ${tmp})
        get_property(deps TARGET ${QT_TARGET} PROPERTY IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG)
        foreach(dep ${deps})
            if(TARGET ${dep})
                getQt5Dlls(${dep} tmp)
                list(APPEND DLLS ${tmp})
            endif()
        endforeach()
        set(result ${${OUT_VAR}})
        list(APPEND result ${DLLS})
        list(REMOVE_DUPLICATES result)
        set(${OUT_VAR} ${result} PARENT_SCOPE)
    endfunction()
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    getQt5Dlls(Qt5::Multimedia qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()

install(TARGETS qr_scanner DESTINATION .)
install(FILES $<TARGET_PDB_FILE:qr_scanner> DESTINATION . OPTIONAL)
if(WIN32)
    #install(FILES ${dll} ${TODO_DLLS} DESTINATION .)
    find_program(QT5_WINDEPLOYQT windeployqt HINTS ${Qt5_DIR}/../../../ PATH_SUFFIXES bin)
    mark_as_advanced(QT5_WINDEPLOYQT)
    install(SCRIPT CODE "execute_process(COMMAND ${QT5_WINDEPLOYQT} ${CMAKE_INSTALL_PREFIX})")
endif()
