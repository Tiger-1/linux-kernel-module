#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/usb.h>
#include<linux/uaccess.h>
//#include<errno.h>

#define MIN(a,b) (((a) <= (b)) ? (a) : (b))
#define CUSTOM_USB_VENDOR_ID0 	0x0483 //vendor specific vendor id and product id
#define CUSTOM_USB_PRODUCT_ID0 	0x5750
#define CUSTOM_USB_VENDOR_ID1	0x1FC9
#define CUSTOM_USB_PRODUCT_ID1	0x0081
#define interrupt_EP_OUT 		0x01  //Interrupt endpoints are used
#define interrupt_EP_IN 		0x81
#define MAX_PKT_SIZE		64

static struct usb_device *device;
static struct usb_class_driver class;
//static unsigned char interrupt_buf[MAX_PKT_SIZE];
//static struct urb *urb;
static unsigned char *interrupt_buf;
static int custom_usb_open(struct inode *inode, struct file *file){
	return 0;
}

static int custom_usb_close(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t custom_usb_read(struct file *f, char __user *buf, size_t cnt, loff_t *off){
	int ret;
	int read_cnt;
	
	ret = usb_interrupt_msg(device, usb_rcvintpipe(device,interrupt_EP_IN),interrupt_buf,MAX_PKT_SIZE, &read_cnt, 5000);
	if(ret){
		printk(KERN_ERR "interrupt message returned %d\n",ret);
		return ret;
	}
	if(copy_to_user(buf, interrupt_buf, MIN(cnt, read_cnt))){
		return -EFAULT;
	}
	
	return MIN(cnt, read_cnt);
}

/*static void custom_usb_write_interrupt_callback(struct urb *urb)
{
	if(urb->status && !(urb->status == -ENOENT || urb->status == -ECONNRESET || urb->status == -ESHUTDOWN)){
		printk(KERN_INFO "%s - nonzero write interrupt status received: %d", __FUNCTION__, urb->status);
	}

	usb_free_coherent(device, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
}*/
static ssize_t custom_usb_write(struct file *file, const char __user *buf, size_t cnt, loff_t *off){
	int ret;
	int write_cnt = MIN(cnt, MAX_PKT_SIZE);
	/*urb = usb_alloc_urb(0, GFP_KERNEL);
	if(!urb){
		ret = -ENOMEM;
	}
	
	buf = usb_alloc_coherent(device, cnt, GFP_KERNEL, &urb->transfer_dma);
	if(!buf){
		ret = -ENOMEM;
	}

	if(copy_from_user(interrupt_buf,buf, write_cnt)){
		ret = -EFAULT;
	}

	usb_fill_int_urb(urb, device, usb_sndintpipe(device, interrupt_EP_OUT), interrupt_buf, write_cnt, custom_usb_write_interrupt_callback, device,5000);
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	
 	ret = usb_submit_urb(urb, GFP_KERNEL);
	if(ret){
		 printk(KERN_ERR "%s - failed submitting write urb, error %d", __FUNCTION__, ret);
	}*/
	if(copy_from_user(interrupt_buf, buf, MIN(cnt, MAX_PKT_SIZE))){
		return -EFAULT;
	}
	printk(KERN_INFO "copied from user.\n");
	ret = usb_interrupt_msg(device, usb_sndintpipe(device, interrupt_EP_OUT), interrupt_buf, MIN(cnt, MAX_PKT_SIZE),&write_cnt, 5000);
	if(ret){
		printk(KERN_ERR "interrupt message returned %d\n",ret);
		return ret;
	}
	printk(KERN_INFO "message send.\n");
	return write_cnt;	
}

/*static void custom_usb_write_interrupt_callback(struct urb *urb)
{
	if(urb->status && !(urb->status == -ENOENT || urb->status == -ECONNRESET || urb->status == -ESHUTDOWN)){
		printk(KERN_INFO "%s - nonzero write interrupt status received: %d", __FUNCTION__, urb->status);
	}

	usb_free_coherent(device, urb->transfer_buffer_length, urb->transfer_buffer, urb->transfer_dma);
}*/


static struct file_operations fops = {
	.open = custom_usb_open,
	.release = custom_usb_close,
	.read = custom_usb_read,
	.write = custom_usb_write,
};

//probe function: called on device insertion if and only no other driver locks that device
static int custom_usb_probe(struct usb_interface *interface, const struct usb_device_id *id){
//	printk(KERN_INFO "Custom device inserted (%04x : %04x)\n",id->idVendor,id->idProduct);
	
	int ret;
	printk(KERN_INFO "Custom device inserted (%04x : %04x)\n",id->idVendor,id->idProduct);
	device = interface_to_usbdev(interface);

	class.name = "usb/custom%d";
	class.fops = &fops;
	ret = usb_register_dev(interface,&class);
	if(ret < 0){
		printk(KERN_ERR "Not able to get a minor for this device.");
	}
	else{
		printk(KERN_INFO "Minor obtained: %d\n",interface->minor);
	}
	
	return ret;
}

//disconnect function: called when device is unplugged
static void custom_usb_disconnect(struct usb_interface *interface){
	printk(KERN_INFO "Custom device removed\n");
	usb_deregister_dev(interface, &class);
}


//table of devices that work with driver
static struct usb_device_id custom_usb_table[] = {
	{USB_DEVICE(CUSTOM_USB_VENDOR_ID0,CUSTOM_USB_PRODUCT_ID0)},
	{USB_DEVICE(CUSTOM_USB_VENDOR_ID1,CUSTOM_USB_PRODUCT_ID1)},
	{} //terminating entry
};
MODULE_DEVICE_TABLE(usb,custom_usb_table);
 
static struct usb_driver custom_usb_driver = {
	.name = "Custom USB device driver",	//name of the driver
	.id_table = custom_usb_table,		//structure of usb device id containing product id and vendor id
	.probe = custom_usb_probe,
	.disconnect = custom_usb_disconnect,
};


static int __init custom_usb_init(void)
{
	int ret;
	interrupt_buf = kmalloc(64,GFP_KERNEL);
	ret = usb_register(&custom_usb_driver);
	if(ret)
		printk(KERN_INFO "Driver is registered with custom device");
	return ret;
}

static void __exit custom_usb_clean(void)
{
	kfree(interrupt_buf);
	usb_deregister(&custom_usb_driver);
	printk(KERN_INFO "Driver deregistration completed");
}

module_init(custom_usb_init);
module_exit(custom_usb_clean);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Abhinav");
MODULE_DESCRIPTION("USB device driver for custom class");
