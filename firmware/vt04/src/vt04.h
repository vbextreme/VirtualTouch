#ifndef __VT04_H__
#define __VT04_H__

#include <Arduino.h>

#define always_inline __attribute((always_inline))

#define VT_SX 0
#define VT_DX 1
#define VT_UP 2
#define VT_DW 3
#define VT_X  0
#define VT_Y  1
#define VT_Z  2

#define VT_ALPHA_TIME     0x01
#define VT_ALPHA_DISTANCE 0x02
#define VT_ALPHA_PLANE    0x04

#define VT_DEFAULT_TIME_MAX        30000
#define VT_DEFAULT_COMMAND         'X'
#define VT_DEFAULT_SPEED           331
#define VT_DEFAULT_ALPHA           32
#define VT_DEFAULT_REALDISTANCE_X  50
#define VT_DEFAULT_REALDISTANCE_Y  55
#define VT_DEFAULT_PESO_X          50
#define VT_DEFAULT_PESO_Y          55
#define VT_DEFAULT_MOOSCOUNT       9
#define VT_DEFAULT_EAMFLAGS        ( VT_ALPHA_TIME | VT_ALPHA_DISTANCE)

#define VT_TIMER 1

#if VT_TIMER == 0
    #define vt_timer_clock() ({\
                                 uint16_t urclk;\
                                 uint8_t  ovf0;\
                                 uint8_t  clk0;\
                                 uint8_t sr = SREG;\
                                 cli();\
                                 ovf0 = timer0_overflow_count;\
                                 clk0 = TCNT0;\
                                 SREG = sr;\
                                 urclk = (((uint16_t)ovf0 << 8) | (uint16_t)clk0) << 3;\
                                 urclk;\
                              })
#elif VT_TIMER == 1
    #define vt_timer_clock() TCNT1
#else
    #pragma error "timer not supported"
#endif

typedef uint8_t mask8_t;
typedef volatile uint8_t port_t;
    
typedef struct pin  
{
    port_t* port;
    mask8_t bits;
}pin_s;


class VT04
{
    public:
      VT04(uint8_t t, uint8_t esx, uint8_t edx, uint8_t eup, uint8_t edw);
      void init(void);
      void setMaxTime(uint16_t t);
      void setSpeed(uint16_t sx, uint16_t dx, uint16_t up, uint16_t dw);
      void setAlpha(uint16_t a);
      void realDistance(uint16_t x, uint16_t y);
      void setPeso(uint16_t x, uint16_t y);
      void setEMSFlags(uint8_t f);
      void readTime(void);
      void readDistance(void);
      void readPlane(void);
      uint16_t readMoos(void);
      uint16_t inline always_inline time(uint8_t s);
      uint16_t inline always_inline distance(uint8_t s);
      int16_t  inline always_inline plane(uint8_t s);
      void debug(void);
      uint8_t serialMode(void);
      
      
    private:
      uint16_t _maxTime;
      uint16_t _speed[4];
      uint16_t _time[4];
      uint16_t _distance[4];
      uint16_t _alpha;
      uint16_t _ahpla;
      uint16_t _realDistance[2];
      int16_t _plane[3];
      uint16_t _peso[2];
      uint8_t _moosCount;
      uint8_t _eamFlags;
      pin_s _trigger;
      pin_s _echo[4];
      
      
};









#endif
