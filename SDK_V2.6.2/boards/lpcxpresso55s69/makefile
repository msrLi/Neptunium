BUILD_LIST_DIRS = driver_examples/hashcrypt/cm33_core0/armgcc
BUILD_LIST_DIRS += driver_examples/sdcard/interrupt/cm33_core0/armgcc
BUILD_LIST_DIRS += driver_examples/sdcard/polling/cm33_core0/armgcc
BUILD_LIST_DIRS += driver_examples/prince/cm33_core0/armgcc 
BUILD_LIST_DIRS += driver_examples/flashiap/cm33_core0/armgcc
BUILD_LIST_DIRS += usb_examples/usb_device_msc_ramdisk/bm/cm33_core0/armgcc
BUILD_LIST_DIRS += usb_examples/usb_device_msc_ramdisk/freertos/cm33_core0/armgcc
BUILD_LIST_DIRS += usb_examples/usb_device_msc_sdcard_lite/bm/cm33_core0/armgcc
BUILD_LIST_DIRS += usb_examples/usb_device_composite_cdc_msc_sdcard/bm/cm33_core0/armgcc
BUILD_LIST_DIRS += usb_examples/usb_device_composite_cdc_msc_sdcard/freertos/cm33_core0/armgcc
# BUILD_LIST_DIRS += 

all:
	for i in ${BUILD_LIST_DIRS};do \
		echo "Build $$i...."; \
		pushd $$i; \
		./build_all.sh;\
		popd; \
	done

clean:
	for i in ${BUILD_LIST_DIRS};do \
		echo "Build $$i....";  \
		pushd $$i; \
		./clean.sh;\
		popd; \
	done
