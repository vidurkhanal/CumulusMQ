BUILD_DIR = build
EXECUTABLE = CumulusMQ
ZIG_OUT_DIR = zig-out/bin

cmake_build:
	cmake --build $(BUILD_DIR)

cmake_clean:
	rm -rf ${BUILD_DIR}

cmake_run:
	./$(BUILD_DIR)/$(EXECUTABLE)

cmake_gen:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B ${BUILD_DIR} -S .

zig_debug:
	zig build -Doptimize=Debug --summary all

zig_release:
	zig build --release=safe --summary all

zig_run:
	./$(ZIG_OUT_DIR)/$(EXECUTABLE)

