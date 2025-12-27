
#include "f256lib.h"
#include "../src/muFilePicker.h"
#include "../src/muUtils.h"


//filePickRecord *fpr;
char name[MAX_FILENAME_LEN];


static inline uint8_t fpr_get8(uint32_t off) {
    return FAR_PEEK(FPR_ADDR(off));
}

static inline void fpr_set8(uint32_t off, uint8_t v) {
    FAR_POKE(FPR_ADDR(off), v);
}

void fpr_set_currentPath(const char *s) {
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        char c = s[i];
        fpr_set8(FPR_currentPath + i, c);
        if (c==0) break;
    }
}
void fpr_get_currentPath(char *out)
{
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        byte c = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
        out[i] = c;
        if (c == 0) {
            return;
        }
    }

    // Safety: ensure null termination even if memory was full
    out[59] = 0;
}

void fpr_get_selectedFile(char *out) {
    for (int i = 0; i < 120; i++) {
        out[i] = fpr_get8(FPR_selectedFile + i);
        if (!out[i]) break;
    }
}

void initFPR()
{
	//**
	//strncpy(fpr->currentPath, "media/vgm", MAX_FILENAME_LEN);
	fpr_set_currentPath("media/vgm");
}

/*
uint8_t getTheFile(char *name) //job is to get a string containing the filename including directory
{
	
//check if the vgm directory is here, if not, target the root
	
	
	//char *dirOpenResult = fileOpenDir(fpr->currentPath);
	char *dirOpenResult;
	fpr_get_currentPath(dirOpenResult);
	
	
	if(dirOpenResult != NULL) 
	{
		strncpy(fpr->currentPath, fpr->currentPath, MAX_FILENAME_LEN);
		fileCloseDir(dirOpenResult);
	}
	else strncpy(fpr->currentPath, "0:/", MAX_FILENAME_LEN);

	uint8_t wantsQuit = filePickModal(fpr, DIRECTORY_X, DIRECTORY_Y, "vgm", "spl", "", "", true);
	if(wantsQuit==1) return 1;
	sprintf(name, "%s%s%s", fpr->currentPath,"/",fpr->selectedFile);
	

	return 0;
}
*/
uint8_t getTheFile_far(char *name)
{
    char localPath[MAX_PATH_LEN];
    void *dirOpenResult;

    // ---------------------------------------------------------
    // 1. Read currentPath from far memory
    // ---------------------------------------------------------
	fpr_get_currentPath(localPath);

	/*
	for (int i = 0; i < MAX_PATH_LEN; i++) {
        localPath[i] = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
        if (localPath[i] == 0) break;
    }
*/
    // ---------------------------------------------------------
    // 2. Try to open directory
    // ---------------------------------------------------------
    dirOpenResult = fileOpenDir(localPath);
printf("\nDEBUG path='%s' (len=%d)\n", localPath, strlen(localPath));
printf("\ndirOpenResult ='%p'\n", dirOpenResult);

hitspace();

    if (dirOpenResult != NULL)
    {
        // Directory exists → close it and keep currentPath as-is
        fileCloseDir(dirOpenResult);
    }
    else
    {
        // Directory missing → set currentPath = "0:/"
        const char *fallback = "0:/";
		fpr_set_currentPath(fallback);
		/*
        for (int i = 0; i < MAX_PATH_LEN; i++) {
            FAR_POKE(FPR_BASE + FPR_currentPath + i, fallback[i]);
            if (fallback[i] == 0) break;
        }
		*/
    }

    // ---------------------------------------------------------
    // 3. Launch modal picker
    // ---------------------------------------------------------
    uint8_t wantsQuit =
        filePickModal_far(DIRECTORY_X, DIRECTORY_Y,
                          "vgm", "spl", "", "", true);

    if (wantsQuit == 1)
        return 1;

    // ---------------------------------------------------------
    // 4. Build final full path into 'name'
    // ---------------------------------------------------------
    char finalPath[MAX_PATH_LEN];
    char selected[MAX_FILENAME_LEN];

    // Read currentPath
	fpr_get_currentPath(finalPath);
	/*
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        finalPath[i] = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
        if (finalPath[i] == 0) break;
    }
	*/

    // Read selectedFile
    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
        selected[i] = FAR_PEEK(FPR_BASE + FPR_selectedFile + i);
        if (selected[i] == 0) break;
    }

    // Build final string
    sprintf(name, "%s/%s", finalPath, selected);

    return 0;
}

