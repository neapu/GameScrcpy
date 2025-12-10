set(LIB_NAME "model")

find_package(Qt6 REQUIRED COMPONENTS Core)

set(LIB_SOURCES
    DeviceModel.cpp
    DeviceModel.h
)

qt_add_library(${LIB_NAME} STATIC ${LIB_SOURCES})

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Core)
target_link_libraries(${LIB_NAME} PRIVATE logger device)