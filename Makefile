# List the source file names here for compilation
CLIENT_FILES = fbc.cc arguments_parser.cc fb_client.cc
SERVER_FILES = fbsd.cc

PROTO_FILES = fb.pb.cc fb.grpc.pb.cc

CXX_CONVENTION = .cc

# Convenience variable to hold all of the files
FILES = $(CLIENT_FILES) $(SERVER_FILES) $(PROTO_FILES)

# Directory names
CLIENT_DIR = client
SERVER_DIR = server
PROTO_DIR = protocol

# Executable names
CLIENT = fbc
SERVER = fbsd

# Generate the full path names for our files
CLIENT_SOURCES = $(patsubst %,$(CLIENT_DIR)/%,$(CLIENT_FILES))
SERVER_SOURCES = $(patsubst %,$(SERVER_DIR)/%,$(SERVER_FILES))
PROTO_SOURCES = $(patsubst %,$(PROTO_DIR)/%,$(PROTO_FILES))
SOURCES = $(CLIENT_SOURCES) $(SERVER_SOURCES)

# Tempdir where are object files will be
OBJECTS_DIR = objects

# Generate the object names for our files
PROTO_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(PROTO_DIR)/%.o,$(PROTO_FILES))
CLIENT_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(CLIENT_DIR)/%.o,$(CLIENT_FILES)) $(PROTO_OBJECTS)
SERVER_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(OBJECTS_DIR)/$(SERVER_DIR)/%.o,$(SERVER_FILES)) $(PROTO_OBJECTS)
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

vpath %$(CXX_CONVENTION) $(CLIENT_DIR):$(SERVER_DIR):$(PROTO_DIR) 

.DEFAULT: all

all: $(PROTO_SOURCES) $(CLIENT) $(SERVER)

$(OBJECTS_DIR)/%.o: %$(CXX_CONVENTION)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -c -o $@

$(OBJECTS_DIR)/%.d: %$(CXX_CONVENTION)
	@mkdir -p $(OBJECTS_DIR)/$(CLIENT_DIR) && mkdir -p $(OBJECTS_DIR)/$(SERVER_DIR) && mkdir -p $(OBJECTS_DIR)/$(PROTO_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(CLIENT): $(CLIENT_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@

$(SERVER): $(SERVER_OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@
	@mkdir -p users # figure out how to move this out

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
	rm -rf $(CLIENT) $(SERVER) objects $(PROTO_DIR)/*.cc $(PROTO_DIR)/*.h

-include $(DEPS)
