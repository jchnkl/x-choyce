all: shader.hpp

shader.hpp: stringifier position.vert focused.frag unfocused.frag
	../aux/stringifier \
		position_src position.vert \
		focused_src focused.frag \
		unfocused_src unfocused.frag \
		> shader.hpp

stringifier:
	make -C ../aux

clean:
	rm -f shader.hpp

.PHONY: clean
