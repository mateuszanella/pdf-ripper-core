# Simple developer entry points for pdf-core.

CMAKE ?= cmake
BUILD_DIR ?= build
BUILD_TYPE ?= Debug
GENERATOR ?=

CMAKE_CONFIGURE_ARGS := -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
ifneq ($(strip $(GENERATOR)),)
CMAKE_CONFIGURE_ARGS += -G "$(GENERATOR)"
endif

.PHONY: help configure build clean rebuild install

help:
	@echo "Available targets:"
	@echo "  make configure    - Configure CMake in $(BUILD_DIR)"
	@echo "  make build        - Configure and build library + examples"
	@echo "  make install      - Install from $(BUILD_DIR)"
	@echo "  make clean        - Remove $(BUILD_DIR)"
	@echo "  make rebuild      - Clean then build"
	@echo "Variables: BUILD_DIR, BUILD_TYPE, GENERATOR"

configure:
	@mkdir -p $(BUILD_DIR)
	$(CMAKE) $(CMAKE_CONFIGURE_ARGS)

build: configure
	$(CMAKE) --build $(BUILD_DIR) -j

install: build
	$(CMAKE) --install $(BUILD_DIR)

clean:
	$(CMAKE) -E rm -rf $(BUILD_DIR)

rebuild: clean build
