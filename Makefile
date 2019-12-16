.PHONY: all tags perf

all: build
	cmake --build build

build:
	cmake -B $@ -S . -GNinja -DCMAKE_BUILD_TYPE=Release

debug:
	cmake -B $@ -S . -GNinja -DCMAKE_BUILD_TYPE=Debug
	cmake --build $@

clean:
	cmake --build $@ clean

realclean:
	cmake -E remove_directory build

check: build
	cd build && CTEST_OUTPUT_ON_FAILURE=1 ctest

perf: build
	build/perf/ictperf --ibits
	build/perf/ictperf --obits

tags:
	@echo Making tags...
	@$(RM) tags; find . -name '*.cpp' \
	-o -name '*.h' > flist && \
	ctags --file-tags=yes -L flist --totals && rm flist
	@echo tags complete.
