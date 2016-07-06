#if false
#include "Test.h"

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
  newSettings.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
  tcsetattr(0, TCSANOW, &newSettings); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &oldSettings);
}

/* Read 1 character without echo */
char getch(void)
{
  return getchar();
}

ConsoleReader::ConsoleReader()
{
  initTermios(0);
}

ConsoleReader::~ConsoleReader()
{
  resetTermios();
}

void ConsoleReader::run()
{
	forever
	{
		std::cout << "Scanning...\n";
		std::cout.flush();
		char key = getch();
		std::cout << "Emitting...\n";
		std::cout.flush();
		emit KeyPressed(key);
	}
}
#endif
