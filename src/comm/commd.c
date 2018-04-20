#include <polysat.h>
#include "comms.h"

// We always start out at a sequence number of 1024.
#define SEQUENCE_NUMBER_START 1024
SequenceId sequence_number = SEQUENCE_NUMBER_START;
ProcessData *processData;


/* Returns 0 if the sequence_number is equal to input number, and increments sequence_number
 * Returns the current sequence_number at the time of execution attempt otherwise
 * As a separate function so it's easier to make into an atomic operation in case we add threading */
SequenceId incSequenceNumberIfEqual(SequenceId cmdId) {
    // Lock
    if (cmdId == sequence_number) {
        sequence_number += 1;
        return 0;
    }
    SequenceId seq_number = sequence_number;
    // Unlock
    return seq_number;
}


int daemonize() {
    return 0;
}

/* Mallocs space for the command result file path returns a pointer to the filename string */
char *resultFilename(SequenceId id, char *filenameFormat) {
    size_t size     = strlen(filenameFormat) + 10 + 1; // The largest integer possible in hex is 8 digits. One extra for null byte.
    char *stdoutFilename = malloc(sizeof(char) * size);
    if (stdoutFilename == 0) {
        DBG_print(DBG_LEVEL_WARN, "Could not malloc for cmdRes filename.\n");
        return NULL;
    }
    sprintf(stdoutFilename, filenameFormat, id);
    return stdoutFilename;
}

/* Helper functions to isolate how the filenames are determined for stderr and stdout results */
char *resultStdoutFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.out"); // Saves with id as hex
}
char *resultStderrFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.err");
}
char *resultStdinfoFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.info");
}


int sendError(RequestType type, SequenceId id, ErrorType err, int socket, struct sockaddr_in *src) {


}


long roundUpDivision(long dividend, int divisor) {
    int extra = dividend % divisor > 0 ? 1 : 0;
    return dividend / divisor + extra;
}


// This is the workhorse for sending any files back to the ground
// It supports sending multiple files back in the same stream of packets so that tiny files may be sent efficiently
// It takes all the files requested, accumulates them, and sends the packets requested associated with them
int sendMultipleFileContents(char **files, const size_t numFiles,
                             const PacketNumber *packetsToSend, PacketNumber numPacketsToSend, FileContentPacket *fcp,
                             int socket, struct sockaddr_in *src) {
    if (fcp->bytesPerPacket <= 0) {
        // LOG this is ridiculous, what do you want from us sender?
        // This would cause divByZero later, might as well check for it now
        return -1;
    }

    if (files == NULL) {
        // LOG
        return -1;
    }

    // Let's actually open the files and check to see if they exist.
    // If any particular one doesn't exist, we will continue; however, if none exist, we will report an error!
    int atLeastOneExists = 0;
    FileData *resultFilesData[numFiles];
    long totalBytesInFiles = 0;
    for (int i = 0; i < numFiles; i++) {
        if (NULL == files[i]) continue;
        resultFilesData[i] = openFile(files[i]);
        if (resultFilesData[i] != NULL) { // Successfully opened the file and read it's length
            totalBytesInFiles += resultFilesData[i]->num_bytes;
            atLeastOneExists = 1;
        } else {
            // LOG
            fprintf(stderr, "Could not open file titled %s\n", files[i]);
        }
    }

    if (!atLeastOneExists) { // This means that none of the requested files exist
        sendError((RequestType) CMD_RES_REQ, fcp->cmdSeqId, ERR_CMD_RES_NOT_EXIST, socket, src);
        // Need to free stuff; LOG
        return -1;
    }

    if (fcp->bytesPerPacket > 256) {
        // LOG
        return -1;
    }
    char buffer[256+1]; // Plus one to protect against segfault on off-by-one errors
    memset(buffer, 0, 256+1);
    PacketNumber totalNumberOfPacketsInFiles = (PacketNumber) roundUpDivision(totalBytesInFiles, fcp->bytesPerPacket);
    fcp->numPackets = totalNumberOfPacketsInFiles;
    // Now we have all of the needed fileData and know the total size of our files, so we can start chunking and sending!
    // If we were to separate the two cases of "Send All Packets" and "Send Specific Packets" into two separate loops,
    // then there would be a lot of repeated code which may do lots of error checking. To avoid this,
    // there will be a flag set which signals whether the looping variable should be considered a packetId, or an index for packetsToSend
    // My apologies if this is confusing; it will make the code much simpler.
    int sendingAllFlag = 0; // By default the flag is off
    if (numPacketsToSend == 0) { // This means we should send all the data
        // This is the actual amount of packets we are going to send
        numPacketsToSend = totalNumberOfPacketsInFiles;
        // Set the flag, since we have been directed to send all the data
        sendingAllFlag = 1;
    }
    int fileIndex = 0; // This will keep track of which file we are gathering data from currently.
    long bytesAlreadyPassedInPreviousFiles = 0;
    for (int i = 0; i < numPacketsToSend; i++) {
        int packetIndex = sendingAllFlag ? i : packetsToSend[i]; // See, wasn't too bad I hope
        int startByte = packetIndex * fcp->bytesPerPacket; // Get where our packetData should start
        size_t bytesRead = 0;
        while (bytesRead < fcp->bytesPerPacket && i < numFiles) {
            // Get as many bytes as we can get from our current file
            int bytesReadFromCurrFile = getFileData(startByte - bytesAlreadyPassedInPreviousFiles, fcp->bytesPerPacket - bytesRead, resultFilesData[i], &buffer + bytesRead);
            if (bytesReadFromCurrFile == 0) {
                i += 1;
            } else {
                bytesRead += bytesReadFromCurrFile;
            }
        }
        // Now we can send the buffered data
        void *packedPacket;
        packRequest();
    }
}

