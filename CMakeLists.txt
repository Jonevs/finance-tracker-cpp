cmake_minimum_required(VERSION 3.10)

project(FinanceTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets Sql REQUIRED)

# Enable automatic MOC for Qt's meta-object system
set(CMAKE_AUTOMOC ON)

add_executable(FinanceTracker
    main.cpp
    mainwindow.cpp
    mainwindow.h
    database.cpp  
    database.h
)

target_link_libraries(FinanceTracker Qt5::Widgets Qt5::Sql)
