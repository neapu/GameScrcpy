set(EXE_NAME GameScrcpy)

find_package(Qt6 REQUIRED COMPONENTS Quick Widgets)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${EXE_NAME}
    main.cpp
        SessionManager.cpp
        SessionManager.h
        Session.cpp
        Session.h
)

qt_add_resources(${EXE_NAME} "resources"
    BASE "view/qml"
    PREFIX "/qml"
    FILES
    view/qml/Main.qml
)

qt_add_resources(${EXE_NAME} "icons"
    BASE "icons"
    PREFIX "/icons"
    FILES
    icons/add.svg
    icons/refresh.svg
    icons/settings.svg
)

set_target_properties(${EXE_NAME} PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(${EXE_NAME}
    PRIVATE Qt6::Quick Qt6::Widgets
)

add_subdirectory(model)
add_subdirectory(device)
add_subdirectory(view)
add_subdirectory(network)

target_link_libraries(${EXE_NAME} PRIVATE model device view network logger)

# Windows平台下自动部署Qt依赖库
if(WIN32)
    # 查找windeployqt.exe
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${QT_INSTALL_PATH} ENV QT6_DIR PATH_SUFFIXES bin)

    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")

        # 添加部署目标
        add_custom_command(TARGET ${EXE_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt libraries..."
                COMMAND ${WINDEPLOYQT_EXECUTABLE}
                --$<LOWER_CASE:$<CONFIG>>
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                $<TARGET_FILE:${EXE_NAME}>
                COMMENT "Deploying Qt libraries to output directory"
                VERBATIM
        )
    else()
        message(WARNING "windeployqt not found. Qt libraries will not be automatically deployed.")
    endif()
endif()