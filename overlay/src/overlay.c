/*
 *	Copyright (c) 2024 Scott Duensing, scott@kangaroopunch.com
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in
 *	all copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */


#define F256LIB_IMPLEMENTATION
#include "f256lib.h"


void firstSegment(int arg1, int arg2);
void secondSegment(int arg1, int arg2);
void moreFirstSegment(int arg1, int arg2);


// This is the first segment we've defined.  The linker will place this code in
// the first available far memory slot (default 0x10000).
#define SEGMENT_FIRST

void firstSegment(int arg1, int arg2) {
	printf("firstSegment = %d\n", arg1 + arg2);
	secondSegment(arg1, arg2);
}

// This is the second segment we've defined.  The linker will place this code in
// the next available far memory slot (default 0x12000).
#define SEGMENT_SECOND

void secondSegment(int arg1, int arg2) {
	printf("secondSegment = %d\n", arg1 + arg2);
	moreFirstSegment(arg1, arg2);
}

// Back to the first segment.  The linker will place this code immediately
// after the previous first segment code.
#define SEGMENT_FIRST

void moreFirstSegment(int arg1, int arg2) {
	printf("moreFirstSegment = %d\n", arg1 + arg2);
}

// Back to near memory, the 64k visible to the processor.  By default, this
// segment begins at 0x300.
#define SEGMENT_MAIN

int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	// The overlay tool will generate trampoline macros for each function
	// that is located in a far segment so no code changes are required.
	firstSegment(1, 2);

	// Spin.
	for (;;);

	return 0;
}
