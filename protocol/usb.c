#include <stdlib.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include "prot.h"

#define HID_OUT_REPORTSIZE 64
#define HID_OUT_EP 0x02
#define HID_IN_REPORTSIZE 64
#define HID_IN_EP 0x81

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0xffff,
	.idProduct = 0x9fab,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

static const uint8_t hid_report_descriptor[] = {
	0x06, 0xFF, 0x00,        // usage page
	0x0A, 0x01, 0x00,        // usage
	0xA1, 0x01,              // Collection 0x01
	0x75, 0x08,              // report size = 8 bits
	0x15, 0x00,              // logical minimum = 0
	0x26, 0xFF, 0x00,        // logical maximum = 255
	0x95, HID_OUT_REPORTSIZE,// report count
	0x09, 0x01,              // usage
	0x81, 0x02,              // Input (array)
	0x95, HID_IN_REPORTSIZE, // report count
	0x09, 0x02,              // usage
	0x91, 0x02,              // Output (array)
	0xC0                     // end collection 
};

static const struct {
	struct usb_hid_descriptor hid_descriptor;
	struct {
		uint8_t bReportDescriptorType;
		uint16_t wDescriptorLength;
	} __attribute__((packed)) hid_report;
} __attribute__((packed)) hid_function = {
	.hid_descriptor = {
		.bLength = sizeof(hid_function),
		.bDescriptorType = USB_DT_HID,
		.bcdHID = 0x0100,
		.bCountryCode = 0,
		.bNumDescriptors = 1,
	},
	.hid_report = {
		.bReportDescriptorType = USB_DT_REPORT,
		.wDescriptorLength = sizeof(hid_report_descriptor),
	},
};

const struct usb_endpoint_descriptor hid_endpoint[2] = {
	{
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = HID_IN_EP,
		.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
		.wMaxPacketSize = HID_IN_REPORTSIZE,
		.bInterval = 0x20,
	},
	{
		.bLength = USB_DT_ENDPOINT_SIZE,
		.bDescriptorType = USB_DT_ENDPOINT,
		.bEndpointAddress = HID_OUT_EP,
		.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
		.wMaxPacketSize = HID_OUT_REPORTSIZE,
		.bInterval = 0x20,
	}
};

const struct usb_interface_descriptor hid_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 0, // no boot interface
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = hid_endpoint,

	.extra = &hid_function,
	.extralen = sizeof(hid_function),
};

const struct usb_interface ifaces[] = {{
	.num_altsetting = 1,
	.altsetting = &hid_iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"DJPSoft",
	"HID Kowhai Demo",
	"DEMO",
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

static int hid_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)usbd_dev;

	if ((req->bmRequestType != (USB_REQ_TYPE_IN | USB_REQ_TYPE_INTERFACE)) ||
	   (req->bRequest != USB_REQ_GET_DESCRIPTOR) ||
	   (req->wValue != USB_DT_REPORT << 8))
		return USBD_REQ_NOTSUPP;

	/* Handle the HID report descriptor. */
	*buf = (uint8_t *)hid_report_descriptor;
	*len = sizeof(hid_report_descriptor);

	return USBD_REQ_HANDLED;
}

static void hid_rx(usbd_device *usbd_dev, uint8_t ep)
{
	uint8_t buf[HID_OUT_REPORTSIZE] = {};

	if (usbd_ep_read_packet(usbd_dev, HID_OUT_EP, buf, HID_OUT_REPORTSIZE) > 0)
	{
		prot_process_packet(buf, HID_OUT_REPORTSIZE);
	}	
}

static void hid_set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

	usbd_ep_setup(usbd_dev, HID_IN_EP, USB_ENDPOINT_ATTR_INTERRUPT, HID_IN_REPORTSIZE, NULL);
	usbd_ep_setup(usbd_dev, HID_OUT_EP, USB_ENDPOINT_ATTR_INTERRUPT, HID_OUT_REPORTSIZE, hid_rx);

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				hid_control_request);
}

static usbd_device *usbd_dev;

static void prot_send_buffer(void* buffer, size_t buffer_size)
{
	uint8_t buf[HID_IN_REPORTSIZE] = {};
	int size = HID_IN_REPORTSIZE;
	if (buffer_size < size)
		size = buffer_size;
	memcpy(buf, buffer, size);
	while (usbd_ep_write_packet(usbd_dev, HID_IN_EP, buf, HID_IN_REPORTSIZE) == 0);
}

void usb_init(void)
{
	int i;

#ifdef STM32F1
	const usbd_driver* driver = &stm32f103_usb_driver;
#elif STM32F4
	const usbd_driver* driver = &otgfs_usb_driver;
#else
ERROR
#endif
	usbd_dev = usbd_init(driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, hid_set_config);

	for (i = 0; i < 0x80000; i++)
		__asm__("nop");

	prot_init(prot_send_buffer);
}

void usb_poll_forever(void)
{
	while (1)
		usbd_poll(usbd_dev);
}

