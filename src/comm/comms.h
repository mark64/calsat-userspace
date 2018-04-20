#ifndef COMMS_H
#define COMMS_H

#include <sys/types.h>                                                                               
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#define MAX_PACKET_SIZE 255
#define DATA_HANDLE_SIZE 0
#define MAX_PACKET_DATA_SIZE (MAX_PACKET_SIZE - DATA_HANDLE_SIZE)
/*
  --------   Radio Modem Information   --------- 
  Our P2400 radio modem allows for packets of 256 bytes. The UDP+IPv4 protocol requires 28 bytes, and so we have 228 bytes left for our command. The P2400 has a 32-bit CRC check and forward error correction, and retransmission settings, so we can avoid using extra CPU time to worry about packet correctness. The P2400 also contains a programmable Network ID to avoid picking up other signals.

  --------   Optional things to implement    ------------
  1. Multiple-packet commands. If we wish to have commands of longer than 200 bytes, then we would need to accept multiple packets for a single command. This would also give us the capability of decreasing the maximum packet size, thus decreasing the probability that any particular packet gets corrupted. However, I cannot tell if this would actually create a benifit as the radio modem does forward error correction and so it might actually increase overhead. There are no drawbacks of this besides code complexity and it would take up packet bytes. If we do not recieve all packets within an acceptable time-frame, we will abort that process. 
  2. Timed commands. Not sure if we need this really, but it is something to consider. We would need a command scheduler on-board, and this would take up more space in the packet.
  3. Space craft ID. Other people are doing this. It would be useful to make sure we don't unpack and use some data from someone else accidentally; however, our P2400 has a Network ID we can set to avoid interference. 
 */
typedef uint32_t SequenceId;
typedef uint16_t PacketNumber;

typedef enum errorType {
    ERR_CMD_RES_FILE_EXISTS,
    ERR_SEQUENCE_MISMATCH,
    ERR_CMD_RES_NOT_EXIST
} ErrorType;

typedef enum compressionMethod {
    COMPRESSION_NONE = 0,
    COMPRESSION_ZLIB = 1,
} CompressionMethod;

#define PATH_COMMAND_OUTPUT_FILE "/home/jtstog/stac/calsat-userspace/src/comm/cmdresults/"
typedef enum FileBasePath {
    PATH_COMMAND_OUTPUT_FILE_SIG,
};

/* These are ordered in the same way that they are ordered in the config file for polysat
   Each one corresponds to a handler that will be called on the cubeSat
   Each one has a corresponding struct below as well which constitutes all the data passed to the cubesat when using the corresponding handler
  */
typedef enum requestType {
    INVALID_REQ,
    COMMAND_REQ,        // Command, which will include a shell command string
    CMD_RES_REQ,    // Command result request
    RESULT_DEL_REQ,    // Command result acknowledgement
    STATUS_REQ,
} RequestType;

/* These settings, along with the Command ID, will determine essentially exactly which byte is in which packet
 * This is because the files will remain the same size, and so the bytesPerPacket+getStdout+getStderr determine
 * how the command output is split across packets. This allows us to use SNACKS while
 * keeping track of a minimal number of things
 * */
typedef struct cmdResultResponseFmt {
    uint8_t             getStdout;
    uint8_t             getStderr;
} __attribute__((packed)) CmdResultResponseFmt;

/* The following packed structures are to be sent across the radio and each determine a single packet */
typedef struct commandRequest {
    SequenceId           cmdSeqId;
    uint8_t              cmdNumArgs;
    uint32_t             delay;   // In milliseconds
    uint16_t             timeout; // In seconds
    uint8_t              overwrite; // Overwrite old files with same cmdSeqId
    uint8_t              inOrder; // Only run command if it aligns with the current sequence number
    // The following are for the response that will be sent
    CmdResultResponseFmt responseFmt;
    uint8_t              padding;
} __attribute__((packed)) CommandRequest;


typedef struct commandResultRequest {
    SequenceId           cmdSeqId;
    CmdResultResponseFmt fmt;
    // The actual packets selected for retransmission are in the variable data section of our packet.
    // numRequestedPackets details how many to expect, and is 0 if we should transmit all the packets.
    PacketNumber         numRequestedPackets;
    uint8_t              padding;
} __attribute__((packed)) CommandResultRequest;

typedef struct commandResultDeleteRequest {
    SequenceId cmdSeqId;
} __attribute__((packed)) CommandResultDeleteRequest;

typedef struct statusRequest {
    int idk;
} __attribute__((packed)) StatusRequest;

typedef struct commandResultPacket {
    SequenceId           cmdSeqId;
    CmdResultResponseFmt fmt;
    PacketNumber         packetNumber;      // 0 indexed packet number
    PacketNumber         numPackets;        // Total number of packets being sent for this cmdId and these fmt settings
    uint8_t              padding;
} __attribute__((packed)) CommandResultPacket;

typedef struct fileContentRequest {
    int   requestId;

};

typedef struct fileContentPacket {
    int                   requestId;
    PacketNumber          packetNumber;       // 0 indexed packet number
    PacketNumber          numPackets;         // Total number of packets that compose the file being sent
    uint16_t              bytesPerPacket;     // Variable amount of bytes per response packet
    CompressionMethod     compressionMethod;
} __attribute__((packed)) FileContentPacket;
/* Function Declarations */
int send_to_sockaddr(char *data, uint16_t dataLen, struct sockaddr_in *addr);
struct sockaddr_in *make_sockaddr(char *ip_address, int port);
int send_to_sat(char *data, uint16_t dataLen);
int send_to_ground(char *data, uint16_t dataLen, struct sockaddr_in *addr);

int encodePacket(void *data, int dataLen, void **result);
int decodePacket(void *data, int dataLen, void **result);

int argvEncode(int argc, char **argv, char **result);
char **argvDecode(char *str, int strLen, int argc);

int packRequest(RequestType type, void *request_struct, uint16_t req_struct_size, 
                                  void *var_data,       uint16_t var_data_size,
                                  void **resultPointer);
int unpackRequest(void *data, size_t dataLen,
                  void **request_struct, size_t request_struct_size,
                  void **variableData);

typedef struct fileData {
    long num_bytes;
    FILE *fp;
} FileData;

FileData *openFile(char *filename);
void closeFile(FileData *fdata);
int getFileData(long startByte, size_t numBytes, FileData *fdata, void *buffer);

#endif
