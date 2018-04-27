#include <polysat.h>
#include "comms.h"

// We always start out at a sequence number of 1024.
#define SEQUENCE_NUMBER_START 1024
SequenceId sequence_number = SEQUENCE_NUMBER_START;
ProcessData *processData;


ClientData *make_client_data(int socket, struct sockaddr_in *src) {
    ClientData *cli = malloc(sizeof(ClientData));
    cli->socket = socket;
    cli->src = src;
    return cli;
}


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
    char *filename = malloc(sizeof(char) * size);
    if (filename == NULL) {
        DBG_print(DBG_LEVEL_WARN, "Could not malloc for cmdRes filename.\n");
        return NULL;
    }
    sprintf(filename, filenameFormat, id);
    return filename;
}

/* Helper functions to isolate how the filenames are determined for stderr and stdout results */
char *resultStdoutFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.out"); // Saves with id as hex
}
char *resultStderrFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.err");
}
char *resultInfoFilename(SequenceId id) {
    return resultFilename(id, "cmdresults/%x.info");
}


int sendError(MessageType type, SequenceId id, ErrorType err, ClientData *cli) {
    fprintf(stderr, "SENDING ERROR %d\n", err);
}


long roundUpDivision(long dividend, int divisor) {
    int extra = dividend % divisor > 0 ? 1 : 0;
    return dividend / divisor + extra;
}


