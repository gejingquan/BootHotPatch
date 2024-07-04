# Section 1: Add the boothotpatch kernel module to the kernel.

**Hardware platform**: Raspberry Pi 4

**Operating system**: Ubuntu 22.04.4 LTS

**Linux kernel version**:  Linux 5.15.0-1046-raspi (aarch64)
 

There are two methods to add the boothotpatch module to the kernel:

## Method 1: 
Modify the kernel source code and Makefile and recompile the kernel. Then, install the newly compiled kernel into the system and reboot.

## Method 2: 
Compile the boothotpatch module separately and configure the module to start automatically at boot.

### Step 1: 
Download BootHotPatch code

`git clone https://github.com/gejingquan/BootHotPatch`

### Step 2: 
Compile BootHotPatch kernel module to get boothotpatch.ko

`cd BootHotPatch`

`sudo apt install build-essential`

`make`

Now, you have got *boothotpatch.ko*.

### Step 3: 
Make *boothotpatch.ko* automatically loaded into the kernel when booting

`sudo cp boothotpatch.ko /lib/modules/$(uname -r)/kernel/drivers/misc`

`sudo cp load-boothotpatch-ko.service /etc/systemd/system/`

`sudo systemctl daemon-reload`

`sudo systemctl enable load-boothotpatch-ko`

# Section 2: Set boothotpatch_monitor as a system service and enable it to start automatically at boot

### Step 1:
Compile the *boothotpatch_monitor* software and set it as the service that starts automatically when booting.

`gcc -o boothotpatch_monitor boothotpatch_monitor.c`

`sudo cp boothotpatch_monitor /sbin/`

`sudo cp boothotpatch_monitor.service /etc/systemd/system/`

`sudo systemctl daemon-reload`

`sudo systemctl enable boothotpatch_monitor`

# Section 3: Install the hot patch deployment tool libcareplus and prepare a demonstration instance (CVE-2021-45985).

### Step 1:
Download the software source code of libcareplus, compile and install it.

`cd ..`

`git clone https://github.com/openeuler-mirror/libcareplus`

`cd libcareplus`

`sudo apt-get install -y binutils elfutils libelf-dev libunwind-dev pkg-config`

`cd src`

`sudo make install`

`sudo chmod 777 /usr/local/bin/*`

### Step 2:
Download the lua software source code and compile a binary file with the CVE-2021-45985 vulnerability and the corresponding hot patch file.

`cd ../samples`

`mkdir lua-2021-45985`

`cd lua-2021-45985`

`git clone https://github.com/lua/lua`

`cd lua`

`git checkout cf613cd`

`git format-patch -1`

`git checkout 066e0f9`

`cp ../../../../BootHotPatch/0001-lua.patch ./`

`patch -p1 < 0001-lua.patch`

`sudo apt-get install libreadline-dev`

`sudo libcare-patch-make --clean -i 0001 0001-Bug-finalizers-can-be-called-with-an-invalid-stack.patch`

`sudo mv patchroot/*.kpatch patchroot/lua.kpatch`

Now we have the binary with the CVE-2021-45985 vulnerability (./lpmake/lua) and hot patch file (./patchroot/lua.kpatch).

Next, let's run the vulnerable binary.

`mkdir seed`

`cp ../../../../BootHotPatch/poc.lua  ./seed/`

`./lpmake/lua seed/poc.lua`

Since the entire BootHotPatch framework has not yet been started, we can see the crash information as shown below.

```sh
gejingquan@gejingquan-desktop:~/projects/libcareplus/samples/lua-2021-45985/lua$ ./lpmake/lua seed/poc.lua
Waiting for the hot patch to be installed...
=================================================================
==40815==ERROR: AddressSanitizer: heap-buffer-overflow on address 0xffffa23006d1 at pc 0xaaaad8d868d0 bp 0xffffe3a572a0 sp 0xffffe3a572b0
READ of size 1 at 0xffffa23006d1 thread T0
    #0 0xaaaad8d868cc in luaG_getfuncline (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x268cc)
    #1 0xaaaad8d86a2c in getcurrentline (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x26a2c)
    #2 0xaaaad8d8bcf8 in luaG_runerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2bcf8)
    #3 0xaaaad8d8af4c in typeerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2af4c)
    #4 0xaaaad8d8b0f4 in luaG_callerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2b0f4)
    #5 0xaaaad8d8f03c in luaD_tryfuncTM (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2f03c)
    #6 0xaaaad8d90dfc in luaD_precall (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x30dfc)
    #7 0xaaaad8d90ec8 in ccall (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x30ec8)
    #8 0xaaaad8d91010 in luaD_callnoyield (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x31010)
```

We now restart the system to start the BootHotPatch framework.

`sudo reboot`

# Section 4: Test the lua demo example CVE-2021-45985.

### Terminal 1:

`cd ~/projects/libcareplus/samples/lua-2021-45985/lua`


`./lpmake/lua seed/poc.lua`

After waiting for more than 10 seconds, no crash information is displayed and the running results are as follows. 


```
gejingquan@gejingquan-desktop:~/projects/libcareplus/samples/lua-2021-45985/lua$ ./lpmake/lua seed/poc.lua
Waiting for the hot patch to be installed...
Waiting for the hot patch to be uninstalled...
```
This shows that the hot patch has worked and the CVE-2021-45985 vulnerability has been patched.

### Terminal 2：
Open another terminal and enter the command:

`sudo libcare-ctl info -p $(pidof lua)`

We can see the hot patch information.

### Terminal 1：

Press *Ctrl+C* in the original terminal to terminate the running of the Lua software.

Close the boothotpatch_monitor system service and run the lua software again with *poc.lua* as input..

`sudo systemctl stop boothotpatch_monitor.service`

`./lpmake/lua seed/poc.lua`

Now you can see the crash information, indicating that the hot patch has not been deployed.

```
gejingquan@gejingquan-desktop:~/projects/libcareplus/samples/lua-2021-45985/lua$ ./lpmake/lua seed/poc.lua
Waiting for the hot patch to be installed...
=================================================================
==1989==ERROR: AddressSanitizer: heap-buffer-overflow on address 0xffff9c2006d1 at pc 0xaaaacdc268d0 bp 0xfffffbcaca80 sp 0xfffffbcaca90
READ of size 1 at 0xffff9c2006d1 thread T0
    #0 0xaaaacdc268cc in luaG_getfuncline (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x268cc)
    #1 0xaaaacdc26a2c in getcurrentline (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x26a2c)
    #2 0xaaaacdc2bcf8 in luaG_runerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2bcf8)
    #3 0xaaaacdc2af4c in typeerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2af4c)
    #4 0xaaaacdc2b0f4 in luaG_callerror (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2b0f4)
    #5 0xaaaacdc2f03c in luaD_tryfuncTM (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x2f03c)
    #6 0xaaaacdc30dfc in luaD_precall (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x30dfc)
    #7 0xaaaacdc30ec8 in ccall (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x30ec8)
    #8 0xaaaacdc31010 in luaD_callnoyield (/home/gejingquan/projects/libcareplus/samples/lua-2021-45985/lua/lpmake/lua+0x31010)
```


