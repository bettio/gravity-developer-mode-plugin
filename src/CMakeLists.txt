set(GravityDeveloperModePlugin_SRCS
    DeveloperModePlugin.cpp
    DeviceInfo.cpp
)

# Hyperspace interface
hyperspace_add_qt5_producer(GravityDeveloperModePlugin_SRCS ${CMAKE_SOURCE_DIR}/share/com.ispirata.Hemera.DeviceInfo.json
                            DeviceInfoProducer DeviceInfoProducer)

# final lib
add_library(gravity-center-plugin-developer-mode SHARED ${GravityDeveloperModePlugin_SRCS})

target_link_libraries(gravity-center-plugin-developer-mode
                      Gravity::Supermassive
                      HemeraQt5SDK::Core
                      HyperspaceQt5::Core HyperspaceQt5::ProducerConsumer
                      Qt5::Core Qt5::Network Qt5::DBus
                      ${ZYPP_LIBRARY})

# Install phase
install(TARGETS gravity-center-plugin-developer-mode
        RUNTIME DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        ARCHIVE DESTINATION "${GRAVITY_PLUGINS_DIR}" COMPONENT shlib
        COMPONENT gravity-developer-mode)
