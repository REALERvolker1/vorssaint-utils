cmake_minimum_required(VERSION 3.24)

project(vorssaint_utils VERSION 4.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Core)

add_executable(vorssaintctl
    src/main.cpp
    src/core/XdgPaths.cpp
    src/core/SettingsStore.cpp
    src/search/FuzzyMatcher.cpp
    src/security/Validation.cpp
    src/system/SystemdManager.cpp
    src/system/SystemMetrics.cpp
    src/system/HyprlandManager.cpp
)

target_include_directories(vorssaintctl PRIVATE src)
target_link_libraries(vorssaintctl PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(TARGETS vorssaintctl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY quickshell/vorssaint
        DESTINATION ${CMAKE_INSTALL_DATADIR}/vorssaint/quickshell)

include(CTest)
if(BUILD_TESTING)
    add_executable(vorssaint-tests
        tests/tests.cpp
        src/search/FuzzyMatcher.cpp
        src/security/Validation.cpp
    )
    target_include_directories(vorssaint-tests PRIVATE src)
    target_link_libraries(vorssaint-tests PRIVATE Qt6::Core)
    add_test(NAME vorssaint-tests COMMAND vorssaint-tests)
endif()
