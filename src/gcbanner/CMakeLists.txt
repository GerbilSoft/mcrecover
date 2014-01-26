PROJECT(gcbanner)
cmake_minimum_required(VERSION 2.6)

# gcbanner source and binary directories.
# Binary directory is required for moc-generated headers.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}")

# Include the previous directory.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../")

# libgctools source and binary directories.
# Binary directory is needed for byteorder.h.
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/../libgctools")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_BINARY_DIR}/../libgctools")

# Main binary directory. Needed for git_version.h
INCLUDE_DIRECTORIES("${mcrecover_base_BINARY_DIR}")

# Sources.
SET(gcbanner_SRCS gcbanner.cpp)

#########################
# Build the executable. #
#########################

ADD_EXECUTABLE(gcbanner ${gcbanner_SRCS})

# libgctools
TARGET_LINK_LIBRARIES(gcbanner gctools)

# Split debug information.
INCLUDE(SplitDebugInformation)
IF(SPLIT_DEBUG)
	#SPLIT_DEBUG_INFORMATION(gcbanner)
ENDIF(SPLIT_DEBUG)
