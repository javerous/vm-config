cmake_minimum_required(VERSION 3.10)

# Set the project name.
project(vm-config)

# Set sources.
set(SOURCE_FILE vm-config/main.c
				vm-config/SMError.c
				vm-config/SMStringHelper.c
				vm-config/SMVMwareNVRAM.c
				vm-config/SMVMwareNVRAMHelper.c
				vm-config/SMVMwareVMX.c
				vm-config/SMVMwareVMXHelper.c
)


# Add the executable
add_executable(vm-config ${SOURCE_FILE})

# Link to iconv.
find_package(Iconv REQUIRED)

target_link_libraries(vm-config Iconv::Iconv)

# Extra handling on non-Apple. Not sure it's the best way to do things with cmake, who know, who care...
if(NOT APPLE)
  add_definitions(-D_GNU_SOURCE)
  
  # Link to BSD.
  find_library(BSD_LIB bsd)

  if(BSD_LIB)
    target_precompile_headers(vm-config PRIVATE <bsd/bsd.h>)
    target_link_libraries(vm-config bsd)
    else()
    message(FATAL_ERROR "libbsd-dev is probaly needed on your system")
  endif()

  # Link to UUID.
  find_library(UUID_LIB uuid)

  if(UUID_LIB)
    target_link_libraries(vm-config uuid)
    else()
        message(FATAL_ERROR "uuid-dev is probaly needed on your system")
  endif()
 endif()
