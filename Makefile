all:
	mkdir -p build
	cd build && cmake .. && make || cd ..

clean:
	rm -rf build

test:
	make -C build test
