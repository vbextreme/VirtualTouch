#include "main.h"

//make DC=-DXORG CL=-lX11 app

struct argdef myarg[] = { { 'd', 'd', "device"   , OPT_ARG  , NULL, "set device (default /dev/ttySAC0)"},
                          { 'b', 'b', "baud"     , OPT_ARG  , NULL, "set baud rate (default 9600)"},
                          { 'm', 'm', "mode"     , OPT_ARG  , NULL, "set mode raw, distance, media, 3d, display, mouse(default media)"},
                          { 'c', 'c', "count"    , OPT_ARG  , NULL, "count alpha for media(default 64)"},
                          { 'p', 'p', "peso"     , OPT_ARG  , NULL, "sensibilità movimento(default 30)"},
                          { '0', 's', "speeddx"  , OPT_ARG  , NULL, "sound speed (default 331)"},
                          { '1', 's', "speeddw"  , OPT_ARG  , NULL, "sound speed (default 331)"},
                          { '2', 's', "speedsx"  , OPT_ARG  , NULL, "sound speed (default 331)"},
                          { '3', 's', "speedup"  , OPT_ARG  , NULL, "sound speed (default 331)"},
                          { 'x', 'x', "sensorx"  , OPT_ARG  , NULL, "mm distance sensor x (default 50)"},
                          { 'y', 'y', "sensory"  , OPT_ARG  , NULL, "mm distance sensor y (default 55)"},
                          { 'z', 'z', "sensorz"  , OPT_ARG  , NULL, "mm distance sensor z (default 35)"},
                          { 'o', 'o', "mooscount", OPT_ARG  , NULL, "moos count(default 9)"},
                          { 'l', 'l', "mooslog"  , OPT_ARG  , NULL, "moos log(default 2)"},
                          { 'i', 'i', "moosinc"  , OPT_ARG  , NULL, "moos increase(default 100)"},
                          { 'Z', 'Z', "zi"       , OPT_ARG  , NULL, "engage < value(default 200)"},
						  { 'h', 'h', "help"     , OPT_NOARG, NULL, "view help"},
						  {   0,   0, NULL       , OPT_ARG  , NULL, NULL}
						};

typedef void_t(*mode_f)(virtualtouch_s*);

#ifdef XORG

Display *dsp;
Window rootwindow;

void_t gui_begin(void_t)
{
    dbg_function();
    dsp = XOpenDisplay(0);
    rootwindow = XRootWindow(dsp, 0);
    dbg_return(0);
}

void_t gui_end(void_t)
{
    dbg_function();
    XCloseDisplay(dsp);
    dbg_return(0);
}

void_t gui_mouse_xy(uint_t y, uint_t x)
{
    dbg_function();
    XSelectInput(dsp, rootwindow, KeyReleaseMask);
    XWarpPointer(dsp, None, rootwindow, 0, 0, 0, 0, x, y);
    XFlush(dsp);
    dbg_return(0);
}

void_t gui_screen_size(int_t* h, int_t* w)
{
    dbg_function();
    XWindowAttributes xwAttr;
    XGetWindowAttributes( dsp, rootwindow, &xwAttr );
    *w = xwAttr.width;
    *h = xwAttr.height;
    dbg_return(0);
}

#endif

