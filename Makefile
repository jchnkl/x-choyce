# http://lackof.org/taggart/hacking/make-example/

PREFIX=/usr
SHADER_PATH=${PREFIX}/share/x:choyce/shaders

DIRS = src

%:
	TARGET=$@ ${MAKE} ${DIRS}

${DIRS}:
	${MAKE} -C $@ ${TARGET} SHADER_PATH=${SHADER_PATH}

.PHONY: ${DIRS}
