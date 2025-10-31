cmake_minimum_required(VERSION 3.16)
project(AppModuleCore VERSION 1.0 LANGUAGES CXX)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Настройки для Windows
if(WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# Путь к ImGui (локально в include)
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/imgui")

# Проверка наличия ImGui
if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
    message(FATAL_ERROR "ImGui не найден в ${IMGUI_DIR}!")
else()
    message(STATUS "Используется локальный ImGui из ${IMGUI_DIR}")
endif()

# ImGui источники (компилируются в GUI.cpp, но добавляем для IDE)
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

set(IMGUI_HEADERS
    "${IMGUI_DIR}/imgui.h"
    "${IMGUI_DIR}/imgui_internal.h"
    "${IMGUI_DIR}/imconfig.h"
    "${IMGUI_DIR}/backends/imgui_impl_win32.h"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.h"
)

# Основные исходные файлы проекта
set(SOURCES
    src/main.cpp
    src/Hook.cpp
    src/Stealth.cpp
    src/UI/GUI.cpp
    src/UI/Config.cpp
    src/UI/Theme.cpp
    src/UI/Tabs.cpp
)

# Заголовочные файлы проекта
set(HEADERS
    src/Hook.h
    src/Stealth.h
    src/UI/GUI.h
    src/UI/Config.h
    src/UI/Theme.h
    src/UI/Tabs.h
)

# Создаем DLL библиотеку
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Включаем директории
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/include/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MinHook
    ${CMAKE_CURRENT_SOURCE_DIR}/include/xostr
    ${CMAKE_CURRENT_SOURCE_DIR}/include/LazyImporter
)

# Подключаем библиотеки
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/libMinHook-x86.lib
    opengl32.lib
)

# Библиотеки Windows
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32 gdi32 kernel32 advapi32 shell32
        winmm shlwapi ntdll
    )
    
    # Настройки DLL
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".dll"
        PREFIX ""
    )
endif()

# Параметры компилятора с обфускацией
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /O2           # Максимальная оптимизация скорости
        /Ob2          # Агрессивная встройка функций
        /Oi           # Генерация intrinsic функций
        /Ot           # Оптимизация для скорости
        /Oy           # Пропуск frame pointer (затрудняет отладку)
        /GL           # Link-time code generation
        /MP           # Многопоточная компиляция
        /GS-          # Отключить проверку переполнения буфера
        /GR-          # Отключить RTTI
        /EHsc         # Exception handling
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
        NDEBUG        # Release mode
    )
    
    # Линковочные флаги для обфускации и скрытия PDB информации
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO"
    )
    
    # Для Release сборки отключаем генерацию PDB полностью
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_CONFIGURATION_TYPES)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            COMPILE_PDB_NAME ""
            COMPILE_PDB_OUTPUT_DIRECTORY ""
        )
        # Используем /Z7 для встраивания отладочной информации в объектные файлы вместо PDB
        target_compile_options(${PROJECT_NAME} PRIVATE /Z7)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -O3 -fomit-frame-pointer -fno-rtti
        -Wall -Wextra -Wpedantic
    )
endif()

# Создаем папку для выходных файлов
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)

# Информация о сборке
message(STATUS "===========================================")
message(STATUS "AppModuleCore - Build Configuration")
message(STATUS "===========================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "ImGui Directory: ${IMGUI_DIR}")
message(STATUS "MinHook Library: ${CMAKE_CURRENT_SOURCE_DIR}/lib/libMinHook-x86.lib")
message(STATUS "Output Directory: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "===========================================")

# Вывод сообщения после успешной сборки
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build completed successfully!"
    COMMAND ${CMAKE_COMMAND} -E echo "DLL: ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll"
)
