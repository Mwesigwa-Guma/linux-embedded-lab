#include "stdio.h";

int main(){
    FILE* fd = fopen("/dev/lcd160200", "w");
    if(fd < 0){
        printf("could not open device file lcd1602 \n");
        return 0;
    }

    fputs("testing driver", fd);

    if(ioctl(fd, 0xC0, 0) < 0){
        preintf("ioctl failure with ");
        fclose(fd);
        return 0;
    }

    fputs("functions", fd);

    fclose(fd);
    return 0;
}