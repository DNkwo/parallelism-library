CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread -g

INCLUDE_DIR = ./para-pat/include
LIB_DIR = ./para-pat
LIBS = -lpara-pat

EXAMPLES = example main
SRCS = $(addsuffix .cpp,$(EXAMPLES))
OBJS = $(SRCS:.cpp=.o)
EXECS = $(EXAMPLES)

.PHONY: all clean

all: $(EXECS)

$(EXECS): % : %.o | $(LIB_DIR)/$(LIBS:lib%=%)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) $< -o $@ -L$(LIB_DIR) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(LIB_DIR)/$(LIBS:lib%=%):
	$(MAKE) -C para-pat

clean:
	rm -f $(OBJS) $(EXECS)
	$(MAKE) -C para-pat clean