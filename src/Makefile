LIBS=xft xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama \
     xcb-xfixes xcb-randr x11 x11-xcb xrender gl xcb-ewmh xcb-icccm xcb-image
CXXFLAGS=-g -std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

EXE=x:choyce

CPPSRCS=main.cpp \
	opengl.cpp \
	data_types.cpp \
	cyclic_iterator.cpp \
	thumbnail_manager.cpp \
	x_xrm.cpp \
	x_connection.cpp \
	x_event_source.cpp \
	x_ewmh.cpp \
	x_client.cpp \
	x_client_icon.cpp \
	x_client_name.cpp \
	x_client_chooser.cpp \
	x_client_thumbnail_gl.cpp \
	grid.cpp

CPPOBJS=$(CPPSRCS:%.cpp=%.o)

all: ${DIRS} ${CPPOBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${CPPOBJS} -o ${EXE}

shader/%.hpp.gch:
	make -C shader

default_icon/%.c:
	make -C default_icon

%.o: %.cpp %.hpp
	${CXX} ${CXXFLAGS} -c $<

%.hpp: %.cpp
	${CXX} ${CXXFLAGS} -c $<

version: ${LIBS}
	@echo -e "\nGCC version:"
	@gcc -v 2>&1

${LIBS}:
	@printf "%-20s" "$@:"
	@pkg-config --modversion $@

clean:
	make -C shader clean
	rm -f ${EXE} ${CPPOBJS}

.PHONY: clean version
