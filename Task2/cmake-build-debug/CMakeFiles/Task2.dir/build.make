# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.12

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /snap/clion/44/bin/cmake/linux/bin/cmake

# The command to remove a file.
RM = /snap/clion/44/bin/cmake/linux/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/masaya/ドキュメント/compiler/Task2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/masaya/ドキュメント/compiler/Task2/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/Task2.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Task2.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Task2.dir/flags.make

CMakeFiles/Task2.dir/main.c.o: CMakeFiles/Task2.dir/flags.make
CMakeFiles/Task2.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/masaya/ドキュメント/compiler/Task2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/Task2.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/Task2.dir/main.c.o   -c /home/masaya/ドキュメント/compiler/Task2/main.c

CMakeFiles/Task2.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/Task2.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/masaya/ドキュメント/compiler/Task2/main.c > CMakeFiles/Task2.dir/main.c.i

CMakeFiles/Task2.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/Task2.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/masaya/ドキュメント/compiler/Task2/main.c -o CMakeFiles/Task2.dir/main.c.s

# Object files for target Task2
Task2_OBJECTS = \
"CMakeFiles/Task2.dir/main.c.o"

# External object files for target Task2
Task2_EXTERNAL_OBJECTS =

Task2: CMakeFiles/Task2.dir/main.c.o
Task2: CMakeFiles/Task2.dir/build.make
Task2: CMakeFiles/Task2.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/masaya/ドキュメント/compiler/Task2/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable Task2"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Task2.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Task2.dir/build: Task2

.PHONY : CMakeFiles/Task2.dir/build

CMakeFiles/Task2.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Task2.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Task2.dir/clean

CMakeFiles/Task2.dir/depend:
	cd /home/masaya/ドキュメント/compiler/Task2/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/masaya/ドキュメント/compiler/Task2 /home/masaya/ドキュメント/compiler/Task2 /home/masaya/ドキュメント/compiler/Task2/cmake-build-debug /home/masaya/ドキュメント/compiler/Task2/cmake-build-debug /home/masaya/ドキュメント/compiler/Task2/cmake-build-debug/CMakeFiles/Task2.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Task2.dir/depend

