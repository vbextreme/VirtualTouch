#include <Arduino.h>
#include "vt04.h"

#define WAIT(US) do{ uint16_t _s = TCNT1; while( ((TCNT1 - _s)>>1) < (US) );  }while(0)
#define mm_alpha(N) (2000/((N)+1))
    
typedef struct softtimercapture
{
    uint16_t time;
    port_t*  port;
    mask8_t  bits;
}stc_s;
    
extern "C"
{    
    void inline always_inline pinOpen(pin_s* p, uint8_t n, uint8_t mode)
    {
        pinMode(n, mode);
        if ( mode == INPUT )
            p->port = portInputRegister(digitalPinToPort(n));
        else
            p->port = portOutputRegister(digitalPinToPort(n));
            
        p->bits = digitalPinToBitMask(n);
    }
    
    void inline always_inline pinLow(pin_s* p)
    {
        *p->port &= ~(p->bits);
    }

    void inline always_inline pinHigh(pin_s* p)
    {
        *p->port |= p->bits;
    }

    uint8_t inline always_inline pinRead(pin_s* p)
    {
        return (*p->port & p->bits);
    }

    uint32_t sqrt32(uint32_t n)  
    {  
        uint32_t c = 0x8000;
        uint32_t g = 0x8000;
  
        for(;;)
        {
            if( g * g > n ) g ^= c;
            c >>= 1;
            if( c == 0 )  return g;
            g |= c;
        }
    }
};

VT04::VT04(uint8_t t, uint8_t esx, uint8_t edx, uint8_t eup, uint8_t edw)
{   
    pinOpen(&_trigger, t, OUTPUT);
    
    pinOpen(&_echo[0], esx, INPUT);
    pinOpen(&_echo[1], edx, INPUT);
    pinOpen(&_echo[2], eup, INPUT);
    pinOpen(&_echo[3], edw, INPUT);
    
    _speed[0] = _speed[1] = _speed[2] = _speed[3] = VT_DEFAULT_SPEED;
    _maxTime         = VT_DEFAULT_TIME_MAX;
    _alpha           = mm_alpha(VT_DEFAULT_ALPHA);
    _ahpla           = 1000 - mm_alpha(VT_DEFAULT_ALPHA);
    _peso[0]         = VT_DEFAULT_PESO_X;
    _peso[1]         = VT_DEFAULT_PESO_Y;
    _realDistance[0] = VT_DEFAULT_REALDISTANCE_X;
    _realDistance[1] = VT_DEFAULT_REALDISTANCE_Y;
    _moosCount       = VT_DEFAULT_MOOSCOUNT;
    _eamFlags        = VT_DEFAULT_EAMFLAGS;
}

void VT04::init(void)
{
    TCCR1A = 0;
    TCCR1B = 0;
    TCCR1B = 0x02;
    TCNT1 = 0;
    
    pinLow(&_trigger);
    
    readTime();
    uint8_t i;
    for( i = 0; i < 4; ++i)
    {
        uint32_t d = ((uint32_t)(_time[i]) * (uint32_t)(_speed[i])) / 2000;
        _distance[i] = d;
    }
}

void VT04::setMaxTime(uint16_t t)
{
    if ( t > 32000) t = 32000;
    _maxTime = t;
}

void VT04::setSpeed(uint16_t sx, uint16_t dx, uint16_t up, uint16_t dw)
{
    _speed[0] = sx;
    _speed[0] = dx;
    _speed[0] = up;
    _speed[0] = dw;
}

void VT04::setAlpha(uint16_t a)
{
    _alpha = mm_alpha(a);
    _ahpla = 1000 - mm_alpha(a);
}

void VT04::realDistance(uint16_t x, uint16_t y)
{
    _realDistance[0] = x;
    _realDistance[1] = y;
}

void VT04::setPeso(uint16_t x, uint16_t y)
{
    _peso[0] = x;
    _peso[1] = y;
}

