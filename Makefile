BUILD_DIR = build
EXECUTABLE = CumulusMQ
ZIG_OUT_DIR = zig-out/bin


build_executable:
	cmake --build $(BUILD_DIR)

clean:
	rm -rf ${BUILD_DIR}

run:
	./$(BUILD_DIR)/$(EXECUTABLE)

gen_build:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B ${BUILD_DIR} -S .

build_zig:
	zig build -Doptimize=Debug --summary all

run_zig:
	./$(ZIG_OUT_DIR)/$(EXECUTABLE)

