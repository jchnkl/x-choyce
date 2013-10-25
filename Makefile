LIBS=xcb xcb-atom xcb-keysyms xcb-composite xcb-damage xcb-xinerama \
		 xcb-renderutil xcb-xfixes x11 x11-xcb gl
CXXFLAGS=-g -std=c++11 -Wall $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})

CPPSRCS=main.cpp \
				data_types.cpp \
				cyclic_iterator.cpp \
				thumbnail_manager.cpp \
				x_connection.cpp \
				x_ewmh.cpp \
				x_client.cpp \
				x_client_chooser.cpp \
				x_client_thumbnail_gl.cpp \
				x_client_thumbnail_factory.cpp \
				grid.cpp

HPPSRCS=chooser_t.hpp \
				cyclic_iterator.hpp \
				data_types.hpp \
				grid.hpp \
				layout_t.hpp \
				thumbnail_manager.hpp \
				thumbnail_t.hpp \
				x_client_chooser.hpp \
				x_client.hpp \
				x_client_thumbnail_factory.hpp \
				x_client_thumbnail_gl.hpp \
				x_client_thumbnail.hpp \
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

%.hpp.gch: %.hpp
	rm -f $(<:%.hpp=%.o)
	${CXX} ${CXXFLAGS} -c $<

clean:
	rm -f ${EXE} ${CPPOBJS} ${HPPOBJS}

.PHONY: clean