// Helper: compare file extension case-insensitively
bool endsWithExt(const char *filename, const char *ext) {
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
/*
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
*/
bool isExtensionAllowed_far(const char *filename)
{
    char extBuf[EXT_LEN + 1];

    for (int i = 0; i < MAX_FILE_EXTS; i++)
    {
        // Read first character of extension from far memory
        byte first = FAR_PEEK(FPR_BASE + FPR_fileExts + (i * EXT_LEN));

        // Skip empty extension slots
        if (first == 0)
            continue;

        // Read full extension into local buffer
        for (int j = 0; j < EXT_LEN; j++) {
            extBuf[j] = FAR_PEEK(FPR_BASE + FPR_fileExts + (i * EXT_LEN) + j);
        }
        extBuf[EXT_LEN] = 0;   // null‑terminate

        // Compare using your existing helper
        if (endsWithExt(filename, extBuf))
            return true;
    }

    return false;
}

// Helper: compare two strings ignoring case

int strcasecmp_local(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
/*
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
*/

void sortFileList_far(void)
{
    // ---------------------------------------------------------
    // 1. Write ".." into fileList[0]
    // ---------------------------------------------------------
    FAR_POKE(FPR_BASE + FPR_fileList + 0, '.');
    FAR_POKE(FPR_BASE + FPR_fileList + 1, '.');
    FAR_POKE(FPR_BASE + FPR_fileList + 2, 0);

    // Zero‑fill the rest
    for (int i = 3; i < MAX_FILENAME_LEN; i++)
        FAR_POKE(FPR_BASE + FPR_fileList + i, 0);

    // Mark index 0 as a directory
    FAR_POKE(FPR_BASE + FPR_isDirList + 0, 1);

    // ---------------------------------------------------------
    // 2. Read fileCount
    // ---------------------------------------------------------
    uint16_t fileCount = FAR_PEEKW(FPR_BASE + FPR_fileCount);
    int start = 1;

    // ---------------------------------------------------------
    // 3. Bubble sort (folders first, then alphabetical)
    // ---------------------------------------------------------
    for (int i = start; i < fileCount - 1; i++)
    {
        for (int j = start; j < fileCount - (i - start) - 1; j++)
        {
            // Read directory flags
            bool aIsDir = FAR_PEEK(FPR_BASE + FPR_isDirList + j);
            bool bIsDir = FAR_PEEK(FPR_BASE + FPR_isDirList + (j + 1));

            bool shouldSwap = false;

            // Rule 1: directories first
            if (!aIsDir && bIsDir)
            {
                shouldSwap = true;
            }
            else if (aIsDir == bIsDir)
            {
                // Rule 2: alphabetical order
                char nameA[MAX_FILENAME_LEN];
                char nameB[MAX_FILENAME_LEN];

                // Read fileList[j]
                uint32_t baseA = FPR_BASE + FPR_fileList + (j * MAX_FILENAME_LEN);
                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    nameA[k] = FAR_PEEK(baseA + k);
                    if (nameA[k] == 0) break;
                }

                // Read fileList[j+1]
                uint32_t baseB = FPR_BASE + FPR_fileList + ((j + 1) * MAX_FILENAME_LEN);
                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    nameB[k] = FAR_PEEK(baseB + k);
                    if (nameB[k] == 0) break;
                }

                if (strcasecmp_local(nameA, nameB) > 0)
                    shouldSwap = true;
            }

            // -------------------------------------------------
            // Perform swap if needed
            // -------------------------------------------------
            if (shouldSwap)
            {
                // Swap filenames
                char temp[MAX_FILENAME_LEN];

                uint32_t baseA = FPR_BASE + FPR_fileList + (j * MAX_FILENAME_LEN);
                uint32_t baseB = FPR_BASE + FPR_fileList + ((j + 1) * MAX_FILENAME_LEN);

                // Read A → temp
                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    temp[k] = FAR_PEEK(baseA + k);
                    if (temp[k] == 0) break;
                }

                // Write B → A
                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    char c = FAR_PEEK(baseB + k);
                    FAR_POKE(baseA + k, c);
                    if (c == 0) break;
                }

                // Write temp → B
                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    FAR_POKE(baseB + k, temp[k]);
                    if (temp[k] == 0) break;
                }

                // Swap directory flags
                bool tempDir = aIsDir;
                FAR_POKE(FPR_BASE + FPR_isDirList + j,
                         FAR_PEEK(FPR_BASE + FPR_isDirList + (j + 1)));
                FAR_POKE(FPR_BASE + FPR_isDirList + (j + 1), tempDir);
            }
        }
    }
}

