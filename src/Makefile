LIBS=xft xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama \
     xcb-xfixes xcb-randr x11 x11-xcb xrender gl xcb-ewmh xcb-icccm xcb-image
CXXFLAGS=-g -std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

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

HPPSRCS=shader/shader.hpp \
				chooser_t.hpp \
				opengl.hpp \
				cyclic_iterator.hpp \
				data_types.hpp \
				grid.hpp \
				layout_t.hpp \
				thumbnail_manager.hpp \
				thumbnail_t.hpp \
				x_client_chooser.hpp \
				x_client.hpp \
				x_client_icon.hpp \
				x_client_name.hpp \
				x_client_thumbnail_gl.hpp \
				x_xft.hpp \
				x_xrm.hpp \
				x_connection.hpp \
				x_event_handler_t.hpp \
				x_event_source.hpp \
				x_event_source_t.hpp \
				x_ewmh.hpp

CPPOBJS=$(CPPSRCS:%.cpp=%.o)
HPPOBJS=$(HPPSRCS:%.hpp=%.hpp.gch)

EXE=x:choyce

all: ${HPPOBJS} ${CPPOBJS}
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${CPPOBJS} -o ${EXE}

shader/%.hpp.gch:
	make -C shader

%.o: %.cpp %.hpp
	${CXX} ${CXXFLAGS} -c $<

%.hpp.gch: %.hpp
	${CXX} ${CXXFLAGS} -c $<

version: ${LIBS}
	@echo -e "\nGCC version:"
	@gcc -v 2>&1

${LIBS}:
	@printf "%-20s" "$@:"
	@pkg-config --modversion $@

clean:
	make -C shader clean
	rm -f ${EXE} ${CPPOBJS} ${HPPOBJS}

.PHONY: clean version