void VT04::setEMSFlags(uint8_t f)
{
    _eamFlags = f;
}

void VT04::readTime(void)
{
    uint16_t old[4];
    uint16_t stout;
    uint8_t si = 0; 
    uint8_t state[4] = {0,0,0,0};
    uint32_t st[4];
    uint8_t i;

    old[0] = _time[0];
    old[1] = _time[1];
    old[2] = _time[2];
    old[3] = _time[3];
    
    pinHigh(&_trigger);
    WAIT(10);
    pinLow(&_trigger);
   
    _time[0] = 0;
    _time[1] = 0;
    _time[2] = 0;
    _time[3] = 0;

    uint16_t mxt = _maxTime << 1;
    
    stout = TCNT1;
    while( (si < 4) && (TCNT1 - stout < mxt) )
    {
        for ( i = 0; i < 4; ++i )
        {
            switch( state[i] )
            {
                case 0:
                    if ( pinRead(&_echo[i]) )
                    {
                        st[i] = TCNT1;
                        ++state[i];
                    }
                break;

                case 1:
                    if ( !pinRead(&_echo[i]) )
                    {
                        _time[i] = TCNT1;
                        ++state[i];
                        ++si;
                    }
                break;
            }
        }
    }
    
    for( i = 0; i < 4; ++i)
    {
        if ( _time[i] == 0 )
        {
            _time[i] = _maxTime;
        }
        else
        {
            _time[i] -= st[i];
            _time[i] >>= 1;
        }
    }

    if ( _eamFlags & VT_ALPHA_TIME )
    {
        _time[0] = ((uint32_t)_alpha * (uint32_t)_time[0] + (uint32_t)_ahpla * (uint32_t)old[0]) / 1000;
        _time[1] = ((uint32_t)_alpha * (uint32_t)_time[1] + (uint32_t)_ahpla * (uint32_t)old[1]) / 1000;
        _time[2] = ((uint32_t)_alpha * (uint32_t)_time[2] + (uint32_t)_ahpla * (uint32_t)old[2]) / 1000;
        _time[3] = ((uint32_t)_alpha * (uint32_t)_time[3] + (uint32_t)_ahpla * (uint32_t)old[3]) / 1000;
    }
}

void VT04::readDistance(void)
{
    readTime();
    uint32_t d;

    if ( _eamFlags & VT_ALPHA_DISTANCE )
    {
        d = ((uint32_t)_time[0] * (uint32_t)_speed[0]) / 2000;
        _distance[0] = ((uint32_t)_alpha * (uint32_t)d + (uint32_t)_ahpla * (uint32_t)_distance[0]) / 1000;
        d = ((uint32_t)_time[1] * (uint32_t)_speed[1]) / 2000;
        _distance[1] = ((uint32_t)_alpha * (uint32_t)d + (uint32_t)_ahpla * (uint32_t)_distance[1]) / 1000;
        d = ((uint32_t)_time[2] * (uint32_t)_speed[2]) / 2000;
        _distance[2] = ((uint32_t)_alpha * (uint32_t)d + (uint32_t)_ahpla * (uint32_t)_distance[2]) / 1000;
        d = ((uint32_t)_time[3] * (uint32_t)_speed[3]) / 2000;
        _distance[3] = ((uint32_t)_alpha * (uint32_t)d + (uint32_t)_ahpla * (uint32_t)_distance[3]) / 1000;
        return;
    }

    d = ((uint32_t)_time[0] * (uint32_t)_speed[0]) / 2000;
    _distance[0] = d;
    d = ((uint32_t)_time[1] * (uint32_t)_speed[1]) / 2000;
    _distance[1] = d;
    d = ((uint32_t)_time[2] * (uint32_t)_speed[2]) / 2000;
    _distance[2] = d;
    d = ((uint32_t)_time[3] * (uint32_t)_speed[3]) / 2000;
    _distance[3] = d;
}