/* Sends the packets corresponding to the ids in packetsToSend (there are numPacketsToSend of them) to the requested
 * socket and sockaddr. If numPacketsToSend == 0 or packetsToSend is NULL, we will send all the data in the file. */
int sendFileContents(char *file, const PacketNumber *packetsToSend, PacketNumber numPacketsToSend, FileContentPacket *packetFormat) {

}


int sendCmdResults(CommandResultRequest *request, const PacketNumber *packetsToSend, PacketNumber numPacketsToSend,
                   int socket, struct sockaddr_in *src) {
    // Let's queue up the desired output file names
    const size_t maxNumFiles = 2; // Right now the possible files we can return are stdout and stderr; maybe we'll add on a log file of CommandRequestData?
    char *resultFiles[maxNumFiles] = {NULL}; // Initialize
    int filesToSend = 0;
    if (request->fmt.getStdout) {
        if ((resultFiles[0] = resultStdoutFilename(request->cmdSeqId)) != NULL) {
            // malloc failed...
        } else {
            filesToSend += 1;
        }
    }
    if (request->fmt.getStderr) {
        if ((resultFiles[1] = resultStderrFilename(request->cmdSeqId)) == NULL) {
            // malloc failed...
        } else {
            filesToSend += 1;
        }
    }
    if (filesToSend == 1) {
        char *file = NULL;
        for (int i = 0; file == NULL && i < maxNumFiles; i++) file = resultFiles[i];
        if (file != NULL) {
            FileContentPacket fcp = NULL;
            sendFileContents(file, packetsToSend, numPacketsToSend, &fcp);
        }
    } else if (filesToSend > 1) {
        FileContentPacket fcp = NULL;
        sendMultipleFileContents(resultFiles, maxNumFiles, packetsToSend, numPacketsToSend,)
    }
}



        int execute_and_wait_for_command(char **argv, int stdoutfd, int stderrfd) {
    pid_t pid = fork();
    if (pid == -1) {
        exit(6);
    }

    if (pid == 0) {
        if (stdoutfd != -1) {
            dup2(stdoutfd, STDOUT_FILENO);
            close(stdoutfd);
        }
        if (stderrfd != -1) { 
            dup2(stderrfd, STDERR_FILENO);
            close(stderrfd);
        } 
        
        close(STDIN_FILENO);
        
        execv(argv[0], argv);
        
        perror("execvp");
        exit(1);
    }

    int status;
    waitpid(pid, &status, 0);
    DBG_print(DBG_LEVEL_INFO, "Completed with status %d!\n", status);
    return status;
}



