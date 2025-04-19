#include "stdio.h"
#include "sys/ioctl.h"
#include "err.h"

int main(){
    FILE* fd = fopen("/dev/lcd160200", "w");
    if(!fd){
        err(1, "could not open device file lcd1602\n");
        return 0;
    }

    fputs("test driver", fd);
    fflush(fd);

    if(ioctl(fileno(fd), 0xC0, 1) < 0){ // Use fileno to get the file descriptor
        err(1, "ioctl failure with lcd1602");
        fclose(fd);
        return 0;
    }

    fputs("functions", fd);

    fclose(fd);

    return 0;
}
