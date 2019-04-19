default: run

run:
	[ -d build ] || mkdir build
	(cd build; cmake -DCMAKE_BUILD_TYPE=Release -DGTEST=FALSE ..; make -j8)
	build/bin/minecraft

.PHONY: test
test:
	[ -d build ] || mkdir build
	(cd build; cmake -DGTEST=TRUE ..; make -j8)
	build/bin/test

speed:
	[ -d build ] || mkdir build
	(cd build; cmake -DCMAKE_BUILD_TYPE=Release -DGTEST=TRUE ..; make -j8)
	build/bin/test

gdb: build
	gdb -q -ex run --args build/bin/minecraft

clean:
	rm -rf build