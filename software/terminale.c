#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/input.h>
#include <errno.h>
#include <termios.h>
#include "terminale.h"

static struct termios initial_settings, new_settings;
static bool_t asyncmode = FALSE;
static uint32_t _mouse_x = 0;
static uint32_t _mouse_y = 0;
static uint32_t _mouse_b = 0;

#define   RD_EOF   -1
#define   RD_EIO   -2
#define DRD_EOF   -1
#define DRD_EIO   -2

#define con_puts(S) print(S)

struct cdirectrw
{
    int fd;
    int saved_errno;
    struct termios  saved;
    struct termios  temporary;
};


inline int con_drd(struct cdirectrw* dc)
{
    unsigned char   buffer[4];
    ssize_t         n;

    while (1) {

        n = read(dc->fd, buffer, 1);
        if (n > (ssize_t)0)
            return buffer[0];

        else
        if (n == (ssize_t)0)
            return DRD_EOF;

        else
        if (n != (ssize_t)-1)
            return DRD_EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return DRD_EIO;
    }
}

inline int con_dwr(struct cdirectrw* dc, const char *const data, const size_t bytes)
{
    const char       *head = data;
    const char *const tail = data + bytes;
    ssize_t           n;

    while (head < tail) {

        n = write(dc->fd, head, (size_t)(tail - head));
        if (n > (ssize_t)0)
            head += n;

        else
        if (n != (ssize_t)-1)
            return EIO;

        else
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
            return errno;
    }

    return 0;
}

inline void con_dsrwhyde(struct cdirectrw* dc)
{
    dc->temporary.c_lflag &= ~ICANON;
    dc->temporary.c_lflag &= ~ECHO;
    dc->temporary.c_cflag &= ~CREAD;
}

inline int con_dsetting(struct cdirectrw* dc)
{
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->temporary);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;

    return 0;
}

inline int con_drestore(struct cdirectrw* dc)
{
    int result;
    do {
        result = tcsetattr(dc->fd, TCSANOW, &dc->saved);
    } while (result == -1 && errno == EINTR);

    if (result == -1) return errno;
    errno = dc->saved_errno;
    return 0;
}

int con_dopen(struct cdirectrw* dc)
{
    const char *dev;
    int result,retval;

    dev = ttyname(STDIN_FILENO);
    if (!dev)
        dev = ttyname(STDOUT_FILENO);
    if (!dev)
        dev = ttyname(STDERR_FILENO);
    if (!dev) {errno = ENOTTY;return -1;}

    do {
        dc->fd = open(dev, O_RDWR | O_NOCTTY);
    } while (dc->fd == -1 && errno == EINTR);
    if (dc->fd == -1)return -1;

    /* Bad tty? */

    dc->saved_errno = errno;

    /* Save current terminal settings. */
    do {
        result = tcgetattr(dc->fd, &dc->saved);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    /* Get current terminal settings for basis, too. */
    do {
        result = tcgetattr(dc->fd, &dc->temporary);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        retval = errno;
        errno = dc->saved_errno;
        return retval;
    }

    return 0;
}


int32_t con_delay(int32_t ms)
{
	struct timespec tv;

	tv.tv_sec = (time_t) ms / 1000;
	
	tv.tv_nsec = (long) ((ms - (tv.tv_sec * 1000)) * 1000000L);

	while (1)
	{
		int rval = nanosleep (&tv, &tv);
		if (rval == 0)
			return 0;
		else if (errno == EINTR)
			continue;
		else
			return rval;
	}
	return 0;
}

void con_flushin(void)
{
    tcflush(0, TCIFLUSH);
}

void con_async(bool_t enable)
{
    if (enable)
    {
        if ( asyncmode ) return;
		asyncmode = TRUE;
        flush();
        con_flushin();
        tcgetattr(0,&initial_settings);
        new_settings = initial_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_lflag &= ~ISIG;
        new_settings.c_lflag &= ~IXON;
        new_settings.c_cc[VMIN] = 1;
        new_settings.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &new_settings);
    }
    else
    {
        if ( !asyncmode ) return;
        flush();
        con_flushin();
        tcsetattr(0, TCSANOW, &initial_settings);
        asyncmode = FALSE;
    }
}

int32_t con_kbhit(void)
{	
	if ( !asyncmode ) return 0;
	int32_t toread;
	ioctl(0, FIONREAD, &toread);
    return toread;
}

static int32_t _kbhit(void)
{	
	int32_t toread;
	ioctl(0, FIONREAD, &toread);
    return toread;
}

int32_t con_getch(void)
{
    char ch;
	if ( read(0,&ch,1) < 1 ) return EOF;
    return ch;
}

int32_t con_getchex(void)
{
	int32_t c;
    
	if ( EOF == (c = con_getch()) ) return EOF;
	
	int32_t ret = c;
	if ( ret == CON_KEY_ESC )
	{
		while( 1 )
		{
			if ( !_kbhit() ) con_delay(TIME_RELAX);
			if ( !_kbhit() ) break;
			if ( EOF == (c = con_getch()) ) break;
			
			ret <<= 8;
			ret |= c;
			if ( ret == CON_KEY_MOUSE )
			{
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_b = con_getch();
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_x = con_getch() - 32;
				while( !_kbhit() ) con_delay(TIME_RELAX);
				_mouse_y = con_getch() - 32;
				break;
			}
		}
	}
	return ret;
}

void con_getmaxrc(unsigned int* r,unsigned int* c)
{
    struct winsize ws;
    ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws);
    *r = ws.ws_row;
    *c = ws.ws_col;
}

inline void con_gotorc(uint32_t r, uint32_t c)
{
    printf("\033[%d;%df",r,c);
}

