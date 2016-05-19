#ifndef MAIN_H_INCLUDE
#define MAIN_H_INCLUDE

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <unistd.h>
#include <math.h>
#ifdef XORG
#include <X11/X.h> 
#include <X11/Xlib.h> 
#include <X11/Xutil.h>
#endif
#include "serfnc.h"
#include "optex.h"
#include "terminale.h"

#define NEW(TYPE) (TYPE*)malloc(sizeof(TYPE))
#define MEW(TYPE,N) (TYPE*)malloc(sizeof(TYPE) * (N))

typedef char char_t;
typedef void void_t;
typedef int  int_t;
typedef unsigned int uint_t;
typedef float float32_t;
typedef double float64_t;

#define EMPTY_MACRO do{}while(0)

typedef void_t(*dbgfail_f)(void_t);

#define DBGINFO 4
#define DBGWARNING 3
#define DBGERROR 2
#define DBGFAIL 1

#if __DEBUG != 0
    #ifndef __DEBUG_FILE
        #define __DEBUG_FILE stderr
    #endif
    #ifndef __DEBUG_LEVEL
        #define __DEBUG_LEVEL 0
    #endif
    #ifdef __DEBUG_TERMINAL
        #define __CFAIL    "\033[41m\033[30m"
        #define __CERROR   "\033[101m"
        #define __CWARNING "\033[103m"
        #define __CINFO    "\033[107m"
        #define __CRESET   "\033[m"
        #define dbg_gotorc(R,C) fprintf(__DEBUG_FILE, "\033[%d;%df", R, C)
        #define dbg_clsl(M)     fprintf(__DEBUG_FILE, "\033[%s", M);
    #else
        #define __CFAIL
        #define __CERROR
        #define __CWARNING
        #define __CINFO
        #define __CRESET
        #define dbg_gotorc(R,C) EMPTY_MACRO
        #define dbg_clsl(M)     EMPTY_MACRO
    #endif
    
    
    
    #define __dbg_print_level(L) do { switch(L){ case DBGFAIL    : fputs(__CFAIL    "fail:" __CRESET, __DEBUG_FILE);break;\
                                                 case DBGERROR   : fputs(__CERROR   "erno:" __CRESET, __DEBUG_FILE);break;\
                                                 case DBGWARNING : fputs(__CWARNING "warn:" __CRESET, __DEBUG_FILE);break;\
                                                 case DBGINFO    : fputs(__CINFO    "info:" __CRESET, __DEBUG_FILE);break;}}while(0)
    
    #define __dbg_print_reference() do{ fprintf(__DEBUG_FILE,"%s[%u] @", __FILE__, __LINE__);}while(0) 
    
    #define __dbg_print_function() do { fprintf(__DEBUG_FILE,"%s()",__FUNCTION__);} while(0)
    
    #define dbg_function() do{ __dbg_print_level(DBGINFO); __dbg_print_reference(); __dbg_print_function(); fputs("enter\n", __DEBUG_FILE); }while(0)
    
    #define dbg_return(R) do{ __dbg_print_level(DBGINFO); __dbg_print_reference(); __dbg_print_function(); fprintf(__DEBUG_FILE, "exit(%d)\n", R); }while(0)
    
    #define dbg_print(LEVEL, FORMAT, arg...) do{\
                                                  if ( LEVEL > __DEBUG_LEVEL ) break;\
                                                  fprintf(__DEBUG_FILE,"\t%s[%u]" FORMAT "\n", __FILE__, __LINE__, ## arg);\
                                                  if ( LEVEL == DBGFAIL ){\
                                                    con_async(0);\
                                                    _exit(-1);\
                                                  }\
                                             }while(0)
    
    #define dbg_info(FORMAT, arg...) dbg_print(DBGINFO, FORMAT, ##arg)
    #define dbg_warning(FORMAT, arg...) dbg_print(DBGWARNING, FORMAT, ##arg)
    #define dbg_error(FORMAT, arg...) dbg_print(DBGERROR, FORMAT, ##arg)
    #define dbg_fail(FORMAT, arg...) dbg_print(DBGFAIL, FORMAT, ##arg)
    
    


