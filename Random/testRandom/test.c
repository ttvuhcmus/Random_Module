#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int open_test() {
    int fd = open("/dev/random_device", O_RDWR);
    
    if (fd < 0) {
        printf("Can not open the device file \n");
        exit(1);
    }

    int num;
    read(fd, &num, sizeof(num));

    printf("%d \n", num);

    return fd;
}

void close_test(int fd) {
    close(fd);
}

int main() {
    int fd = open_test();
    close_test(fd);
    
    return 0;   
}