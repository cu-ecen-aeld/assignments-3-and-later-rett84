# OOPS Analysis
#### L. Rettore

- From the output console after running: `echo “hello_world” > /dev/faulty`

Internal error: Oops: 0000000096000045 [#1] SMP\
Modules linked in: hello(O) faulty(O) scull(O)\
CPU: 0 PID: 154 Comm: sh Tainted: G           O       6.1.44 #1\
Hardware name: linux,dummy-virt (DT)\
>pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)\
>pc : faulty_write+0x10/0x20 [faulty]\
lr : vfs_write+0xc8/0x390\
sp : ffffffc008e03d20\
x29: ffffffc008e03d80 x28: ffffff8001f85cc0 x27: 0000000000000000\
x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000\
x23: 0000000000000012 x22: 0000000000000012 x21: ffffffc008e03dc0\
x20: 000000558cdcc990 x19: ffffff8001c15000 x18: 0000000000000000\
x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000\
x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000\
x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000\
x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000\
x5 : 0000000000000001 x4 : ffffffc000787000 x3 : ffffffc008e03dc0\
x2 : 0000000000000012 x1 : 0000000000000000 x0 : 0000000000000000\
Call trace:\
 faulty_write+0x10/0x20 [faulty]     `--> fault writing to byte 10 of 20 of [faulty] module`\
 ksys_write+0x74/0x110\
 __arm64_sys_write+0x1c/0x30\

