LIBS=xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama
CXXFLAGS=-g -std=c++11 -Wall $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

SRCS=winswitch.cpp \
		 data_types.cpp \
		 x_connection.cpp \
		 x_client.cpp \
		 x_clients_preview.cpp \
		 grid.cpp

EXE=winswitch

all:
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${SRCS} -o ${EXE}
