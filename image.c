#include "image.h"

#include <stdio.h>  // size_t, perror, fprintf, stderr
#include <stdlib.h>  // NULL
#include <sys/mman.h>  // PROT_READ, PROT_WRITE, MAP_PRIVATE, MAP_FAILED
#include <fcntl.h>  // open, O_RDONLY
#include <unistd.h>  // close
#include <sys/stat.h>  // stat, fstat
// #include <string.h>


Image *load(const char *file_path) {
    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return NULL;
    }

    size_t file_size = st.st_size;
    if (file_size < 4) {
        fprintf(stderr, "File is too small to contain valid width/height information.\n");
        close(fd);
        return NULL;
    }

    uint8_t *file_data = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    close(fd);

    Image *image = (Image *)malloc(sizeof(Image));
    if (!image) {
        perror("malloc");
        munmap(file_data, file_size);
        return NULL;
    }

    image->width = file_data[0] | (file_data[1] << 8);
    image->height = file_data[2] | (file_data[3] << 8);
    image->data = file_data + 4;
    return image;
}

void unload(Image *image) {
    if (image) {
        size_t file_size = 4 + image->width * image->height;
        munmap(image->data - 4, file_size);
        free(image);
    }
}
