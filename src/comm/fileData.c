#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct fileData {
    long num_bytes;
    FILE *fp;
} FileData;

// Open File, this will also get the file size
FileData *openFile(char *filename) {
    FILE *fp;
    if ((fp = fopen(filename, "rb")) == NULL) {
        return NULL;
    }
    // Go to end of file
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    // Fetch the number of bytes
    long num_bytes;
    if ((num_bytes = ftell(fp)) == -1) {
        fclose(fp);
        return NULL;
    }
    // Go back to start of file
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    // Package up our data
    FileData *fdata = malloc(sizeof(FileData));
    fdata->num_bytes = num_bytes;
    fdata->fp = fp;
    return fdata;
}

// Close File, this will delete file and the memory for the struct
void closeFile(FileData *fdata) {
    if (fdata->fp != NULL) {
        fclose(fdata->fp);
    }
    free(fdata);
}

// Get File Packet #, int getFileData(long startByte, long endByte, FileData file, void *buffer);
// This function should only really be called in sequence
// Optionally, inputting -1 into startByte will just start the read from wherever the fp is
size_t getFileData(long startByte, size_t numBytes, FileData *fdata, void *buffer) {
    FILE *fp = fdata->fp;

    if (fdata->num_bytes == 0) {
        return 0;  // File has zero size, so we read nothing
    }
    if (fp == NULL) {
        return 0;
    }
    
    if (startByte >= 0) {
      if (fseek(fp, startByte, SEEK_SET) != 0) {
        return 0;
      }
    }

    return fread(buffer, 1, numBytes, fp);
} 

int testPrintFile(char *filename) {
    FileData *ptr = openFile(filename);
    printf("%s is %li bytes long\n", filename, ptr->num_bytes);

    char buffer[256];
    memset(buffer, 0, 256);
    size_t size;
    long byteCounter = 0;
    while ((size = getFileData(byteCounter, 255, ptr, buffer)) > 0) {
        printf("%.*s", (int) size, buffer);
        byteCounter += 2*255;
    }

    closeFile(ptr);
    return 0;
}
/*
int main(int argc, char **argv) {
    testPrintFile(argv[1]);
}
 */
