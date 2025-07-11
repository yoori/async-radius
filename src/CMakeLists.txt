configure_file(version.h.in version.h ESCAPE_QUOTES @ONLY)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME}
  PRIVATE
    socket.cpp
    packet.cpp
    attribute.cpp
    vendor_attribute.cpp
    utils.cpp
    dictionaries.cpp
    error.cpp
    type_decoder.cpp
    packet_reader.cpp
)

target_link_libraries(${PROJECT_NAME}
  OpenSSL::Crypto
  Boost::boost
  Threads::Threads)

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/radproto>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
  $<INSTALL_INTERFACE:include/radproto>
  $<INSTALL_INTERFACE:include>
)
