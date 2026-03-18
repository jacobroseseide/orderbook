CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I include
LDFLAGS  :=

# macOS needs pthreads for httplib
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
  LDFLAGS += -lpthread
else
  LDFLAGS += -lpthread
endif

.PHONY: all server clean

# default: build the original CLI demo
all: orderbook

orderbook: main.cpp orderbook.hpp priceLevels.hpp order.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp $(LDFLAGS)

# build the HTTP server
server: orderbook_server

orderbook_server: main_server.cpp orderbook.hpp priceLevels.hpp order.hpp
	$(CXX) $(CXXFLAGS) -o $@ main_server.cpp $(LDFLAGS)

clean:
	rm -f orderbook orderbook_server
