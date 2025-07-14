#include "D:\F256\llvm-mos\code\mp3wSA\.builddir\trampoline.h"


#include "f256lib.h"
#include "../src/muFilePicker.h"
#include "../src/muTextUI.h"
#include "../src/muUtils.h"

// Helper: compare file extension case-insensitively
static bool endsWithExt(const char *filename, const char *ext) {
    size_t fileLen = strlen(filename);
    if (fileLen < 4) return false;  // File name too short for extension + dot

    const char *dot = strrchr(filename, '.');
    if (!dot || strlen(dot + 1) != 3) return false;

    // Compare the last 3 characters case-insensitively
    for (int i = 0; i < 3; i++) {
        if (tolower(dot[1 + i]) != tolower(ext[i])) return false;
    }

    return true;
}

// Main filter function
bool isExtensionAllowed(const filePickRecord *picker, const char *filename) {
    for (int i = 0; i < MAX_FILE_EXTS; i++) {
        // Skip undefined extensions (e.g. all zeroes or empty string)
        if (picker->fileExts[i][0] == '\0') continue;

        if (endsWithExt(filename, picker->fileExts[i])) {
            return true;
        }
    }
    return false;
}

// Helper: compare two strings ignoring case

static int strcasecmp_local(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

void sortFileList(filePickRecord *picker) {
    // Insert the '...' breadcrumb at the top
    strcpy(picker->fileList[0], "..");
    picker->isDirList[0] = true;  // Treat it like a special folder
    int start = 1;

    // Bubble sort: folders first, then case-insensitive alphabetical
    for (int i = start; i < picker->fileCount - 1; i++) {
        for (int j = start; j < picker->fileCount - (i - start) - 1; j++) {
            bool aIsDir = picker->isDirList[j];
            bool bIsDir = picker->isDirList[j + 1];

            bool shouldSwap = false;

            // Rule 1: Folders come before files
            if (!aIsDir && bIsDir) {
                shouldSwap = true;
            } else if (aIsDir == bIsDir) {
                // Rule 2: Alphabetical order (case-insensitive)
                if (strcasecmp_local(picker->fileList[j], picker->fileList[j + 1]) > 0) {
                    shouldSwap = true;
                }
            }

            if (shouldSwap) {
                char temp[MAX_FILENAME_LEN];
                strcpy(temp, picker->fileList[j]);
                strcpy(picker->fileList[j], picker->fileList[j + 1]);
                strcpy(picker->fileList[j + 1], temp);

                bool tempDir = picker->isDirList[j];
                picker->isDirList[j] = picker->isDirList[j + 1];
                picker->isDirList[j + 1] = tempDir;
            }
        }
    }
}

void initFilePickRecord(filePickRecord *picker, uint8_t x, uint8_t y) {
    // Clear current path and selected file
    memset(picker->currentPath, 0, MAX_PATH_LEN);
    memset(picker->selectedFile, 0, MAX_FILENAME_LEN);

    // Clear all file names in the file list, and directory bool to false by default
    for (int i = 0; i < MAX_FILES; i++) {
        memset(picker->fileList[i], 0, MAX_FILENAME_LEN);
		picker->isDirList[i] = false;
    }

    // Clear file extensions
    for (int i = 0; i < MAX_FILE_EXTS; i++) {
        memset(picker->fileExts[i], 0, EXT_LEN);
    }

    // Reset counters and UI state
    picker->fileCount = 0;
    picker->cursorIndex = 0;
    picker->scrollOffset = 0;
	picker->tlX = x;
	picker->tlY = y;
}

void directory(filePickRecord *fP)
{
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;
	uint8_t x = fP->tlX;
	uint8_t y = fP->tlY;
	
	fP->cursorIndex = 0;
	fP->fileCount = 0;
	//checking the contents of the directory
	dirOpenResult = fileOpenDir("mp3");
	
	myDirEntry = fileReadDir(dirOpenResult);
	while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
	{
		if(_DE_ISREG(myDirEntry->d_type))
		{
			textGotoXY(x,y);printf("%s", myDirEntry->d_name);
			y++; fP->fileCount++;
			/*
			if(y==DIR_NEXT_COL)
			{
				x+=20;
				y=tly;
			}
			*/
		}
	}
	fileCloseDir(dirOpenResult);
	textGotoXY(fP->tlX-1,fP->tlY+fP->cursorIndex);textSetColor(textColorOrange,0x00);printf("%c",0xFA);textSetColor(4,0x00);
}

void backUpDirectory(filePickRecord *picker) {
    // Skip if currentPath is already root or empty
    if (picker->currentPath[0] == '\0' || strcmp(picker->currentPath, "/") == 0) {
        return;
    }

    // Find the last '/' in the path and truncate
    char *lastSlash = strrchr(picker->currentPath, '/');
    if (lastSlash && lastSlash != picker->currentPath) {
        *lastSlash = '\0';  // Cut off last folder
    } else {
        // If it's at root level (e.g. "/folder"), reset to root
        strcpy(picker->currentPath, ".");
    }
}

__attribute__((optnone))
uint8_t pickFile(filePickRecord *fP)
//  0 backtracking a folder level, 1 forward giong to a new folder, 2 a file is picked, 3 should never be reached
{
	for(;;)
	{
	kernelNextEvent();
	if(kernelEventData.type == kernelEvent(key.PRESSED))
		{
		switch(kernelEventData.key.raw)
			{
			case 148: //enter
			/*
				dirOpenResult = fileOpenDir("mp3");
	
				myDirEntry = fileReadDir(dirOpenResult);
				while((myDirEntry = fileReadDir(dirOpenResult))!= NULL)
				{
					if(_DE_ISREG(myDirEntry->d_type))
					{
						//if(fP->cursorIndex++ != myFilePicked.choice) continue;
						strcpy(fP->selectedFile,myDirEntry->d_name);
						break;
					}
				}
				fileCloseDir(dirOpenResult);
				*/

				memcpy(fP->selectedFile, fP->fileList[fP->cursorIndex], MAX_FILENAME_LEN);
				if(fP->cursorIndex == RESERVED_ENTRY_INDEX) return 0;
				else if(fP->isDirList[fP->cursorIndex]) return 1;
				else return 2;
			case 0xb6: //up arrow
			if(fP->cursorIndex!=0)
				{
				textGotoXY(fP->tlX,fP->tlY+fP->cursorIndex);printf("%c",32);
				fP->cursorIndex--;
				textGotoXY(fP->tlX,fP->tlY+fP->cursorIndex);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
				}
			break;
			case 0xb7: //down arrow
				if(fP->cursorIndex<fP->fileCount-1)
					{
					textGotoXY(fP->tlX,fP->tlY+fP->cursorIndex);printf("%c",32);
					fP->cursorIndex++;
					textGotoXY(fP->tlX,fP->tlY+fP->cursorIndex);printf("%c",0xFA);textSetColor(textColorWhite,0x00);
					}
			break;
			}
		}
	}
	return 3;
}

void readDirectory(filePickRecord *picker) {
	char *dirOpenResult;
	struct fileDirEntS *myDirEntry;
	
	//checking the contents of the directory
	dirOpenResult = fileOpenDir(picker->currentPath);
	
    if (!dirOpenResult) return;

    int count = 1;  // Start from index 1, reserve index 0 for '..'

	myDirEntry = fileReadDir(dirOpenResult);
    while (((myDirEntry = fileReadDir(dirOpenResult))!= NULL) && (count < MAX_FILES)) {
        // Skip "." and optionally ".."
        if (strcmp(myDirEntry->d_name, ".") == 0) continue;
        if (strcmp(myDirEntry->d_name, "..") == 0) continue;


        // Filter by extension if needed
        if (!isExtensionAllowed(picker, myDirEntry->d_name) && _DE_ISREG(myDirEntry->d_type)) continue;

        // Store filename safely
        strncpy(picker->fileList[count], myDirEntry->d_name, MAX_FILENAME_LEN - 1);
        picker->fileList[count][MAX_FILENAME_LEN - 1] = '\0';
		if(_DE_ISDIR(myDirEntry->d_type)) picker->isDirList[count] = true;
		else picker->isDirList[count] = false;

        count++;
    }

    picker->fileCount = count;

    // Place '..' breadcrumb at the top manually
    strncpy(picker->fileList[0], "..", MAX_FILENAME_LEN);
	
	
	fileCloseDir(dirOpenResult);
}

void displayFileList(const filePickRecord *picker, int scrollOffset) {
    int visibleStart = RESERVED_ENTRY_INDEX + scrollOffset + 1;  // skip '..'
    int visibleEnd = visibleStart + (MAX_VISIBLE_FILES - 1);     // 1 reserved + 19 scrollable

    if (visibleEnd >= picker->fileCount) {
        visibleEnd = picker->fileCount;
    }
	
	textGotoXY(picker->tlX, picker->tlY);
    // Always show '..' first
    printf("%c%s%s\n", picker->cursorIndex == RESERVED_ENTRY_INDEX ? 0xFA : ' ', picker->fileList[RESERVED_ENTRY_INDEX], picker->isDirList[0]?"/":" ");

    // Show up to 19 more entries, scrolled by scrollOffset
    for (int i = visibleStart; i < visibleEnd; i++) {
        int isCursor = (picker->cursorIndex == i);
		if(isCursor) printf("%c", 0xFA);
		else printf("  ");
        printf("%s%s\n", picker->fileList[i], picker->isDirList[i]?"/":" ");
    }
}

uint8_t filePickModal(filePickRecord *fpr, uint8_t x, uint8_t y, char *initFolder, char *ext0, char *ext1, char *ext2, char *ext3){
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000000); //font overlay, double height text, 320x240 at 60 Hz;
	
	initFilePickRecord(fpr, x, y);// Set starting path
    strncpy(fpr->currentPath, initFolder, MAX_PATH_LEN);
    // Set 3 out of 4 allowed file extensions
    memcpy(fpr->fileExts[0], ext0, EXT_LEN);
    memcpy(fpr->fileExts[1], ext1, EXT_LEN);
    memcpy(fpr->fileExts[2], ext2, EXT_LEN);
    memcpy(fpr->fileExts[3], ext3, EXT_LEN);
	
	readDirectory(fpr);
	sortFileList(fpr);

// Display up to 20 entries (1 fixed + 19 scrollable)
    displayFileList(fpr, fpr->scrollOffset);
	for(;;)
	{
	uint8_t result = pickFile(fpr);
	if(result == 0)
		{
		textGotoXY(0,20);printf("backing a level");
		strncpy(fpr->currentPath, fpr->selectedFile, MAX_PATH_LEN);
		backUpDirectory(fpr);
		}
	else if(result == 1)
		{
		textGotoXY(0,20);printf("this folder was picked %s", fpr->selectedFile);
		strncpy(fpr->currentPath, fpr->selectedFile, MAX_PATH_LEN);
		backUpDirectory(fpr);
		}
	else if(result == 2)
		{
		textGotoXY(0,20);printf("this file was picked %s\n", fpr->selectedFile);

		break;
		}
	}
	
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;

	return 0;
}
