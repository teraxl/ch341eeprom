#include <libusb-1.0/libusb.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define USB_LOGITEC_VID 0x046d
#define USB_LOGITEC_PID 0x0a45

#define USB_TOSHIBA_VID 0x0930
#define USB_TOSHIBA_PID 0x6545

#define DEFAULT_INTERFACE 0x00
#define DEFAULT_CONFIGURATION 0x01

struct libusb_device_handle *ch341configure(uint16_t vid, uint16_t pid) {
	
	int i;
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

	fprintf(stdout, "Searching USB buses for [%04x:%04x]\n", vid, pid);

	if (!(devHandle = libusb_open_device_with_vid_pid(NULL, vid, pid))) {
		fprintf(stderr, "Couldn't open device [%04x:%04x]\n", vid, pid);
		return NULL;
	}

	if (!(dev = libusb_get_device(devHandle))) {
		fprintf(stderr, "Couldn't get bus number and address of device\n");
		return NULL;
	}

	fprintf(stdout, "Found [%04x:%04x] as device [%d] on USB bus [%d]\n", vid, pid,
			libusb_get_device_address(dev), libusb_get_bus_number(dev));
	fprintf(stdout, "Open device [%04x:%04x]\n", vid, pid);

	if (libusb_kernel_driver_active(devHandle, DEFAULT_INTERFACE)) {
		ret = libusb_detach_kernel_driver(devHandle, DEFAULT_INTERFACE);
		if (ret) {
			fprintf(stderr, "Failed to detach kernel driver: '%s'\n",
					strerror(-ret));
			return NULL;
		} else {
			fprintf(stdout, "Detached kernel driver\n");
		}
	}

	ret = libusb_get_configuration(devHandle, &currentConfig);
	if (ret) {
		fprintf(stderr, "Failed to get current device configuration: '%s'\n",
				strerror(-ret));
		return NULL;
	}

	if (currentConfig != DEFAULT_CONFIGURATION) {
		ret = libusb_set_configuration(devHandle, currentConfig);
	}

	if (ret) {
		fprintf(stderr, "Failed to set device configuration to %d: '%s'\n",
				DEFAULT_CONFIGURATION, strerror(-ret));
		return NULL;
	}

	ret = libusb_claim_interface(devHandle, DEFAULT_INTERFACE);

	if (ret) {
		fprintf(stderr, "Failed to claim interface %d: '%s'\n",
				DEFAULT_INTERFACE, strerror(-ret));
		return NULL;
	}

	fprintf(stdout, "Claim device interface [%d]\n", DEFAULT_INTERFACE);

	ret = libusb_get_descriptor(devHandle, LIBUSB_DT_DEVICE,
			0x00, ch341DescriptorBuffer, 0x12);

	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor: '%s'\n", 
				strerror(-ret));
		return NULL;
	}

	fprintf(stdout, "Device reported its revision [%d.%02d]\n",
			ch341DescriptorBuffer[12], ch341DescriptorBuffer[13]);
	
	for (i=0; i < 0x12; i++) {
		fprintf(stdout, "%02x ", ch341DescriptorBuffer[i]);	
	}
	
	fprintf(stdout, "\n");

	return devHandle;
}

int main(int argc, char **argv) {
	
	struct libusb_device_handle *devHandle = NULL;
	struct libusb_device *dev = NULL;
	devHandle = ch341configure(USB_LOGITEC_VID, USB_LOGITEC_PID);
	dev = libusb_get_device(devHandle);
	uint8_t address = libusb_get_device_address(dev);
	int device_speed = libusb_get_device_speed(dev);
	fprintf(stdout, "Address: %d\n", address);
	fprintf(stdout, "Speed: %d\n", device_speed);
	
	libusb_release_interface(devHandle, DEFAULT_INTERFACE);
	libusb_close(devHandle);
	libusb_exit(NULL);
}
