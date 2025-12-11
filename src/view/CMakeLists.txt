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
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Quick Qt6::Widgets Qt6::GuiPrivate)
target_link_libraries(${LIB_NAME} PRIVATE logger msft_proxy4::proxy)
