#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct pattern
{
	char * str;
	int length;
	int * pi;
};

void initialize(const char * str, struct pattern * P)
{
	P->length = strlen(str);
	if (NULL == (P->str = (char*)malloc(P->length))) {
		perror("can't allocate memory");
		exit(0);
	}
	if (NULL == (P->pi = (int*)malloc(sizeof(int)*(P->length)))) {
		perror("can't allocate memory");
		exit(0);
	}
	strcpy(P->str, str);

	// init pi
	int * pi = P->pi;
	int k = 0;
	pi[0] = 0;
	for (int i = 1; i < P->length; ++i) {
		while (k > 0 && str[k] != str[i])
			k = pi[k - 1];
		if ( str[k] == str[i])
			k = k + 1;
		pi[i] = k;
	}

}
// PI <--- 1

int find(const char* text, const struct pattern * P)
{	
	if (text[0] == '\0')//empty string
		return 1;

	int length = strlen(text);
	int q = 0;
	for (int i = 0; i < length; ++i) {
		while ( q > 0 && P->str[q] != text[i])
			q = P->pi[q - 1];
		if (P->str[q] == text[i])
			q = q + 1;
		if (q == P->length)
			return i - P->length + 1;
	}
	return 0;
}

void destroy(struct pattern * P)
{
	free(P->str);
	free(P->pi);
}

struct pattern PAT;
