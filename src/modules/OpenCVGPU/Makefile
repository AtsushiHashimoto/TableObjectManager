all:
	cd src/cuda;$(MAKE)
	cd src;$(MAKE)
	mkdir -p ../../lib
	cp src/libskl_opencvgpu.a ../../lib/

.PHONY: clean debug

check-syntax:
	g++ -o null -Wall -S ${CHK_SOURCES}

debug:
	cd src/cuda;$(MAKE) debug
	cd src;$(MAKE) debug
	mkdir -p ../../lib
	cp src/libskl_opencvgpu_d.a ../../lib/

clean:
	cd src;$(MAKE) clean
	cd src/cuda;$(MAKE) clean
