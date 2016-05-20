#include "main.h"

//make DC=-DXORG CL=-lX11 app

struct argdef myarg[] = { { 'd', 'd', "device"   , OPT_ARG  , NULL, "set device (default " VT_DEFAULT_DEVICE ")"},
                          { 'b', 'b', "baud"     , OPT_ARG  , NULL, "set baud rate (default " TSV(VT_DEFAULT_BAUD) ")"},
                          { 'm', 'm', "mode"     , OPT_ARG  , NULL, "set mode raw, distance, 3d, display(default media)"},
                          { 'c', 'c', "count"    , OPT_ARG  , NULL, "count alpha for media(default " TSV(VT_DEFAULT_COUNTALPHA) ")"},
                          { 'X', 'X', "pesox"    , OPT_ARG  , NULL, "sensibilità movimento x(default " TSV(VT_DEFAULT_PESOX) ")"},
                          { 'Y', 'Y', "pesoy"    , OPT_ARG  , NULL, "sensibilità movimento y(default " TSV(VT_DEFAULT_PESOY) ")"},
                          { 'Z', 'Z', "pesoz"    , OPT_ARG  , NULL, "sensibilità movimento z(default " TSV(VT_DEFAULT_PESOZ) ")"},
                          { '0', 's', "speeddx"  , OPT_ARG  , NULL, "sound speed (default " TSV(VT_DEFAULT_SPEED_DX) ")"},
                          { '1', 's', "speeddw"  , OPT_ARG  , NULL, "sound speed (default " TSV(VT_DEFAULT_SPEED_DW) ")"},
                          { '2', 's', "speedsx"  , OPT_ARG  , NULL, "sound speed (default " TSV(VT_DEFAULT_SPEED_SX) ")"},
                          { '3', 's', "speedup"  , OPT_ARG  , NULL, "sound speed (default " TSV(VT_DEFAULT_SPEED_UP) ")"},
                          { 'x', 'x', "sensorx"  , OPT_ARG  , NULL, "mm distance sensor x (default " TSV(VT_DEFAULT_VXDISTANCE_X) ")"},
                          { 'y', 'y', "sensory"  , OPT_ARG  , NULL, "mm distance sensor y (default " TSV(VT_DEFAULT_VXDISTANCE_Y) ")"},
                          { 'z', 'z', "sensorz"  , OPT_ARG  , NULL, "mm distance sensor z (default " TSV(VT_DEFAULT_VXDISTANCE_Z) ")"},
                          { 'o', 'o', "mooscount", OPT_ARG  , NULL, "moos count(default " TSV(VT_DEFAULT_MOOS_COUNT) ")"},
                          { 'l', 'l', "mooslog"  , OPT_ARG  , NULL, "moos log(default " TSV(VT_DEFAULT_MOOS_LOG) ")"},
                          { 'i', 'i', "moosinc"  , OPT_ARG  , NULL, "moos increase(default " TSV(VT_DEFAULT_MOOS_INC) ")"},
                          { 'f', 'f', "eamflags" , OPT_ARG  , NULL, "select to apply alpha <n> || <r><d><p>(default rdp )"},
                          { 'I', 'I', "zi"       , OPT_ARG  , NULL, "engage < value(default " TSV(VT_DEFAULT_ZI) ")"},
                          { 'M', 'M', "zm"       , OPT_ARG  , NULL, "engage move > value(default " TSV(VT_DEFAULT_ZM) ")"},
                          { 'D', 'D', "zd"       , OPT_ARG  , NULL, "engage action > value(default " TSV(VT_DEFAULT_ZD) ")"},
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

void_t mode_3d(virtualtouch_s* vt)
{    
    vt_raw(vt);
    
    vt_timeto_distance(vt);
    vt_inclined_plane(vt);
    
    printf("%d %d %d\n", vt->p3d.x, vt->p3d.y, vt->p3d.z);
}

void_t mode_display(virtualtouch_s* vt)
{
    #ifdef XORG
        int_t gry, grx;
        int_t gscrh, gscrw;
    
        gui_begin();
        gui_screen_size(&gscrh, &gscrw);
    
        grx = gscrw / 2;
        gry = gscrh / 2;
    #endif
    
    dbg_function();
    
    int_t my,mx;
    uint_t scrh,scrw;
    int_t oldx, oldy,rx,ry;
    uint_t cbk = CON_COLOR_BK_LRED;
    
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
        vt_raw(vt);
        vt_timeto_distance(vt);
        vt_inclined_plane(vt);
        vt_moos(vt);
        if ( vt->p3d.z >= (int_t)vt->zi)
        {
            cbk = CON_COLOR_BK_LRED;
        }
        else if ( vt->p3d.z > (int_t)vt->zm )
        {
            cbk = CON_COLOR_BK_BLACK;
        }
        else if ( vt->p3d.z > (int_t)vt->zd )
        {
            cbk = CON_COLOR_BK_GREEN;
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
        #ifdef XORG
        printf("info: grx(%5u) gry(%5d) scrw(%3u) scrh(%3u)\n", grx, gry, gscrw, gscrh);
        #endif

        rx = (vt->p3d.x * (int)(scrw/4)) / vt->peso.x;
        rx = (int)((scrw/4)/2) + rx;
        if ( rx < 1 ) rx = 1;
        if ( rx > (int)(scrw/4)) rx = (int)(scrw/4);
        
        ry = (vt->p3d.y * (int)(scrh/4)) / vt->peso.y;
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
            con_setcolor(cbk, CON_COLOR_LBLUE);
        else if (vt->p3d.x <= -(vt->vxdistance.x/4) || vt->p3d.x >= (vt->vxdistance.x/4) ||
                 vt->p3d.y <= -(vt->vxdistance.y/4) || vt->p3d.y >= (vt->vxdistance.y/4) )
            con_setcolor(cbk, CON_COLOR_GREEN);
        else
            con_setcolor(cbk, 0);
        
        
        con_gotorc(1 + oldy, 1 + oldx);
        putchar('*');
        con_setcolor(0,0);
        flush();
        
        #ifdef XORG
            /*
            grx += vt->moosval.x / 10;
            gry -= vt->moosval.y / 10;
            */
            grx = (vt->p3d.x * (int)gscrw) / (vt->peso.x * 2);
            grx = (int)gscrw / 2 + grx;
        
            gry = (vt->p3d.y * (int)gscrh) / (vt->peso.y * 2);
            gry = (int)gscrh / 2 - gry;
        
            if ( grx < 1 ) grx = 0;
            if ( grx > gscrw ) grx = gscrw;
            if ( gry < 1 ) ry = 0;
            if ( gry >  gscrh ) gry = gscrh;
            
            gui_mouse_xy(gry, grx);
        #endif
    }
    
    con_async(0);
    con_cls();
    #ifdef XORG
        gui_end();
    #endif
}

int main(int argc, char** argv) 
{
    dbg_function();
    
    char_t device[512] = VT_DEFAULT_DEVICE;
    uint_t baud = VT_DEFAULT_BAUD;
    uint_t mode = MODE_RAW;
    virtualtouch_s vt;
    mode_f mf[] = { mode_raw, mode_distance, mode_3d, mode_display};
    
    vt_init(&vt);
    
    opt_init(myarg,argv,argc);
    
    int_t ret;
    char* carg;
    char* cp;
    
    while ( (ret = opt_parse(&carg)) >= 0 )
    {
        switch ( ret )
        {
            case 'd': strcpy(device, carg); break;
            case 'b': baud = strtol(carg, NULL, 10); break;
            case 'h': opt_help(); return 0;
            case 'c': vt.countalpha = strtol(carg, NULL, 10); break;
            case 'X': vt.peso.x = strtol(carg, NULL, 10); break;
            case 'Y': vt.peso.y = strtol(carg, NULL, 10); break;
            case 'Z': vt.peso.z = strtol(carg, NULL, 10); break;
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
            case 'I': vt.zi = strtol(carg, NULL, 10); break;
            case 'M': vt.zm = strtol(carg, NULL, 10); break;
            case 'D': vt.zd = strtol(carg, NULL, 10); break;
            
            case 'f':
                vt.eamflags = 0;
                cp = carg;
                while ( *cp )
                {
                    if ( *cp == 'n' ) break;
                    if ( *cp == 'r' ) vt.eamflags |= VT_EAMF_TIME;
                    if ( *cp == 'd' ) vt.eamflags |= VT_EAMF_DISTANCE;
                    if ( *cp == 'p' ) vt.eamflags |= VT_EAMF_PLANE;
                    ++cp;
                }
            break;
            
            case 'm':
                if      ( 0 == strcmp(carg, "raw")      ) mode = MODE_RAW;
                else if ( 0 == strcmp(carg, "distance") ) mode = MODE_DISTANCE;
                else if ( 0 == strcmp(carg, "3d")       ) mode = MODE_3D;
                else if ( 0 == strcmp(carg, "display")  ) mode = MODE_DISPLAY;
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
