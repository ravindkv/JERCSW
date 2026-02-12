# Compiler and standard
#GCC = g++ -fsanitize=address -g -O1 -std=c++17
#GCC = g++ -g -std=c++17
GCC = g++ -std=c++17

# Directories
SRCDIR   = cpp
OBJDIR   = obj

# Sources and objects
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SOURCES))
BINS     := runMain

# Include directories
ROOT_I         = -I`root-config --incdir` -I./header -I./hpp
CORRECTION_INC = -I$(pwd)./corrlib/include

# Compiler flags:
#   -MMD: Generate .d dependency files
#   -MP : Add phony targets for dependencies to avoid "No such file or directory" errors.
CXXFLAGS = $(ROOT_I) $(CORRECTION_INC) -MMD -MP

# Libraries
#ROOT_L         = `root-config --libs`
# take all of root-config --libs, but drop -lROOTNTupleUtil
ROOT_L := $(filter-out -lROOTNTupleUtil,$(shell root-config --libs))

CORRECTION_LIB = -L$(pwd)./corrlib/lib -lcorrectionlib

# Linker flags
LDFLAGS = $(ROOT_L) $(CORRECTION_LIB)

# Add clang-tidy check
CLANG_TIDY = clang-tidy
#@echo "--> Running clang-tidy on $<"
#@$(CLANG_TIDY) $<  --  -std=c++17 $(CXXFLAGS)
#@$(CLANG_TIDY) $< -fix --  -std=c++17 $(CXXFLAGS) #to fix the warnings

#############################
# Build rules
#############################

# Primary target: build the main executable
$(BINS): $(OBJECTS) main.cpp
	@echo "--> Creating executable $@"
	@$(GCC) main.cpp $(OBJECTS) -o $@ $(CXXFLAGS) $(LDFLAGS)

# Rule for building object files + .d dependency files
# Note that we do NOT specify header/%.h here; automatic dependencies from -MMD -MP do it for us.
#@$(GCC) -c $< -o $@ $(CXXFLAGS) $(LDFLAGS)
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo "--> Creating object $@"
	@mkdir -p $(OBJDIR)
	@$(GCC) -c $< -o $@ $(CXXFLAGS)

# Include automatically generated dependency files (.d)
# By doing this, if ANY of the #included headers change, make will rebuild the affected .o
-include $(OBJECTS:.o=.d)

#############################
# Cleanup
#############################
clean:
	rm -f $(wildcard $(OBJDIR)/*.o) \
	      $(wildcard $(OBJDIR)/*.d) \
	      $(BINS)

.PHONY: clean

