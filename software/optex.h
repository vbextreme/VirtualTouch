#ifndef __OPTEX_H_INCLUDE__
#define __OPTEX_H_INCLUDE__

#define OPT_END -1
#define OPT_ERROR_NOOPT -2
#define OPT_ERROR_NOPARAM -3
#define OPT_NOARG 0
#define OPT_ARG 1
#define OPT_OPTIONALARG 2

struct argdef
{
	int retval;
	char vshort;
	char* vlong;
	int haveparam;
	int* autoset;
	char* descript;
};

void opt_init(struct argdef* ar, char** mainargv, int mainargc);
int opt_parse(char** carg);
void opt_help();

#endif

