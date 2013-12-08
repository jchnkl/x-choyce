# http://lackof.org/taggart/hacking/make-example/

PREFIX=/usr

DIRS = src

%:
	TARGET=$@ ${MAKE} ${DIRS}

${DIRS}:
	${MAKE} -C $@ ${TARGET}

install: all
	install -D src/x:choyce ${PREFIX}/bin/x:choyce

.PHONY: ${DIRS}