// This is the workhorse for sending any files back to the ground
// It supports sending multiple files back in the same stream of packets so that tiny files may be sent efficiently
// It takes all the files requested, accumulates them, and sends the packets requested associated with them
int sendCommandResultFiles(char **files, const size_t numFiles,
                            const PacketNumber *packetsToSend, PacketNumber numPacketsToSend, CommandResultRequest *req,
                            ClientData *cli) {
    int numBytesPerPacket = req->respFmt.numBytesPerPacket;
    if (numBytesPerPacket <= 0) {
        // LOG
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
        sendError((MessageType) CMD_RES_REQ, req->cmdSeqId, ERR_CMD_RES_NOT_EXIST, cli);
        // Need to free stuff; LOG
        return -1;
    }

    if (numBytesPerPacket > MAX_PACKET_DATA_SIZE) {
        // LOG
        return -1;
    }

    PacketNumber totalNumberOfPacketsInFiles = (PacketNumber) roundUpDivision(totalBytesInFiles, numBytesPerPacket);
    // Now we have all of the needed fileData and know the total size of our files, so we can start chunking and sending!
    // If we were to separate the two cases of "Send All Packets" and "Send Specific Packets" into two separate loops,
    //      then there would be a lot of repeated code which may do lots of error checking. To avoid this,
    //      there will be a flag set which signals whether the looping variable should be considered a packetId, or an index for packetsToSend
    // My apologies if this is confusing; it will make the code much simpler.
    int sendingAllFlag = 0; // By default the flag is off
    if (numPacketsToSend == 0) { // This means we should send all the data
        // Set the flag, since we have been directed to send all the data
        sendingAllFlag = 1;
        // This is the actual amount of packets we are going to send
        numPacketsToSend = totalNumberOfPacketsInFiles;
    }

    // This is our buffer for file reading
    char buffer[MAX_PACKET_DATA_SIZE+1]; // Plus one to protect against segfault on off-by-one errors
    memset(buffer, 0, MAX_PACKET_DATA_SIZE+1);

    // Let's create the basic packet struct and the buffer for our packets so that it may be sent repeatedly in the loop
    CommandResultPacket *crp = calloc(1, sizeof(CommandResultPacket));
    crp->numPackets = totalNumberOfPacketsInFiles;
    crp->packetNumber = 0; // THIS IS SET FOR EACH PACKET IN THE LOOP
    crp->fmt = req->respFmt;
    crp->cmdSeqId = req->cmdSeqId;
    size_t packetBufferSize = MAX_PACKET_SIZE;
    void *packetBuffer = calloc(1, packetBufferSize);

    int currFileIndex = 0;
    int bytesPassedInPrevFiles = 0;
    for (int i = 0; i < numPacketsToSend; i++) {
        int packetIndex = sendingAllFlag ? i : packetsToSend[i];
        // - Seek currFileIndex to correct file containing the start of our packet
        // - Read into buffer numBytesPerPacket from our file, overflowing into the next file if necessary
        // - Update our crp and send a packet
        
        // SEEKING
        int startByte = packetIndex * numBytesPerPacket;
        while (currFileIndex < numFiles) {
            if (bytesPassedInPrevFiles <= startByte && startByte < bytesPassedInPrevFiles + resultFilesData[currFileIndex]->num_bytes) {
                break;
            }
            bytesPassedInPrevFiles += resultFilesData[currFileIndex]->num_bytes;
            currFileIndex++;
        }
        if (currFileIndex == numFiles) {
            // LOG, the requested packetIndex is not contained within our files
            fprintf(stderr, "the requested packetIndex is not contained within our files\n");
            break;
        }
        fprintf(stderr, "Finished seek step for packetIndex %d\n", packetIndex);
        // Now currFileIndex should be pointing to a file which contains our desired data
        // READING
        int bytesRead = 0;
        while (bytesRead < numBytesPerPacket && currFileIndex < numFiles) {
            // readFileData(position_in_current_file_to_read, bytes_to_read, file_data, buffer)
            int numRead = readFileData(startByte + bytesRead - bytesPassedInPrevFiles, numBytesPerPacket - bytesRead, resultFilesData[currFileIndex], &buffer + bytesRead);
            if (numRead < numBytesPerPacket - bytesRead) {
                // We did not read everything we expected to, so the file has ended, we must move onto the next file
                bytesPassedInPrevFiles += resultFilesData[currFileIndex]->num_bytes;
                currFileIndex += 1;
            }
            bytesRead += numRead;
        }
        if (currFileIndex == numFiles) {
            // We have bottomed out of files! this is to be expected and OK when it happens
        }
        fprintf(stderr, "Finished reading packetIndex %d, numBytes %d, with data:\n%.*s\n", packetIndex, bytesRead, bytesRead, buffer);
        // UPDATE CRP AND SEND PACKET
        crp->packetNumber = (uint16_t) packetIndex;
        int packetSize = packMessage(CMD_RES_RESP, crp, sizeof(*crp), &buffer, bytesRead, packetBuffer, packetBufferSize);
        if (send_to_client(packetBuffer, packetSize, cli) != 0) {
            fprintf(stderr, "Send to client failure\n");
            return -1;
        }
        fprintf(stderr, "Sent packet successfully\n");
    }
    return 0;
/*
    int fileIndex = 0; // This will keep track of which file we are gathering data from currently.
    long bytesPassedInPrevFiles = 0; // This is the number of bytes total which come from previous files
    long bytesReadFromCurrentFile = 0;
    for (int i = 0; i < numPacketsToSend; i++) {
        int packetIndex = sendingAllFlag ? i : packetsToSend[i];        // See above large block comment
        int startByte = packetIndex * numBytesPerPacket;                // Get where our packetData should start
        size_t bytesReadCurrPacket = 0;
        // Let's first seek to the correct file (first file where bytesPassedInPrevFiles <= startByte < bytesPassedInPrevFiles + numbytes(currFile))
        for (; fileIndex < numFiles;) {
            if (bytesPassedInPrevFiles <= startByte && startByte < bytesPassedInPrevFiles + resultFilesData[fileIndex]->num_bytes) {
                break;
            }
            bytesPassInPrevFiles += resultFilesData[fileIndex]->num_bytes;
            fileIndex++;
        }
        // The requested packet is outside of the files that exist
        if (fileIndex == numFiles) {
            return -1;
        }
        // We're at the correct file
        while (bytesReadCurrPacket < numBytesPerPacket && i < numFiles) {
            // Get as many bytes as we can get from our current file
            size_t numBytesToRead = numBytesPerPacket - bytesReadCurrPacket;
            int bytesRead = readFileData(startByte + bytesReadCurrPacket - bytesPassedInPrevFiles, numBytesToRead, resultFilesData[i], &buffer + bytesRead);
            bytesReadCurrPacket += bytesRead;
            bytesReadFromCurrentFile += bytesRead;
            fprintf(stderr, "Loaded in %d bytes from file\n", bytesRead);
            if (bytesRead < numBytesToRead) { // Current file is over, move on to next file
                fprintf(stderr, "Moving on to next file");
                fileIndex += 1;
                bytesPassedInPrevFiles += bytesReadFromCurrentFile;
                bytesReadFromCurrentFile = 0;
            }
        }
        fprintf(stderr, "Sending packetIndex %d\n With data %.*s\n", packetIndex, MAX_PACKET_DATA_SIZE, buffer);
        // Now we can send the buffered data
        crp->packetNumber = (uint16_t) packetIndex;
        packMessage(CMD_RES_RESP, crp, sizeof(*crp), &buffer, bytesReadCurrPacket, packetBuffer, packetBufferSize);
        if (send_to_client(packetBuffer, packetBufferSize, cli) != 0) {
            fprintf(stderr, "Send to client failure\n");
            return -1;
        }
    }
    return 0;*/
}



