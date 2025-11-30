#include "D:\F256\llvm-mos\code\vgmplayer2\.builddir\trampoline.h"


#include "f256lib.h"
#include "../src/muFilePicker.h"

filePickRecord fpr;
char name[120];

void initFPR()
{
	strncpy(fpr.currentPath, "media/vgm", MAX_PATH_LEN);
}
uint8_t getTheFile(char *name) //job is to get a string containing the filename including directory
{
	
//check if the vgm directory is here, if not, target the root
	char *dirOpenResult = fileOpenDir(fpr.currentPath);
	if(dirOpenResult != NULL)
	{
		strncpy(fpr.currentPath, fpr.currentPath, MAX_PATH_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr.currentPath, "0:", MAX_PATH_LEN);


	uint8_t wantsQuit = filePickModal(&fpr, DIRECTORY_X, DIRECTORY_Y, "vgm", "", "", "", true);
	if(wantsQuit==1) return 1;
	sprintf(name, "%s%s%s", fpr.currentPath,"/", fpr.selectedFile);
	return 0;
}


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
bool isExtensionAllowed(filePickRecord *picker, const char *filename) {
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

void reprepFPR(filePickRecord *picker, bool newFolder)
{
	// Clear all file names in the file list, and directory bool to false by default
    for (int i = 1; i < MAX_FILES; i++) {
        memset(picker->fileList[i], 0, MAX_FILENAME_LEN);
		picker->isDirList[i] = false;
    }
	if(newFolder)
		{
			// Reset counters and UI state
		picker->fileCount = 0;
		picker->cursorIndex = 0;
		picker->scrollOffset = 0;
		picker->visualIndex = 0;
		}
}
void initFilePickRecord(filePickRecord *picker, uint8_t x, uint8_t y, bool firstTime) {
    // Clear current path and selected file
    memset(picker->selectedFile, 0, MAX_FILENAME_LEN);
	
	reprepFPR(picker, firstTime); //what to do between directory changes
    // Clear file extensions
    for (int i = 0; i < MAX_FILE_EXTS; i++) {
        memset(picker->fileExts[i], 0, EXT_LEN);
    }
	    // Reset counters and UI state
	picker->tlX = x;
	picker->tlY = y;
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
        strcpy(picker->currentPath, "");
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
			case 146: //esc
					return 3;
			case 148: //enter
				memcpy(fP->selectedFile, fP->fileList[fP->cursorIndex], MAX_FILENAME_LEN);
				if(fP->visualIndex == 0) return 0; //back up a folder
				else if(fP->isDirList[fP->cursorIndex]) return 1;
				else return 2;
			case 0xb6: //up arrow
			if(fP->cursorIndex>0)
				{
				if(fP->visualIndex > 0) //only move the cursor up if you're past visual index 0
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->cursorIndex--; fP->visualIndex--;
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
				else if(fP->visualIndex == 0)
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->visualIndex=MAX_VISIBLE_FILES-1;
					fP->scrollOffset-=MAX_VISIBLE_FILES-1;
					displayFileList(fP, fP->scrollOffset);
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
				}
			//textGotoXY(0,29);printf("vidx %d cIdx %d offs %d fCnt %d ", fP->visualIndex, fP->cursorIndex, fP->scrollOffset, fP->fileCount);
				break;
			case 0xb7: //down arrow
				if(fP->cursorIndex<fP->fileCount-1)//don't go beyond the directory's filtered content
				{
				if(fP->visualIndex < MAX_VISIBLE_FILES - 1) //only move the cursor down if you're under the MAX VIS constant
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->cursorIndex++; fP->visualIndex++;
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
				else if(fP->visualIndex == MAX_VISIBLE_FILES - 1 //if you're at the end of the visible max
				   && fP->fileCount > fP->cursorIndex) //and there's more files to explore
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->visualIndex=0;
					fP->scrollOffset+=MAX_VISIBLE_FILES-1;
					displayFileList(fP, fP->scrollOffset);
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
				}
			//textGotoXY(0,29);printf("vidx %d cIdx %d offs %d fCnt %d ", fP->visualIndex, fP->cursorIndex, fP->scrollOffset, fP->fileCount);
				break;
			case 0xB8: //left arrow
			    if(fP->cursorIndex>= MAX_VISIBLE_FILES -1)
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->cursorIndex -= MAX_VISIBLE_FILES-1;
					fP->scrollOffset-=MAX_VISIBLE_FILES-1;
					displayFileList(fP, fP->scrollOffset);
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
			//textGotoXY(0,29);printf("vidx %d cIdx %d offs %d fCnt %d ", fP->visualIndex, fP->cursorIndex, fP->scrollOffset, fP->fileCount);
				break;
			case 0xB9: //right arrow
				if(fP->fileCount > (fP->cursorIndex - fP->visualIndex) + MAX_VISIBLE_FILES) //and there's more files to explore
					{
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",32); //erase old
					fP->cursorIndex += (MAX_VISIBLE_FILES-1 - fP->visualIndex);
					fP->visualIndex=0;
					fP->scrollOffset +=MAX_VISIBLE_FILES-1;
					displayFileList(fP, fP->scrollOffset);
					textGotoXY(fP->tlX,fP->tlY+fP->visualIndex);printf("%c",0xFA); //bring in new position
					}
			//textGotoXY(0,29);printf("vidx %d cIdx %d offs %d fCnt %d ", fP->visualIndex, fP->cursorIndex, fP->scrollOffset, fP->fileCount);
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
void wipeArea(filePickRecord *fpr)
{
	for(uint8_t i=0; i<MAX_VISIBLE_FILES+1; i++)
	{
		for(uint8_t j=fpr->tlY; j<80; j++)
		{
		printf(" ");
		}
		printf("\n");
	}
}
void displayFileList(filePickRecord *picker, int scrollOffset) {
    int visibleStart = RESERVED_ENTRY_INDEX + scrollOffset + 1;  // skip '..'
    int visibleEnd = visibleStart + (MAX_VISIBLE_FILES - 1);     // 1 reserved + 19 scrollable

    if (visibleEnd >= picker->fileCount) {
        visibleEnd = picker->fileCount;
    }
	textGotoXY(picker->tlX, picker->tlY);
	wipeArea(picker);
	textGotoXY(picker->tlX, picker->tlY);
    // Always show '..' first
	printf("%c%s%s", picker->visualIndex == 0 ? 0xFA : ' ', picker->fileList[RESERVED_ENTRY_INDEX], picker->isDirList[0]?"/":" ");

    // Show up to 19 more entries, scrolled by scrollOffset
    for (int i = visibleStart; i < visibleEnd; i++) {
	
		textGotoXY(picker->tlX, picker->tlY + i - visibleStart + 1);
        int isCursor = (picker->cursorIndex == i);
		if(isCursor) printf("%c", 0xFA);
		else printf("%c", ' ');
        printf("%.75s%1s", picker->fileList[i], picker->isDirList[i]?"/":" ");
    }
	textGotoXY(picker->tlX + 2, picker->tlY + visibleEnd - visibleStart + 1);
	printf("%c %c",
	picker->cursorIndex >= (MAX_VISIBLE_FILES-1)?0xFB:' ',
	picker->fileCount > (picker->cursorIndex - picker->visualIndex + MAX_VISIBLE_FILES) ?0xF8:' ');\
}


uint8_t filePickModal(filePickRecord *fpr, uint8_t x, uint8_t y, char *ext0, char *ext1, char *ext2, char *ext3, bool firstTime){

	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	//POKE(VKY_MSTR_CTRL_1, 0b00000000); //font overlay, double height text, 320x240 at 60 Hz;
	
	initFilePickRecord(fpr, x, y, firstTime);// Set starting path
    // Set 3 out of 4 allowed file extensions
    memcpy(fpr->fileExts[0], ext0, EXT_LEN);
    memcpy(fpr->fileExts[1], ext1, EXT_LEN);
    memcpy(fpr->fileExts[2], ext2, EXT_LEN);
    memcpy(fpr->fileExts[3], ext3, EXT_LEN);
	
	readDirectory(fpr);
	sortFileList(fpr);
// Display up to 20 entries (1 fixed + 19 scrollable)

	textSetColor(10,0);
    displayFileList(fpr, fpr->scrollOffset);
	
	for(;;)
	{
	uint8_t result = pickFile(fpr);
	if(result == 0) //choosing to back a folder level
		{
		backUpDirectory(fpr);
		reprepFPR(fpr, true);
		readDirectory(fpr);
		sortFileList(fpr);
		displayFileList(fpr, fpr->scrollOffset);
	
		}
	else if(result == 1) //choosing to go deeper a folder level
		{
		char finalDir[64];
		sprintf(finalDir,"%s%s%s",fpr->currentPath,"/",fpr->selectedFile); //add folder level

		reprepFPR(fpr, true);
		strncpy(fpr->currentPath, finalDir, MAX_PATH_LEN);
		readDirectory(fpr);
	
		sortFileList(fpr);
		displayFileList(fpr, fpr->scrollOffset);
	
		}
	else if(result == 2)
		{

		break;
		}
	else if(result == 3) //wants to escape
		return 1;
	}
	
	// XXX XXX  FON_SET FON_OVLY | MON_SLP DBL_Y  DBL_X  CLK_70
	//POKE(VKY_MSTR_CTRL_1, 0b00000100); //font overlay, double height text, 320x240 at 60 Hz;
	textGotoXY(fpr->tlX, fpr->tlY);
	wipeArea(fpr);
	return 0;}
