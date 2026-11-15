cmake_minimum_required(VERSION 3.16)
project(pr4_vizprog VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Автоматически обрабатываем классы с Q_OBJECT.
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Цель сборки – приложение PrinterQueue.
add_executable(pr4_vizprog WIN32
    src/main.cpp src/mainwindow.h src/mainwindow.cpp
    src/printjob.h src/jobmodel.h src/jobmodel.cpp
    src/simulation.h src/simulation.cpp
)
target_include_directories(pr4_vizprog PRIVATE src)
target_link_libraries(pr4_vizprog PRIVATE Qt6::Widgets)
if(MSVC)
  # Корректное чтение русских строк и комментариев компилятором MSVC.
  target_compile_options(pr4_vizprog PRIVATE /utf-8)
endif()
