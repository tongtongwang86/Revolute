#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

static struct k_work report_send;
static struct k_work report_clear;
static bool configured;
static const struct device *hdev;
static const uint8_t hid_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

static ATOMIC_DEFINE(hid_ep_in_busy, 1);
#define HID_EP_BUSY_FLAG	0





static void send_report(struct k_work *work)
{

	if (!atomic_test_and_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {

		uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		rep[7] = HID_KEY_Z;
		int ret = hid_int_ep_write(hdev,rep,sizeof(rep), NULL);

		
		if (ret != 0) {
			/*
			 * Do nothing and wait until host has reset the device
			 * and hid_ep_in_busy is cleared.
			 */
			printk("Failed to submit report");
		} else {
			printk("Report submitted");
		}
	} else {
		printk("HID IN endpoint busy");
	}

}

static void clear_report(struct k_work *work)
{

	if (!atomic_test_and_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {

		uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		//rep[7] = HID_KEY_Z;
		int ret = hid_int_ep_write(hdev,rep,sizeof(rep), NULL);

		
		if (ret != 0) {
			/*
			 * Do nothing and wait until host has reset the device
			 * and hid_ep_in_busy is cleared.
			 */
			printk("Failed to submit report");
		} else {
			printk("Report submitted");
		}
	} else {
		printk("HID IN endpoint busy");
	}
	
}

static void int_in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);
	if (!atomic_test_and_clear_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG)) {
		printk("IN endpoint callback without preceding buffer write");
	}
}

static const struct hid_ops ops = {
	.int_in_ready = int_in_ready_cb,
};

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	switch (status) {
	case USB_DC_RESET:
		configured = false;
		break;
	case USB_DC_CONFIGURED:
		if (!configured) {
			int_in_ready_cb(hdev);
			configured = true;
		}
		break;
	case USB_DC_SOF:
		break;
	default:
		printk("status %u unhandled", status);
		break;
	}
}

int main(void)
{

	// hdev = device_get_binding("HID_0");
	// usb_hid_register_device(hdev, hid_kbd_report_desc,sizeof(hid_kbd_report_desc), &ops);

	int ret = usb_enable(status_cb);

	if (ret != 0) {
		printk("Failed to enable USB");
		return 0;
	}

	//usb_hid_init(hdev);
	k_work_init(&report_send, send_report);
	k_work_init(&report_clear, clear_report);

	while (1) {
	

	// uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	// rep[7] = HID_KEY_Z;
	// hid_int_ep_write(hdev,rep,sizeof(rep), NULL);
	k_sleep(K_SECONDS(1));
	k_work_submit(&report_send);
	k_sleep(K_SECONDS(1));
	k_work_submit(&report_clear);


	}
}

static int composite_pre_init(void)
{
	hdev = device_get_binding("HID_0");
	if (hdev == NULL) {
		printk("Cannot get USB HID Device");
		return -ENODEV;
	}

	printk("HID Device: dev %p", hdev);

	usb_hid_register_device(hdev, hid_kbd_report_desc,sizeof(hid_kbd_report_desc), &ops);


	atomic_set_bit(hid_ep_in_busy, HID_EP_BUSY_FLAG);


	if (usb_hid_set_proto_code(hdev, HID_BOOT_IFACE_CODE_NONE)) {
		printk("Failed to set Protocol Code");
	}

	return usb_hid_init(hdev);
}



SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);