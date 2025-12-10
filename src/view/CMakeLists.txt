set(LIB_NAME "view")

find_package(Qt6 6.8 REQUIRED COMPONENTS Quick Widgets)

qt_add_library(
        ${LIB_NAME} STATIC
        QMLAdapter.cpp
        QMLAdapter.h
        DeviceWidget.cpp
        DeviceWidget.h
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Quick Qt6::Widgets)
target_link_libraries(${LIB_NAME} PRIVATE logger)
