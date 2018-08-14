include_directories(../../include)
if (APPLE)
    include_directories( /usr/local/opt/openssl/include )
else()
    include_directories( /usr/include/openssl/include )
endif()

add_library(
  fiat

  curve25519.c
)
