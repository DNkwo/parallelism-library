CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
INCLUDE_DIR = -I./include
SRCS = Farm.cpp Pipeline.cpp Pipe.cpp StageManager.cpp
OBJS = $(SRCS:.cpp=.o)
LIB = libpara-pat.a

.PHONY: all clean

all: $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB)