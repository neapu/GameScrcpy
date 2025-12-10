set(LIB_NAME "device")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Concurrent)

qt_add_library(
        ${LIB_NAME} STATIC
        DeviceInfo.cpp
        DeviceInfo.h
        AdbHelper.cpp
        AdbHelper.h
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Core Qt6::Concurrent)
target_link_libraries(${LIB_NAME} PRIVATE logger)