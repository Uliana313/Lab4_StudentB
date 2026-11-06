cmake_minimum_required(VERSION 3.20)
project(AnomalyLib VERSION 1.0.0 LANGUAGES CXX)

# Етап 1: -DANOMALY_SHARED=OFF  -> статична бібліотека (.lib)
# Етап 2: -DANOMALY_SHARED=ON   -> динамічна бібліотека (.dll + імпортна .lib)
option(ANOMALY_SHARED "Збирати як динамічну бібліотеку (DLL)" ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(ANOMALY_SHARED)
    add_library(AnomalyLib SHARED src/anomaly.cpp)
    # Макрос лише для збірки самої DLL: dllexport замість dllimport
    target_compile_definitions(AnomalyLib PRIVATE ANOMALY_EXPORTS)
else()
    add_library(AnomalyLib STATIC src/anomaly.cpp)
    # Для статичної бібліотеки декоратори dllexport/dllimport не потрібні
    target_compile_definitions(AnomalyLib PUBLIC ANOMALY_STATIC)
endif()

# PUBLIC: шлях до include/ автоматично отримає кожен, хто лінкує AnomalyLib
target_include_directories(AnomalyLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(AnomalyLib PUBLIC cxx_std_20)

# Локальний тестовий застосунок (етап 1 і перевірка етапу 2)
add_executable(LocalTest test/main.cpp)
target_link_libraries(LocalTest PRIVATE AnomalyLib)

# SDK-пакунок для партнера (лише для DLL):
#   cmake --install build --config Release --prefix ../AnomalySDK
# створить: README.md, include/, lib/ (імпортна .lib), bin/ (.dll)
if(ANOMALY_SHARED)
    install(TARGETS AnomalyLib
            RUNTIME DESTINATION bin      # AnomalyLib.dll
            ARCHIVE DESTINATION lib      # AnomalyLib.lib (імпортна)
            LIBRARY DESTINATION lib)
    install(FILES include/anomaly.h DESTINATION include)
    install(FILES sdk/README.md DESTINATION .)
endif()
