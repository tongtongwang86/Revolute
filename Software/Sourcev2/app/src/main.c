#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/sensor.h>

#define STACKSIZE 1024
#define PRIORITY 7
#define SLEEPTIME 500
K_THREAD_STACK_DEFINE(threadA_stack_area, STACKSIZE);
K_THREAD_STACK_DEFINE(threadB_stack_area, STACKSIZE);
static struct k_thread threadA_data;
static struct k_thread threadB_data;

K_SEM_DEFINE(my_sem, 0, 10);

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

void threadA(void *dummy1, void *dummy2, void *dummy3)
{	
	const struct device *const as = DEVICE_DT_GET(DT_INST(0,ams_as5600));

	if (as == NULL || !device_is_ready(as)) {
		printk("ono bad stuff sad no device tree\n");
		return;
	}
	printk("device is %p, name is %s\n", as, as->name);

	int lastDegree = as5600_refresh(as);
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	printk("thread_a: thread started \n");

	while (1)
	{

		int degrees = as5600_refresh(as);
        int deltaDegrees = degrees-lastDegree;
        if (deltaDegrees >= 12 ) {
          //  printk("1\n");
           // k_work_submit(&report_send);
		k_sem_give(&my_sem);
		printk("gave a sem \n");
		printk("%d\n", as5600_refresh(as)); 
            lastDegree=degrees;
        }else if(deltaDegrees <= -12 ){
           // printk("-1\n");
           // k_work_submit(&report_send);
		   k_sem_give(&my_sem);
		printk("gave a sem \n");
            
            lastDegree=degrees;
        }

		
		//k_msleep(SLEEPTIME);
	}

}

void threadB(void *dummy1, void *dummy2, void *dummy3)
{
	ARG_UNUSED(dummy1);
	ARG_UNUSED(dummy2);
	ARG_UNUSED(dummy3);

	printk("thread_B: thread started \n");

	while (1)
	{	

		if (k_sem_take(&my_sem, K_MSEC(50)) != 0) {
       // printk("Input data not available!\n");
    } else {
	printk("took a sem \n");
    }
		// printk("thread_B: thread loop \n");
		// k_msleep(SLEEPTIME);
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

	/* Poll if the DTR flag was set */
	while (!dtr) {
		uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
		/* Give CPU resources to low priority threads. */
		k_sleep(K_MSEC(100));
	}
	
	k_thread_create(&threadA_data, threadA_stack_area,
			K_THREAD_STACK_SIZEOF(threadA_stack_area),
			threadA, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadA_data, "thread_a");

	k_thread_start(&threadA_data);

	k_thread_create(&threadB_data, threadB_stack_area,
			K_THREAD_STACK_SIZEOF(threadB_stack_area),
			threadB, NULL, NULL, NULL,
			PRIORITY, 0, K_FOREVER);
	k_thread_name_set(&threadB_data, "thread_b");

	k_thread_start(&threadB_data);


	while (1) {
		printk("Hello World! %s\n", CONFIG_ARCH);
		k_sleep(K_SECONDS(1));
	}
}