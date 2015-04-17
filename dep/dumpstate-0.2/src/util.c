
#include <windows.h>
#include "util.h"

void _ass(const char *file, int line, const char *cond) {
	char *def = "couldn't globalalloc!";
	char *buffer = GlobalAlloc(GPTR,8192);
	if(buffer) 
		wsprintf(buffer, "%s:%d\n\n%s", file, line, cond);
	else
		buffer = def;
	
	MessageBox(NULL, buffer, "Assertion failed", MB_OK);

	if(buffer != def)
		GlobalFree(buffer);

}

void dprintf(const char *fmt, ...) {
	va_list args;
	char *def = "couldn't globalalloc!";
	char *buffer = (char *)GlobalAlloc(GPTR, 8192);
	
	va_start(args, fmt);
	if(buffer)
		wvsprintf(buffer, fmt, args);
	else
		buffer = def;
	va_end(args);

	OutputDebugString(buffer);

	if(buffer != def)
		GlobalFree(buffer);
}
