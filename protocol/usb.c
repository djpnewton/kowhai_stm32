/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2010 Gareth McMullin <gareth@blacksphere.co.nz>
 * Copyright (C) 2013 Daniel Newton <djpnewton@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/usb/hid.h>
#include "prot.h"

#define CDC_ACM

#define CDC_COMMS_IN_EP 0x85

#define CDC_DATA_SIZE 64
#define CDC_DATA_OUT_EP 0x04
#define CDC_DATA_IN_EP  0x83

#define HID_REPORTSIZE 64
#define HID_OUT_EP 0x01
#define HID_IN_EP  0x82

const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
        .bDeviceClass = 0xEF,	/* Miscellaneous Device */
        .bDeviceSubClass = 2,	/* Common Class */
        .bDeviceProtocol = 1,	/* Interface Association */
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
	0x95, HID_REPORTSIZE,    // report count
	0x09, 0x01,              // usage
	0x81, 0x02,              // Input (array)
	0x95, HID_REPORTSIZE,    // report count
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

#ifdef CDC_ACM
/*
 * This notification endpoint isn't implemented. According to CDC spec its
 * optional, but its absence causes a NULL pointer dereference in Linux
 * cdc_acm driver.
 */
static const struct usb_endpoint_descriptor cdc_comm_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = CDC_COMMS_IN_EP,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = 16,
	.bInterval = 255,
}};

static const struct usb_endpoint_descriptor cdc_data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = CDC_DATA_OUT_EP,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDC_DATA_SIZE,
	.bInterval = 1,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = CDC_DATA_IN_EP,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = CDC_DATA_SIZE,
	.bInterval = 1,
}};
#endif

static const struct usb_endpoint_descriptor hid_endpoint[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = HID_OUT_EP,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = HID_REPORTSIZE,
	.bInterval = 0x20,
}, {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = HID_IN_EP,
	.bmAttributes = USB_ENDPOINT_ATTR_INTERRUPT,
	.wMaxPacketSize = HID_REPORTSIZE,
	.bInterval = 0x20,
}};

#ifdef CDC_ACM
static const struct {
	struct usb_cdc_header_descriptor header;
	struct usb_cdc_call_management_descriptor call_mgmt;
	struct usb_cdc_acm_descriptor acm;
	struct usb_cdc_union_descriptor cdc_union;
} __attribute__((packed)) cdcacm_functional_descriptors = {
	.header = {
		.bFunctionLength = sizeof(struct usb_cdc_header_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_HEADER,
		.bcdCDC = 0x0110,
	},
	.call_mgmt = {
		.bFunctionLength =
			sizeof(struct usb_cdc_call_management_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_CALL_MANAGEMENT,
		.bmCapabilities = 0,
		.bDataInterface = 1,
	},
	.acm = {
		.bFunctionLength = sizeof(struct usb_cdc_acm_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_ACM,
		.bmCapabilities = 0,
	},
	.cdc_union = {
		.bFunctionLength = sizeof(struct usb_cdc_union_descriptor),
		.bDescriptorType = CS_INTERFACE,
		.bDescriptorSubtype = USB_CDC_TYPE_UNION,
		.bControlInterface = 0,
		.bSubordinateInterface0 = 1,
	 },
};

static const struct usb_interface_descriptor cdc_comm_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 1,
	.bInterfaceClass = USB_CLASS_CDC,
	.bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
	.bInterfaceProtocol = USB_CDC_PROTOCOL_AT,
	.iInterface = 4,

	.endpoint = cdc_comm_endp,

	.extra = &cdcacm_functional_descriptors,
	.extralen = sizeof(cdcacm_functional_descriptors),
}};

static const struct usb_interface_descriptor cdc_data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_DATA,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = cdc_data_endp,
}};
#endif

const struct usb_interface_descriptor hid_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
#ifdef CDC_ACM
	.bInterfaceNumber = 2,
#else
	.bInterfaceNumber = 0,
#endif
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_HID,
	.bInterfaceSubClass = 0, // no boot interface
	.bInterfaceProtocol = 0,
	.iInterface = 0,

	.endpoint = hid_endpoint,

	.extra = &hid_function,
	.extralen = sizeof(hid_function),
}};

#ifdef CDC_ACM
static const struct usb_iface_assoc_descriptor cdc_assoc = {
	.bLength = USB_DT_INTERFACE_ASSOCIATION_SIZE,
	.bDescriptorType = USB_DT_INTERFACE_ASSOCIATION,
	.bFirstInterface = 0,
	.bInterfaceCount = 2,
	.bFunctionClass = USB_CLASS_CDC,
	.bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
	.bFunctionProtocol = USB_CDC_PROTOCOL_AT,
	.iFunction = 0,
};
#endif

const struct usb_interface ifaces[] = {
#ifdef CDC_ACM
{
	.num_altsetting = 1,
	.iface_assoc = &cdc_assoc,
	.altsetting = cdc_comm_iface,
}, {
	.num_altsetting = 1,
	.altsetting = cdc_data_iface,
},
#endif
{
	.num_altsetting = 1,
	.altsetting = hid_iface,
}};

