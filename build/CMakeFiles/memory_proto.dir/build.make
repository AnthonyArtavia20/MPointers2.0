# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.31

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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/sergiodanielalvarezchanto/desktop/MPointers2.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build

# Include any dependencies generated for this target.
include CMakeFiles/memory_proto.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/memory_proto.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/memory_proto.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/memory_proto.dir/flags.make

memory_manager.pb.h: /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/memory_manager.proto
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating memory_manager.pb.h, memory_manager.pb.cc, memory_manager.grpc.pb.h, memory_manager.grpc.pb.cc"
	/usr/local/opt/protobuf/bin/protoc --proto_path=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0 --cpp_out=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build --grpc_out=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build --plugin=protoc-gen-grpc=/usr/local/opt/grpc/bin/grpc_cpp_plugin /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/memory_manager.proto

memory_manager.pb.cc: memory_manager.pb.h
	@$(CMAKE_COMMAND) -E touch_nocreate memory_manager.pb.cc

memory_manager.grpc.pb.h: memory_manager.pb.h
	@$(CMAKE_COMMAND) -E touch_nocreate memory_manager.grpc.pb.h

memory_manager.grpc.pb.cc: memory_manager.pb.h
	@$(CMAKE_COMMAND) -E touch_nocreate memory_manager.grpc.pb.cc

CMakeFiles/memory_proto.dir/codegen:
.PHONY : CMakeFiles/memory_proto.dir/codegen

CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o: CMakeFiles/memory_proto.dir/flags.make
CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o: memory_manager.pb.cc
CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o: CMakeFiles/memory_proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o -MF CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o.d -o CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o -c /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.pb.cc

CMakeFiles/memory_proto.dir/memory_manager.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/memory_proto.dir/memory_manager.pb.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.pb.cc > CMakeFiles/memory_proto.dir/memory_manager.pb.cc.i

CMakeFiles/memory_proto.dir/memory_manager.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/memory_proto.dir/memory_manager.pb.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.pb.cc -o CMakeFiles/memory_proto.dir/memory_manager.pb.cc.s

CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o: CMakeFiles/memory_proto.dir/flags.make
CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o: memory_manager.grpc.pb.cc
CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o: CMakeFiles/memory_proto.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o -MF CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o.d -o CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o -c /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.grpc.pb.cc

CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.i"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.grpc.pb.cc > CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.i

CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.s"
	/Library/Developer/CommandLineTools/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/memory_manager.grpc.pb.cc -o CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.s

# Object files for target memory_proto
memory_proto_OBJECTS = \
"CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o" \
"CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o"

# External object files for target memory_proto
memory_proto_EXTERNAL_OBJECTS =

libmemory_proto.a: CMakeFiles/memory_proto.dir/memory_manager.pb.cc.o
libmemory_proto.a: CMakeFiles/memory_proto.dir/memory_manager.grpc.pb.cc.o
libmemory_proto.a: CMakeFiles/memory_proto.dir/build.make
libmemory_proto.a: CMakeFiles/memory_proto.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library libmemory_proto.a"
	$(CMAKE_COMMAND) -P CMakeFiles/memory_proto.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/memory_proto.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/memory_proto.dir/build: libmemory_proto.a
.PHONY : CMakeFiles/memory_proto.dir/build

CMakeFiles/memory_proto.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/memory_proto.dir/cmake_clean.cmake
.PHONY : CMakeFiles/memory_proto.dir/clean

CMakeFiles/memory_proto.dir/depend: memory_manager.grpc.pb.cc
CMakeFiles/memory_proto.dir/depend: memory_manager.grpc.pb.h
CMakeFiles/memory_proto.dir/depend: memory_manager.pb.cc
CMakeFiles/memory_proto.dir/depend: memory_manager.pb.h
	cd /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/sergiodanielalvarezchanto/desktop/MPointers2.0 /Users/sergiodanielalvarezchanto/desktop/MPointers2.0 /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build /Users/sergiodanielalvarezchanto/desktop/MPointers2.0/build/CMakeFiles/memory_proto.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/memory_proto.dir/depend

