cmake_minimum_required(VERSION 3.20)
project(ExplicitClient LANGUAGES CXX)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Для явного зв'язування потрібна ЛИШЕ DLL партнера. Заголовок і .lib НЕ використовуються.
set(PARTNER_DLL "${CMAKE_CURRENT_SOURCE_DIR}/RegressionLib.dll")

if(NOT EXISTS "${PARTNER_DLL}")
    message(FATAL_ERROR
        "Не знайдено RegressionLib.dll. Скопіюйте DLL партнера (з RegressionSDK/bin) у корінь цього проєкту.")
endif()

add_executable(ExplicitClient main.cpp)
target_compile_features(ExplicitClient PRIVATE cxx_std_20)

# Копіюємо DLL поруч з .exe після збірки, щоб LoadLibrary знайшла її за коротким ім'ям
add_custom_command(TARGET ExplicitClient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PARTNER_DLL}"
        "$<TARGET_FILE_DIR:ExplicitClient>"
)
