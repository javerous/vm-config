cmake_minimum_required(VERSION 3.10)


# Fetch project version.
find_program(AWK awk mawk gawk)

execute_process(
	COMMAND "${AWK}" "-F" "=" "length(\$1) >= 5 && \$1 ~ /[:blank:]*CURRENT_PROJECT_VERSION[:blank:]*/ { gsub(/^[[:space:]]+|[[:space:]]+\$|;/, \"\", \$2); printf \"%s\", \$2; exit; }" "${CMAKE_CURRENT_SOURCE_DIR}/vm-config.xcodeproj/project.pbxproj"
	RESULT_VARIABLE AWK_EXITCODE
	OUTPUT_VARIABLE AWK_OUTPUT
)

if(NOT AWK_EXITCODE EQUAL "0")
	message(FATAL_ERROR "unable to fetch project version from Xcode project")
endif()

string(STRIP ${AWK_OUTPUT} PROJ_VERSION)


# Create project.
project(vm-config)


# Define sources files.
set(SOURCE_FILE	vm-config/main.c
				vm-config/SMError.c
				vm-config/SMVersion.c
				vm-config/SMStringHelper.c
				vm-config/SMBytesWritter.c
				vm-config/SMVMwareVMXHelper.c
				vm-config/SMVMwareNVRAM.c
				vm-config/SMVMwareNVRAMHelper.c
				vm-config/SMVMwareVMX.c
				vm-config/SMVMwareVMXHelper.c
)


# Add the executable.
add_executable(vm-config ${SOURCE_FILE})


# Set compile parameters.
add_compile_options(-Werror)
add_definitions(-DPROJ_VERSION=${PROJ_VERSION})


# Link to iconv.
find_package(Iconv REQUIRED)

target_link_libraries(vm-config Iconv::Iconv)


# Extra handling on non-Apple. Not sure it's the best way to do things with cmake, who know, who care...
if(NOT APPLE)
	add_definitions(-D_GNU_SOURCE)
  
  
	# Link to BSD.
	find_library(BSD_LIB libbsd.a bsd)

	if(BSD_LIB)
		target_precompile_headers(vm-config PRIVATE <bsd/bsd.h>)
		target_link_libraries(vm-config ${BSD_LIB})
	else()
		message(FATAL_ERROR "libbsd-dev is probaly needed on your system")
	endif()


	# Link to UUID.
	find_library(UUID_LIB libuuid.a uuid)

	if(UUID_LIB)
		target_link_libraries(vm-config ${UUID_LIB})
	else()
		message(FATAL_ERROR "uuid-dev is probaly needed on your system")
  endif()
endif()
