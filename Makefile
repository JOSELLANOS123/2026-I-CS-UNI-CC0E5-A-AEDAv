CXX      = g++
CXXFLAGS = -std=c++2b -Wall -g -pthread
LDFLAGS  = -pthread
TARGET   = main
SRCS     = main.cpp \
           containers/vector.cpp \
           containers/ListsDemo.cpp \
           containers/TreeDemo.cpp \
           containers/AVLDemo.cpp \
           containers/HeapDemo.cpp \
           containers/HashDemo.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	del /Q $(subst /,\,$(OBJS)) $(TARGET).exe 2>nul || true
.PHONY: all clean