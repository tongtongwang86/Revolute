/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <string.h>
#include <zephyr/random/rand32.h>

#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/class/usb_hid.h>
#include <zephyr/usb/class/usb_cdc.h>

// #define LOG_LEVEL LOG_LEVEL_DBG
// LOG_MODULE_REGISTER(main);

// #define SW0_NODE DT_ALIAS(sw0)

// #if DT_NODE_HAS_STATUS(SW0_NODE, okay)
// static const struct gpio_dt_spec sw0_gpio = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
// #endif

// #define SW1_NODE DT_ALIAS(sw1)

// #if DT_NODE_HAS_STATUS(SW1_NODE, okay)
// static const struct gpio_dt_spec sw1_gpio = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
// #endif

// #define SW2_NODE DT_ALIAS(sw2)

// #if DT_NODE_HAS_STATUS(SW2_NODE, okay)
// static const struct gpio_dt_spec sw2_gpio = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
// #endif

// #define SW3_NODE DT_ALIAS(sw3)

// #if DT_NODE_HAS_STATUS(SW3_NODE, okay)
// static const struct gpio_dt_spec sw3_gpio = GPIO_DT_SPEC_GET(SW3_NODE, gpios);
// #endif

/* Event FIFO */

K_FIFO_DEFINE(evt_fifo);

enum evt_t {
	TRIGGER	= 0x00,
	CLEAR	= 0x01,
};

struct app_evt_t {
	sys_snode_t node;
	enum evt_t event_type;
};

#define FIFO_ELEM_MIN_SZ        sizeof(struct app_evt_t)
#define FIFO_ELEM_MAX_SZ        sizeof(struct app_evt_t)
#define FIFO_ELEM_COUNT         255
#define FIFO_ELEM_ALIGN         sizeof(unsigned int)

K_HEAP_DEFINE(event_elem_pool, FIFO_ELEM_MAX_SZ * FIFO_ELEM_COUNT + 256);

static inline void app_evt_free(struct app_evt_t *ev)
{
	k_heap_free(&event_elem_pool, ev);
}

static inline void app_evt_put(struct app_evt_t *ev)
{
	k_fifo_put(&evt_fifo, ev);
}

static inline struct app_evt_t *app_evt_get(void)
{
	return k_fifo_get(&evt_fifo, K_NO_WAIT);
}

static inline void app_evt_flush(void)
{
	struct app_evt_t *ev;

	do {
		ev = app_evt_get();
		if (ev) {
			app_evt_free(ev);
		}
	} while (ev != NULL);
}

static inline struct app_evt_t *app_evt_alloc(void)
{
	struct app_evt_t *ev;

	ev = k_heap_alloc(&event_elem_pool,
			  sizeof(struct app_evt_t),
			  K_NO_WAIT);
	if (ev == NULL) {
		LOG_ERR("APP event allocation failed!");
		app_evt_flush();

		ev = k_heap_alloc(&event_elem_pool,
				  sizeof(struct app_evt_t),
				  K_NO_WAIT);
		if (ev == NULL) {
			LOG_ERR("APP event memory corrupted.");
			__ASSERT_NO_MSG(0);
			return NULL;
		}
		return NULL;
	}

	return ev;
}

/* HID */


static const uint8_t hid_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

static K_SEM_DEFINE(evt_sem, 0, 1);	/* starts off "not available" */
static K_SEM_DEFINE(usb_sem, 1, 1);	/* starts off "available" */
static struct gpio_callback callback[4];

static char data_buf_kbd[64];
static char string[64];
static uint8_t chr_ptr_kbd, str_pointer;



static void in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);

	k_sem_give(&usb_sem);
}

static const struct hid_ops ops = {
	.int_in_ready = in_ready_cb,
};


static void clear_kbd_report(void)
{
	struct app_evt_t *new_evt = app_evt_alloc();

	new_evt->event_type = HID_KBD_CLEAR;
	app_evt_put(new_evt);
	k_sem_give(&evt_sem);
}




static void btn0(const struct device *gpio, struct gpio_callback *cb,
		 uint32_t pins)
{
	struct app_evt_t *ev = app_evt_alloc();

	ev->event_type = TRIGGER,
	app_evt_put(ev);
	k_sem_give(&evt_sem);
}





int callbacks_configure(const struct gpio_dt_spec *gpio,
			void (*handler)(const struct device *, struct gpio_callback*,
					uint32_t),
			struct gpio_callback *callback)
{
	if (!device_is_ready(gpio->port)) {
		LOG_ERR("%s: device not ready.", gpio->port->name);
		return -ENODEV;
	}

	gpio_pin_configure_dt(gpio, GPIO_INPUT);

