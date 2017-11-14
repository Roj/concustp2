# makefile parameters
SRCDIR      := src
LIBSDIR     := libs
TESTDIR     := tests
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := c

# compiler parameters
CC          := gcc
CFLAGS      := -g3 -std=c99 -Wall -Wpedantic -Werror
LIB         :=
INC         := /usr/local/include libs
DEFINES     :=


#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# binaries file names
# each binary source code is expected to be in $(SRCDIR)/<binary>
TARGET := $(patsubst $(SRCDIR)/%/,%,$(dir $(wildcard $(SRCDIR)/*/)))

# sets the src directory in the VPATH
VPATH := $(SRCDIR)

# sets the build directory based on the binary
BUILDDIR := $(BUILDDIR)/$(BIN)

# source files
SRCS := $(shell find $(SRCDIR)/$(BIN) -type f -name *.$(SRCEXT))
SRCS += $(shell find $(LIBSDIR) -type f -name *.$(SRCEXT))

# object files
OBJS := $(patsubst %,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

# includes the flag to generate the dependency files when compiling
CFLAGS += -MD


# special definitions used for the unit tests
ifeq ($(MAKECMDGOALS),$(TARGETDIR)/tests)

	# sets the special define for tests
	DEFINES := __TESTS__ $(DEFINES)

endif

# adds the include prefix to the include directories
INC := $(addprefix -I,$(INC))

# adds the lib prefix to the libraries
LIB := $(addprefix -l,$(LIB))

# adds the define prefix to the defines
DEFINES := $(addprefix -D,$(DEFINES))


# default: compiles all the binaries
all: $(addprefix bin-,$(TARGET))

# compiles the binary
bin-%:
	@$(MAKE) $(TARGETDIR)/$* BIN=$*

# compiles and runs the unit tests
tests:
	@$(MAKE) $(TARGETDIR)/tests BIN=tests
	./$(TARGETDIR)/tests

# shows usage
help:
	@echo "To compile all binaries:"
	@echo
	@echo "\t\033[1;92m$$ make\033[0m"
	@echo
	@echo "To compile and run the tests:"
	@echo
	@echo "\t\033[1;92m$$ make tests\033[0m"
	@echo
	@echo "To compile just one binary:"
	@echo
	@echo "\t\033[1;92m$$ make bin-\033[1;31m<name>\033[0m"
	@echo
	@echo "where \033[1;31m<name>\033[0m is the name of the binary to compile."
	@echo "Source files for each binary are expected to be in \033[1;92m$(SRCDIR)/\033[0m\033[1;31m<name>\033[0m."
	@echo "Additional source files in \033[1;92m$(LIBSDIR)\033[0m are available for every binary."
	@echo
	@echo "Compiled binaries can be found in \033[1;92m$(TARGETDIR)\033[0m."
	@echo
	@echo "\033[1;92mmake format\033[0m runs clang-format on every source and header file."
	@echo

# clean objects and binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

# creates the directories
dirs:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# runs clang-format in all the project source files
format:
	@find libs/ -iname *.h -o -iname *.$(SRCEXT) | xargs clang-format -i -style=file
	@find src/ -iname *.h -o -iname *.$(SRCEXT) | xargs clang-format -i -style=file

# INTERNAL: builds the binary
$(TARGETDIR)/$(BIN): $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $@
	@echo "LD $@"

# rule to build object files
$(BUILDDIR)/%.o: %.$(SRCEXT)
	@mkdir -p $(basename $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $(LIB) -c -o $@ $<


.PHONY: clean dirs tests all

# includes generated dependency files
-include $(OBJS:.o=.d)