int handle_cmd_exec(int socket, unsigned char cmd, 
                     void *data, size_t dataLen, struct sockaddr_in *src) {    
    CommandRequest *request;
    char *cmdStr, **argv;
    // Points request and cmdStr to the proper parts of data
    int cmdStrLen = unpackRequest(data, dataLen, (void**) &request, sizeof(CommandRequest), (void**) &cmdStr);
    if (cmdStrLen == -1) { 
        return -1;
    }

    // Turns the cmdStr into the format execvp would expect
    argv = argvDecode(cmdStr, cmdStrLen, request->cmdNumArgs);

    DBG_print(DBG_LEVEL_INFO, "Command to execute: %s\n", cmdStr);
   
    // The remainder of the function just opens logs, executes command, and sends the result back to the ground upon finishing
    char *stdoutFilename = resultStdoutFilename(request->cmdSeqId);
    char *stderrFilename = resultStderrFilename(request->cmdSeqId);

    // Open output files. If either exists already, then we must exit.
    // O_EXCL is an atomic open & create if it doesn't exist. We use this to avoid multiple executions of cmds with the same id
    int stdoutfd = open(stdoutFilename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (stdoutfd < 0) {
        sendError((RequestType) cmd, request->cmdSeqId, ERR_CMD_RES_FILE_EXISTS, socket, src);
        return -1;
    }
    int stderrfd = open(stderrFilename, O_WRONLY | O_EXCL | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (stderrfd < 0) {
        sendError((RequestType) cmd, request->cmdSeqId, ERR_CMD_RES_FILE_EXISTS, socket, src);
        return -1;
    }

    // OK, just because there are no files with our name, it does not mean we're free to execute yet!
    // If the command request desires sequential execution, then its id must match the current sequence_number
    // LOCK the sequence number.
    if (request->inOrder) {
        SequenceId currentIfFailure = incSequenceNumberIfEqual(request->cmdSeqId);
        if (currentIfFailure != 0) { // Signals sequence number mismatch
            sendError((RequestType) cmd, request->cmdSeqId, ERR_SEQUENCE_MISMATCH, socket, src);
            // free things
            return -1;
        }
    }

    // At this point, we know that our command is free to execute, as there are no files
    // Execute command
    execute_and_wait_for_command(argv, stdoutfd, stderrfd);
    close(stdoutfd);
    close(stderrfd);

    // Let's build up a command response request structure
    CommandResultRequest resp;
    resp.cmdSeqId            = request->cmdSeqId;
    resp.fmt                 = request->responseFmt;
    resp.numRequestedPackets = 0; // Send all the packets
    // Send the command Response
    sendCmdResults(&resp, NULL, 0, socket, src);

    free(stdoutFilename);
    free(stderrFilename);
    free(argv);
    return 0;
}

// Returns the number of bytes sent
int sendFilesInPackets(char **files, size_t numFiles,
                       PacketNumber *packetsToSend, size_t numPacketsToSend,
                       size_t bytesPerPacket, CommandResultPacket baseDescription) {

}


int handle_cmd_result_request(int socket, unsigned char cmd,
                              void *data, size_t dataLen, struct sockaddr_in *src) {
    printf("handling cmd result request\n");
    CommandResultRequest *request;
    PacketNumber *packets;
    int varDataBytes = unpackRequest(data, dataLen, (void**) &request, sizeof(CommandRequest), (void**) &packets);
    if (varDataBytes == -1) { // On error
        return -1;
    }
    PacketNumber numRequestedPackets = varDataBytes / sizeof(PacketNumber);
    if (request->numRequestedPackets != numRequestedPackets) {
        return -1;
    }
    int success = sendCmdResults(request, packets, request->numRequestedPackets, socket, src);
}

void handle_cmd_result_delete(int socket, unsigned char cmd, 
                              void *data, size_t dataLen, struct sockaddr_in *src) {
    printf("handling cmd result delete\n");
}

void handle_status_request(int socket, unsigned char cmd,
                           void *data, size_t dataLen, struct sockaddr_in *src) {
    printf("handling status request\n");
}

int sigint_handler(int signum, void *arg) {
    fprintf(stderr, "exiting!\n");
    EVT_exit_loop(PROC_evt(arg));
    return EVENT_KEEP;
}

int main(int argc, char **argv) {
    // Become a Daemon
    if (daemonize() != 0) {
        perror("Daemon start");
        return 1;
    }
    ProcessData *proc = PROC_init("commd", WD_ENABLED);
    processData = proc;
    if (!proc) {
        return -1;
    }
    DBG_setLevel(DBG_LEVEL_INFO);
    // Setup sigint handler
    PROC_signal(proc, SIGINT, &sigint_handler, proc);
    // Start event loop
    printf("Starting event loop\n");
    EVT_start_loop(PROC_evt(proc));
    PROC_cleanup(proc);
    return 0;
}