void_t mode_raw(virtualtouch_s* vt)
{
    vt_raw(vt);
    printf("%u %u %u %u\n", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
}

void_t mode_distance(virtualtouch_s* vt)
{
    vt_raw(vt);
    vt_timeto_distance(vt);
    printf("%u %u %u %u\n", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
}

void_t mode_media(virtualtouch_s* vt)
{
    uint_t i = vt->countalpha;
    
    vt_raw(vt);
    
    while( i-- > 0)
        vt_mmex(vt);
    
    vt_timeto_distance(vt);
    
    printf("%u %u %u %u\n", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
}

void_t mode_3d(virtualtouch_s* vt)
{
    uint_t i = vt->countalpha;
    
    vt_raw(vt);
    
    while( i-- > 0)
        vt_mmex(vt);
    
    vt_timeto_distance(vt);
    vt_inclined_plane(vt);
    
    printf("%d %d %d\n", vt->p3d.x, vt->p3d.y, vt->p3d.z);
}

void_t mode_display(virtualtouch_s* vt)
{
    dbg_function();
    
    int_t my,mx;
    uint_t scrh,scrw;
    int_t oldx, oldy,rx,ry;
    
    dbg_info("draw border");
    
    con_getmaxrc(&scrh, &scrw);
    oldx = rx = scrw/4;
    oldy = ry = scrh/4;
    
    con_cls();
    vt_raw(vt);
    
    con_async(1);
    
    ///container
    con_gotorc(1, 1);
    for (mx = 0; mx < (int)scrw / 4 + 2; ++mx)
        putchar('#');
        
    con_gotorc(scrh/4, 1);
    for (mx = 0; mx < (int)scrw / 4 + 2; ++mx)
        putchar('#');
    
    for (my = 0; my < (int)scrh / 4; ++my)
    {
        con_gotorc(my,1);
        putchar('#');
        con_gotorc(my, scrw / 4 + 2);
        putchar('#');
    }
    
    while( !con_kbhit() )
    {
        vt_mmex(vt);
        vt_timeto_distance(vt);
        vt_inclined_plane(vt);
        vt_moos(vt);
        if ( vt->p3d.z > (int_t)vt->zi)
        {
            con_gotorc(18,1);
            printf("out of rage\n");
            flush();
            con_delay(50);
            continue;
        }
        
        con_gotorc(1 + oldy, 1 + oldx);
        putchar(' ');
        con_gotorc(18,1);
        printf("           \n");
        printf("time: sx(%5u) dx(%5u) up(%5u) dw(%5u)\n", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
        printf("dist: sx(%5u) dx(%5u) up(%5u) dw(%5u)\n", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
        printf("3dpo:  x(%5d)  y(%5d)  z(%5d)\n", vt->p3d.x, vt->p3d.y, vt->p3d.z);
        printf("moos:  x(%5d)  y(%5d)  z(%5d) vx(%5d) vy(%5d) vz(%5d)\n", vt->moos.x, vt->moos.y, vt->moos.z, vt->moosval.x, vt->moosval.y, vt->moosval.z);
        printf("info: rx(%5u) ry(%5d)\n", rx, ry);

        rx = (vt->p3d.x * (int)(scrw/4)) / vt->peso;
        rx = (int)((scrw/4)/2) + rx;
        if ( rx < 1 ) rx = 1;
        if ( rx > (int)(scrw/4)) rx = (int)(scrw/4);
        
        ry = (vt->p3d.y * (int)(scrh/4)) / vt->peso;
        ry = (int)((scrh/4)/2) - ry;
        if ( ry < 1 ) ry = 1;
        if ( ry >= (int)(scrh/4)-1) ry = (int)(scrh/4)-2;
        
        oldx=rx;
        oldy=ry;
        
        con_setcolor(CON_COLOR_BK_BLACK,CON_COLOR_RED);
        con_gotorc(1 + (scrh/4)/2, 1 + (scrw/4)/2);
        putchar('+');
        
        if (vt->p3d.x <= -(vt->vxdistance.x/3) || vt->p3d.x >= (vt->vxdistance.x/3) ||
            vt->p3d.y <= -(vt->vxdistance.y/3) || vt->p3d.y >= (vt->vxdistance.y/3) )
            con_setcolor(0, CON_COLOR_LBLUE);
        else if (vt->p3d.x <= -(vt->vxdistance.x/4) || vt->p3d.x >= (vt->vxdistance.x/4) ||
                 vt->p3d.y <= -(vt->vxdistance.y/4) || vt->p3d.y >= (vt->vxdistance.y/4) )
            con_setcolor(0, CON_COLOR_GREEN);
        else
            con_setcolor(0, 0);
        
        
        con_gotorc(1 + oldy, 1 + oldx);
        putchar('*');
        con_setcolor(0,0);
        
        flush();
    }
    
    con_async(0);
    con_cls();
}

void_t mode_mouse(virtualtouch_s* vt)
{
    #ifdef XORG
    int_t ry,rx;
    int_t scrh,scrw;
    //int_t bstate = 0;
    
    gui_begin();
    gui_screen_size(&scrh, &scrw);
    
    rx = scrw / 2;
    ry = scrh / 2;
    
    puts("press any key to exit");
    flush();
    
    vt_raw(vt);
    
    con_async(1);
    
    con_cls();
    while( !con_kbhit() )
    {
        vt_mmex(vt);
        vt_timeto_distance(vt);
        vt_inclined_plane(vt);
        vt_moos(vt);
        if ( vt->p3d.z > (int_t)vt->zi) {con_delay(50); continue;}
        
        rx += vt->moosval.x / 10;
        ry -= vt->moosval.y / 10;
        
        if ( rx < 1 ) rx = 0;
        if ( rx > scrw ) rx = scrw;
        if ( ry < 1 ) ry = 0;
        if ( ry >  scrh ) ry = scrh;
        
        
        gui_mouse_xy(ry, rx);
        con_gotorc(1,1);
        /*
        printf("time: sx(%5u) dx(%5u) up(%5u) dw(%5u)\n", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
        printf("dist: sx(%5u) dx(%5u) up(%5u) dw(%5u)\n", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
        printf("3dpo:  x(%5d)  y(%5d)  z(%5d)\n", vt->p3d.x, vt->p3d.y, vt->p3d.z);
        printf("moos:  x(%5d)  y(%5d)  z(%5d) vx(%5d) vy(%5d) vz(%5d)\n", vt->moos.x, vt->moos.y, vt->moos.z, vt->moosval.x, vt->moosval.y, vt->moosval.z);
        printf("info: rx(%5u) ry(%5d) bs(%5d)\n", rx, ry, bstate);
        */
        con_delay(5);
    }
    
    con_async(0);
    gui_end();
    
    #else
    puts("xorg not enable");
    return;
    #endif
}

int main(int argc, char** argv) 
{
    dbg_function();
    
    char_t device[512] = VT_DEFAULT_DEVICE;
    uint_t baud = VT_DEFAULT_BAUD;
    uint_t mode = MODE_MEDIA;
    virtualtouch_s vt;
    mode_f mf[] = { mode_raw, mode_distance, mode_media, mode_3d, mode_display, mode_mouse};
    
    vt_init(&vt);
    
    opt_init(myarg,argv,argc);
    
    int_t ret;
    char* carg;
    
    while ( (ret = opt_parse(&carg)) >= 0 )
    {
        switch ( ret )
        {
            case 'd': strcpy(device, carg); break;
            case 'b': baud = strtol(carg, NULL, 10); break;
            case 'h': opt_help(); return 0;
            case 'c': vt.countalpha = strtol(carg, NULL, 10); break;
            case 'p': vt.peso = strtol(carg, NULL, 10); break;
            case '0': vt.speed.dx = strtol(carg, NULL, 10); break;
            case '1': vt.speed.dw = strtol(carg, NULL, 10); break;
            case '2': vt.speed.sx = strtol(carg, NULL, 10); break;
            case '3': vt.speed.up = strtol(carg, NULL, 10); break;
            case 'x': vt.vxdistance.x = strtol(carg, NULL, 10); break;
            case 'y': vt.vxdistance.y = strtol(carg, NULL, 10); break;
            case 'z': vt.vxdistance.z = strtol(carg, NULL, 10); break;
            case 'o': vt.mooscount = strtol(carg, NULL, 10); break;
            case 'l': vt.mooslog = strtol(carg, NULL, 10); break;
            case 'i': vt.moosinc = strtol(carg, NULL, 10); break;
            case 'Z': vt.zi = strtol(carg, NULL, 10); break;
            
            case 'm':
                if      ( 0 == strcmp(carg, "raw")      ) mode = MODE_RAW;
                else if ( 0 == strcmp(carg, "distance") ) mode = MODE_DISTANCE;
                else if ( 0 == strcmp(carg, "media")    ) mode = MODE_MEDIA;
                else if ( 0 == strcmp(carg, "3d")       ) mode = MODE_3D;
                else if ( 0 == strcmp(carg, "display")  ) mode = MODE_DISPLAY;
                else if ( 0 == strcmp(carg, "mouse")  ) mode = MODE_MOUSE;
                else {fprintf(stderr,"error mode\n"); return -1;} 
            break;
            
            default:
                 fprintf(stderr,"error argument\n");
                 dbg_return(-1);
            return -1;
        }
    }
    
    if ( vt_open_device(&vt, device, baud) ) return -1;
    
    mf[mode](&vt);
    
    vt_close(&vt);
    
    dbg_return(0);
    return 0;
}