#else
    #define dbg_function() EMPTY_MACRO
    #define dbg_return(R) EMPTY_MACRO
    #define dbg_print(LEVEL, FORMAT, arg...) EMPTY_MACRO
    #define dbg_info(FORMAT, arg...) EMPTY_MACRO
    #define dbg_warning(FORMAT, arg...) EMPTY_MACRO
    #define dbg_error(FORMAT, arg...) EMPTY_MACRO
    #define dbg_fail(FORMAT, arg...) EMPTY_MACRO
    #define __CFAIL
    #define __CERROR
    #define __CWARNING
    #define __CINFO
    #define __CRESET
    #define dbg_gotorc(R,C) EMPTY_MACRO
    #define dbg_clsl(M)     EMPTY_MACRO
#endif //debug

#if __ASSERT != 0
    #ifndef __DEBUG_FILE
        #define __DEBUG_FILE stderr
    #endif
    #define iassert(A) do{ if ( !(A) ) {\
                                        fprintf(__DEBUG_FILE, "%s[%u] @%s() " __CFAIL "assertion fail" __CRESET " %s\n", __FILE__, __LINE__, __FUNCTION__, #A);\
                                        con_async(0);\
                                        _exit(-1);\
                                       }\
                         }while(0)
#else
    #define iassert(A) EMPTY_MACRO
#endif






#define MODE_RAW      0
#define MODE_DISTANCE 1
#define MODE_MEDIA    2
#define MODE_3D       3
#define MODE_DISPLAY  4
#define MODE_MOUSE    5

#define PROTO_READ 'X'

#define VT_DEFAULT_BAUD         115200
#define VT_DEFAULT_DEVICE       "/dev/ttySAC0"
#define VT_DEFAULT_COUNTALPHA   32
#define VT_DEFAULT_VXDISTANCE_X 50
#define VT_DEFAULT_VXDISTANCE_Y 55
#define VT_DEFAULT_VXDISTANCE_Z 35
#define VT_DEFAULT_SPEED_DX     331
#define VT_DEFAULT_SPEED_DW     331
#define VT_DEFAULT_SPEED_SX     331
#define VT_DEFAULT_SPEED_UP     331
#define VT_DEFAULT_PESO         80
#define VT_DEFAULT_MOOS_COUNT   9
#define VT_DEFAULT_MOOS_LOG     2
#define VT_DEFAULT_MOOS_INC     100
#define VT_DEFAULT_ZI           400

typedef struct vertex
{
    int_t x,y,z;
}vertex_s;

typedef struct sensor
{
    uint_t dx,dw,sx,up;
}sensor_s;

typedef struct virtualtouch
{
    hsrl_s serial;
    
    uint_t countalpha;
    int_t peso;
    
    //vertex_s tollerance;
    vertex_s vxdistance;
    sensor_s speed;
    
    sensor_s time;
    sensor_s distance;
    vertex_s p3d;
    
    vertex_s moos;
    vertex_s moosval;
    uint_t mooscount;
    uint_t mooslog;
    uint_t moosinc;
    
    uint_t zi;
}virtualtouch_s;


#define mm_alpha(N) (2000/((N)+1))
#define mm_calc(A,RA,N,O) (((A) * (N) + (RA) * (O))/1000)

uint_t sqrt32(uint_t n);
void_t vt_raw(virtualtouch_s* vt);
void_t vt_mmex(virtualtouch_s* vt);
void_t vt_timeto_distance(virtualtouch_s* vt);
void_t vt_distanceto_3d(virtualtouch_s* vt);
int_t vt_open_device(virtualtouch_s* vt, char_t* device, uint_t baud);
void_t vt_init(virtualtouch_s* vt);
void_t vt_close(virtualtouch_s* vt);
void_t vt_motion(virtualtouch_s* vt, int_t* mmy, int_t* mmx);
void_t vt_distanceto_fromc(virtualtouch_s* vt);
void_t vt_inclined_plane(virtualtouch_s* vt);
void_t vt_moos(virtualtouch_s* vt);

#endif
