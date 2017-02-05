# List the source file names here for compilation
FBC_FILES = fbc.cc
FBSD_FILES = fbsd.cc
PROTOCOL_FILE = fb.proto
INCLUDE_DIR = $(PROTOCOL_DIR)

PROTO_FILES = fb.pb.cc fb.grpc.pb.cc

CXX_CONVENTION = .cc

# Convenience variable to hold all of the files
FILES = $(FBC_FILES) $(FBSD_FILES) $(PROTO_FILES)

# Directory names
FBC_DIR = client
FBSD_DIR = server
PROTOCOL_DIR = protocol

# Executable names
FBC = fbc
FBSD = fbsd

# Generate the full path names for our files
FBC_SOURCES = $(patsubst %,$(FBC_DIR)/%,$(FBC_FILES))
FBSD_SOURCES = $(patsubst %,$(FBSD_DIR)/%,$(FBSD_FILES))
SOURCES = $(FBC_SOURCES) $(FBSD_SOURCES)

# Tempdir where are object files will be
TEMPDIR = objects

# Generate the object names for our files
FBC_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/%.o,$(FBC_FILES))
FBSD_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/%.o,$(FBSD_FILES))
PROTO_OBJECTS = $(patsubst %$(CXX_CONVENTION),$(TEMPDIR)/%.o,$(PROTO_FILES))
OBJECTS = $(FBC_OBJECTS) $(FBSD_OBJECTS) $(PROTO_OBJECTS)

# Generate the dependencies of our source files
DEPS = $(OBJECTS:.o=.d)

# Conventional environment variables
CXX ?= $(which g++)
CXXFLAGS ?= -std=c++11 -ggdb -I$(INCLUDE_DIR)
LDFLAGS ?= -lpthread -lprotobuf -lgrpc -lgrpc++

# GRPC stuff
PROTOC ?= protoc
PROTOS_PATH = $(PROTOCOL_DIR)
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

.DEFAULT: all

all: $(FBC) $(FBSD)

$(TEMPDIR)/%.o: $(FBC_DIR)/%$(CXX_CONVENTION)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(TEMPDIR)/%.o: $(FBSD_DIR)/%$(CXX_CONVENTION)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(TEMPDIR)/%.o: $(PROTOCOL_DIR)/%$(CXX_CONVENTION)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Because we can't always guarantee which rule executes first, make directory in both
$(TEMPDIR)/%.d: $(FBC_DIR)/%$(CXX_CONVENTION)
	mkdir -p $(TEMPDIR)
	$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
$(TEMPDIR)/%.d: $(FBSD_DIR)/%$(CXX_CONVENTION)
	mkdir -p $(TEMPDIR)
	$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@
$(TEMPDIR)/%.d: $(PROTOCOL_DIR)/%$(CXX_CONVENTION)
	mkdir -p $(TEMPDIR)
	$(CXX) $(CXXFLAGS) $< -MM -MT $(@:.d=.o) >$@

$(FBC): $(FBC_OBJECTS) $(PROTO_OBJECTS)
	$(CXX) $(LDFLAGS) $(FBC_OBJECTS) -o $@

$(FBSD): $(FBSD_OBJECTS) $(PROTO_OBJECTS)
	$(CXX) $(LDFLAGS) $(FBSD_OBJECTS) -o $@
	@mkdir -p users # figure out how to move this out

.PRECIOUS: %.grpc.pb.cc
%.grpc.pb.cc: %.proto %.pb.cc
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=$(PROTOCOL_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

.PRECIOUS: %.pb.cc
%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=$(PROTOCOL_DIR) $<

.PHONY: clean
clean:
	rm -rf $(FBC) $(FBSD) objects $(PROTOCOL_DIR)/*.cc $(PROTOCOL_DIR)/*.h

-include $(DEPS)
