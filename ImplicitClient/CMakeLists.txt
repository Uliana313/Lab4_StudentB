cmake_minimum_required(VERSION 3.20)
project(ImplicitClient LANGUAGES CXX)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# SDK-пакунок партнера (студент А): include/, lib/, bin/, README.md
set(SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/RegressionLib")

if(NOT EXISTS "${SDK_DIR}/lib/RegressionLib.lib" OR NOT EXISTS "${SDK_DIR}/bin/RegressionLib.dll")
    message(FATAL_ERROR
        "Не знайдено SDK партнера. Скопіюйте вміст RegressionSDK (include, lib, bin, README.md) "
        "у папку 3rdParty/RegressionLib")
endif()

# 1. Виконуваний файл
add_executable(ImplicitClient main.cpp)
target_compile_features(ImplicitClient PRIVATE cxx_std_20)

# 2. Шлях до заголовкового файлу бібліотеки
target_include_directories(ImplicitClient PRIVATE "${SDK_DIR}/include")

# 3. Імпортна бібліотека (.lib) для етапу компонування (link-time)
target_link_libraries(ImplicitClient PRIVATE "${SDK_DIR}/lib/RegressionLib.lib")

# 4. Копіюємо .dll поруч з .exe після збірки (інакше при запуску Windows
#    не знайде DLL і програма не стартує)
add_custom_command(TARGET ImplicitClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDK_DIR}/bin/RegressionLib.dll"
        "$<TARGET_FILE_DIR:ImplicitClient>"
)
