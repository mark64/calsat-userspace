#include <polysat.h>
#include "comms.h"


SequenceId sequence_number = 0;
char *satIPaddress = "127.0.0.1";
char *service_name = "commd";

int sendRequestToSat(char *data, uint16_t dataLength, RequestType type) {
    int result;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    int sockfd;
    if((sockfd = socket_init(0)) < 0) {
        return -1;
    }
    addr.sin_family = AF_INET;
    //addr.sin_addr.s_addr = 0;
    addr.sin_port = htons(socket_get_addr_by_name(service_name));
    // We must send the command type as the first byte of data for polysat library to work
    int txSize = sizeof(uint8_t) + dataLength * sizeof(char);
    char *txData = malloc(txSize);
    txData[0] = (uint8_t) type;
    memcpy(txData+1, data, dataLength);
    result = socket_write(sockfd, txData, txSize, &addr);
    free(txData);
    return result;
}

int sendCommand(char *cmd, uint16_t cmdLength, uint16_t numArgs) {
   CommandRequest request;
   memset(&request, 0, sizeof(request));

   // Load up the CommandRequest
   request.cmdLength = cmdLength;
   request.cmdSeqId = sequence_number;
   request.cmdNumArgs = numArgs;
   // The size of the command we wish to send is the size of the Request data and the command
   int dataLength = sizeof(request) + cmdLength * sizeof(char);
   // We allocate enough for the entire message to fit in one place
   char *data = malloc(dataLength);
   // Place the request in the first bit of DATA memory
   memcpy(data, &request, sizeof(request));
   // Then place the cmd string in the rest of the DATA memory

   memcpy(data+sizeof(request), cmd, cmdLength);
   // We send the data to the sat with COMMAND_REQ so it knows it's a command execution request
   int result = sendRequestToSat(data, dataLength, COMMAND_REQ);
   free(data);
   return result;
}

char *argvEncode(int argc, char **argv, int *res_size) {
    int *lengths = malloc(sizeof(int) * argc);
    int total_size = 0;
    
    for (int i = 0; i < argc; i++) {
        lengths[i] = strlen(argv[i]) + 1; // Null byte
        total_size += lengths[i];
    }
    
    if (total_size > MAX_CMD_SIZE) {
        fprintf(stderr, "Command size is %d bytes but max allowable is %d bytes", 
                total_size, (int) MAX_CMD_SIZE);
    }
    
    char *res = malloc(sizeof(char)*total_size);
    int p = 0;
    for (int i = 0; i < argc; i++) {
        strncpy(res + p, argv[i], lengths[i]);
        p += lengths[i];
    }
    free(lengths);
    
    *res_size = total_size;
    return res;
}

int main(int argc, char **argv) {
    argv++;
    argc--;
    int cmdLength;
    char *cmdStr = argvEncode(argc, argv, &cmdLength);
    fprintf(stderr, "%s %d\n", cmdStr, argc);
    sendCommand(cmdStr, cmdLength, argc);
    free(cmdStr);
    return 0;
}

