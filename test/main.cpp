#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *fp0 = NULL;
    char buffer[10];

    // init
    if (argc>1) {
        strcpy(buffer, argv[1]);
    } else {
        strcpy(buffer, "Hello");
    }
    printf("Buffer : %s\n", buffer);

    // open the device
    fp0 = fopen("/dev/ledmatrix", "r+");
    if (!fp0) {
        printf("Open /dev/ledmatrix fail!\n");
        return -1;
    }

    //fprintf(fp0, "%s", buffer);
    fwrite(buffer, sizeof(buffer), 1, fp0);

    fclose(fp0);

    return 0;
}
