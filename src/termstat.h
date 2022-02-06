#ifndef _TERM_STAT
#define _TERM_STAT

#include <sys/ioctl.h>
#include <stdio.h>


struct termstats{
	int width;
	int height;
	
};//termstats


struct termstats get_terminal(){

	struct termstats term;
	struct winsize win;
	ioctl(0, TIOCGWINSZ, &win);
	term.height = win.ws_row;
	term.width = win.ws_col;
	return(term);

}//get_terminal




#endif