/*
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
*/

void reprepFPR_far(bool newFolder)
{
    // ---------------------------------------------------------
    // Clear fileList[1..MAX_FILES-1] and isDirList[]
    // ---------------------------------------------------------
    for (int i = 1; i < MAX_FILES; i++) {

        // Clear fileList[i] (120 bytes)
        uint32_t base = FPR_BASE + FPR_fileList + (i * MAX_FILENAME_LEN);
        for (int j = 0; j < MAX_FILENAME_LEN; j++) {
            FAR_POKE(base + j, 0);
        }

        // Clear isDirList[i]
        FAR_POKE(FPR_BASE + FPR_isDirList + i, 0);
    }

    // ---------------------------------------------------------
    // Reset counters and UI state if entering a new folder
    // ---------------------------------------------------------
    if (newFolder) {
        FAR_POKEW(FPR_BASE + FPR_fileCount,    0);
        FAR_POKEW(FPR_BASE + FPR_cursorIndex,  0);
        FAR_POKEW(FPR_BASE + FPR_scrollOffset, 0);
        FAR_POKEW(FPR_BASE + FPR_visualIndex,  0);
    }
}


/*
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
*/
void initFilePickRecord_far(uint8_t x, uint8_t y, bool firstTime)
{
    // ---------------------------------------------------------
    // Clear selectedFile (120 bytes)
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
        FAR_POKE(FPR_BASE + FPR_selectedFile + i, 0);
    }

    // ---------------------------------------------------------
    // Clear currentPath (60 bytes)
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        FAR_POKE(FPR_BASE + FPR_currentPath + i, 0);
    }

    // ---------------------------------------------------------
    // Call your far‑memory version of reprepFPR()
    // (You will need to convert reprepFPR() the same way)
    // ---------------------------------------------------------
    reprepFPR_far(firstTime);

    // ---------------------------------------------------------
    // Clear file extensions (4 × 3 bytes)
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_FILE_EXTS; i++) {
        for (int j = 0; j < EXT_LEN; j++) {
            FAR_POKE(FPR_BASE + FPR_fileExts + (i * EXT_LEN) + j, 0);
        }
    }

    // ---------------------------------------------------------
    // Set tlX and tlY
    // ---------------------------------------------------------
    FAR_POKE(FPR_BASE + FPR_tlX, x);
    FAR_POKE(FPR_BASE + FPR_tlY, y);
}

/*
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
*/

