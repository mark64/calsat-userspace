#include <polysat.h>
#include "comms.h"

SequenceId sequence_number = 0;
char *satIPaddress = "127.0.0.1";
char *service_name = "commd";


int sendCommandRequestToSat(int argc, char **argv, CommandRequest *req) {
   char *argvEncoded;
   int argvEncodedLen = argvEncode(argc, argv, &argvEncoded);
   if (argvEncodedLen == -1) {
       return -1;
   }

   char data[MAX_PACKET_SIZE];
   int dataLen = packMessage(COMMAND_REQ, req, sizeof(CommandRequest),
                             argvEncoded, (uint16_t) argvEncodedLen,
                             &data, MAX_PACKET_SIZE);
   if (dataLen == -1) {
       return -1;
   }

   int result = send_to_sat(data, (uint16_t) dataLen);
   free(argvEncoded);
   return result;
}

int sendCommandToSat(int argc, char **argv) {
   CommandRequest request;
   memset(&request, 0, sizeof(request));
   request.cmdSeqId = sequence_number;
   request.cmdNumArgs = (uint8_t) argc;
   request.respFmt.numBytesPerPacket = 50;
   request.respFmt.getStderr = 1;
   request.respFmt.getStdout = 1;
   request.respFmt.compressionMethod = COMPRESSION_NONE;
   request.overwrite = 1;
   request.inOrder   = 0;
   return sendCommandRequestToSat(argc, argv, &request);
}

int main(int argc, char **argv) {
    int opt = 0;
    CommandRequest request;

    sendCommandToSat(argc-1, argv+1);
    return 0;
}

