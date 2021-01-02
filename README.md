# Rick-OS

## Description
This is a (very) simple operating system to rickroll whoever you want. It displays 4 pictures of Rick Astley, waiting between them and looping forever.

## Compile instructions
* In the **Makefile** Replace the values of *GCC*, *LD*, *GDB*, *QEMU* and *GMKR* with your own tools.
* Run **make iso**.
It outputs a bootable iso called *image.iso*.

## To modify the images
The images are stored in a list of structures in *picture.c* containing the image width, its height and the image itself. The format used is RGBA 32bits which means that each pixel fills up 32 bits: one byte for the alpha channel, one for the red, one for the green and one for the blue.

To produce such data you can use the *export to c format* option of GIMP.

## Disclaimer
The sleep function is just horrible.