void backUpDirectory_far(void)
{
    // ---------------------------------------------------------
    // 1. Read first character of currentPath
    // ---------------------------------------------------------
    byte first = FAR_PEEK(FPR_BASE + FPR_currentPath);

    // If empty or root "/", do nothing
    if (first == 0) {
        return;
    }

    if (first == '/' && FAR_PEEK(FPR_BASE + FPR_currentPath + 1) == 0) {
        return;
    }

    // ---------------------------------------------------------
    // 2. Find last '/' in currentPath
    // ---------------------------------------------------------
    int lastSlashIndex = -1;

    for (int i = 0; i < MAX_PATH_LEN; i++) {
        byte c = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
        if (c == 0) break;
        if (c == '/') lastSlashIndex = i;
    }

    // ---------------------------------------------------------
    // 3. If no slash found or slash is at index 0 → reset to ""
    // ---------------------------------------------------------
    if (lastSlashIndex <= 0) {
        // Clear entire currentPath
        for (int i = 0; i < MAX_PATH_LEN; i++) {
            FAR_POKE(FPR_BASE + FPR_currentPath + i, 0);
        }
        return;
    }

    // ---------------------------------------------------------
    // 4. Truncate at last slash
    // ---------------------------------------------------------
    FAR_POKE(FPR_BASE + FPR_currentPath + lastSlashIndex, 0);
}
/*
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
*/
__attribute__((optnone))
uint8_t pickFile_far(void)
{
    for (;;)
    {
        kernelNextEvent();

        if (kernelEventData.type == kernelEvent(key.PRESSED))
        {
            switch (kernelEventData.key.raw)
            {
                // ---------------------------------------------------------
                // ESC → exit modal
                // ---------------------------------------------------------
                case 146:
                    return 3;

                // ---------------------------------------------------------
                // ENTER → choose file or folder
                // ---------------------------------------------------------
                case 148:
                {
                    // Read cursorIndex
                    uint16_t cursorIndex = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);

                    // Copy fileList[cursorIndex] → selectedFile
                    uint32_t src = FPR_BASE + FPR_fileList + (cursorIndex * MAX_FILENAME_LEN);
                    uint32_t dst = FPR_BASE + FPR_selectedFile;

                    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
                        byte c = FAR_PEEK(src + i);
                        FAR_POKE(dst + i, c);
                        if (c == 0) break;
                    }

                    // Read visualIndex
                    uint16_t visualIndex = FAR_PEEKW(FPR_BASE + FPR_visualIndex);

                    if (visualIndex == 0)
                        return 0;   // go up a folder

                    // Read isDirList[cursorIndex]
                    byte isDir = FAR_PEEK(FPR_BASE + FPR_isDirList + cursorIndex);

                    if (isDir)
                        return 1;   // go deeper
                    else
                        return 2;   // file picked
                }

                // ---------------------------------------------------------
                // UP ARROW
                // ---------------------------------------------------------
                case 0xB6:
                {
                    uint16_t cursorIndex  = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);
                    uint16_t visualIndex  = FAR_PEEKW(FPR_BASE + FPR_visualIndex);
                    uint16_t scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
                    uint8_t  tlX          = FAR_PEEK(FPR_BASE + FPR_tlX);
                    uint8_t  tlY          = FAR_PEEK(FPR_BASE + FPR_tlY);

                    if (cursorIndex > 0)
                    {
                        if (visualIndex > 0)
                        {
                            // erase old cursor
                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 32);

                            cursorIndex--;
                            visualIndex--;

                            FAR_POKEW(FPR_BASE + FPR_cursorIndex, cursorIndex);
                            FAR_POKEW(FPR_BASE + FPR_visualIndex, visualIndex);

                            // draw new cursor
                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 0xFA);
                        }
                        else
                        {
                            // wrap upward
                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 32);

                            visualIndex = MAX_VISIBLE_FILES - 1;
                            scrollOffset -= (MAX_VISIBLE_FILES - 1);

                            FAR_POKEW(FPR_BASE + FPR_visualIndex, visualIndex);
                            FAR_POKEW(FPR_BASE + FPR_scrollOffset, scrollOffset);

                            displayFileList_far(scrollOffset);

                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 0xFA);
                        }
                    }
                }
                break;

                // ---------------------------------------------------------
                // DOWN ARROW
                // ---------------------------------------------------------
                case 0xB7:
                {
                    uint16_t cursorIndex  = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);
                    uint16_t visualIndex  = FAR_PEEKW(FPR_BASE + FPR_visualIndex);
                    uint16_t scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
                    uint16_t fileCount    = FAR_PEEKW(FPR_BASE + FPR_fileCount);
                    uint8_t  tlX          = FAR_PEEK(FPR_BASE + FPR_tlX);
                    uint8_t  tlY          = FAR_PEEK(FPR_BASE + FPR_tlY);

                    if (cursorIndex < fileCount - 1)
                    {
                        if (visualIndex < MAX_VISIBLE_FILES - 1)
                        {
                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 32);

                            cursorIndex++;
                            visualIndex++;

                            FAR_POKEW(FPR_BASE + FPR_cursorIndex, cursorIndex);
                            FAR_POKEW(FPR_BASE + FPR_visualIndex, visualIndex);

                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 0xFA);
                        }
                        else if (visualIndex == MAX_VISIBLE_FILES - 1 &&
                                 fileCount > cursorIndex)
                        {
                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 32);

                            visualIndex = 0;
                            scrollOffset += (MAX_VISIBLE_FILES - 1);

                            FAR_POKEW(FPR_BASE + FPR_visualIndex, visualIndex);
                            FAR_POKEW(FPR_BASE + FPR_scrollOffset, scrollOffset);

                            displayFileList_far(scrollOffset);

                            textGotoXY(tlX, tlY + visualIndex);
                            printf("%c", 0xFA);
                        }
                    }
                }
                break;

                // ---------------------------------------------------------
                // LEFT ARROW
                // ---------------------------------------------------------
                case 0xB8:
                {
                    uint16_t cursorIndex  = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);
                    uint16_t visualIndex  = FAR_PEEKW(FPR_BASE + FPR_visualIndex);
                    uint16_t scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
                    uint8_t  tlX          = FAR_PEEK(FPR_BASE + FPR_tlX);
                    uint8_t  tlY          = FAR_PEEK(FPR_BASE + FPR_tlY);

                    if (cursorIndex >= MAX_VISIBLE_FILES - 1)
                    {
                        textGotoXY(tlX, tlY + visualIndex);
                        printf("%c", 32);

                        cursorIndex -= (MAX_VISIBLE_FILES - 1);
                        scrollOffset -= (MAX_VISIBLE_FILES - 1);

                        FAR_POKEW(FPR_BASE + FPR_cursorIndex, cursorIndex);
                        FAR_POKEW(FPR_BASE + FPR_scrollOffset, scrollOffset);

                        displayFileList_far(scrollOffset);

                        textGotoXY(tlX, tlY + visualIndex);
                        printf("%c", 0xFA);
                    }
                }
                break;

                // ---------------------------------------------------------
                // RIGHT ARROW
                // ---------------------------------------------------------
                case 0xB9:
                {
                    uint16_t cursorIndex  = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);
                    uint16_t visualIndex  = FAR_PEEKW(FPR_BASE + FPR_visualIndex);
                    uint16_t scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
                    uint16_t fileCount    = FAR_PEEKW(FPR_BASE + FPR_fileCount);
                    uint8_t  tlX          = FAR_PEEK(FPR_BASE + FPR_tlX);
                    uint8_t  tlY          = FAR_PEEK(FPR_BASE + FPR_tlY);

                    if (fileCount > (cursorIndex - visualIndex) + MAX_VISIBLE_FILES)
                    {
                        textGotoXY(tlX, tlY + visualIndex);
                        printf("%c", 32);

                        cursorIndex += (MAX_VISIBLE_FILES - 1 - visualIndex);
                        visualIndex = 0;
                        scrollOffset += (MAX_VISIBLE_FILES - 1);

                        FAR_POKEW(FPR_BASE + FPR_cursorIndex, cursorIndex);
                        FAR_POKEW(FPR_BASE + FPR_visualIndex, visualIndex);
                        FAR_POKEW(FPR_BASE + FPR_scrollOffset, scrollOffset);

                        displayFileList_far(scrollOffset);

                        textGotoXY(tlX, tlY + visualIndex);
                        printf("%c", 0xFA);
                    }
                }
                break;
            }
        }
    }

    return 3;
}

