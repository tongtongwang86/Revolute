#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>

static struct k_work report_send;
static struct k_work report_clear;

static const struct device *hdev;
static const uint8_t hid_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

static const struct hid_ops ops = {
	
};

static void send_report(struct k_work *work)
{
	uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	rep[7] = HID_KEY_Z;
	hid_int_ep_write(hdev,rep,sizeof(rep), NULL);
}

static void clear_report(struct k_work *work)
{
	uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//rep[7] = HID_KEY_Z;
	hid_int_ep_write(hdev,rep,sizeof(rep), NULL);
}

int main(void)
{

	hdev = device_get_binding("HID_0");
	usb_hid_register_device(hdev, hid_kbd_report_desc,sizeof(hid_kbd_report_desc), &ops);

	k_work_init(&report_send, send_report);
	k_work_init(&report_clear, clear_report);

	while (1) {
	
	k_sleep(K_SECONDS(1));
	k_work_submit(&report_send);
	k_sleep(K_SECONDS(1));
	k_work_submit(&report_clear);


	}
}