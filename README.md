# Section 1: Add the boothotpatch kernel module to the kernel.

**Hardware platform**: Raspberry Pi 4

**Operating system**: Ubuntu 22.04.4 LTS

**Linux kernel version**:  Linux 5.15.0-1046-raspi (aarch64)
 

There are two ways to add the boothotpatch module to the kernel:

## Method 1: 
Modify the kernel source code and Makefile and recompile the kernel. Then, install the newly compiled kernel into the system and reboot.

## Method 2: 
Compile the boothotpatch module separately and configure the module to start automatically at boot.

### Step 1: Download BootHotPatch code
`git clone https://github.com/gejingquan/BootHotPatch`

### Step2: Compile BootHotPatch kernel module to get boothotpatch.ko
`cd BootHotPatch`
``

