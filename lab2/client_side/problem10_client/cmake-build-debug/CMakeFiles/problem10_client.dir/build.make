# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.20

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/vladbogdan/Downloads/CLion-2021.2.2/clion-2021.2.2/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /home/vladbogdan/Downloads/CLion-2021.2.2/clion-2021.2.2/bin/cmake/linux/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/problem10_client.dir/depend.make
# Include the progress variables for this target.
include CMakeFiles/problem10_client.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/problem10_client.dir/flags.make

CMakeFiles/problem10_client.dir/main.c.o: CMakeFiles/problem10_client.dir/flags.make
CMakeFiles/problem10_client.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/problem10_client.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/problem10_client.dir/main.c.o -c /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/main.c

CMakeFiles/problem10_client.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/problem10_client.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/main.c > CMakeFiles/problem10_client.dir/main.c.i

CMakeFiles/problem10_client.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/problem10_client.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/main.c -o CMakeFiles/problem10_client.dir/main.c.s

# Object files for target problem10_client
problem10_client_OBJECTS = \
"CMakeFiles/problem10_client.dir/main.c.o"

# External object files for target problem10_client
problem10_client_EXTERNAL_OBJECTS =

problem10_client: CMakeFiles/problem10_client.dir/main.c.o
problem10_client: CMakeFiles/problem10_client.dir/build.make
problem10_client: CMakeFiles/problem10_client.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable problem10_client"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/problem10_client.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/problem10_client.dir/build: problem10_client
.PHONY : CMakeFiles/problem10_client.dir/build

CMakeFiles/problem10_client.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/problem10_client.dir/cmake_clean.cmake
.PHONY : CMakeFiles/problem10_client.dir/clean

CMakeFiles/problem10_client.dir/depend:
	cd /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug /home/vladbogdan/CLionProjects/Labs/Lab2/problem10_client/cmake-build-debug/CMakeFiles/problem10_client.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/problem10_client.dir/depend

