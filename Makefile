dev:
	cmake --build build
	make run

clean:
	rm -rf build

run:
	./build/Apop

fix_lsp:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B build -S .