	gpio_init_callback(callback, handler, BIT(gpio->pin));
	gpio_add_callback(gpio->port, callback);
	gpio_pin_interrupt_configure_dt(gpio, GPIO_INT_EDGE_TO_ACTIVE);

	return 0;
}

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	LOG_INF("Status %d", status);
}

#define DEVICE_AND_COMMA(node_id) DEVICE_DT_GET(node_id),

int main(void)
{
	const struct device *cdc_dev[] = {
		DT_FOREACH_STATUS_OKAY(zephyr_cdc_acm_uart, DEVICE_AND_COMMA)
	};
	BUILD_ASSERT(ARRAY_SIZE(cdc_dev) >= 2, "Not enough CDC ACM instances");
	const struct device *hid0_dev;
	struct app_evt_t *ev;
	uint32_t dtr = 0U;
	int ret;

	/* Configure devices */

	hid0_dev = device_get_binding("HID_0");
	if (hid0_dev == NULL) {
		LOG_ERR("Cannot get USB HID 0 Device");
		return 0;
	}


	if (callbacks_configure(&sw0_gpio, &btn0, &callback[0])) {
		LOG_ERR("Failed configuring button 0 callback.");
		return 0;
	}



	/* Initialize HID */


	usb_hid_register_device(hid0_dev, hid_kbd_report_desc,
				sizeof(hid_kbd_report_desc), &ops);

	usb_hid_init(hid0_dev);

	ret = usb_enable(status_cb);
	if (ret != 0) {
		LOG_ERR("Failed to enable USB");
		return 0;
	}

	/* Initialize CDC ACM */

	/* Wait 1 sec for the host to do all settings */
	k_busy_wait(USEC_PER_SEC);


	while (true) {
		k_sem_take(&evt_sem, K_FOREVER);

		while ((ev = app_evt_get()) != NULL) {
			switch (ev->event_type) {

			case TRIGGER:
			{
				/* Clear kbd report */
				uint8_t rep[] = {0x00, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};

				k_sem_take(&usb_sem, K_FOREVER);
				hid_int_ep_write(hid1_dev, rep, sizeof(rep), NULL);
				break;
			}

			case CLEAR:
			{
				/* Clear kbd report */
				uint8_t rep[] = {0x00, 0x00, 0x00, 0x00,
					      0x00, 0x00, 0x00, 0x00};

				k_sem_take(&usb_sem, K_FOREVER);
				hid_int_ep_write(hid1_dev, rep, sizeof(rep), NULL);
				break;
			}
			
			break;
			}
			app_evt_free(ev);
		}
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>


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

        k_sleep(K_MSEC(5));
        rep[7] = 0x00;
       // uint8_t rek[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		//rep[7] = HID_KEY_Z;
		ret = hid_int_ep_write(hdev,rep,sizeof(rep), NULL);

		
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

int as5600_refresh(const struct device *dev)
{
	int ret;
    struct sensor_value rot_raw;
    ret = sensor_sample_fetch_chan(dev,SENSOR_CHAN_ROTATION);
	if (ret != 0){
			printk("ono dis not good, ur err code is :,%d\n", ret);
		}
    sensor_channel_get(dev,SENSOR_CHAN_ROTATION, &rot_raw);
	

    
	// printk("%d",rot_raw.val1)
    return rot_raw.val1;
}



int main(void)
{

const struct device *const as = DEVICE_DT_GET(DT_INST(0,ams_as5600));



     int ret = usb_enable(status_cb);

	if (ret != 0) {
		printk("Failed to enable USB");
		return 0;
	}
	//usb_hid_init(hdev);
	k_work_init(&report_send, send_report);
	k_work_init(&report_clear, clear_report);





	if (as == NULL || !device_is_ready(as)) {
		printk("\nono bad stuff sad no device tree\n");
		return;
	}


	printk("device is %p, name is %s\n", as, as->name);


   int lastDegree = as5600_refresh(as);
    printk("%d", lastDegree);

	while (1) {
        
		//printk("Hello World! %s\n", CONFIG_ARCH);
		//k_sleep(K_MSEC(100));
		
		int degrees = as5600_refresh(as);
        int deltaDegrees = degrees-lastDegree;
        if (deltaDegrees > 5 ) {
            printk("1\n");
            k_work_submit(&report_send);
         
            lastDegree=degrees;
        }else if(deltaDegrees < -5 ){
            printk("-1\n");
            k_work_submit(&report_send);
            
            lastDegree=degrees;
        }
        

        // if (lastDegree - degrees < 10 || lastDegree - degrees > -10){

        //         printk("z");
        //         lastDegree = degrees;
        // }
    
       // printk("delta: %d\n ", deltaDegrees);

        

		


	
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