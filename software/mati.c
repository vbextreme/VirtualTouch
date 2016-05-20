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
    
    if ( vt->eamflags & VT_EAMF_TIME )
    {
        sensor_s m;
        uint_t alpha = mm_alpha(vt->countalpha);
        uint_t ahpla = 1000 - mm_alpha(vt->countalpha);

        serWrite(vt, PROTO_READ);
        m.sx = serRead16(vt);
        m.dx = serRead16(vt);
        m.up = serRead16(vt);
        m.dw = serRead16(vt);
    
        vt->time.dx = (alpha * m.dx + ahpla * vt->time.dx)/1000;
        vt->time.dw = (alpha * m.dw + ahpla * vt->time.dw)/1000;
        vt->time.sx = (alpha * m.sx + ahpla * vt->time.sx)/1000;
        vt->time.up = (alpha * m.up + ahpla * vt->time.up)/1000;
    }
    else
    {
        serWrite(vt, PROTO_READ);
        vt->time.sx = serRead16(vt);
        vt->time.dx = serRead16(vt);
        vt->time.up = serRead16(vt);
        vt->time.dw = serRead16(vt);
    }
    
    
    dbg_info("time: sx(%5u) dx(%5u) up(%5u) dw(%5u)", vt->time.sx, vt->time.dx, vt->time.up, vt->time.dw);
    dbg_return(0);
}

void_t vt_timeto_distance(virtualtouch_s* vt)
{
    dbg_function();
    
    if ( vt->eamflags & VT_EAMF_DISTANCE )
    {
        sensor_s m;
        uint_t alpha = mm_alpha(vt->countalpha);
        uint_t ahpla = 1000 - mm_alpha(vt->countalpha);

        m.dx = (vt->time.dx * vt->speed.dx) / 2000;
        m.dw = (vt->time.dw * vt->speed.dw) / 2000;
        m.sx = (vt->time.sx * vt->speed.sx) / 2000;
        m.up = (vt->time.up * vt->speed.up) / 2000;
    
        vt->distance.dx = (alpha * m.dx + ahpla * vt->distance.dx)/1000;
        vt->distance.dw = (alpha * m.dw + ahpla * vt->distance.dw)/1000;
        vt->distance.sx = (alpha * m.sx + ahpla * vt->distance.sx)/1000;
        vt->distance.up = (alpha * m.up + ahpla * vt->distance.up)/1000;
    }
    else
    {
        vt->distance.dx = (vt->time.dx * vt->speed.dx) / 2000;
        vt->distance.dw = (vt->time.dw * vt->speed.dw) / 2000;
        vt->distance.sx = (vt->time.sx * vt->speed.sx) / 2000;
        vt->distance.up = (vt->time.up * vt->speed.up) / 2000;
    }
    
    dbg_info("dist: sx(%5u) dx(%5u) up(%5u) dw(%5u)", vt->distance.sx, vt->distance.dx, vt->distance.up, vt->distance.dw);
    dbg_return(0);
}

void_t vt_inclined_plane(virtualtouch_s* vt)
{
    dbg_function();
    int tmp;
    int_t alpha = mm_alpha(vt->countalpha);
    int_t ahpla = 1000 - mm_alpha(vt->countalpha);
    
    tmp = (vt->distance.dx + vt->distance.sx + vt->distance.dw + vt->distance.up) / 4;
    if ( vt->eamflags & VT_EAMF_PLANE )
        vt->p3d.z = (alpha * tmp + ahpla * vt->p3d.z)/1000;
    else
        vt->p3d.z = tmp;
    
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
    
    tmp = ((vt->peso.x * h) / l) * f;
    if ( vt->eamflags & VT_EAMF_PLANE )
        vt->p3d.x = (alpha * tmp + ahpla * vt->p3d.x)/1000;
    else
        vt->p3d.x = tmp;
        
    dbg_info("plane: [p=%3d z=%3d h=%3d b=%3d l=%3d f=%3d][px=%3d]",
               vt->peso.x, vt->p3d.z, h, b, l, f, vt->p3d.x);
    
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
    
    tmp = ((vt->peso.y * h) / l) * f;
    if ( vt->eamflags & VT_EAMF_PLANE )
        vt->p3d.y = (alpha * tmp + ahpla * vt->p3d.y)/1000;
    else
        vt->p3d.y = tmp;
        
    dbg_info("plane: [p=%3d z=%3d h=%3d b=%3d l=%3d f=%3d][px=%3d]",
               vt->peso.y, vt->p3d.z, h, b, l, f, vt->p3d.y);
    
    dbg_return(0);
}

void_t vt_moos(virtualtouch_s* vt)
{
    dbg_function();
    
    int_t stepx = vt->peso.x / vt->mooscount;
    int_t stepy = vt->peso.y / vt->mooscount;
    int_t stepz = vt->peso.z / vt->mooscount;
    
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
    vt->countalpha   = VT_DEFAULT_COUNTALPHA;
    vt->peso.x       = VT_DEFAULT_PESOX;
    vt->peso.y       = VT_DEFAULT_PESOY;
    vt->peso.z       = VT_DEFAULT_PESOZ;
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
    vt->zm = VT_DEFAULT_ZM;
    vt->zd = VT_DEFAULT_ZD;
    vt->eamflags = VT_DEFAULT_EAMFLAGS;
    dbg_return(0);
}

void_t vt_close(virtualtouch_s* vt)
{
    dbg_function();    
    if ( vt->serial )
        srl_close(vt->serial);
    dbg_return(0);
}
