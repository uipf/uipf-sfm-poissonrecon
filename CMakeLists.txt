cmake_minimum_required(VERSION 2.8.8)
project(uipf-sfm-poissonrecon)

#c++11 support
if(${CMAKE_VERSION} VERSION_LESS 3.1.0)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
else()
	set(CMAKE_CXX_STANDARD 11)
endif()

if(UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

	# sanitize memory issues https://gcc.gnu.org/gcc-4.8/changes.html
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
endif()

message("CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message("CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")

# PoissonRecon
include(ExternalProject)
# build bundler as external project
ExternalProject_Add(PoissonRecon
	SOURCE_DIR ${uipf-sfm-bundler_SOURCE_DIR}/PoissonRecon
	CONFIGURE_COMMAND rsync -r ${uipf-sfm-bundler_SOURCE_DIR}/PoissonRecon .
	# parallelize the build to be faster
	BUILD_COMMAND cd PoissonRecon && make -j 4
	TEST_COMMAND ""
	INSTALL_COMMAND "" # skip install step for COLMAP
)
ExternalProject_Get_Property(PoissonRecon binary_dir)
set(PoissonRecon_BINARY_DIR "${binary_dir}")


# find libuipf-module
# search in normal system paths as well as installation prefix for installation as unprivileged user.
find_library(
	ModuleBase uipf-module
	PATHS ${CMAKE_INSTALL_PREFIX}/lib
)

find_library(
	uipf-sfm uipf-sfm
	PATHS ${CMAKE_INSTALL_PREFIX}/lib
)
message("uipf-sfm: ${uipf-sfm}")

message("ModuleBase: ${ModuleBase}")
include_directories(${CMAKE_INSTALL_PREFIX}/include)


add_library(PoissonReconModule SHARED PoissonReconModule.cpp)
target_link_libraries(PoissonReconModule ${ModuleBase} ${uipf-sfm})
add_dependencies(PoissonReconModule PoissonRecon)

add_library(SSDReconModule SHARED SSDReconModule.cpp)
target_link_libraries(SSDReconModule ${ModuleBase} ${uipf-sfm})
add_dependencies(SSDReconModule PoissonRecon)

add_library(SurfaceTrimmerModule SHARED SurfaceTrimmerModule.cpp)
target_link_libraries(SurfaceTrimmerModule ${ModuleBase} ${uipf-sfm})
add_dependencies(SurfaceTrimmerModule PoissonRecon)


# installation targets
install(TARGETS
	PoissonReconModule
	SSDReconModule
	SurfaceTrimmerModule

	LIBRARY DESTINATION lib/uipf
	ARCHIVE DESTINATION lib/uipf
)

install(FILES ${PoissonRecon_BINARY_DIR}/Bin/Linux/PoissonRecon
	DESTINATION bin/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	RENAME uipf-poisson-recon
)
install(FILES ${PoissonRecon_BINARY_DIR}/Bin/Linux/SSDRecon
	DESTINATION bin/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	RENAME uipf-ssd-recon
)
install(FILES ${PoissonRecon_BINARY_DIR}/Bin/Linux/SurfaceTrimmer
	DESTINATION bin/
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
	RENAME uipf-surface-trimmer
)

