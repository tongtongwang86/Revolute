#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/usb/class/usb_hid.h>
 
// #define HID_REVOLUTE_REPORT_DESC() {				\
// 	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),			\
// 	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),			\
// 	HID_COLLECTION(HID_COLLECTION_APPLICATION),		\
// 		HID_COLLECTION(HID_COLLECTION_PHYSICAL),	\
// 			HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),	\
// 			HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),	\
// 			HID_LOGICAL_MIN8(-127),			\
// 			HID_LOGICAL_MAX8(127),	
// 			HID_REPORT_SIZE(8),			\
// 			HID_REPORT_COUNT(1),			\
// 			/* HID_INPUT (Data,Var,Rel) */		\
// 			HID_INPUT(0x06),			\
// 		HID_END_COLLECTION,				\
// 	HID_END_COLLECTION,					\
// }

#define HID_REVLOTUE_REPORT_DESC(bcnt) {				\
	HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),			\
	HID_USAGE(HID_USAGE_GEN_DESKTOP_MOUSE),			\
	HID_COLLECTION(HID_COLLECTION_APPLICATION),		\
		HID_USAGE(HID_USAGE_GEN_DESKTOP_POINTER),	\
		HID_COLLECTION(HID_COLLECTION_PHYSICAL),	\
			/* Bits used for button signalling */	\
			HID_USAGE_PAGE(HID_USAGE_GEN_BUTTON),	\
			HID_USAGE_MIN8(1),			\
			HID_USAGE_MAX8(bcnt),			\
			HID_LOGICAL_MIN8(0),			\
			HID_LOGICAL_MAX8(1),			\
			HID_REPORT_SIZE(1),			\
			HID_REPORT_COUNT(bcnt),			\
			/* HID_INPUT (Data,Var,Abs) */		\
			HID_INPUT(0x02),			\
			/* Unused bits */			\
			HID_REPORT_SIZE(8 - bcnt),		\
			HID_REPORT_COUNT(1),			\
			/* HID_INPUT (Cnst,Ary,Abs) */		\
			HID_INPUT(1),				\
			/* X and Y axis, scroll */		\
			HID_USAGE_PAGE(HID_USAGE_GEN_DESKTOP),	\
			HID_USAGE(HID_USAGE_GEN_DESKTOP_X),	\
			HID_USAGE(HID_USAGE_GEN_DESKTOP_Y),	\
			HID_USAGE(HID_USAGE_GEN_DESKTOP_WHEEL),	\
			HID_LOGICAL_MIN8(-127),			\
			HID_LOGICAL_MAX8(127),			\
			HID_REPORT_SIZE(8),			\
			HID_REPORT_COUNT(3),			\
			/* HID_INPUT (Data,Var,Rel) */		\
			HID_INPUT(0x06),			\
		HID_END_COLLECTION,				\
	HID_END_COLLECTION,					\
}

#define wheelResolution 1
#define MOUSE_X_REPORT_POS	3
// #define MOUSE_X_REPORT_POS	3
uint8_t report[4] = { 0x00 };
static volatile uint8_t status[4];

#define KEY_1_CONFIGURE HID_KEY_S //clockwise key
#define KEY_1_MODIFYER 0b00000010  // | RGUI| RALT| RSHIFT| RCONTROL | LGUI| LALT| LSHIFT| LCONTROL|

#define KEY_2_CONFIGURE HID_KEY_S //counter clockwise key'
#define KEY_2_MODIFYER 0b00000000  // | RGUI| RALT| RSHIFT| RCONTROL | LGUI| LALT| LSHIFT| LCONTROL|

#define STACKSIZE 1024
#define PRIORITY 1
#define SLEEPTIME 500
#define IDENT_OFFSET 1
K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);

static struct k_thread threadA_data;

static const struct device *hdev;

static const uint8_t hid_rvl_report_desc[] = HID_REVLOTUE_REPORT_DESC(2);

K_SEM_DEFINE(my_sem, 0, 10);
static K_SEM_DEFINE(usb_sem, 1, 1);	/* starts off "available" */

static void int_in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);
	
	k_sem_give(&usb_sem);

}


static const struct hid_ops ops = {
	.int_in_ready = int_in_ready_cb,
};

int as5600_refresh(const struct device *dev)
{
	int ret;
    struct sensor_value rot_raw;
    ret = sensor_sample_fetch_chan(dev,SENSOR_CHAN_ROTATION);
	if (ret != 0){
			printk("sample fetch error:,%d\n", ret);
		}
    sensor_channel_get(dev,SENSOR_CHAN_ROTATION, &rot_raw);
	

    return rot_raw.val1;
}

// int getNearestSnap (int num){




// }



