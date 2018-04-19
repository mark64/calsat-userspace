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

   void *data;
   int dataLen = packRequest(COMMAND_REQ, req, sizeof(CommandRequest),
                                          argvEncoded, argvEncodedLen,
                                          &data);
   if (dataLen == -1) {
       return -1;
   }

   int result = send_to_sat(data, dataLen);
   free(argvEncoded);
   free(data);
   return result;
}

int sendCommandToSat(int argc, char **argv) {
   CommandRequest request;
   memset(&request, 0, sizeof(request));
   request.cmdSeqId = sequence_number;
   request.cmdNumArgs = argc;
   return sendCommandRequestToSat(argc, argv, &request);
}

int main(int argc, char **argv) {
    argv++;
    argc--;
    sendCommandToSat(argc, argv);
    return 0;
}

