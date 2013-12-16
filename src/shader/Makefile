all: shader.hpp

shader.hpp: stringifier position.vert focused.frag unfocused.frag
	./stringifier \
		position_src position.vert \
		focused_src focused.frag \
		unfocused_src unfocused.frag \
		> shader.hpp

stringifier: stringifier.cpp
	g++ -Wall stringifier.cpp -o stringifier

clean:
	rm -f stringifier shader.hpp

.PHONY: clean