const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
#ifdef CDC_ACM
	.bNumInterfaces = 3,
#else
	.bNumInterfaces = 1,
#endif
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0xC0,
	.bMaxPower = 0x32,

	.interface = ifaces,
};

static const char *usb_strings[] = {
	"DJPSoft",
	"Kowhai Demo",
	"DEMO",
	"Kowhai Serial Demo"
};

/* Buffer to be used for control requests. */
uint8_t usbd_control_buffer[128];

#ifdef CDC_ACM
static int cdc_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)usbd_dev;

	switch (req->bRequest)
	{
		case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
		{
			/*
			 * This Linux cdc_acm driver requires this to be implemented
			 * even though it's optional in the CDC spec, and we don't
			 * advertise it in the ACM functional descriptor.
			 */
			char local_buf[10];
			struct usb_cdc_notification *notif = (void *)local_buf;

			/* We echo signals back to host as notification. */
			notif->bmRequestType = 0xA1;
			notif->bNotification = USB_CDC_NOTIFY_SERIAL_STATE;
			notif->wValue = 0;
			notif->wIndex = 0;
			notif->wLength = 2;
			local_buf[8] = req->wValue & 3;
			local_buf[9] = 0;
			// usbd_ep_write_packet(0x83, buf, 10);
			return 1;
		}
		case USB_CDC_REQ_SET_LINE_CODING:
			if (*len < sizeof(struct usb_cdc_line_coding))
				return 0;
			return 1;
	}
	return USBD_REQ_NOTSUPP;
}
#endif

static int hid_control_request(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)complete;
	(void)usbd_dev;

	if (req->bRequest != USB_REQ_GET_DESCRIPTOR ||
		req->bmRequestType != (USB_REQ_TYPE_IN | USB_REQ_TYPE_INTERFACE) ||
		req->wValue != USB_DT_REPORT << 8)
		return USBD_REQ_NOTSUPP;

	/* Handle the HID report descriptor. */
	*buf = (uint8_t *)hid_report_descriptor;
	*len = sizeof(hid_report_descriptor);
	return USBD_REQ_HANDLED;
}

#ifdef CDC_ACM
static void cdcacm_data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
	(void)usbd_dev;

	char buf[CDC_DATA_SIZE];
	int len = usbd_ep_read_packet(usbd_dev, CDC_DATA_OUT_EP, buf, CDC_DATA_SIZE);

	if (len)
	{
		usbd_ep_write_packet(usbd_dev, CDC_DATA_IN_EP, buf, len);
	}
}
#endif

static void hid_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	uint8_t buf[HID_REPORTSIZE] = {};

	if (usbd_ep_read_packet(usbd_dev, HID_OUT_EP, buf, HID_REPORTSIZE) > 0)
	{
		prot_process_packet(buf, HID_REPORTSIZE);
	}
}

static void set_config(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	(void)usbd_dev;

#ifdef CDC_ACM
	usbd_ep_setup(usbd_dev, CDC_COMMS_IN_EP, USB_ENDPOINT_ATTR_INTERRUPT, 16, NULL);
	usbd_ep_setup(usbd_dev, CDC_DATA_OUT_EP, USB_ENDPOINT_ATTR_BULK, CDC_DATA_SIZE, cdcacm_data_rx_cb);
	usbd_ep_setup(usbd_dev, CDC_DATA_IN_EP, USB_ENDPOINT_ATTR_BULK, CDC_DATA_SIZE, NULL);
#endif

	usbd_ep_setup(usbd_dev, HID_OUT_EP, USB_ENDPOINT_ATTR_INTERRUPT, HID_REPORTSIZE, hid_rx_cb);
	usbd_ep_setup(usbd_dev, HID_IN_EP, USB_ENDPOINT_ATTR_INTERRUPT, HID_REPORTSIZE, NULL);

#ifdef CDC_ACM
	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_CLASS | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				cdc_control_request);
#endif

	usbd_register_control_callback(
				usbd_dev,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				hid_control_request);
}

static usbd_device *usbd_dev;

static void prot_send_buffer(void* buffer, size_t buffer_size)
{
	uint8_t buf[HID_REPORTSIZE] = {};
	int size = HID_REPORTSIZE;
	if (buffer_size < size)
		size = buffer_size;
	memcpy(buf, buffer, size);
	while (usbd_ep_write_packet(usbd_dev, HID_IN_EP, buf, HID_REPORTSIZE) == 0);
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
	usbd_dev = usbd_init(driver, &dev, &config, usb_strings, 4, usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, set_config);

	for (i = 0; i < 0x80000; i++)
		__asm__("nop");

	prot_init(prot_send_buffer);
}

void usb_poll_forever(void)
{
	while (1)
		usbd_poll(usbd_dev);
}

