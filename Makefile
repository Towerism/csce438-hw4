# List the source file names here for compilation
CLIENT_FILES = fbc.cc arguments_parser.cc fb_client.cc command_stream.cc \
               join_command.cc list_command.cc chat_command.cc leave_command.cc \
               command_factory.cc command_line.cc master_client.cc
SERVER_FILES = fbsd.cc MasterChannel.cc file_locking.cc
MASTER_FILES = fbmaster.cc

PROTO_FILES = fb.proto master.proto

CXX_CONVENTION = .cc

# Generate file names for source files generated by protoc
PROTO_SOURCE_FILES = $(PROTO_FILES:.proto=.pb.cc) $(PROTO_FILES:.proto=.grpc.pb.cc)

# Convenience variable to hold all of the files
FILES = $(CLIENT_FILES) $(SERVER_FILES) $(PROTO_SOURCE_FILES) $(MASTER_FILES)

# Directory names
CLIENT_DIR = client
SERVER_DIR = server
PROTO_DIR = protocol
MASTER_DIR = master
DIRS = $(CLIENT_DIR) $(SERVER_DIR) $(PROTO_DIR) $(MASTER_DIR)

# Executable names
CLIENT = fbc
SERVER = fbsd
MASTER = fbmaster

# Generate the full path names for our files
CLIENT_SOURCES = $(patsubst %,$(CLIENT_DIR)/%,$(CLIENT_FILES))
SERVER_SOURCES = $(patsubst %,$(SERVER_DIR)/%,$(SERVER_FILES))
MASTER_SOURCES = $(patsubst %,$(MASTER_DIR)/%,$(MASTER_FILES))
PROTO_SOURCES = $(patsubst %,$(PROTO_DIR)/%,$(PROTO_SOURCE_FILES))
SOURCES = $(CLIENT_SOURCES) $(SERVER_SOURCES)

# Tempdir where are object files will be
OBJECTS_DIR = objects
OBJECTS_DIRS = $(patsubst %,$(OBJECTS_DIR)/%,$(DIRS))

# List of stuff to clean
CLEAN = $(CLIENT) $(SERVER) $(MASTER) $(OBJECTS_DIR) $(PROTO_DIR)/*.cc $(PROTO_DIR)/*.h

# Generate the object names for our files
PROTO_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(PROTO_DIR)/%.o,$(PROTO_SOURCE_FILES))
CLIENT_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(CLIENT_DIR)/%.o,$(CLIENT_FILES)) $(PROTO_OBJECTS)
SERVER_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(SERVER_DIR)/%.o,$(SERVER_FILES)) $(PROTO_OBJECTS)
MASTER_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(MASTER_DIR)/%.o,$(MASTER_FILES)) $(PROTO_OBJECTS)
OBJECTS = $(CLIENT_OBJECTS) $(SERVER_OBJECTS)

# Generate the dependencies of our source files
DEPS = $(OBJECTS:.o=.d)

# Libraries which may not be in standard locations
NON_STANDARD_LIBS = grpc++ grpc

# Generate ldflags for non standard libs
NON_STANDARD_LDFLAGS = $(shell pkg-config --libs $(NON_STANDARD_LIBS))

# Conventional environment variables
CXX ?= $(which g++)
CPPFLAGS ?= -I$(INCLUDE_DIR) -I/usr/local/include
CXXFLAGS ?= -std=c++11 -ggdb
LDFLAGS ?= $(NON_STANDARD_LDFLAGS) -Wl,--no-as-needed -lgrpc++_reflection \
	-Wl,--as-needed -lpthread -lprotobuf -ldl

# Specify folders to add to the compiler's Include path
INCLUDE_DIR = $(PROTO_DIR)

# GRPC stuff
PROTOC ?= protoc
PROTOS_PATH = $(PROTO_DIR)
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

# rm command
RM = rm -rf
# mkdir comman
MKDIR = mkdir -p

vpath %$(CXX_CONVENTION) $(CLIENT_DIR):$(SERVER_DIR):$(MASTER_DIR):$(PROTO_DIR) 

.DEFAULT: all

all: $(PROTO_SOURCES) $(CLIENT) $(SERVER) $(MASTER)

$(OBJECTS_DIR)/%.o: %$(CXX_CONVENTION) $(PROTO_SOURCES)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -c -o $@

$(OBJECTS_DIR)/%.d: %$(CXX_CONVENTION) $(PROTO_SOURCES)
	@$(MKDIR) $(OBJECTS_DIRS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(CLIENT): $(CLIENT_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(MASTER): $(MASTER_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(SERVER): $(SERVER_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@
	@$(MKDIR) users

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto %.pb.cc
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=$(PROTO_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=$(PROTO_DIR) $<

%.pb.o: %.pb.cc
%.grpc.pb.o: %.grpc.pb.cc

.PHONY: clean
clean:
	$(RM) $(CLEAN)

-include $(DEPS)
