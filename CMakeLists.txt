cmake_minimum_required(VERSION 3.11)

project(libcppsockets VERSION 0.0.0 LANGUAGES CXX)


#
#	C++
#
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# library_compile_options(libcppsockets PRIVATE -Wall -Wextra -Wpedantic -Werror)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)


#
#  CMake options
#
option(build-examples	"Builds the list of examples in ./examples" OFF)
option(use-ssl			"Enables tls features, requires OpenSSL library, paths to it must be defined below in the SSL section" OFF)

#
#	SSL
#
if (use-ssl)

	set(OPENSSL_ROOT_DIR	/usr/local/opt/openssl@3)
	set(OPENSSL_INCLUDE_DIR	/usr/local/opt/openssl@3/include)
	set(OPENSSL_LIBRARIES	/usr/local/opt/openssl@3/lib)
	find_package(OpenSSL REQUIRED)

	add_compile_definitions(ENABLE_TLS)

	include_directories(${OPENSSL_INCLUDE_DIR})
	list(APPEND LIB_LIST ${OPENSSL_LIBRARIES})

endif (use-ssl)



#
#	Library target
#

set(CMAKE_SOURCE_DIR ./src)

include_directories(./includes)
#TODO delete
include_directories(./examples/includes)


add_library(cppsockets STATIC
	src/common/socket_handlers/poll_sockethandler.cpp
	src/common/socket_handlers/select_sockethandler.cpp

	src/common/protocols/packet_manager.cpp

	src/common/inet_address.cpp
	src/common/tcp_socket.cpp

	src/server/server_endpoint.cpp
	src/server/server_client.cpp

	src/server.cpp
)



#
#	Examples targets
#

if (build-examples) 

	include_directories(./examples/includes)

	# add_executable(example-client
	# 	examples/example-client/main.cpp
	# )


	add_executable(example-server
		examples/example-server/main.cpp
	)

	target_link_libraries(example-server cppsockets)

endif(build-examples)



#
#	TESTS targets
#

if (MAKE_TESTS)

# include(CTest)

# if (BUILD_TESTING)
# 	add_subdirectory(tests)
# endif()

# add_test(NAME TestName COMMAND TargetName)

# add_test(test_1
# 	tests/test_client/main.c
# )%

endif(MAKE_TESTS)