/*
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
*/

void readDirectory_far(void)
{
    char localPath[MAX_PATH_LEN];
    char *dirOpenResult;
    struct fileDirEntS *myDirEntry;

    // ---------------------------------------------------------
    // 1. Read currentPath from far memory into a local buffer
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        localPath[i] = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
        if (localPath[i] == 0) break;
    }

    // ---------------------------------------------------------
    // 2. Open directory
    // ---------------------------------------------------------
    dirOpenResult = fileOpenDir(localPath);
    if (!dirOpenResult)
        return;

    // ---------------------------------------------------------
    // 3. Start filling fileList at index 1
    // ---------------------------------------------------------
    int count = 1;

    // Prime read (your original code did this)
    myDirEntry = fileReadDir(dirOpenResult);

    // ---------------------------------------------------------
    // 4. Read directory entries
    // ---------------------------------------------------------
    while (((myDirEntry = fileReadDir(dirOpenResult)) != NULL) &&
           (count < MAX_FILES))
    {
        // Skip "." and ".."
        if (strcmp(myDirEntry->d_name, ".") == 0)  continue;
        if (strcmp(myDirEntry->d_name, "..") == 0) continue;

        // Extension filter
        if (!isExtensionAllowed_far(myDirEntry->d_name) &&
            _DE_ISREG(myDirEntry->d_type))
        {
            continue;
        }

        // -----------------------------------------------------
        // 5. Write filename into far-memory fileList[count]
        // -----------------------------------------------------
        uint32_t base = FPR_BASE + FPR_fileList + (count * MAX_FILENAME_LEN);

        // Copy name
        int j = 0;
        for (; j < MAX_FILENAME_LEN - 1; j++) {
            char c = myDirEntry->d_name[j];
            FAR_POKE(base + j, c);
            if (c == 0) break;
        }

        // Ensure null termination
        FAR_POKE(base + (MAX_FILENAME_LEN - 1), 0);

        // -----------------------------------------------------
        // 6. Write isDirList[count]
        // -----------------------------------------------------
        FAR_POKE(FPR_BASE + FPR_isDirList + count,
                 _DE_ISDIR(myDirEntry->d_type) ? 1 : 0);

        count++;
    }

    // ---------------------------------------------------------
    // 7. Write fileCount
    // ---------------------------------------------------------
    FAR_POKEW(FPR_BASE + FPR_fileCount, count);

    // ---------------------------------------------------------
    // 8. Write ".." into fileList[0]
    // ---------------------------------------------------------
    {
        uint32_t base = FPR_BASE + FPR_fileList; // index 0
        FAR_POKE(base + 0, '.');
        FAR_POKE(base + 1, '.');
        FAR_POKE(base + 2, 0);

        // Zero-fill the rest (optional but clean)
        for (int i = 3; i < MAX_FILENAME_LEN; i++) {
            FAR_POKE(base + i, 0);
        }
    }

    // ---------------------------------------------------------
    // 9. Close directory
    // ---------------------------------------------------------
    fileCloseDir(dirOpenResult);
}
/*
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
*/
void wipeArea_far(void)
{
    // Read tlY from far memory
    uint8_t tlY = FAR_PEEK(FPR_BASE + FPR_tlY);

    for (uint8_t i = 0; i < MAX_VISIBLE_FILES + 1; i++)
    {
        for (uint8_t j = tlY; j < 80; j++)
        {
            printf(" ");
        }
        printf("\n");
    }
}


