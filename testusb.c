#include <libusb-1.0/libusb.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

FILE *debugout, *verbout;

struct libusb_device_handle *ch341configure(uint16_t vid, uint16_t pid) {
	
	struct libusb_device *dev;
	struct libusb_device_handle *devHandle;
	int32_t ret = 0;
	uint32_t currentConfig = 0;

	uint8_t ch341DescriptorBuffer[0x12];

	ret = libusb_init(NULL);
	if(ret < 0) {
		fprintf(stderr, "Couldnt initialise libusb\n");
		return NULL;
	}

#if LIBUSB_API_VERSION < 0x01000106
	libusb_set_debug(NULL, 3);
#else
	libusb_set_option(NULL, LIBUSB_OPTION_LOG_LEVEL, 3);
#endif

	fprintf(verbout, "Searching USB buses for [%04x:%04x]\n",
			USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

	if (!(devHandle = libusb_open_device_with_vid_pid(NULL,
					USB_LOCK_VENDOR, USB_LOCK_PRODUCT))) {
		fprintf(stderr, "Couldn't open device [%04x:%04x]\n",
				USB_LOCK_VENDOR, USB_LOCK_PRODUCT);
		return NULL;
	}

	if (!(dev = libusb_get_device(devHandle))) {
		fprintf(stderr, "Couldn't get bus number and address of device\n");
		return NULL;
	}

	fprintf(verbout, "Found [%04x:%04x] as device [%d] on USB bus [%d]\n",
			USB_LOCK_VENDOR, USB_LOCK_PRODUCT,
			libusb_get_device_address(dev), libusb_get_bus_number(dev));
	fprintf(verbout, "Open device [%04x:%04x]\n", 
			USB_LOCK_VENDOR, USB_LOCK_PRODUCT);

	if (libusb_kernel_driver_active(devHandle, DEFAULT_INTERFACE)) {
		ret = libusb_detach_kernel_driver(devHandle, DEFAULT_INTERFACE);
		if (ret) {
			fprintf(stderr, "Failed to detach kernel driver: '%s'\n",
					stderror(-ret));
			return NULL;
		} else {
			fprintf(verbout, "Detached kernel driver\n");
		}
	}

	ret = libusb_get_configuration(devHandle, &currentConfig);
	if (ret) {
		fprintf(stderr, "Failed to get current device configuration: '%s'\n",
				stderror(-ret));
		return NULL;
	}

	if (currentConfig != DEFAULT_CONFIGURATION) {
		ret = libusb_set_configuration(devHandle, currentConfig);
	}

	if (ret) {
		fprintf(stderr, "Failed to set device configuration to %d: '%s'\n",
				DEFAULT_CONFIGURATION, stderror(-ret));
		return NULL;
	}

	ret = libusb_claim_interface(devHandle, DEFAULT_INTERFACE);

	if (ret) {
		fprintf(stderr, "Failed to claim interface %d: '%s'\n",
				DEFAULT_INTERFACE, stderror(-ret));
		return NULL;
	}

	fprintf(verbout, "Claim device interface [%d]\n", DEFAULT_INTERFACE);

	ret = libusb_get_descriptor(devHandle, LIBUSB_DT_DEVICE,
			0x00, ch341DEscriptorBuffer, 0x12);

	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor: '%s'\n", 
				strerror(-ret));
		return NULL;
	}
}

int main(int argc, char **argv) {
	
	debugout = (debug == TRUE) ? stdout : fopen("/dev/null", "w");
	verbout = (verbout == TRUE) ? stdout : fopen("/dev/null", "w");
	fprintf(debugout, "Debug Enabled\n");



}
