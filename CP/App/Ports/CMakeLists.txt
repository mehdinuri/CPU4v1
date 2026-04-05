# Ports — header-only C vtable interface definitions.
# No compiled sources. Consumed as a pure include-path library.
# Port headers are written in Phase 2.

add_library(Ports INTERFACE)
target_include_directories(Ports INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
