all:
	mkdir -p build
	cd build && cmake .. && make || cd ..

clean:
	rm -rf build

test:
	make -C build test

tags:
	mkdir -p o
	@echo Making tags...
	/usr/bin/find . src -name '*.c' -o -name '*.cpp' -o -name '*.h' | grep -v "moc_" | grep -v "ui_" | grep -v "/o/"> o/flist && \
	ctags --file-tags=yes --language-force=C++ -L o/flist
	@echo tags complete.

# tags on mac
mtags:
	mkdir -p o
	@echo Making tags...
	/usr/bin/find . src -name '*.c' -o -name '*.cpp' -o -name '*.h' | grep -v "moc_" | grep -v "ui_" | grep -v "/o/"> o/flist && \
	/usr/local/bin/ctags --file-tags=yes --language-force=C++ -L o/flist
	@echo tags complete.
