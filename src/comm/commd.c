#include <polysat.h>
#include "comms.h"

SequenceId sequence_number = 0;
ProcessData *processData;


int daemonize() {
    return 0;
}


int sendCmdResponse(int socket, struct sockaddr_in *src, SequenceId seqId) {
    
}


int execute_and_wait_for_command(char **argv, int stdoutfd, int stderrfd) {
    pid_t pid = fork();
    if (pid == -1) {
        exit(6);
    }

    if (pid == 0) {
        dup2(stdoutfd, STDOUT_FILENO);
        dup2(stderrfd, STDERR_FILENO);
        close(stdoutfd);
        close(stderrfd);
        
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

/* Mallocs space for the command result file path returns a pointer to the filename string */
char *resultFilename(SequenceId id, char *filenameFormat) {
    int size     = strlen(filenameFormat) + 10 + 1; // The largest integer possible in hex is 8 digits. One extra for null byte.
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

    // Open output files 
    int stdoutfd = open(stdoutFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    int stderrfd = open(stderrFilename, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
    // Execute command
    execute_and_wait_for_command(argv, stdoutfd, stderrfd);
    close(stdoutfd);
    close(stderrfd);
    
    // Send the command Response
    sendCmdResponse(socket, src, request->cmdSeqId);

    free(argv);
    return 0;
}

void handle_cmd_result_request(int socket, unsigned char cmd, 
                             void *data, size_t dataLen, struct sockaddr_in *src) {
    printf("handling cmd result request\n");
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

