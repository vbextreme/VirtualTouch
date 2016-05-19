#include "main.h"

uint_t sqrt32(uint_t n)  
{  
    uint_t c = 0x8000;
    uint_t g = 0x8000;
  
    for(;;)
    {
        if( g * g > n ) g ^= c;
        c >>= 1;
        if( c == 0 )  return g;
        g |= c;
    }
}

static void_t serWrite(virtualtouch_s* vt, uint8_t d)
{
    srl_write(vt->serial, &d, 1);
}

static uint16_t serRead16(virtualtouch_s* vt)
{
    uint8_t d;
    uint16_t ret;
    
    srl_read(vt->serial, &d, 1);
    ret = d << 8;
    srl_read(vt->serial, &d, 1);
    ret |= d;
    return ret;
}

void_t vt_raw(virtualtouch_s* vt)
{
    dbg_function();
    
    serWrite(vt, PROTO_READ);
    vt->time.sx = serRead16(vt);
    vt->time.dx = serRead16(vt);
    vt->time.up = serRead16(vt);
    vt->time.dw = serRead16(vt);
    
    dbg_info("time: sx(%5u) dx(%5u) up(%5u) dw(%5u)", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
    dbg_return(0);
}

void_t vt_mmex(virtualtouch_s* vt)
{
    dbg_function();
    
    sensor_s m;
    uint_t alpha = mm_alpha(vt->countalpha);
    uint_t ahpla = 1000 - mm_alpha(vt->countalpha);

    m = vt->time;
    vt_raw(vt);

    vt->time.dx = (alpha * vt->time.dx + ahpla * m.dx)/1000;
    vt->time.dw = (alpha * vt->time.dw + ahpla * m.dw)/1000;
    vt->time.sx = (alpha * vt->time.sx + ahpla * m.sx)/1000;
    vt->time.up = (alpha * vt->time.up + ahpla * m.up)/1000;
    
    dbg_info("mmex: sx(%5u) dx(%5u) up(%5u) dw(%5u)", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
    dbg_return(0);
}

void_t vt_timeto_distance(virtualtouch_s* vt)
{
    dbg_function();
    
    vt->distance.dx = (vt->time.dx * vt->speed.dx) / 2000;
    vt->distance.dw = (vt->time.dw * vt->speed.dw) / 2000;
    vt->distance.sx = (vt->time.sx * vt->speed.sx) / 2000;
    vt->distance.up = (vt->time.up * vt->speed.up) / 2000;
    
    dbg_info("mmex: sx(%5u) dx(%5u) up(%5u) dw(%5u)", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
    dbg_return(0);
}

void_t vt_inclined_plane(virtualtouch_s* vt)
{
    dbg_function();
    
    vt->p3d.z = (vt->distance.dx + vt->distance.sx + vt->distance.dw + vt->distance.up) / 4;
    
    dbg_info("plane: [z=%3d]", vt->p3d.z);
    
    int_t h, f, l;
    int_t b = vt->vxdistance.x;
    
    if (vt->distance.sx > vt->distance.dx)
    {
        h = vt->distance.sx - vt->distance.dx;
        f = 1;
    }
    else
    {
        h = vt->distance.dx - vt->distance.sx;
        f = -1;
    }
    
    l = sqrt32(b*b + h*h);
    
    vt->p3d.x = ((vt->peso * h) / l) * f;
    
    dbg_info("plane: [p=%3d z=%3d h=%3d b=%3d l=%3d f=%3d][px=%3d]",
               vt->peso, vt->p3d.z, h, b, l, f, vt->p3d.x);
    
    b = vt->vxdistance.y;
    if (vt->distance.up > vt->distance.dw)
    {
        h = vt->distance.up - vt->distance.dw;
        f = -1;
    }
    else
    {
        h = vt->distance.dw - vt->distance.up;
        f = 1;
    }
    
    l = sqrt32(b*b + h*h);
    
    vt->p3d.y = ((vt->peso * h) / l) * f;
    
    dbg_info("plane: [p=%3d z=%3d h=%3d b=%3d l=%3d f=%3d][px=%3d]",
               vt->peso, vt->p3d.z, h, b, l, f, vt->p3d.y);
    
    dbg_return(0);
}

void_t vt_moos(virtualtouch_s* vt)
{
    dbg_function();
    
    int_t stepx = vt->peso / vt->mooscount;
    int_t stepy = vt->peso / vt->mooscount;
    int_t stepz = vt->peso / vt->mooscount;
    
    vt->moos.x = vt->p3d.x / stepx;
    vt->moos.y = vt->p3d.y / stepy;
    vt->moos.z = vt->p3d.z / stepz;
    
    dbg_info("moos: x(%5u) y(%5u) z(%5u) ", vt->moos.x, vt->moos.y, vt->moos.z);
    
    double_t v = vt->moos.x;
    v *= log(vt->mooslog) * vt->moosinc;
    vt->moosval.x = v;
    
    v = vt->moos.y;
    v *= log(vt->mooslog) * vt->moosinc;
    vt->moosval.y = v;
    
    v = vt->moos.z;
    v *= log(vt->mooslog) * vt->moosinc;
    vt->moosval.z = v;
    
    dbg_info("moov: x(%5u) y(%5u) z(%5u) ", vt->moosval.x, vt->moosval.y, vt->moosval.z);
    dbg_return(0);
}

int_t vt_open_device(virtualtouch_s* vt, char_t* device, uint_t baud)
{
    dbg_function();
    
    if ( vt->serial ) srl_close(vt->serial);
    vt->serial = srl_open(device, SRL_NONCANONICAL, baud, 8, 0, 1, 0, 1);
    if ( vt->serial == NULL ) 
    {
        fputs("error open device\n", stderr);
        dbg_return(-1);
        return -1;
    }
    dbg_return(0);
    return 0;
}

void_t vt_init(virtualtouch_s* vt)
{
    dbg_function();
    vt->serial = NULL;
    vt->countalpha = VT_DEFAULT_COUNTALPHA;
    vt->peso       = VT_DEFAULT_PESO;
    vt->vxdistance.x = VT_DEFAULT_VXDISTANCE_X;
    vt->vxdistance.y = VT_DEFAULT_VXDISTANCE_Y;
    vt->vxdistance.z = VT_DEFAULT_VXDISTANCE_Z;
    vt->speed.dx = VT_DEFAULT_SPEED_DX;
    vt->speed.dw = VT_DEFAULT_SPEED_DW;
    vt->speed.sx = VT_DEFAULT_SPEED_SX;
    vt->speed.up = VT_DEFAULT_SPEED_UP;
    vt->mooscount =VT_DEFAULT_MOOS_COUNT;
    vt->moosinc = VT_DEFAULT_MOOS_INC;
    vt->mooslog = VT_DEFAULT_MOOS_LOG;
    vt->zi = VT_DEFAULT_ZI;
    dbg_return(0);
}

void_t vt_close(virtualtouch_s* vt)
{
    dbg_function();    
    if ( vt->serial )
        srl_close(vt->serial);
    dbg_return(0);
}
