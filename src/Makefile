LIBS=xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama \
		 xcb-xfixes x11 x11-xcb xrender gl xcb-ewmh xcb-icccm xcb-image xft
CXXFLAGS=-g -std=c++11 -Wall -O3 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

ifneq (${SHADER_PATH},)
	CXXFLAGS+=-DSHADER_PATH='"${SHADER_PATH}"'
endif

CPPSRCS=main.cpp \
				data_types.cpp \
				cyclic_iterator.cpp \
				thumbnail_manager.cpp \
				x_xft.cpp \
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

HPPSRCS=chooser_t.hpp \
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

%.o: %.cpp %.hpp
	${CXX} ${CXXFLAGS} -c $<

%.hpp.gch: %.hpp
	${CXX} ${CXXFLAGS} -c $<

version: ${LIBS}

${LIBS}:
	@printf "%-20s" "$@:"
	@pkg-config --modversion $@

clean:
	rm -f ${EXE} ${CPPOBJS} ${HPPOBJS}

.PHONY: clean version
