//
// This kernel module allows you to replace a kernel function.
// This is amd64 specific code.
//

#include <linux/module.h>
#include <linux/kernel.h>

#define STRINGIFY2(s) #s
#define STRINGIFY(s) STRINGIFY2(s)

#define CONCATENATE2(a,b) a ## b
#define CONCATENATE(a,b) CONCATENATE2(a,b)

#define CODESIZE 12

static unsigned char original_code[CODESIZE];
static unsigned char jump_code[CODESIZE] =
    "\x48\xb8\x00\x00\x00\x00\x00\x00\x00\x00" /* movq $0, %rax */ /* We will replace the 0x0000000000000000 address later */
    "\xff\xe0"				       /* jump *%rax */
    	;

static void intercept_init(void);
static void intercept_start(void);
static void intercept_stop(void);


#include <linux/blkdev.h>


// The function we are replacing
//void blk_plug_device(struct request_queue *q)
void (*kern_blk_plug_device)(struct request_queue *q) = (void (*)(struct request_queue *q) )0xffffffff812a3c70;
void my_blk_plug_device(struct request_queue *q)
{
//  printk(KERN_INFO"blk_plug_device()\n");
//  intercept_stop();
//  kern_blk_plug_device(q);
//  intercept_start();

        WARN_ON(!irqs_disabled());

        /*
         * don't plug a stopped queue, it must be paired with blk_start_queue()
         * which will restart the queueing
         */
        if (blk_queue_stopped(q))
                return;

        if (!queue_flag_test_and_set(QUEUE_FLAG_PLUGGED, q)) {
                //mod_timer(&q->unplug_timer, jiffies + q->unplug_delay);
                mod_timer(&q->unplug_timer, jiffies );
                //trace_block_plug(q);
        }

  return;
}

#define FUNCTION_NAME blk_plug_device
#define FUNCTION_STRING STRINGIFY(FUNCTION_NAME)
#define ORIGINAL_FUNC CONCATENATE(kern_,FUNCTION_NAME)
#define REPLACEMENT_FUNC CONCATENATE(my_,FUNCTION_NAME)

static int __init mod_init(void)
{
    if (ORIGINAL_FUNC != FUNCTION_NAME)
    {
      // If the symbol is exported we could simply use that address, but for safety
      // we make sure it matches the hardcoded address in case the kernel changes.
      // If the symbol is not exported we need to comment out this section of code...
      printk(KERN_INFO "Addresses do not match\n");
      return -1;
    }
    printk(KERN_INFO "Replacing " FUNCTION_STRING "()\n" );

    intercept_init();
    intercept_start();

    return 0;
}

static void __exit mod_exit(void)
{
    printk(KERN_INFO "Reverting to original " FUNCTION_STRING "()\n" );

    intercept_stop();
    return;
}

static inline unsigned long native_pax_open_kernel(void)
{
    unsigned long cr0;

    preempt_disable();
    barrier();
    cr0 = read_cr0() ^ X86_CR0_WP;
    BUG_ON(unlikely(cr0 & X86_CR0_WP));
    write_cr0(cr0);
    return cr0 ^ X86_CR0_WP;
}

static inline unsigned long native_pax_close_kernel(void)
{
    unsigned long cr0;

    cr0 = read_cr0() ^ X86_CR0_WP;
    BUG_ON(unlikely(!(cr0 & X86_CR0_WP)));
    write_cr0(cr0);
    barrier();
    preempt_enable_no_resched();
    return cr0 ^ X86_CR0_WP;
}

static void intercept_init()
{
    *(long *)&jump_code[2] = (long)REPLACEMENT_FUNC;
    memcpy( original_code, ORIGINAL_FUNC, CODESIZE );

//    {
//      int i;
//      for (i=0;i<CODESIZE;++i) printk(KERN_INFO "orig%d 0x%x\n",i,original_code[i]);
//      for (i=0;i<CODESIZE;++i) printk(KERN_INFO "new%d 0x%x\n",i,jump_code[i]);
//    }

    return;
}

static void intercept_start()
{
    native_pax_open_kernel();
    memcpy( ORIGINAL_FUNC, jump_code, CODESIZE );
    native_pax_close_kernel();
}

static void intercept_stop()
{
    native_pax_open_kernel();
    memcpy( ORIGINAL_FUNC, original_code, CODESIZE );
    native_pax_close_kernel();
}

module_init( mod_init );
module_exit( mod_exit );

