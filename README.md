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
|`SSBTRANSLATI0NS_0`|There are two 4 byte blocks after the signature. They contain the info about amount of rows and cols respectively. Data type: uint32_t little endian|1 byte of uint8_t type for storing the size of data, then the data next to it. If size is equal to maximum value for that type, that means that it's new row, not data size.|Maximum amount of data size - 254 bytes. Total amount of data is limited by hardware address arithmetic
|`SSBTRANSLATI0NS_1`|Similar ↑|Similar ↑, but 2 bytes with uint16_t type little endian|Similar ↑, but max. data size is 65534|
|`SSBTRANSLATI0NS_2`|Similar ↑|Similar ↑, but 4 bytes with uint32_t type little endian|Similar ↑, but max. data size is 4294967294|
|`SSBTRANSLATI0NS_3`|Similar ↑|Similar ↑, but 8 bytes with uint64_t type little endian|Similar ↑, but max. data size is 18446744073709551614|
## libtssb

libtssb is a TSSB implementation from TSSB developer.

It allows you to read a TSSB file, get a 2D array, which points to data blocks, and recieve sizes of these blocks.

Initally TSSB was developed as the format for storing short strings and translations for them in a table way. The whole point of that was to get the pointer to necessary cell as fast as possible (from CPU cycle POV) if you know which cell have your string.
However you should remember that string is only one of many ways of data representation. TSSB can store ANY kind and any size of binary data.

### Using libtssb

An example(s) is stored in examples directory.
API and its description is located in libtssb.h header file.
You can also embed libtssb in your project just by including libtssb.c to your source code, or by including libssb.h and linking with precompiled libtssb library.

## ESSB

ESSB is a format that stores data just like in pure SSB, but some data records_amount (we're going to call them "keys") are intended for special usage. Such records_amount must be detected with checking the size of record. If it's negative, then current record is "key".
Originally, ESSB was created to use it as storage for templates, so ESSB parser implementations can provide mechanism to particular processing.

Right after signature there is two 4 byte blocks. They contain information about total amount of records_amount and storage that takes records_amount themselves (without sizes of each record).
Then, records_amount themselves are placed in sequence, right after other.
Then, 1dimentional table with sizes of each record are located. There could be some space between table and records_amount in favor of 4 byte aligning.

|Signature|Metadata|Data storing scheme|Limitations|
|---|---|---|---|
|`SSBTEMPLATE0`|There are two 4 byte blocks after the signature. They contain the info about number of records_amount and amount of bytes that they takes. Data type: uint32_t little endian|Records themselves in linear sequence: right after other. Possible spare space for aligning. Table with sizes of each record (int32_t little endian each cell)|Maximum size of each record is limited to 2^32/2-1|
## libessb

libessb is a ESSB implementation from ESSB developer.

It allows you to read a ESSB file (or memory area), get two arrays with sizes of each record and address of each record.
API and it's description is located in libessb.h header file.
You can also embed libessb in your project just by including libessb.c to your source code, or by including libessb.h and linking with precompiled libessb library.

### See also

Errata for existing libraries implementations:

1. libtssb can't work without POSIX. In future, if I will need in-memory support in order to get support on MCU, I will fix it
2. libessb is expecting .ssb files which generate on same platform. In other words, ssb with essb format inside generated on big endian platfor will not work on little endian platform. And vice versa. That small flaw will be fixed once I'll be interested in it.
3. Currently libessb is not support retrieving data from internet.
4. Currently libessb performs only basic checks for ESSB format correctness. Therefore, if any invalid data would be inside - it may cause crash or data corruption. Be careful with that. 

 * https://github.com/xdevelnet/tcsv2tssb - csv to TSSB converter
 * https://github.com/xdevelnet/template2essb - template to ESSB converter

### Many thanks for

 * Plekhanov Artem - https://github.com/PlekhanovA
 * NicklausBrain - https://github.com/NicklausBrain 
