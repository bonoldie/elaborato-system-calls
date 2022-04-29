#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h>

#include "checkMethods.h"
#include "errExit.h"

// mode variable selects the seach method (by name, permissions or size)
unsigned int mode =        0;
#define SC_BY_NAME         1
#define SC_BY_PERMISSIONS  2
#define SC_BY_SIZE         3
// if mode is SC_BY_NAME, then name2match is the file name we have to search for
char *name2match = NULL;
// if mode is SC_BY_PERMISSIONS, then mode2match is the file permissions we have to
// search for
mode_t mode2match = 0;
// if mode is SC_BY_SIZE, then size2match is the file size we have to search for
off_t size2match = 0;

// the current seachPath. It is updated by search method to recursively
// traverse the filesystem
char seachPath[250];

// append2Path enxtends the current seachPath with a given directory.
// it returns the length of the current seachPath before extending it
size_t append2Path(char *directory) {
    size_t lastPathEnd = strlen(seachPath);
    // extends current seachPath: seachPath + / + directory
    strcat(strcat(&seachPath[lastPathEnd], "/"), directory);
    return lastPathEnd;
}

// search method recursively traverse the filesystem taking the value of seachPath
// as root directory
void search() {
    // open the current seachPath
    DIR *dirp = opendir(seachPath);
    if (dirp == NULL) return;
    // readdir returns NULL when end-of-directory is reached.
    // In oder to get when an error occurres, we set errno to zero, and the we
    // call readdir. If readdir returns NULL, and errno is different from zero,
    // an error must have occurred.
    errno = 0;
    // iter. until NULL is returned as a result
    struct dirent *dentry;
    while ( (dentry = readdir(dirp)) != NULL) {
        // Skip . and ..
        if (strcmp(dentry->d_name, ".") == 0 ||
            strcmp(dentry->d_name, "..") == 0)
        {  continue;  }

        // is the current dentry a regular file?
        if (dentry->d_type == DT_REG) {
            // exetend current seachPath with the file name
            size_t lastPath = append2Path(dentry->d_name);
            // checking the properties of the file according to mode
            int match =
              // if mode is equal to SC_BY_NAME, then we check the file name
              (mode == SC_BY_NAME)? checkFileName(dentry->d_name, name2match)
              // if mode is equal to SC_BY_PERMISSIONS, then we check the file permissions
            : (mode == SC_BY_PERMISSIONS)? checkPermissions(seachPath, mode2match)
              // if mode is equal to SC_BY_NAME, then we check the file size
            : (mode == SC_BY_SIZE)? checkFileSize(seachPath, size2match) : 0;
            // if match is 1, then a research ...
            if (match == 1)
                printf("%s\n", seachPath);
            // reset current seachPath
            seachPath[lastPath] = '\0';
        // is the current dentry a directory
        } else if (dentry->d_type == DT_DIR) {
            // exetend current seachPath with the directory name
            size_t lastPath = append2Path(dentry->d_name);
            // call search method
            search();
            // reset current seachPath
            seachPath[lastPath] = '\0';
        }
        errno = 0;
    }

    if (errno != 0)
        errExit("readdir failed");

    if (closedir(dirp) == -1)
        errExit("closedir failed");
}

// The string2Mode method turns the file permissions from string format into
// mode_t format (bit mask). The least significant bit of mode_t follows the
// following template:
// |r|w|x| |r|w|x| |r|w|x|  (owner, group, other)
// The string r-x is converted into:
// |1|0|1| |0|0|0| |0|0|0|  (owner, group, other)
mode_t string2Mode(char * p) {
    mode_t mode = 0;

    if (strlen(p) == 3) {
        if (p[0] == 'r')
            mode |= 1 << 8;
        if (p[1] == 'w')
            mode |= 1 << 7;
        if (p[2] == 'x')
            mode |= 1 << 6;
    }

    return mode;
}

int main (int argc, char *argv[]) {
    if (argc != 3 && argc != 4) {
        printf("Usage: \n");
        printf("%s <directory> <filename>  (search by name)\n", argv[0]);
        printf("%s -p P <directory>\t  (search by permissions)\n", argv[0]);
        printf("%s -s S <directory>\t  (search by size)\n", argv[0]);
        return 1;
    }

    if (argc == 3) {
        mode = SC_BY_NAME;
        strcat(seachPath, argv[1]);
        name2match = argv[2];
    }
    else {
        if (strcmp(argv[1], "-p") == 0) {
            mode = SC_BY_PERMISSIONS;
            mode2match = string2Mode(argv[2]);
            strcat(seachPath, argv[3]);
        } else if (strcmp(argv[1], "-s") == 0) {
            mode = SC_BY_SIZE;
            size2match = atoi(argv[2]);
            strcat(seachPath, argv[3]);
        } else {
            printf("Flag %s not recognizes\n", argv[1]);
            return 1;
        }
    }

    search();
    return 0;
}