int sendCmdResults(CommandResultRequest *req,
                   const PacketNumber *packetsToSend, PacketNumber numPacketsToSend,
                   ClientData *cli) {
    // Let's queue up the desired output file names
    // These are possible files we could use, and functions that will generate their filenames
    uint8_t shouldUse[] = {req->respFmt.getStdout, req->respFmt.getStderr};
    char *(*func_ptr[3])(SequenceId) = {resultStdoutFilename, resultStderrFilename};
    const size_t maxNumFiles = 2; // Right now the possible files we can return are stdout and stderr; maybe we'll add on a log file of CommandRequestData?
    char *resultFiles[maxNumFiles];      // These will hold the names of the files we wish to respond with
    memset(resultFiles, 0, maxNumFiles); // Initialize to NULL
    size_t filesToSend = 0;                       // Counter for number of files we're actually using
    fprintf(stderr, "Beginning sendCmdResults\n");
    for (int i = 0; i < maxNumFiles; i++) {
        if (!shouldUse[i]) continue; // We didn't request this, so don't add thefilename to the queue
        if ((resultFiles[filesToSend] = func_ptr[i](req->cmdSeqId)) == NULL) {
            // Malloc failed, we must go free all the malloc's that succeeded
            for (int k = 0; k < filesToSend; k++) {
                free(resultFiles[k]);
            }
            fprintf(stderr, "Failed when attepting to get %d", i);
            return -1;
        } else {
            fprintf(stderr, "Added file %s to the queue\n", resultFiles[filesToSend]);
            filesToSend++;
        }
    }
    fprintf(stderr, "Done queuing filenames\n");
    if (filesToSend > 0) {
        return sendCommandResultFiles(resultFiles, filesToSend, packetsToSend, numPacketsToSend, req, cli);
    }
    return 0;
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
    ClientData *cli = make_client_data(socket, src);
    CommandRequest *request;
    char *cmdStr, **argv;
    fprintf(stderr, "Handling command execution\n");
    // Points request and cmdStr to the proper parts of data
    int cmdStrLen = unpackMessage(data, dataLen, (void **) &request, sizeof(CommandRequest), (void **) &cmdStr);
    if (cmdStrLen == -1) {
        return -1;
    }

    // Turns the cmdStr into the format execvp would expect
    argv = argvDecode(cmdStr, cmdStrLen, request->cmdNumArgs);
    fprintf(stderr, "Decoded argv\n");
    DBG_print(DBG_LEVEL_INFO, "Command to execute: %s\n", cmdStr);

    // The remainder of the function just opens logs, executes command, and sends the result back to the ground upon finishing
    char *stdoutFilename = resultStdoutFilename(request->cmdSeqId);
    char *stderrFilename = resultStderrFilename(request->cmdSeqId);

    // Open output files. If either exists already, then we must exit.
    // O_EXCL is an atomic open & create if it doesn't exist. We use this to avoid multiple executions of cmds with the same id
    int stdoutfd = open(stdoutFilename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (stdoutfd < 0) {
        sendError((MessageType) cmd, request->cmdSeqId, ERR_CMD_RES_FILE_EXISTS, cli);
        return -1;
    }
    int stderrfd = open(stderrFilename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    if (stderrfd < 0) {
        sendError((MessageType) cmd, request->cmdSeqId, ERR_CMD_RES_FILE_EXISTS, cli);
        return -1;
    }

    fprintf(stderr, "Opened files\n");
    // OK, just because there are no files with our name, it does not mean we're free to execute yet!
    // If the command request desires sequential execution, then its id must match the current sequence_number
    if (request->inOrder) {
        SequenceId currentIfFailure = incSequenceNumberIfEqual(request->cmdSeqId);
        if (currentIfFailure != 0) { // sequence number mismatch
            sendError((MessageType) cmd, request->cmdSeqId, ERR_SEQUENCE_MISMATCH, cli);
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
    resp.respFmt             = request->respFmt;
    resp.numRequestedPackets = 0; // Send all the packets
    // Send the command Response
    sendCmdResults(&resp, NULL, 0, cli);

    free(stdoutFilename);
    free(stderrFilename);
    free(argv);
    return 0;
}

int handle_cmd_result_request(int socket, unsigned char cmd,
                              void *data, size_t dataLen, struct sockaddr_in *src) {
    ClientData *cli = make_client_data(socket, src);
    printf("handling cmd result request\n");
    CommandResultRequest *request;
    PacketNumber *packets;

    int varDataBytes = unpackMessage(data, dataLen, (void **) &request, sizeof(CommandRequest), (void **) &packets);
    if (varDataBytes == -1) { // On error
        return -1;
    }
    PacketNumber numRequestedPackets = varDataBytes / sizeof(PacketNumber);
    if (request->numRequestedPackets != numRequestedPackets) { // A sanity check that the number of packets requested is the same as the sender says
        return -1;
    }
    int success = sendCmdResults(request, packets, request->numRequestedPackets, cli);
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

