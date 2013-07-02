LIBS=xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama x11
CXXFLAGS=-g -std=c++11 -Wall $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

SRCS=main.cpp \
		 data_types.cpp \
		 x_connection.cpp \
		 x_client.cpp \
		 x_client_chooser.cpp \
		 grid.cpp

OBJS=$(SRCS:%.cpp=%.o)

EXE=x:choyce

all: ${OBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${OBJS} -o ${EXE}

clean:
	rm -f ${EXE} ${OBJS}

.PHONY: clean
