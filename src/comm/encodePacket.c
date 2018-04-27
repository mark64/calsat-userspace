#include "comms.h"

/* This function does anything that we need to do before sending it over the radio 
   Such actions may include data compression or adding a CRC or FEC
   Currently, it will just copy the data to malloc'd memory so that we can place frees into all the right places without erroring during development
   The function takes in the data to prepare, its length, and a pointer reference to point the resulting wrapped data to. It returns the length of the prepared data on success and returns -1 on failure.
 */
int encodePacket(void *data, int dataLen, void **result) {
    void *newData = malloc(dataLen);
    if (!newData) {
        return -1;
    }
    *result = newData;
    return dataLen;
}


/* Same contract as above, but for decoding */
int decodePacket(void *data, int dataLen, void **result) {
    void *newData = malloc(dataLen);
    if (!newData) {
        return -1;
    }
    *result = newData;
    return dataLen;
}
