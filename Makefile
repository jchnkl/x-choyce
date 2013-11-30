# http://lackof.org/taggart/hacking/make-example/

PREFIX=/usr
SHADER_PATH=${PREFIX}/share/x:choyce/shaders

DIRS = src

%:
	TARGET=$@ ${MAKE} ${DIRS}

${DIRS}:
	${MAKE} -C $@ ${TARGET} SHADER_PATH=${SHADER_PATH}

install: all
	install -D -m 644 src/normal.frag ${SHADER_PATH}/normal.frag
	install -D -m 644 src/grayscale.frag ${SHADER_PATH}/grayscale.frag
	install -D src/x:choyce ${PREFIX}/bin/x:choyce

.PHONY: ${DIRS}
