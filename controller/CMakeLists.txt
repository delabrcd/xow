set(SOURCE_FILES
    controller.cpp
    gip.cpp
    input.cpp
    UsbController.cpp
    usb.cpp
)

add_library(controller ${SOURCE_FILES})
target_link_libraries(controller usb-1.0 util)
target_include_directories(controller PUBLIC ../utils)