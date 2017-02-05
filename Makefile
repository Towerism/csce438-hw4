# List the source file names here for compilation
FBC_FILES = fbc.cc arguments_parser.cc fb_client.cc
FBSD_FILES = fbsd.cc
PROTOCOL_FILE = fb.proto
INCLUDE_DIR = $(PROTO_DIR)

PROTO_FILES = fb.pb.cc fb.grpc.pb.cc

CXX_CONVENTION = .cc

# Convenience variable to hold all of the files
FILES = $(FBC_FILES) $(FBSD_FILES) $(PROTO_FILES)

# Directory names
FBC_DIR = client
FBSD_DIR = server
PROTO_DIR = protocol

# Executable names
FBC = fbc
FBSD = fbsd

# Generate the full path names for our files
FBC_SOURCES = $(patsubst %,$(FBC_DIR)/%,$(FBC_FILES))
FBSD_SOURCES = $(patsubst %,$(FBSD_DIR)/%,$(FBSD_FILES))
PROTO_SOURCES = $(patsubst %,$(PROTO_DIR)/%,$(PROTO_FILES))
SOURCES = $(FBC_SOURCES) $(FBSD_SOURCES)

# Tempdir where are object files will be
TEMPDIR = objects

# Generate the object names for our files
PROTO_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/$(PROTO_DIR)/%.o,$(PROTO_FILES))
FBC_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/$(FBC_DIR)/%.o,$(FBC_FILES)) $(PROTO_OBJECTS)
FBSD_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/$(FBSD_DIR)/%.o,$(FBSD_FILES)) $(PROTO_OBJECTS)
OBJECTS = $(FBC_OBJECTS) $(FBSD_OBJECTS)

# Generate the dependencies of our source files
DEPS = $(OBJECTS:.o=.d)

# Conventional environment variables
CXX ?= $(which g++)
CXXFLAGS ?= -std=c++11 -ggdb -I$(INCLUDE_DIR)
LDFLAGS ?= -lpthread -lprotobuf -lgrpc -lgrpc++

# GRPC stuff
PROTOC ?= protoc
PROTOS_PATH = $(PROTO_DIR)
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

vpath %$(CXX_CONVENTION) $(FBC_DIR)
vpath %$(CXX_CONVENTION) $(FBSD_DIR)
vpath %$(CXX_CONVENTION) $(PROTO_DIR)

.DEFAULT: all

all: $(PROTO_SOURCES) $(FBC) $(FBSD)

$(TEMPDIR)/%.o: %$(CXX_CONVENTION)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEMPDIR)/%.d: %$(CXX_CONVENTION)
	@mkdir -p $(TEMPDIR)/$(FBC_DIR) && mkdir -p $(TEMPDIR)/$(FBSD_DIR) && mkdir -p $(TEMPDIR)/$(PROTO_DIR)
	$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(FBC): $(FBC_OBJECTS)
	$(CXX) $(LDFLAGS) $(FBC_OBJECTS) -o $@

$(FBSD): $(FBSD_OBJECTS)
	$(CXX) $(LDFLAGS) $(FBSD_OBJECTS) -o $@
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
	rm -rf $(FBC) $(FBSD) objects $(PROTO_DIR)/*.cc $(PROTO_DIR)/*.h

-include $(DEPS)
