#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "optex.h"

#define STATUS_NEWOPT 0
#define STATUS_CONTINUEOPT 1

static struct argdef* ad;
static char** argv;
static int argc;
static int curarg;
static int curmulti;
static int idmulti;
static int status;

void opt_init(struct argdef* ar, char** mainargv, int mainargc)
{
	curarg = 0;
	curmulti = 0;
	idmulti = 0;
	status = STATUS_NEWOPT;
	ad = ar;
	argv = mainargv;
	argc = mainargc;
	
	int i = 0;
	while ( ad[i].vshort )
	{
		if ( ad[i].autoset ) *ad[i].autoset = 0;
		++i;
	}
}

static int _opt_getarg(char** carg, struct argdef* iad)
{
	*carg = NULL;
	
	if ( iad->haveparam == 0 ) return 0;
	
	if ( !argv[curarg + 1] || argv[curarg + 1][0] == '-' ) 
	{
		if ( iad->haveparam == 1 ) return -1;
		return 0;
	}
	
	++curarg;
	*carg = argv[curarg];
	return 0;
}

static struct argdef* _opt_find(char c, char* s)
{
	int i = 0;
	
	if ( s )
	{
		while ( ad[i].vshort )
		{
			if ( !strcmp(s, ad[i].vlong) ) return &ad[i];
			++i;
		}
		return NULL;
	}
	
	while ( ad[i].vshort )
	{
		if ( ad[i].vshort == c ) return &ad[i];
		++i;
	}
	return NULL;
}

int opt_parse(char** carg)
{	
	struct argdef* a;
	
	if ( status == STATUS_NEWOPT )
	{
		if ( ++curarg >= argc ) return OPT_END;
		
		if ( argv[curarg][0] == '-' )
		{
			if ( argv[curarg][1] == '-' )
			{
				if ( !(a = _opt_find(0,&argv[curarg][2])) ) return OPT_ERROR_NOOPT;
				if ( a->autoset ) 
				{
					*a->autoset = 1;
					return opt_parse(carg);
				}
				if ( _opt_getarg(carg,a) ) return OPT_ERROR_NOPARAM;
				return a->retval;
			}
			
			curmulti = curarg;
			idmulti = 1;
			status = STATUS_CONTINUEOPT;
			goto CONTINUE_OPT;
		}
		return OPT_ERROR_NOOPT;
	}

	CONTINUE_OPT:
	
	if ( !(a = _opt_find(argv[curmulti][idmulti],NULL)) ) return OPT_ERROR_NOOPT;
	if ( a->autoset ) 
	{
		*a->autoset = 1;
		++idmulti;
		if ( !argv[curmulti][idmulti] )
			status = STATUS_NEWOPT;
		return opt_parse(carg);
	}
	if ( _opt_getarg(carg,a) ) return OPT_ERROR_NOPARAM;
	++idmulti;
	if ( !argv[curmulti][idmulti] )
			status = STATUS_NEWOPT;
	return a->retval;
}

void opt_help()
{
	int i = 0;
	while ( ad[i].vshort )
	{
		printf("-%c --%s <%s> #%s\n", ad[i].vshort, ad[i].vlong, (ad[i].haveparam == 0) ? "No" : (ad[i].haveparam == 1) ? "Need" : "Optional",ad[i].descript); 
		if ( ad[i].autoset ) *ad[i].autoset = 0;
		++i;
	}
}
