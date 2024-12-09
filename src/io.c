#include "io.h"
#include <stdio.h>
#include <stdlib.h>

const char* read_file(const char* path)
{
	FILE* file = fopen(path, "r");
	if (!file) {
		printf("No file found named \"%s\"\n", path);
		return (void*)0;
	} else {
		fseek(file, 0, SEEK_END);
		unsigned len = ftell(file);
		char* content = calloc(len + 1, sizeof(char));
		fseek(file, 0, SEEK_SET);
		if (content) {
			fread(content, sizeof(char), len, file);
		}
		fclose(file);
		content[len] = '\0';
		return content;
	}
}
