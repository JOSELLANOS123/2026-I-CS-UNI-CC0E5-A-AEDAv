CXX      = g++
CXXFLAGS = -std=c++2b -Wall -g -pthread
LDFLAGS  = -pthread
TARGET   = main
SRCS     = main.cpp \
           containers/vector.cpp \
           containers/ListsDemo.cpp \
           containers/BTreeDemo.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)
$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) $^ -o $@
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS) $(TARGET)
.PHONY: all clean