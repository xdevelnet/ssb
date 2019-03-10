# SSB

## What is SSB?

SSB is the simple way of data organisation of storing information of any size any type. The abbreviation stands for Size Separated Binary.
According to the format, blocks are stored sequentially (in a linear space like memory or file) one after another. Each block consists of data size and data itself.

Thus, we can easily determine the exact position and the size of the next block. Also, after some preparations, we can immediatly get the data and its size.

![ssb](https://user-images.githubusercontent.com/16417758/52917366-39d1c000-32f3-11e9-8f23-09cae1123a5b.png)

SSB itself is not as useful as the derivative formats.

## TSSB

TSSB is a format that stores binary data in a table form.

There is a signature at the beginning of file (or any other linear object), which is a sign for particular form of TSSB and lets programs (or library, for example) understand what to do next.
There is a metadata after the signature, which is followed by the data (with sizes) itself.

The way of storing data depends on the signature. Currently available signatures and their features are listed in the table below

|Signature|Metadata|Data storing scheme|Limitations|
|---|---|---|---|
|`SSBTRANSLATI0NS_0`|There are two 4 byte blocks after the signature. They contain the info about amount of rows and cols respectively. Data type: uint32_t little endian.|1 byte of uint8_t type for storing the size of data, then the data next to it. If size is equal to maximum value for that type, that means that it's new row, not data size.|Maximum amount of data size - 254 bytes. Total amount of data is limited by hardware address arithmetic
|`SSBTRANSLATI0NS_1`|Similar ↑|Similar ↑, but 2 bytes with uint16_t type little endian|Similar ↑, but max. data size is 65534|
|`SSBTRANSLATI0NS_2`|Similar ↑|Similar ↑, but 4 bytes with uint32_t type little endian|Similar ↑, but max. data size is 4294967294|
|`SSBTRANSLATI0NS_3`|Similar ↑|Similar ↑, but 8 bytes with uint64_t type little endian|Similar ↑, but max. data size is 18446744073709551614|

## libtssb

libtssb is a TSSB implementation from TSSB developer.

It allows you to read a TSSB file, get a 2D array, which points to data blocks, and recieve sizes of these blocks.

Initally TSSB was developed as the format for storing short strings and translations for them in a table way. The whole point of that was to get the pointer to necessary cell as fast as possible (from CPU cycle POV) if you know which cell have your string.
However you should remember that string is only one of many ways of data representation. TSSB can store ANY kind and any size of binary data.

### Using libtssb

An example(s) is stored in examples directory from libtssb subproject.
API and its description is located in tssb.h header file.
You can use libssb in your project just by including libssb.c to your source code, or by including libssb.h and linking with precompiled libssb library.

### See also

 * https://github.com/xdevelnet/tcsv2tssb - csv to TSSB converter

### Many thanks for

 * Plekhanov Artem - https://github.com/PlekhanovA
 * NicklausBrain - https://github.com/NicklausBrain 