void VT04::readPlane(void)
{
    readDistance();
    
    int16_t old[3];
    old[0] = _plane[0];
    old[1] = _plane[1];
    old[2] = _plane[2];
    
    _plane[2] = (_distance[0] + _distance[1] + _distance[2] + _distance[3]) / 4;
    
    int32_t h, f, l;
    int32_t b = _realDistance[0];
    
    if ( _distance[0] > _distance[1] )
    {
        h = _distance[0] - _distance[1];
        f = 1;
    }
    else
    {
        h = _distance[1] - _distance[0];
        f = -1;
    }
    
    l = sqrt32(b*b + h*h);

    _plane[0] = (((int32_t)_peso[0] * h) / l) * f;
    
    b = _realDistance[1];
    if ( _distance[2] > _distance[3] )
    {
        h = _distance[2] - _distance[3];
        f = -1;
    }
    else
    {
        h = _distance[3] - _distance[2];
        f = 1;
    }
    
    l = sqrt32(b*b + h*h);
    
    _plane[1] = (((int32_t)_peso[1] * h) / l) * f;

    if ( _eamFlags & VT_ALPHA_PLANE )
    {
        _plane[0] = ((int32_t)_alpha * (int32_t)_plane[0] + (int32_t)_ahpla * (int32_t)old[0]) / 1000;
        _plane[1] = ((int32_t)_alpha * (int32_t)_plane[1] + (int32_t)_ahpla * (int32_t)old[1]) / 1000;
        _plane[2] = ((int32_t)_alpha * (int32_t)_plane[2] + (int32_t)_ahpla * (int32_t)old[2]) / 1000;
    }
}

uint16_t VT04::readMoos(void)
{
    readPlane();
    
    int stepx = _peso[0] / _moosCount;
    int stepy = _peso[1] / _moosCount;
    
    int8_t x = _plane[0] / stepx;
    int8_t y = _plane[1] / stepy;
    
    
    return ((x << 8) | y);
}

uint16_t VT04::time(uint8_t s)
{ 
    return _time[s];
}

uint16_t VT04::distance(uint8_t s)
{ 
    return _distance[s];
}

int16_t VT04::plane(uint8_t s)
{ 
    return _plane[s];
}

void VT04::debug(void)
{
    int16_t r = readMoos();
    int8_t x = r >> 8;
    int8_t y = r & 0xFF;

    static uint32_t mst = 0;
    if ( mst == 0 ) mst = millis();

    if ( millis() - mst < 1000 ) return;
    mst = millis();

    char o[150];

    snprintf(o, sizeof o,"t[sx](%5u) t[dx](%5u) t[up](%5u) t[dw](%5u) d[sx](%4u) d[dx](%4u) d[up](%4u) d[dw](%4u) x(%3d) y(%3d) z(%4d) mx(%3d) my(%3d)\n",
             _time[0],_time[1],_time[2],_time[3],_distance[0],_distance[1],_distance[2],_distance[3],_plane[0],_plane[1],_plane[2],x,y);
    
    Serial.print(o);    
}

uint8_t VT04::serialMode(void)
{
    readTime();
    if ( Serial.available() < 1 ) return 0;
    uint8_t cmd = Serial.read();
    if ( cmd != VT_DEFAULT_COMMAND ) return cmd;
    
    Serial.write( (uint8_t)(_time[0]>>8) );
    Serial.write( (uint8_t)(_time[0] & 0xFF) );
    Serial.write( (uint8_t)(_time[1]>>8) );
    Serial.write( (uint8_t)(_time[1] & 0xFF) );
    Serial.write( (uint8_t)(_time[2]>>8) );
    Serial.write( (uint8_t)(_time[2] & 0xFF) );
    Serial.write( (uint8_t)(_time[3]>>8) );
    Serial.write( (uint8_t)(_time[3] & 0xFF) );

    return cmd;
}
