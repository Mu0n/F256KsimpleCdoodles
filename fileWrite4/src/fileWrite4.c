#define F256LIB_IMPLEMENTATION

#include "f256lib.h"


static bool
write_bytes(FILE *fp, void *bytes, size_t len, char **error)
{
    size_t ret = fwrite(bytes, 1, len, fp);
	kernelNextEvent();	
    if (ret < 0) {
        *error = "fwrite returned an error.";
        return false;
    }

    if (ret < len) {
        *error = "fwrite didn't write the full payload.";
        return false;
    }

    return true;
}

static bool
write_test(FILE *fp, char **error)
{
    static char test[] = "This is a test.\n";
 
    int i;
    for (i = 0; i < 10; i++) {
        printf("Write %d\n", i);
        if (!write_bytes(fp, test, sizeof(test), error)) {
            return false;
        }
    }

    return true;
}

static bool
write_file(char *fname, 
           bool (*fn)(FILE *fp, char **error),
           char **error)
{
    FILE *fp;
    bool ret;

    fp = fopen(fname, "w");
    if (fp == NULL) {
        *error = "fopen failed";
        return false;
    };

    ret = fn(fp, error);

    if (fclose(fp) < 0) {
        if (ret) {
            *error = "close() failed.";
        }
        return false;
    };

    return ret;
}

int
main(int argc, char *argv[])
{
    char *error;
    if (!write_file("test.txt", write_test, &error)) {
        printf("%s\n", error);
		while(true)
			;
        return 1;
    }

    printf("Success!\n");
	while(true)
		;
    return 0;}
}