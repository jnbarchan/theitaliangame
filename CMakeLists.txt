cmake_minimum_required(VERSION 3.5)

project(theitaliangame VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(theitaliangame
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        aimodel.cpp aimodel.h baizescene.cpp baizescene.h baizeview.cpp baizeview.h card.cpp card.h carddeck.cpp carddeck.h cardgroup.cpp cardgroup.h cardhand.cpp cardhand.h cardimages.cpp cardimages.h LICENSE logicalmodel.cpp logicalmodel.h main.cpp mainwindow.cpp mainwindow.h README.md
        selectcardmenu.h selectcardmenu.cpp
    )
endif()

target_link_libraries(theitaliangame PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS theitaliangame
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(theitaliangame)
endif()
