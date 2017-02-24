#ifndef _VIDEO_H_
#define _VIDEO_H_
char loaded;
void enablelogging();
void initFB();
void drawChar(unsigned char c, int x, int y, int colour);
void drawString(const char* str, int x, int y, int colour);
void println(const char* message, int colour);
void printHex(const char* message, int hexi, int colour);
void videotest();
void dumpDebug();

#endif
