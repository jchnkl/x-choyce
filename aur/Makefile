# TMPDIR=/tmp/x-choyce-release-tmp-$(shell id -un)
# VERSION=$(shell date +%Y.%m.%d)
VERSION=
AURUSER=$(shell head -1 .login)
AURPASSWORD=$(shell head -2 .login | tail -1)
PACKAGE=x-choyce-${VERSION}-0.src.tar.gz

package:
	@ if [ -z "${VERSION}" ]; then echo "VERSION not set!"; exit 1; fi
	@ sed -e "s/VERSION/${VERSION}/g" PKGBUILD.in > PKGBUILD

	@ echo "Making source package"
	@ makepkg -f -g >> PKGBUILD
	@ makepkg -f --source

	@ echo "Login to AUR"
	@ curl -o /dev/null \
		-d "user=${AURUSER}&passwd=${AURPASSWORD}&submit=Login" \
		--dump-header .headers \
		'https://aur.archlinux.org/login/'

	@ echo "Upload package"
	@ curl -L -b .headers -D - -o /dev/null \
		-F "token=$$(GREP_OPTIONS= grep -o -P '(?<=AURSID=).*?(?=;)' .headers)" \
		-F "pkgsubmit=1" \
		-F "category=17" \
		-F "pfile=@${PACKAGE}" \
		'https://aur.archlinux.org/submit/'

	@ rm .headers

clean: VERSION=*
clean:
	rm -rf PKGBUILD ${PACKAGE}

.PHONY: clean package
