#ifndef MUFILEPICKER_H
#define MUFILEPICKER_H
#include "f256lib.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_FILES 100
#define MAX_FILENAME_LEN 44
#define MAX_PATH_LEN 32
#define MAX_FILE_EXTS 4
#define EXT_LEN 3
#define MAX_VISIBLE_FILES 12
#define RESERVED_ENTRY_INDEX 0

// Main picker record
typedef struct {
	uint8_t tlX, tlY;									   // top left x and y for the directory listing printout
    char currentPath[MAX_PATH_LEN];                        // Current directory
    char selectedFile[MAX_FILENAME_LEN];                   // Chosen file name
    char fileList[MAX_FILES][MAX_FILENAME_LEN];            // Visible file list
	bool isDirList[MAX_FILES];
    int fileCount;                                         // Number of files in list
    int cursorIndex;                                       // Currently highlighted index
    int visualIndex;                                       // bounded by MAX_VISIBLE_FILES, 0 = always '../' to backtrack
    int scrollOffset;                                      // Scroll offset for visual rendering
    char fileExts[MAX_FILE_EXTS][EXT_LEN];                 // Allowed file extensions
} filePickRecord;

// Core API
uint8_t pickFile(filePickRecord *picker);
uint8_t filePickModal(filePickRecord *picker, uint8_t, uint8_t, char *, char *, char *, char *, bool);
void reprepFPR(filePickRecord *, bool);

void backUpDirectory(filePickRecord *picker);
void initFilePickRecord(filePickRecord *picker, uint8_t, uint8_t, bool);
void readDirectory(filePickRecord *picker);
bool isExtensionAllowed(filePickRecord *picker, const char *filename);
void sortFileList(filePickRecord *picker);
void displayFileList(filePickRecord *picker, int scrollOffset);
void wipeArea(filePickRecord *picker);

#endif // MUFILEPICKER_H
