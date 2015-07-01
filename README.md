Description:
====
This is a linux character device driver for LED matrix controlled by GPIO in raspberry pi.

TCP server and client are also provided (in test/) to let you control LED matrix remotely.

Build:
====
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
## 30 is the Major number
$> sudo mknod /dev/ledmatrix c 30 0
```
6. Run the test program. "server.c" is a tcp server on raspberry pi. "app.c" is the corresponding client. **Note**, you should modify the ip address these files to let them work.

7. Remove the module from your system
