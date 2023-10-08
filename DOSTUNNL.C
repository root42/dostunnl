#include <stdio.h>
#include <stdlib.h>
#include <alloc.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <mem.h>

#include "types.h"
#include "fix.h"
#include "vga.h"
#include "pal.h"

#define SIN_SIZE 512
#define COS_OFF 128

long SIN[ SIN_SIZE + COS_OFF ];
long *COS = SIN + COS_OFF;

#define LEVELS 128
#define POINTS 64

int tun_coords[LEVELS][POINTS][2];
int liss_coords[LEVELS][2];

void init_sin()
{
    int i;
    double v;
    for(i = 0; i < SIN_SIZE + COS_OFF; ++i) {
        v = sin(2.0 * M_PI * i / (double)SIN_SIZE);
        SIN[i] = TO_FIX( v );
    }
}

void liss(int *x, int *y, long t)
{
    const long scale = TO_FIX(50);
    *x = TO_LONG(fix_mul(COS[(t*2+64) % SIN_SIZE], scale));
    *y = TO_LONG(fix_mul(COS[(t*3) % SIN_SIZE], scale));
}

void init_tunnel()
{
    int i, j;
    long scale_x, scale_y, proj;
    for(i = 0; i < LEVELS; ++i) {
	scale_x = TO_FIX(50);
	scale_y = TO_FIX(31); /* 50 * 320/200 = 50 * 0.625 ~= 31 */
	proj = fix_div(TO_FIX(i+1), TO_FIX(LEVELS));
	for(j = 0; j < POINTS; ++j) {
	    tun_coords[i][j][0] = TO_LONG(fix_div(fix_mul(COS[j * (SIN_SIZE/POINTS)], scale_x), proj)) + 160;
	    tun_coords[i][j][1] = TO_LONG(fix_div(fix_mul(SIN[j * (SIN_SIZE/POINTS)], scale_y), proj)) + 100;
	}
    }
    for(i = 0; i < LEVELS; ++i) {
	liss(&liss_coords[i][0],
	     &liss_coords[i][1], i);
    }
}

void draw_tunnel(long t)
{
    byte col;
    int i, j, x, y;
    t+=LEVELS;
    liss(&liss_coords[t % LEVELS][0],
	 &liss_coords[t % LEVELS][1], t);
    for(i = 0; i < LEVELS; ++i) {
	if((t+i) % 10 < 5) continue;
	col = 31 - i / (LEVELS/16);
	for(j = 0; j < POINTS; ++j) {
	    x = tun_coords[i][j][0] + liss_coords[(t+i+1) % LEVELS][0];
	    y = tun_coords[i][j][1] + liss_coords[(t+i+1) % LEVELS][1];
	    if(x >= 0 && y >= 0 && x < 320 && y < 200) {
		SETPIX(x, y, col);
	    }
	}
    }
}

int main()
{
    long t=0;
    char kc=0;
    byte far *buf=farmalloc(64000);
    BUF=buf;

    init_sin();
    init_tunnel();
    set_graphics_mode();
    while(kc!=0x1b) {
        if(kbhit()) kc=getch();
        memset(buf,0,64000);
        draw_tunnel(t);
        wait_for_retrace();
        memcpy(VGA,buf,64000);
	t++;
    }
    set_text_mode();

    return 0;
}

