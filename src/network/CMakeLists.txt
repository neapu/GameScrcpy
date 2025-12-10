set(LIB_NAME "network")

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Network)

qt_add_library(
        ${LIB_NAME} STATIC
        Network.cpp
        Network.h
)

target_link_libraries(${LIB_NAME} PRIVATE Qt6::Core Qt6::Network)
target_link_libraries(${LIB_NAME} PRIVATE logger)
