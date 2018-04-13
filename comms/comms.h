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
/*
  
  --------   Radio Modem Information   --------- 
  Our P2400 radio modem allows for packets of 256 bytes. The UDP+IPv4 protocol requires 28 bytes, and so we have 228 bytes left for our command. The P2400 has a 32-bit CRC check and forward error correction, and retransmission settings, so we can avoid using extra CPU time to worry about packet correctness. The P2400 also contains a programmable Network ID to avoid picking up other signals.

  --------   Optional things to implement    ------------
  1. Multiple-packet commands. If we wish to have commands of longer than 200 bytes, then we would need to accept multiple packets for a single command. This would also give us the capability of decreasing the maximum packet size, thus decreasing the probability that any particular packet gets corrupted. However, I cannot tell if this would actually create a benifit as the radio modem does forward error correction and so it might actually increase overhead. There are no drawbacks of this besides code complexity and it would take up packet bytes. If we do not recieve all packets within an acceptable time-frame, we will abort that process. 
  2. Timed commands. Not sure if we need this really, but it is something to consider. We would need a command scheduler on-board, and this would take up more space in the packet.
  3. Space craft ID. Other people are doing this. It would be useful to make sure we don't unpack and use some data from someone else accidentally; however, our P2400 has a Network ID we can set to avoid interference. 
 */
#define MAX_ARG_NUM 10
typedef uint32_t SequenceId;


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


typedef struct commandRequest {
    SequenceId cmdSeqId;
    uint16_t cmdLength; // After this struct is sent, there is cmdLen more bytes in the packet which correspond to a variable length command
    uint8_t  cmdNumArgs;
} __attribute__((packed)) CommandRequest;
#define MAX_CMD_SIZE (MAX_PACKET_SIZE - sizeof(CommandRequest) - sizeof(uint8_t))

typedef struct commandResultRequest {
    SequenceId cmdSeqId;
} __attribute__((packed)) CommandResultRequest;


typedef struct commandResultDeleteRequest {
    SequenceId cmdSeqId;
} __attribute__((packed)) CommandResultDeleteRequest;


typedef struct statusRequest {
    int idk;
} __attribute__((packed)) StatusRequest;


#endif
