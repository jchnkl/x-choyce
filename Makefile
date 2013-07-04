LIBS=xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama xcb-renderutil x11
CXXFLAGS=-g -std=c++11 -Wall $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

SRCS=main.cpp \
		 data_types.cpp \
		 cyclic_iterator.cpp \
		 x_connection.cpp \
		 x_ewmh.cpp \
		 x_client.cpp \
		 x_client_chooser.cpp \
		 x_client_thumbnail.cpp \
		 x_client_thumbnail_manager.cpp \
		 grid.cpp

OBJS=$(SRCS:%.cpp=%.o)

EXE=x:choyce

all: ${OBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${OBJS} -o ${EXE}

clean:
	rm -f ${EXE} ${OBJS}

.PHONY: clean
