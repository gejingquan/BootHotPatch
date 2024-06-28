#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/smp.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h>


MODULE_AUTHOR("Gerty");
MODULE_DESCRIPTION("boothotpatch_driver");
MODULE_VERSION("v1.0");
MODULE_LICENSE("GPL");


#define DEVICE_NAME "boothotpatch_module"
#define BUF_SIZE 1024
#define MAX_LINES 10
#define MAX_LENGTH 200



pid_t monitor_pid;
struct task_struct *task;
struct kernel_siginfo info;
static char *current_app;

struct HotPatchAppInfo {
    char hotpatch_app_name[MAX_LENGTH];
    char hotpatch_app_path[MAX_LENGTH];
    char hotpatch_lib_path[MAX_LENGTH];
};
static struct HotPatchAppInfo hotpatchapp[MAX_LINES];


static int boothotpatch_driver_major;
static struct class*  boothotpatch_driver_class   = NULL;
static struct device* boothotpatch_driver_device  = NULL;


static struct kprobe kp = {
    .symbol_name = "finalize_exec",
};







char *get_current_process_exe(void) {
    struct mm_struct *mm;
	struct dentry *dentry;
	
    // Get the current task_struct using the current pointer
    struct task_struct *task = current;
    if (!task) {
        return NULL;
    }
    // Get the mm_struct of the current process
    mm = task->mm;
    if (!mm) {
        return NULL;
    }
    // Access the executable file name from mm_struct
    struct file *exe_file = mm->exe_file;
    if (exe_file) {
        dentry = exe_file->f_path.dentry;
        if (dentry) {
			return dentry->d_iname;			
        }
    }

    return NULL;
}




static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {

	int i;
	current_app=get_current_process_exe();
	printk("boothotpatch: Current App Name: %s\n", current_app);
    printk(KERN_INFO "boothotpatch: finalize_exec has been executed\n");
	

	for(i=0;i<10;i++){
		printk("boothotpatch: current_app: %s\n", current_app);
		printk("boothotpatch: hotpatchapp[%d].hotpatch_app_name: %s\n", 
			i, hotpatchapp[i].hotpatch_app_name);
		printk("strcmp(hotpatchapp[%d].hotpatch_app_name, current_app) == %d\n", 
			i, strcmp(hotpatchapp[i].hotpatch_app_name, current_app));
		
		
		if (strcmp(hotpatchapp[i].hotpatch_app_name, current_app) == 0) {
			printk("boothotpatch: I find the correct HotPatchApp Name is: %s\n", hotpatchapp[i].hotpatch_app_name);
			printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
				hotpatchapp[0].hotpatch_app_name);
			printk(KERN_INFO "boothotpatch: We are patching %s now!\n",hotpatchapp[i].hotpatch_app_name);

			
			task = pid_task(find_vpid(monitor_pid), PIDTYPE_PID);
			if (!task) {
				printk(KERN_INFO "Process with PID %d not found\n", monitor_pid);
				return;
			}
			memset(&info, 0, sizeof(struct kernel_siginfo));
			info.si_signo = SIGUSR1;
			info.si_code = SI_QUEUE;  
			info.si_int = 1234; 
			printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
				hotpatchapp[0].hotpatch_app_name);			
			if (send_sig_info(SIGUSR1, &info, task) < 0) {
				printk(KERN_INFO "Failed to send signal SIGUSR1 to process %d\n", monitor_pid);
			} else {
				printk(KERN_INFO "Signal SIGUSR1 sent to process %d\n", monitor_pid);
				printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
					hotpatchapp[0].hotpatch_app_name);			
			}

			break;
		}
	} 
	
	printk("boothotpatch: i= %d\n", i);
	


			
}




static int boothotpatch_open(struct inode * inode, struct file * filp)
{
	printk("boothotpatch: Open boothotpatch!");
	printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
		hotpatchapp[0].hotpatch_app_name);	
	return 0;
}


