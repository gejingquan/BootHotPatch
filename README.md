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

`cd samples`

`mkdir lua-2021-45985`

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

### Step 3:

Test the lua demo example.

`mkdir seed`

`cp ../../../../BootHotPatch/poc.lua  ./seed/`

`./lpmake/lua seed/poc.lua`

Wait for more than 10 seconds, and if no crash information appears, it means that the hot patch has taken effect.

Open another terminal and enter the command:

`sudo libcare-ctl info -p $(pidof lua)`

We can see the hot patch information.


