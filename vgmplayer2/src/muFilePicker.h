#ifndef MUFILEPICKER_H
#define MUFILEPICKER_H
#include "f256lib.h"
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_FILES 100
#define MAX_FILENAME_LEN 120
#define MAX_PATH_LEN 60
#define MAX_FILE_EXTS 4
#define EXT_LEN 3
#define MAX_VISIBLE_FILES 16
#define RESERVED_ENTRY_INDEX 0
#define DIRECTORY_X 0
#define DIRECTORY_Y 3
#define FPR_BASE 0x20000

#define FPR_tlX              0
#define FPR_tlY              1

#define FPR_currentPath      2
#define FPR_selectedFile     (FPR_currentPath + MAX_PATH_LEN)

#define FPR_fileList         (FPR_selectedFile + MAX_FILENAME_LEN)
#define FPR_fileListEntry(i) (FPR_fileList + ((i) * MAX_FILENAME_LEN))

#define FPR_isDirList        (FPR_fileList + (MAX_FILES * MAX_FILENAME_LEN))
#define FPR_isDir(i)         (FPR_isDirList + (i))

#define FPR_fileCount        (FPR_isDirList + MAX_FILES)
#define FPR_cursorIndex      (FPR_fileCount + 4)
#define FPR_visualIndex      (FPR_cursorIndex + 4)
#define FPR_scrollOffset     (FPR_visualIndex + 4)

#define FPR_fileExts         (FPR_scrollOffset + 4)
#define FPR_fileExt(i)       (FPR_fileExts + ((i) * 3))

#define FPR_TOTAL_SIZE       (FPR_fileExts + 12)

#define FPR_ADDR(off)   (FPR_BASE + (uint32_t)(off))


// Main picker record
/*
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
*/

static inline uint8_t fpr_get8(uint32_t);
static inline void fpr_set8(uint32_t, uint8_t);


// Core API
void fpr_set_currentPath(const char *);
void fpr_get_currentPath(char *);
void fpr_get_selectedFile(char *);

//uint8_t pickFile(filePickRecord *picker);
uint8_t pickFile_far(void);

//uint8_t filePickModal(filePickRecord *picker, uint8_t, uint8_t, char *, char *, char *, char *, bool);
uint8_t filePickModal_far(uint8_t, uint8_t,
                          char *, char *, char *, char *,
                          bool);
//void reprepFPR(filePickRecord *, bool);
void reprepFPR_far(bool);
//void initFilePickRecord(filePickRecord *picker, uint8_t, uint8_t, bool);
void initFilePickRecord_far(uint8_t, uint8_t, bool);

void initFPR(void);
//uint8_t getTheFile(char *);
uint8_t getTheFile_far(char *);
//void backUpDirectory(filePickRecord *picker);
void backUpDirectory_far(void);
//void readDirectory(filePickRecord *picker);
void readDirectory_far(void);
//bool isExtensionAllowed(filePickRecord *picker, const char *filename);
bool isExtensionAllowed_far(const char *);

//void sortFileList(filePickRecord *picker);
void sortFileList_far(void);
//void displayFileList(filePickRecord *picker, int scrollOffset);
void displayFileList_far(int);
//void wipeArea(filePickRecord *picker);
void wipeArea_far(void);

//extern filePickRecord *fpr;
extern char name[];

#endif // MUFILEPICKER_H
