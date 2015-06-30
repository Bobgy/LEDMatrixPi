Description:
This is an example of linux device driver.

Build:
1. Build your linux kernel tree
2. Configure correct linux kernel source path in Makefile
3. Build the module
4. Install the module
5. Build the test program in test directory
6. Add a device using the driver, the process should be like
```
$> dmesg | tail
[12595.546469] Device number is 31457280
[12595.546474] Major number is 30
[12595.546476] Minor number is 0
## replace xx with your Major number
$> sudo mknod /dev/chardev0 c xx 0
```
6. Run the test program
7. Remove the module from your system
