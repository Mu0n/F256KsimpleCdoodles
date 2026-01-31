
#include "f256lib.h"
#include "../src/muFilePicker.h"
#include "../src/muUtils.h"
#include "../src/muTextUI.h"


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
	fpr_set_currentPath("media/midi");
}

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

    if (dirOpenResult != NULL)
    {
        // Directory exists → close it and keep currentPath as-is
        fileCloseDir(dirOpenResult);
    }
    else
    {
        // Directory missing → set currentPath = "0:/"
        const char *fallback = "0:";
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
                          "mid", "", "", "", true);

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



void sortFileList_far(void)
{
    // ---------------------------------------------------------
    // 1. Write ".." into fileList[0]
    // ---------------------------------------------------------
    FAR_POKE(FPR_BASE + FPR_fileList + 0, '.');
    FAR_POKE(FPR_BASE + FPR_fileList + 1, '.');
    FAR_POKE(FPR_BASE + FPR_fileList + 2, 0);

    for (int i = 3; i < MAX_FILENAME_LEN; i++)
        FAR_POKE(FPR_BASE + FPR_fileList + i, 0);

    FAR_POKE(FPR_BASE + FPR_isDirList + 0, 1);

    // ---------------------------------------------------------
    // 2. Read fileCount
    // ---------------------------------------------------------
    uint16_t fileCount = FAR_PEEKW(FPR_BASE + FPR_fileCount);
    int start = 1;

    // ---------------------------------------------------------
    // 3. Insertion sort (folders first, then alphabetical)
    // ---------------------------------------------------------
    for (int i = start + 1; i < fileCount; i++)
    {
        // Load key entry (filename + dir flag)
        char keyName[MAX_FILENAME_LEN];
        bool keyIsDir = FAR_PEEK(FPR_BASE + FPR_isDirList + i);

        uint32_t keyBase = FPR_BASE + FPR_fileList + (i * MAX_FILENAME_LEN);
        for (int k = 0; k < MAX_FILENAME_LEN; k++) {
            keyName[k] = FAR_PEEK(keyBase + k);
            if (keyName[k] == 0) break;
        }

        int j = i - 1;

        // Shift entries until correct position is found
        while (j >= start)
        {
            bool jIsDir = FAR_PEEK(FPR_BASE + FPR_isDirList + j);

            // Compare directory priority
            bool shouldShift = false;

            if (!keyIsDir && jIsDir) {
                // key is file, j is dir → key goes after j → no shift
                break;
            }
            else if (keyIsDir && !jIsDir) {
                // key is dir, j is file → key goes before j → shift
                shouldShift = true;
            }
            else {
                // Same type → alphabetical compare
                char jName[MAX_FILENAME_LEN];
                uint32_t jBase = FPR_BASE + FPR_fileList + (j * MAX_FILENAME_LEN);

                for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                    jName[k] = FAR_PEEK(jBase + k);
                    if (jName[k] == 0) break;
                }

                if (strcasecmp_local(jName, keyName) > 0)
                    shouldShift = true;
            }

            if (!shouldShift)
                break;

            // Shift j → j+1
            uint32_t srcBase = FPR_BASE + FPR_fileList + (j * MAX_FILENAME_LEN);
            uint32_t dstBase = FPR_BASE + FPR_fileList + ((j + 1) * MAX_FILENAME_LEN);

            for (int k = 0; k < MAX_FILENAME_LEN; k++) {
                FAR_POKE(dstBase + k, FAR_PEEK(srcBase + k));
                if (FAR_PEEK(srcBase + k) == 0) break;
            }

            FAR_POKE(FPR_BASE + FPR_isDirList + (j + 1), jIsDir);

            j--;
        }

        // Insert key at j+1
        uint32_t dstBase = FPR_BASE + FPR_fileList + ((j + 1) * MAX_FILENAME_LEN);
        for (int k = 0; k < MAX_FILENAME_LEN; k++) {
            FAR_POKE(dstBase + k, keyName[k]);
            if (keyName[k] == 0) break;
        }

        FAR_POKE(FPR_BASE + FPR_isDirList + (j + 1), keyIsDir);
    }
}


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


void initFilePickRecord_far(uint8_t x, uint8_t y, bool firstTime)
{
    // ---------------------------------------------------------
    // Clear selectedFile (120 bytes)
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
        FAR_POKE(FPR_BASE + FPR_selectedFile + i, 0);
    }

/*
    // ---------------------------------------------------------
    // Clear currentPath (60 bytes)
    // ---------------------------------------------------------
    for (int i = 0; i < MAX_PATH_LEN; i++) {
        FAR_POKE(FPR_BASE + FPR_currentPath + i, 0);
    }
*/
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

uint8_t filePickModal_far(uint8_t x, uint8_t y,
                          char *ext0, char *ext1, char *ext2, char *ext3,
                          bool firstTime)
{
	const char *queryFudge[] = {
"                                                       ",
"        Choose a MIDI File for playback                ", 
"     The default directory is 0:media/midi/            ",
"                                                       "
};

modalHelp(queryFudge, sizeof(queryFudge)/sizeof(queryFudge[0]));
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

	eraseModalHelp(sizeof(queryFudge)/sizeof(queryFudge[0]));
    return 0;
}

	}