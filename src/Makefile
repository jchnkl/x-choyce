LIBS=xft xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama \
     xcb-xfixes xcb-randr x11 x11-xcb xrender gl xcb-ewmh xcb-icccm xcb-image
CXXFLAGS=-g -std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

EXE=x:choyce

DIRS=shader default_icon
CLEANDIRS=$(DIRS:%=clean-%)

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
	${CXX} ${CXXFLAGS} ${CPPOBJS} ${LDFLAGS} -o ${EXE}

%.o: %.cpp %.hpp
	${CXX} ${CXXFLAGS} -c $<


version: ${LIBS}
	@echo -e "\nGCC version:"
	@gcc -v 2>&1

${LIBS}:
	@printf "%-20s" "$@:"
	@pkg-config --modversion $@

${DIRS}:
	make -C $@

clean: ${CLEANDIRS}
	rm -f ${EXE} ${CPPOBJS}

clean-%:
	make -C $(@:clean-%=%) clean

.PHONY: ${DIRS} ${CLEANDIRS} clean version
