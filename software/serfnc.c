#include "serfnc.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include "main.h"

typedef struct __hsrl
{
	int f;
	srlmode_e m;
	struct termios oldsetting;
	struct termios newsetting;
}_hsrl;

hsrl_s srl_open(char* p, srlmode_e m, uint32_t baud, uint32_t bit, uint32_t parity, uint32_t bstop, uint32_t timeout, uint32_t nchar)
{
    dbg_function();
    
	_hsrl* h = malloc(sizeof(_hsrl));
	iassert(h != NULL);
	
	h->m = m;
	
	if ( m == SRL_ACANONICAL || m == SRL_ANONCANONICAL )
		h->f = open(p, O_RDWR | O_NOCTTY | O_NONBLOCK );
	else
		h->f = open(p, O_RDWR | O_NOCTTY );
		
    if ( h->f < 0)
    {
        free(h);
        dbg_warning("can't open serial");
        dbg_return(-1);
        return NULL;
    }
	
	tcgetattr(h->f,&h->oldsetting);
	bzero(&h->newsetting, sizeof(struct termios));
	
	switch (bit)
    {
		case 8: default: bit = CS8; break;
        case 7: bit = CS7; break;
        case 6: bit = CS6; break;
        case 5: bit = CS5; break;
    }  
    
    switch (bstop)
    {
		case 1: default: bstop = 0; break;
        case 2: bstop = CSTOPB; break;
    }
    
    switch (parity)
    {
		case 0: default: parity = 0; break;
        case 1: parity = PARENB | PARODD; break;
        case 2: parity = PARENB; break;
    }
	
	switch (baud)
	{
		case 4800: baud = B4800; break;
		case 9600: baud = B9600; break;
		case 19200: baud =  B19200; break;
		case 38400: baud = B38400; break;
		case 57600: baud = B57600; break;
		case 115200: baud = B115200; break;
		case 230400: baud = B230400; break;
		case 460800: baud = B460800; break;
		case 500000: baud = B500000; break;
		case 576000: baud = B576000; break;
		case 921600: baud = B921600; break;
		case 1000000: baud = B1000000; break;
		case 1152000: baud = B1152000; break;
		case 1500000: baud = B1500000; break;
		case 2000000: baud = B2000000; break;
		case 2500000: baud = B2500000; break;
		case 3000000: baud = B3000000; break;
		case 3500000: baud = B3500000; break;
		case 4000000: baud = B4000000; break;
		
        default: 
            dbg_warning("invalid baud rate");
            free(h); 
            dbg_return(-1);
        return NULL;
	}
	
	h->newsetting.c_cflag = baud | CRTSCTS | bit | parity | bstop| CLOCAL | CREAD;
	
	if ( m == SRL_CANONICAL || m == SRL_ACANONICAL)
	{
		h->newsetting.c_iflag = IGNPAR | ICRNL;
		h->newsetting.c_lflag = ICANON;
		
		h->newsetting.c_cc[VINTR] = 0;
		h->newsetting.c_cc[VQUIT] = 0;
		h->newsetting.c_cc[VERASE] = 0;
		h->newsetting.c_cc[VKILL] = 0;
		h->newsetting.c_cc[VEOF] = 4;
		h->newsetting.c_cc[VTIME] = 0;
		h->newsetting.c_cc[VMIN] = 1;
		h->newsetting.c_cc[VSWTC] = 0;
		h->newsetting.c_cc[VSTART] = 0;
		h->newsetting.c_cc[VSTOP] = 0;
		h->newsetting.c_cc[VSUSP] = 0;
		h->newsetting.c_cc[VEOL] = 0;
		h->newsetting.c_cc[VREPRINT] = 0;
		h->newsetting.c_cc[VDISCARD] = 0;
		h->newsetting.c_cc[VWERASE] = 0;
		h->newsetting.c_cc[VLNEXT] = 0;
		h->newsetting.c_cc[VEOL2] = 0;
	}
	else
	{
		h->newsetting.c_iflag = IGNPAR;
		h->newsetting.c_lflag = 0;
		h->newsetting.c_cc[VTIME] = timeout;
		h->newsetting.c_cc[VMIN] = nchar;
	}
	
	h->newsetting.c_oflag = 0;
	
	tcflush(h->f, TCIFLUSH);
	tcsetattr(h->f,TCSANOW,&h->newsetting);
	
    dbg_return(0);
	return h;
}

uint32_t srl_available(hsrl_s h)
{
    iassert(h != NULL);
    iassert(h->f > -1);
	int32_t toread;
	ioctl(h->f, FIONREAD, &toread);
    return toread;
}

uint32_t srl_read(hsrl_s h, void* data, uint32_t sz)
{
    iassert(h != NULL);
    iassert(h->f > -1);
	if ( sz >= SRL_MAX_INP ) return 0;
	return read(h->f,data,sz);
}

uint32_t srl_write(hsrl_s h, const void* data, uint32_t sz)
{
    iassert(h != NULL);
    iassert(h->f > -1);
	return ( write(h->f,data,sz) < 0 ) ? 0 : 1;
}

void srl_close(hsrl_s h)
{
    iassert(h != NULL);
    iassert(h->f > -1);
	tcsetattr(h->f,TCSANOW,&h->oldsetting);
	close(h->f);
	free(h);
}

void srl_modem(hsrl_s h, int DTR, int CTS)
{
    iassert(h != NULL);
    iassert(h->f > -1);
    
    int s;
    if ( DTR > 0 )
    {
        s = TIOCM_DTR;
        s = ioctl(h->f, TIOCMBIS, &s);
    }
    else if ( DTR == 0 )
    {
        s = TIOCM_DTR;
        s = ioctl(h->f, TIOCMBIC, &s);
    }
    
    if ( CTS > 0 )
    {
        s = TIOCM_CTS;
        s = ioctl(h->f, TIOCMBIS, &s);
    }
    else if ( CTS == 0 )
    {
        s = TIOCM_CTS;
        ioctl(h->f, TIOCMBIC, &s);
    }
}