static int boothotpatch_release(struct inode * inode, struct file *filp)
{
	return 0;
}


ssize_t boothotpatch_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{	
	printk("boothotpatch: Read boothotpatch!");
	printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
		hotpatchapp[0].hotpatch_app_name);	
	
	if (copy_to_user(buf, current_app, (strlen(current_app)+1)) != 0) {
		return -EFAULT;  
    }
	printk("boothotpatch: hotpatchapp[0].hotpatch_app_name: %s\n", 
		hotpatchapp[0].hotpatch_app_name);	
    return 0;
}

ssize_t boothotpatch_write(struct file* file, const char __user* buf, size_t size, loff_t *ppos)
{

	printk("boothotpatch: Write boothotpatch!");

    if (size > BUF_SIZE) {
        return -EINVAL; 
    }
    if (copy_from_user(&monitor_pid, buf, size)) {
        return -EFAULT;
    }
    printk(KERN_INFO "Received PID from user space: %d\n", monitor_pid);

				
	return size;
}




static long boothotpatch_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {

	int err = 0;
	int i;
    switch (cmd) {
        case 0:
            // 从用户空间读取数据
            err = copy_from_user(hotpatchapp, (struct HotPatchAppInfo *)arg, sizeof(hotpatchapp));
            if (err) {
                return -EFAULT;
            }
			for (i = 0; i < 10; i++) {		
				printk("HotPatchApp Name: %s\n", hotpatchapp[i].hotpatch_app_name);
				printk("HotPatchApp Path: %s\n", hotpatchapp[i].hotpatch_app_path);
				printk("HotPatch Lib Path: %s\n", hotpatchapp[i].hotpatch_lib_path);
			}
            break;
        default:
            return -ENOTTY; // 不支持的命令
    }

    return 0;
}

static struct file_operations boothotpatch_driver_fops = {
	.owner   =    THIS_MODULE,
	.open    =    boothotpatch_open,
	.release =    boothotpatch_release,
	.read    =    boothotpatch_read,
	.write   =    boothotpatch_write,
	.unlocked_ioctl = boothotpatch_ioctl,
};


static int __init boothotpatch_init(void)
{
	printk("boothotpatch: I am in boothotpatch_init.\n");		
	boothotpatch_driver_major = register_chrdev(0, DEVICE_NAME, &boothotpatch_driver_fops);
	if(boothotpatch_driver_major < 0){
		printk("failed to register device.\n");
		return -1;
	}

	boothotpatch_driver_class = class_create(THIS_MODULE, "boothotpatch_driver");
    if (IS_ERR(boothotpatch_driver_class)){
        printk("failed to create boothotpatch moudle class.\n");
        unregister_chrdev(boothotpatch_driver_major, DEVICE_NAME);
        return -1;
    }	

    boothotpatch_driver_device = device_create(boothotpatch_driver_class, NULL, MKDEV(boothotpatch_driver_major, 0), NULL, "boothotpatch_device");
    if (IS_ERR(boothotpatch_driver_device)){
        printk("failed to create device .\n");
        unregister_chrdev(boothotpatch_driver_major, DEVICE_NAME);
        return -1;
    }

	
    kp.post_handler = handler_post;
    register_kprobe(&kp);
    printk(KERN_INFO "boothotpatch: kprobe initialized\n");

	
	return 0;
}


static void __exit boothotpatch_exit(void)
{	
    device_destroy(boothotpatch_driver_class, MKDEV(boothotpatch_driver_major, 0));
    class_unregister(boothotpatch_driver_class);
	class_destroy(boothotpatch_driver_class);
	unregister_chrdev(boothotpatch_driver_major, DEVICE_NAME);
	
	
    unregister_kprobe(&kp);
    printk(KERN_INFO "boothotpatch: kprobe exited\n");
	
}


module_init(boothotpatch_init);
module_exit(boothotpatch_exit);



