#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

int read_file(int fd, char **buffer, size_t *length)
{
	int result = -1, bytes = 0;
	size_t chunk = 256;
	char *tmp = NULL;

	*length = chunk + 1;
	*buffer = (char *) malloc(*length);
	if (!buffer)
		goto exit;

	while (((result = read(fd, (*buffer + bytes), chunk)) != -1) &&
	       (result != 0)) {
		*length += chunk;
		tmp = realloc(*buffer, *length + chunk);
		if (!tmp) {
			result = -1;
			goto error;
		}
		*buffer = tmp;
		bytes += result;
	}

	tmp[bytes] = '\0';
	result = 0;
	goto exit;

error:
	free(*buffer);
	*buffer = NULL;

exit:
	return result;

}

char *find_file_path(char *file_name)
{
	char *path, *tmp = NULL, *pos;
	int len = 0;
	path = getcwd(NULL, 0);
	if (path == NULL)
		printf("cannot get working directory!");

	/* This should run in a subdirectory 'build' */
	len = strrchr(path, '/') - path;
	tmp = malloc(len + strlen(file_name) + 1);

	pos = strstr(path, "build");
	if (pos) {
		--pos;
		*pos = '\0';
	}

	strcpy(tmp, path);
	strncat(tmp, file_name, len + strlen(file_name) + 1);

	free(path);
	return tmp;

}


int find_load_file(char *path, char **file_content)
{
  	int fd, res;
	size_t len = 0;
	char *file_name = path;
	char *tmp;

	tmp = find_file_path(file_name);
	fd = open(tmp, O_RDONLY);
	if (fd == -1) {
		printf("cannot open XML file: Path = %s.\n",
			tmp);
		return -1;
	}

	res = read_file(fd, file_content, &len);
	free(tmp);
	close(fd);

	return res;
}

int find_load_photo(char *path, char **file_content, size_t *length)
{
  	int fd, res;
	char *file_name = path;
	char *tmp;

	tmp = find_file_path(file_name);
	fd = open(tmp, O_RDONLY);
	if (fd == -1) {
		printf("cannot open photo file: Path = %s.\n",
			tmp);
		return -1;
	}

	res = read_file(fd, file_content, length);
	free(tmp);
	close(fd);

	return res;
}

