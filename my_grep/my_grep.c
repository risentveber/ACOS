#include <stdio.h>
#include <string.h>
#include "KMP.h"

void find_in_file(const char *pattern, const char * filename)
{
	FILE * file = fopen(filename, "r");
	if (file == NULL){
		perror("cann't open file");
		exit(1);
	}

	printf("%s contains:\n", filename);
	char tmpstr[1024];
	while(!feof(file)){
		if(fgets(tmpstr, 1024, file) == NULL){
			if (feof(file) )
				return;
			perror("problems with reading");
			fclose(file);
			return;
		}
		if (find(tmpstr, &PAT)) {
			printf("%s", tmpstr);
		}
	}

	fclose(file);
	return;
}


int main(int argc, char const *argv[])
{
	if (argc < 3) {
		printf("usage of %s is:\n", argv[0] );
		printf("\t%s [pattern] [file1] ...\n", argv[0] );
		return 1;
	}
	const char * pattern = argv[1];
	initialize(pattern, &PAT);

	for (int i = 2; i < argc; ++i)
	{
		find_in_file(pattern, argv[i]);
	}

	destroy(&PAT);
	return 0;
}
