#ifndef _CHECKMETHODS_HH
#define _CHECKMETHODS_HH

#include <sys/types.h>

// checkPermissions method is called when a search by permissions is performed.
// The method returns 1 when the file has owner-permission equals to st_mode.
int checkPermissions(char *pathname, mode_t st_mode);

// checkFileSize method is called when a search by size is performed.
// The method returns 1 when the size in bytes of the file addressed by pathname
// is equal to, or greater than, size.
int checkFileSize(char *pathname, off_t size);

// checkFileName method is called when a search by name is performed.
// The method returns 1 when filename1 is equal to filename2
int checkFileName(char *filename1, char *filename2);

#endif
