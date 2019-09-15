# (c) Itarge.Inc

-include makerules/Rules.make
-include Rules.make

all: sdk
clean: sdk_clean 
#build_clean

help:
	@echo  "uboot:          uboot"
	@echo  "lsp:            lsp"
	@echo  "uboot_clean:    clean uboot"
	@echo  "lsp_clean:      clean lsp"

sdk:
	@echo "do nothing"
	make -C SDK_V2.6.2/boards/lpcxpresso55s69;

sdk_clean:
	make -C SDK_V2.6.2/boards/lpcxpresso55s69 clean

exit:
	cd SDK_V2.6.2/boards/lpcxpresso55s69/usb_examples/usb_device_msc_sdcard/bm/cm33_core0/armgcc; \
		./build_all.sh;\
	cd -;

build:
	@echo "runing stm32f407-ejd build..."
	pushd rt-thread/bsp/stm32/stm32f407-ejd;export PATH=${PLATFORM_PATH}/utils/env/tools/scripts:${PATH}; \
		pkgs --update;scons -j8 --verbose; popd;

	

build_clean:
# pushd rt-thread/bsp/stm32/stm32f407-ejd; scons -c; popd;
