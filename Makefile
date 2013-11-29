# http://lackof.org/taggart/hacking/make-example/

DIRS = src

%:
	TARGET=$@ ${MAKE} ${DIRS}

${DIRS}:
	${MAKE} -C $@ ${TARGET}

.PHONY: ${DIRS}
