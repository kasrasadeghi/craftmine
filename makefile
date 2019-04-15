run:
	[ -d build ] || mkdir build
	(cd build; cmake ..; make -j8)
	build/bin/minecraft

gdb: build
	gdb -q -ex run --args build/bin/minecraft