set(LIB_NAME "view")

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Widgets)
if(Qt6_VERSION VERSION_GREATER_EQUAL "6.9")
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
endif()

qt_add_library(
        ${LIB_NAME} STATIC
        QMLAdapter.cpp
        QMLAdapter.h
        DeviceWindow.cpp
        DeviceWindow.h
        CentralWidget.cpp
        CentralWidget.h
        ControlDock.cpp
        ControlDock.h
        VideoRenderer.cpp
        VideoRenderer.h
        YuvTexturesSrb.cpp
        YuvTexturesSrb.h
        EmptySrb.cpp
        EmptySrb.h
        Uniforms.cpp
        Uniforms.h
        VaapiTexturesSrb.cpp
        VaapiTexturesSrb.h
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Quick Qt6::Widgets Qt6::GuiPrivate)
target_link_libraries(${LIB_NAME} PRIVATE logger msft_proxy4::proxy)

if (LINUX)
    message("Configuring for Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBVA REQUIRED libva libva-drm)
    pkg_check_modules(LIBDRM REQUIRED libdrm)
    pkg_check_modules(EGL REQUIRED egl)
    target_include_directories(${LIB_NAME} PRIVATE ${LIBVA_INCLUDE_DIRS} ${LIBDRM_INCLUDE_DIRS} ${EGL_INCLUDE_DIRS})
    target_link_libraries(${LIB_NAME} PRIVATE ${LIBVA_LIBRARIES} ${LIBDRM_LIBRARIES} ${EGL_LIBRARIES})
endif()
