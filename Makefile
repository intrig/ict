.PHONY: all tags perf

all: build
	cmake --build build

build:
	cmake -B $@ -S . -GNinja -DCMAKE_BUILD_TYPE=Release

debug:
	cmake -B $@ -S . -GNinja -DCMAKE_BUILD_TYPE=Debug
	cmake --build $@

clang:
	mkdir $@ && cd $@ && cmake -GNinja -DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_C_COMPILER=/usr/local/clang_9.0.0/bin/clang \
	-DCMAKE_CXX_COMPILER=/usr/local/clang_9.0.0/bin/clang++ ..

clean:
	cmake --build build --target clean

realclean:
	cmake -E remove_directory build
	cmake -E remove_directory debug

check: all
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
