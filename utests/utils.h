#ifndef __UTILS_UTEST__
#define __UTILS_UTEST__

int read_file(int fd, char **buffer, size_t *length);
char *find_file_path(char *file_name);
int find_load_file(char *path, char **file_content);
int find_load_photo(char *path, char **file_content, size_t *length);

#endif