/*
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
*/
void displayFileList_far(int scrollOffset)
{
    // ---------------------------------------------------------
    // Read UI coordinates from far memory
    // ---------------------------------------------------------
    uint8_t tlX = FAR_PEEK(FPR_BASE + FPR_tlX);
    uint8_t tlY = FAR_PEEK(FPR_BASE + FPR_tlY);

    // ---------------------------------------------------------
    // Read counters
    // ---------------------------------------------------------
    uint16_t fileCount    = FAR_PEEKW(FPR_BASE + FPR_fileCount);
    uint16_t cursorIndex  = FAR_PEEKW(FPR_BASE + FPR_cursorIndex);
    uint16_t visualIndex  = FAR_PEEKW(FPR_BASE + FPR_visualIndex);

    // ---------------------------------------------------------
    // Compute visible range
    // ---------------------------------------------------------
    int visibleStart = RESERVED_ENTRY_INDEX + scrollOffset + 1;  
    int visibleEnd   = visibleStart + (MAX_VISIBLE_FILES - 1);

    if (visibleEnd >= fileCount)
        visibleEnd = fileCount;

    // ---------------------------------------------------------
    // Clear the display area
    // ---------------------------------------------------------
    textGotoXY(tlX, tlY);
    wipeArea_far();     // You will convert wipeArea() similarly
    textGotoXY(tlX, tlY);

    // ---------------------------------------------------------
    // Display ".." entry at index 0
    // ---------------------------------------------------------
    {
        char nameBuf[MAX_FILENAME_LEN];
        // Read fileList[0] from far memory
        for (int j = 0; j < MAX_FILENAME_LEN; j++) {
            nameBuf[j] = FAR_PEEK(FPR_BASE + FPR_fileList + j);
            if (nameBuf[j] == 0) break;
        }

        uint8_t isDir0 = FAR_PEEK(FPR_BASE + FPR_isDirList + 0);

        printf("%c%s%s",
               (visualIndex == 0 ? 0xFA : ' '),
               nameBuf,
               isDir0 ? "/" : " ");
    }

    // ---------------------------------------------------------
    // Display visible entries (up to 19)
    // ---------------------------------------------------------
    for (int i = visibleStart; i < visibleEnd; i++)
    {
        // Move cursor to correct row
        textGotoXY(tlX, tlY + (i - visibleStart + 1));

        // Determine cursor marker
        int isCursor = (cursorIndex == i);
        printf("%c", isCursor ? 0xFA : ' ');

        // Read filename from far memory
        char nameBuf[MAX_FILENAME_LEN];
        uint32_t base = FPR_BASE + FPR_fileList + (i * MAX_FILENAME_LEN);

        int j = 0;
        for (; j < MAX_FILENAME_LEN - 1; j++) {
            nameBuf[j] = FAR_PEEK(base + j);
            if (nameBuf[j] == 0) break;
        }
        nameBuf[MAX_FILENAME_LEN - 1] = 0;

        // Read directory flag
        uint8_t isDir = FAR_PEEK(FPR_BASE + FPR_isDirList + i);

        // Print filename (trimmed to 75 chars)
        printf("%.75s%s", nameBuf, isDir ? "/" : " ");
    }

    // ---------------------------------------------------------
    // Draw scroll indicators
    // ---------------------------------------------------------
    textGotoXY(tlX + 2, tlY + (visibleEnd - visibleStart + 1));

    char upArrow   = (cursorIndex >= (MAX_VISIBLE_FILES - 1)) ? 0xFB : ' ';
    char downArrow = (fileCount > (cursorIndex - visualIndex + MAX_VISIBLE_FILES)) ? 0xF8 : ' ';

    printf("%c %c", upArrow, downArrow);
}
/*
uint8_t filePickModal(uint8_t x, uint8_t y, char *ext0, char *ext1, char *ext2, char *ext3, bool firstTime){

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
*/


