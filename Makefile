# http://lackof.org/taggart/hacking/make-example/

VERSION=$(shell date +%Y.%m.%d)

PREFIX=/usr

DIRS = src

%:
	TARGET=$@ ${MAKE} ${DIRS}

${DIRS}:
	${MAKE} -C $@ ${TARGET}

install: all
	install -D src/x:choyce ${PREFIX}/bin/x:choyce

release:
	@ echo "Updating version in README"
	@ sed -i '/### Version ###/{n;s/.*/${VERSION}/g;}' README.md

	@ echo "Commit new version"
	@ git commit -a -m "Bump to ${VERSION}"

	@ echo "Creating tag for ${VERSION}"
	@ git tag ${VERSION}

	@ echo "Pushing tags"
	@ git push --tags

	make -C aur package VERSION=${VERSION}

.PHONY: release ${DIRS}