void threadA(void *dummy1, void *dummy2, void *dummy3)
{	
	const struct device *const as = DEVICE_DT_GET(DT_INST(0,ams_as5600));

	if (as == NULL || !device_is_ready(as)) {
		printk("as5600 device tree not configured\n");
		return;
	}

	int lastIdent = (as5600_refresh(as) - (as5600_refresh(as) % 12))/12 ;
	
	//int lastDeviation = as5600_refresh(as) % 12;
	
	
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);





	while (1)
	{
		uint8_t state = status[MOUSE_X_REPORT_POS];
		int degrees = as5600_refresh(as) ;
		int usefulDegrees = as5600_refresh(as) ;
		int lastDegree = as5600_refresh(as);
		int deltadegrees = 0;
		int lastDeltadegrees = 0;
		int deltaDeltadegrees = 0;
		int lastdeltaDeltadegrees = 0;
		deltadegrees = (degrees-lastDegree);
		deltaDeltadegrees = (deltadegrees-lastDeltadegrees);

		//int deviation = degrees % 12;
		// if (degrees-lastDegree < -200 ){
		// 	if((degrees-lastDegree)-deltadegrees < -360){

		// 		deltadegrees = 360*2 -(degrees-lastDegree);
		// 		printk("aaaaa");

		// 	}else{

		// 		deltadegrees = 360 +(degrees-lastDegree);
		// 	}
			

		// } else if (degrees-lastDegree > 200){

		// 	if((degrees-lastDegree)-deltadegrees > 20){

		// 		deltadegrees = 360*2 -(degrees-lastDegree);
		// 		printk("eeeee");

		// 	}else{

		// 		deltadegrees = 360 -(degrees-lastDegree);
		// 	}

		// } else {

		// 	deltadegrees = (degrees-lastDegree);

		// }

		if (deltaDeltadegrees < -200){
			usefulDegrees = lastDeltadegrees ;
			deltaDeltadegrees = lastdeltaDeltadegrees;

		} else if(deltaDeltadegrees > 200) {
			usefulDegrees = lastDeltadegrees ;
			deltaDeltadegrees = lastdeltaDeltadegrees;

		}else{
			usefulDegrees = (degrees-lastDegree) ;

		}
		// lastDegree = degrees;
		
		//printk("%d\n", (degrees - (degrees % 12))/12);
		// printk("%d\n", usefulDegrees);
if (degrees != lastDegree){
		if (lastIdent != (((degrees+6+IDENT_OFFSET)) - ((degrees+6+IDENT_OFFSET) % 12))/12 && (((degrees+6+IDENT_OFFSET)) - ((degrees+6+IDENT_OFFSET) % 12))/12!= 30) {

		// if (degrees != lastDegree){


		
		uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

		
		if (deltaDeltadegrees > 0){
			// state += (deltaDeltadegrees);
			state += 1U;

		}else{
			// state += (deltaDeltadegrees);
			state -= 1U;
		}

		status[MOUSE_X_REPORT_POS] = state;
		report[MOUSE_X_REPORT_POS] = status[MOUSE_X_REPORT_POS];
		status[MOUSE_X_REPORT_POS] = 0U;

	

		k_sem_take(&usb_sem, K_FOREVER);
		int ret = hid_int_ep_write(hdev,report,sizeof(report), NULL);
		// printk("snap:%d\n",degrees);
		// printk("degrees:%d\n",as5600_refresh(as));
		
	
		k_sem_give(&my_sem);
		}
			lastIdent = ((degrees+6+IDENT_OFFSET) - ((degrees+6+IDENT_OFFSET) % 12))/12;
			lastDegree = degrees;
			lastDeltadegrees = deltadegrees;
			lastdeltaDeltadegrees = deltaDeltadegrees;
			//lastDeviation = deviation;
		}


	}

}






BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
	     "Console device is not ACM CDC UART device");

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
USBD_CONFIGURATION_DEFINE(config_1,
			  USB_SCD_SELF_POWERED,
			  200);

USBD_DESC_LANG_DEFINE(sample_lang);
USBD_DESC_MANUFACTURER_DEFINE(sample_mfr, "ZEPHYR");
USBD_DESC_PRODUCT_DEFINE(sample_product, "Zephyr USBD ACM console");
USBD_DESC_SERIAL_NUMBER_DEFINE(sample_sn, "0123456789ABCDEF");

USBD_DEVICE_DEFINE(sample_usbd,
		   DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   0x2fe3, 0x0001);

static int enable_usb_device_next(void)
{

	usbd_add_descriptor(&sample_usbd, &sample_lang);
	usbd_add_descriptor(&sample_usbd, &sample_mfr);
	usbd_add_descriptor(&sample_usbd, &sample_product);
	usbd_add_descriptor(&sample_usbd, &sample_sn);
	usbd_add_configuration(&sample_usbd, &config_1);
	usbd_register_class(&sample_usbd, "cdc_acm_0", 1);
	usbd_init(&sample_usbd);
	usbd_enable(&sample_usbd);
	
	return 0;
}

#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK_NEXT) */

int main(void)
{
	const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
	if (enable_usb_device_next()) {
		return 0;
	}
#else
	if (usb_enable(NULL)) {
		return 0;
	}
#endif

	
	k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_a");

	k_thread_start(&threadA_data);

	


while (1)
	{	

		if (k_sem_take(&my_sem, K_MSEC(50)) != 0) {
  
    } else {
		
		

		// uint8_t rep[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
		// k_sem_take(&usb_sem, K_FOREVER);
		// int ret = hid_int_ep_write(hdev,rep,sizeof(rep), NULL);

		
		

    }
	
	}

	
}


static int composite_pre_init(void)
{
	hdev = device_get_binding("HID_0");
	if (hdev == NULL) {
		printk("Cannot get USB HID Device");
		return -ENODEV;
	}


	usb_hid_register_device(hdev, hid_rvl_report_desc,sizeof(hid_rvl_report_desc), &ops);

	return usb_hid_init(hdev);
}


SYS_INIT(composite_pre_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);