uint8_t filePickModal_far(uint8_t x, uint8_t y,
                          char *ext0, char *ext1, char *ext2, char *ext3,
                          bool firstTime)
{
    // ---------------------------------------------------------
    // Initialize far‑memory filePickRecord
    // ---------------------------------------------------------
    initFilePickRecord_far(x, y, firstTime);

    // ---------------------------------------------------------
    // Write allowed extensions into far memory
    // ---------------------------------------------------------
    for (int i = 0; i < EXT_LEN; i++) FAR_POKE(FPR_BASE + FPR_fileExts + (0 * EXT_LEN) + i, ext0[i]);
    for (int i = 0; i < EXT_LEN; i++) FAR_POKE(FPR_BASE + FPR_fileExts + (1 * EXT_LEN) + i, ext1[i]);
    for (int i = 0; i < EXT_LEN; i++) FAR_POKE(FPR_BASE + FPR_fileExts + (2 * EXT_LEN) + i, ext2[i]);
    for (int i = 0; i < EXT_LEN; i++) FAR_POKE(FPR_BASE + FPR_fileExts + (3 * EXT_LEN) + i, ext3[i]);

    // ---------------------------------------------------------
    // Read directory + sort
    // ---------------------------------------------------------
    readDirectory_far();
    sortFileList_far();

    // ---------------------------------------------------------
    // Display initial list
    // ---------------------------------------------------------
    textSetColor(10, 0);

    uint16_t scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
    displayFileList_far(scrollOffset);

    // ---------------------------------------------------------
    // Main modal loop
    // ---------------------------------------------------------
    for (;;)
    {
        uint8_t result = pickFile_far();

        // -----------------------------------------------------
        // 0 = go up one folder
        // -----------------------------------------------------
        if (result == 0)
        {
            backUpDirectory_far();
            reprepFPR_far(true);
            readDirectory_far();
            sortFileList_far();

            scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
            displayFileList_far(scrollOffset);
        }

        // -----------------------------------------------------
        // 1 = go deeper into folder
        // -----------------------------------------------------
        else if (result == 1)
        {
            char currentPath[MAX_PATH_LEN];
            char selectedFile[MAX_FILENAME_LEN];
            char finalDir[64];

            // Read currentPath from far memory
            for (int i = 0; i < MAX_PATH_LEN; i++) {
                currentPath[i] = FAR_PEEK(FPR_BASE + FPR_currentPath + i);
                if (currentPath[i] == 0) break;
            }

            // Read selectedFile from far memory
            for (int i = 0; i < MAX_FILENAME_LEN; i++) {
                selectedFile[i] = FAR_PEEK(FPR_BASE + FPR_selectedFile + i);
                if (selectedFile[i] == 0) break;
            }

            // Build new path
            sprintf(finalDir, "%s/%s", currentPath, selectedFile);

            // Reset picker state
            reprepFPR_far(true);

            // Write new path back to far memory
            for (int i = 0; i < MAX_PATH_LEN; i++) {
                FAR_POKE(FPR_BASE + FPR_currentPath + i, finalDir[i]);
                if (finalDir[i] == 0) break;
            }

            readDirectory_far();
            sortFileList_far();

            scrollOffset = FAR_PEEKW(FPR_BASE + FPR_scrollOffset);
            displayFileList_far(scrollOffset);
        }

        // -----------------------------------------------------
        // 2 = file chosen → exit modal
        // -----------------------------------------------------
        else if (result == 2)
        {
            break;
        }

        // -----------------------------------------------------
        // 3 = escape modal
        // -----------------------------------------------------
        else if (result == 3)
        {
            return 1;
        }
    }

    // ---------------------------------------------------------
    // Cleanup UI area
    // ---------------------------------------------------------
    uint8_t tlX = FAR_PEEK(FPR_BASE + FPR_tlX);
    uint8_t tlY = FAR_PEEK(FPR_BASE + FPR_tlY);

    textGotoXY(tlX, tlY);
    wipeArea_far();

    return 0;
}

	}