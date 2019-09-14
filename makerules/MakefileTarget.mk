# (c) Itarge.Inc

-include makerules/Rules.make
-include Rules.make

all: build
clean: build_clean

help:
	@echo  "uboot:          uboot"
	@echo  "lsp:            lsp"
	@echo  "uboot_clean:    clean uboot"
	@echo  "lsp_clean:      clean lsp"

build:
	@echo "runing stm32f407-ejd build..."
	pushd rt-thread/bsp/stm32/stm32f407-ejd;export PATH=${PLATFORM_PATH}/utils/env/tools/scripts:${PATH}; \
		pkgs --update;scons -j8 --verbose; popd;

	

build_clean:
	pushd rt-thread/bsp/stm32/stm32f407-ejd; scons -c; popd;