void con_getrc(uint32_t* r, uint32_t* c)
{
	bool_t restore = FALSE;
	
	if ( asyncmode )
	{
		restore = TRUE;
		con_async(0);
	}
	
    struct cdirectrw dc;

    if ( con_dopen(&dc) ) return;

    con_dsrwhyde(&dc);

    int rows,cols,result;

    do {

        if ( con_dsetting(&dc) ) break;

        if ( con_dwr(&dc, "\033[6n", 4) ) break;


        if ( (result = con_drd(&dc)) != 27) break;
        if ( (result = con_drd(&dc)) != '[')  break;

        /* Parse rows. */
        rows = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            rows = 10 * rows + result - '0';
            result = con_drd(&dc);
        }

        if (result != ';') break;

        /* Parse cols. */
        cols = 0;
        result = con_drd(&dc);
        while (result >= '0' && result <= '9') {
            cols = 10 * cols + result - '0';
            result = con_drd(&dc);
        }

        if (result != 'R') break;
        /* Success! */
        if (r) *r = rows;
        if (c) *c = cols;

    } while (0);

    /* Restore saved terminal settings. */
    con_drestore(&dc);
    
    if ( restore )
    {
		con_async(1);
	}
}

inline void con_cls()
{
    con_puts("\033[H\033[J");
}

inline void con_clsline(char* mode)
{
    printf("\033[%s",mode);
}


void con_setcolor(uint8_t b, uint8_t f)
{
    if (!b && !f)
    {
        con_puts("\033[m");
        return;
    }

    if (b) printf("\033[%um",b);
    if (f) printf("\033[%um",f);
}

void con_setcolor256(uint8_t b, uint8_t f)
{
    if (!b && !f)
    {
        con_puts("\033[m");
        return;
    }

    if (b) printf("\033[48;5;%um",b);
    if (f) printf("\033[38;5;%um",f);
}

void con_showcursor(bool_t enable)
{
	printf("\033[?25%c",(enable)?'h':'l');
}

inline void con_special(char v)
{
    printf("\033(0%c\033(B",v);
}

void con_carret_up(uint32_t n)
{
	printf("\033[%dA",n);
}

void con_carret_down(uint32_t n)
{
	printf("\033[%dB",n);
}

void con_carret_next(uint32_t n)
{
	printf("\033[%dC",n);
}

void con_carret_prev(uint32_t n)
{
	printf("\033[%dD",n);
}

void con_carret_home()
{
	con_puts("\033[H");
}

void con_carret_end()
{
	con_puts("\033[H");
}

void con_carret_save()
{
	con_puts("\033[s");
}

void con_carret_restore()
{
	con_puts("\033[u");
}

void con_scrool_up()
{
	con_puts("\033[M");
}

void con_scrool_down()
{
	con_puts("\033[D");
}

void con_carret_delete(uint32_t n)
{
	printf("\033[%dP",n);
}

void con_mode_ins(bool_t enable)
{
	printf("\033[4%c",(enable)?'h':'l');
}

void con_linewrap(bool_t enable)
{
	printf("\033[?7%c",(enable)?'h':'l');
}

void con_vt100_reset()
{
	con_puts("\033[c");
}

void con_font_attribute(uint32_t a)
{
	printf("\033[%dm",a);
}

int con_inp(char* inp, uint32_t sz, uint32_t offr, uint32_t offc, int getn, onrelax_f fnc)
{
    uint32_t i = 0;
    int32_t k = 0;
    int ins = 1;
    
    *inp = 0;
    con_gotorc(offr, offc+1);
    con_clsline(CON_CLLS_RIGHT);
    flush();
    
    con_async(1);
    
    while( k != CON_KEY_ESC && k != CON_KEY_ENTER)
    {
        if ( con_kbhit() < 1 ) 
        { 
            if ( fnc ) 
            {
                if ( fnc() ) 
                {
                    con_delay(TIME_RELAX);
                }
                else
                {
                    con_gotorc(offr, offc + i + 1);
                    flush();
                }
                continue; 
            }
            con_delay(TIME_RELAX);
        }
            
        k = con_getchex();
        if ( k == CON_KEY_ESC ) break;
            
        switch ( k )
        {
            case CON_KEY_LEFT:
                if ( i > 0 ) --i;
            break;
                
            case CON_KEY_RIGHT:
                if ( inp[i] != 0 ) ++i;
            break;
                
            case CON_KEY_BACKSPACE:
                if ( i == 0 ) break;
                --i;
                memmove(&inp[i], &inp[i+1], strlen(&inp[i+1])+1);
            break;
                
            case CON_KEY_CANC:
                if ( inp[i] == 0 ) break;
                memmove(&inp[i], &inp[i+1], strlen(&inp[i+1])+1);
            break;
                
            case CON_KEY_ENTER:
                if ( getn )
                {
                    inp[i++] = '\n';
                    inp[i] = 0;
                }
            break;
                
            default:
                if ( i >= sz - 1 ) break;
                    
                if ( !ins )
                {
                    if ( inp[i] == 0 ) inp[i+1] = 0;
                    inp[i++] = k;
                }
                else
                {
                    memmove(&inp[i+1], &inp[i], strlen(&inp[i])+1);
                    inp[i++] = k;
                }
                    
            break;
        }//switch k
            
        if ( i > 0 )
        {
            con_gotorc(offr, offc + i);
            con_clsline(CON_CLLS_RIGHT);
            print(&inp[i-1]);
        }
        else
        {
            con_gotorc(offr, offc + 1);
            con_clsline(CON_CLLS_RIGHT);
            print(inp);
        }
                
        con_gotorc(offr, offc + i + 1);
        flush();
            
    }//while k
    
    con_async(0);
    
    return k == CON_KEY_ESC ? 0 : 1;
}
