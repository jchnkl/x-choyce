default_application_icon_argb.c: default_application_icon_abgr.c abgr2argb
	./abgr2argb > default_application_icon_argb.c

abgr2argb:
	g++ -std=c++11 \
	     -DSOURCE=default_application_icon_abgr.c \
	     abgr2argb.cpp \
	     -o abgr2argb

clean:
	rm -f abgr2argb default_application_icon_argb.c

.PHONY: clean
