#include "comms.h"

/* This function packs up the entire request into a single point in memory and does error checking to make sure the packet length does not exceed the maximum.
   It returns the size of the data, and sets resultPointer to the malloc'd memory which needs to be freed by the caller.
   It does NOT add any CRC or error correction. This is done just before sending a packet and just after recieving one. 
   - The first byte of data in the packet is just the MessageType
   - The next two bytes are the size of the variable data
   - The next req_struct_size bytes consist of the request_struct
   - The next var_data_size bytes consist of the var_data, the variable sized data
   This is the final function before this data gets send to the recepient
 */
int packMessage(MessageType type, void *msg_struct, uint16_t msg_struct_size,
                void *var_data, uint16_t var_data_size,
                void *buffer, size_t buffer_size) {
    uint8_t *txData, *txDataHead; // int8_t so it does byte pointer arithmetic
    
    size_t txSize = sizeof(uint8_t)  // MessageType
                  + sizeof(uint16_t) // For the length of the variable data
                  + msg_struct_size  // For the request_struct
                  + var_data_size;   // For the variable data

    if (txSize > MAX_PACKET_DATA_SIZE) {
        // NEEDS LOG MESSAGE
        fprintf(stderr, "Packet data exceeds maximum limit; Limit: %d; Actual: %zu\n",
                       MAX_PACKET_DATA_SIZE, txSize);
        return -1;
    }
    txData = (uint8_t *) buffer;
    if (buffer_size < txSize) {
        // LOG
        fprintf(stderr, "Packet data size exceeds buffer size; BufSize: %zu; Actual: %zu\n",
                buffer_size, txSize);
        return -1;
    }
    if (!txData) {
        // NEEDS LOG MESSAGE "Could not allocate memory for request"
        fprintf(stderr, "Could not allocate memory for request");
        return -1;
    }
    // Let's start populating the memory with our request!
    txDataHead = txData;                        // Save head for return
    txData[0] = (uint8_t) type;
    txData += sizeof(uint8_t);                          // We have used the first byte of data
    
    memcpy(txData, &var_data_size, sizeof(uint16_t));   // Copy over the req_struct_size
    txData += sizeof(uint16_t);                         // uint16_t from previous line, char*

    memcpy(txData, msg_struct, msg_struct_size);
    txData += msg_struct_size;

    if (var_data_size > 0) {                            // Only if we have variable sized data
        memcpy(txData, var_data, var_data_size);        // Variable length data
        // No increment because we are done packing up our data
    }

    // Save the results and return success
    return (int) txSize;
}

/*
int stripBytes(uint8_t **data_src, uint8_t *data_dst, int amount, int *remaining_byte_count) {
    if (*remaining_byte_count < amount) {
        return -1;
    }
    memcpy(data_dst, *data_src, amount);
    *data_src += amount;
    *remaining_byte_count -= amount;
    return 0;
}*/


/* This function follows similar semantics to packMessage,
   The first byte of data, the MessageType, should NOT be a part of the data; it needs to be stripped beforehand
   Caller should NOT free any data, as the data is pointed to and not malloc'd
   Thus the Caller should likely not mutate any pointers provided by this function, as those pointers refer to the original data
   Returns the size of the variable length data on success, and -1 if error.
 */
int unpackMessage(void *data, size_t dataLen,
                  void **msg_struct, size_t msg_struct_size,
                  void **variableData) {
    uint8_t *byteData = (uint8_t *) data;
    uint8_t *endOfData = byteData + dataLen;    // This is just outside our allocated memory and should never be dereferenced

    
    // Let's get the size of the variable data
    if (byteData + sizeof(uint16_t) > endOfData) {               // Check that we have enough memory for the length of the variable-sized-data
        return -1;
    }
    uint16_t var_data_size = 0;
    memcpy(&var_data_size, byteData, sizeof(uint16_t));           // copies two bytes to our var_data_size variable
    byteData += sizeof(uint16_t);                                 // Move the data pointer
    
    if (var_data_size < 0) {                                      // Check to make sure the claimed size of the data isn't negative; if it's too large it'll get caught later
        return -1;
    }
    
    // Let's get the struct data
    if (byteData + msg_struct_size > endOfData) {                 // We check to make sure there exists enough data left
        return -1;
    }
    *msg_struct = byteData;                                       // Point the argument to this point in the data
    byteData += msg_struct_size;                                  // Move up the data pointer
    
    // Let's get the variable-sized-data
    if (byteData + var_data_size > endOfData) {                    // Check that we have enough data left to actually account for the var_data
        return -1;
    }
    *variableData = byteData;                                     // Point the argument to this point in the data
    
    // We can return the size of the variable Data
    return var_data_size;
}


/* This is a simple test */

struct TEST_PACK_STRUCT {
    int x;
    uint16_t y;
    int z;
    char a;
} __attribute__((packed));

int test_pack_unpack() {
    // Let's just populate up some data  
    struct TEST_PACK_STRUCT sIn;
    sIn.x = 0x12345678;
    sIn.y = 0xABCD;
    sIn.z = 0xBAD000AF;
    sIn.a = 'y';
    MessageType typeIn = COMMAND_REQ;
    char *varData = "jnhbvgcfdxfchjvkjnklnjmb vcdfxdfhtgyhuknjbhvtgybh   ";
    
    // Let's pack it up
    void *data = malloc(300);
    int dataSize  = packMessage(typeIn, &sIn, sizeof(sIn), varData, (uint16_t) strlen(varData) + 1, data, 300);
    if (dataSize == -1) {
        fprintf(stderr, "Failure to pack");
        return -1;
    }

    // Let's set up some variables to unpack it with
    struct TEST_PACK_STRUCT *sOut;
    void *unpackedVarData;
    // Plus one minus one so that we ignore the first byte
    int varDataLen = unpackMessage(data + 1, dataSize - 1, (void **) &sOut, sizeof(*sOut), &unpackedVarData);
    if (varDataLen == -1) {
        fprintf(stderr, "Failure to unpack");
        return -1;
    }
    uint8_t typeOut = *((uint8_t*) data);
    fprintf(stderr, "%d\n", varDataLen);
    fprintf(stderr, "%d:%d, %d:%d, %d:%d, %c:%c\n", sIn.x, sOut->x, sIn.y, sOut->y, sIn.z, sOut->z, sIn.a, sOut->a);
    fprintf(stderr, "%d:%d\n", typeIn, typeOut);
    fprintf(stderr, "expected: %s\n", varData);
    fprintf(stderr, "and got : %s\n", (char *) unpackedVarData);
    free(data);
    return 0;
}

/*
int main() {
    test_pack_unpack();
}
*/




