BUILD_DIR = build
EXECUTABLE = CumulusMQ

build_executable:
	cmake --build $(BUILD_DIR)

clean:
	rm -rf ${BUILD_DIR}

run:
	./$(BUILD_DIR)/$(EXECUTABLE)

gen_build:
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -B ${BUILD_DIR} -S .
