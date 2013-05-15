blk_plug_device
===============

Modify blk_plug_device() from a kernel module

You need to obtain the address of the blk_plug_device() 
from your kernel eg:

cat /proc/kallsyms | grep blk_plug_device
...
ffffffff812a3c70 T blk_plug_device
...

