#include "io.h"

bool check_if_seal_file(const char* path)
{
  if (strlen(path) <= 5) return false;
  const char ext[] = ".seal";
  int pathlen = strlen(path), extlen = 5;
  for (int i = pathlen - extlen, j = 0; i < pathlen; i++, j++) {
    if (path[i] != ext[j]) return false;
  }
  return true;
}

const char* read_file(const char* path)
{
	FILE* file = fopen(path, "r");
	if (!file) {
		return NULL;
	} else {
		fseek(file, 0, SEEK_END);
		unsigned len = ftell(file);
		char* content = SEAL_CALLOC(len + 1, sizeof(char));
		fseek(file, 0, SEEK_SET);
		if (content) {
			fread(content, sizeof(char), len, file);
		}
		fclose(file);
		content[len] = '\0';
		return content;
	}
}
