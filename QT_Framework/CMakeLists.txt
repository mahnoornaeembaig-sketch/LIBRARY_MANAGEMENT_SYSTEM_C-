cmake_minimum_required(VERSION 3.19)
project(G2-7LibraryManagementSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

qt_add_executable(G2-7LibraryManagementSystem
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp mainwindow.h
    adminwindow.cpp adminwindow.h
    addbookdialog.cpp addbookdialog.h
    adduserdialog.cpp adduserdialog.h
    overduedialog.cpp overduedialog.h
    globals.h date.h logger.h records.h resource.h user.h admin.h
    userwindow.h
    userwindow.cpp
    userstabledialog.h
    userstabledialog.cpp
    registerdialog.h
    registerdialog.cpp
)

target_link_libraries(G2-7LibraryManagementSystem
    PRIVATE
        Qt6::Core
        Qt6::Widgets
)

include(GNUInstallDirs)
install(TARGETS G2-7LibraryManagementSystem
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
