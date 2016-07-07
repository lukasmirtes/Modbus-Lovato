#include "KeyboardScanner.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

static struct termios oldSettings;
static struct termios newSettings;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
  tcgetattr(0, &oldSettings); /* grab old terminal i/o settings */
  newSettings = oldSettings; /* make new settings same as old settings */
  newSettings.c_lflag &= ~ICANON; /* disable buffered i/o */
  newSettings.c_cc[VMIN] = newSettings.c_cc[VTIME] = 0; /* set no waiting for more keypresses */
  newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &oldSettings);
}

KeyboardScanner::KeyboardScanner() :
	doRun(1)
{
  initTermios(0);
}

KeyboardScanner::~KeyboardScanner()
{
  resetTermios();
}

void KeyboardScanner::run()
{
	while(doRun)
	{
		int c = getchar();
		if(c > 0) {
			emit KeyPressed(c);
		}
		else {
			msleep(50);
		}
	}
}

void KeyboardScanner::finish()
{
	doRun=0;
}
