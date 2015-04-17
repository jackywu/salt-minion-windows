
#ifndef __UTIL_H_INCLUDED
#define __UTIL_H_INCLUDED

// most annoying, this - there _is_ an RtlZeroMemory in kernel32.dll
// but on most platforms ZeroMemory is macro'd to memset instead
#ifdef ZeroMemory
#undef ZeroMemory
#endif

#ifdef CopyMemory
#undef CopyMemory
#endif

static void ZeroMemory(void *p, int n) {
	unsigned char *c = (unsigned char *)p;
	while(n >= 0) 
		c[n--] = 0;
}

static void CopyMemory(void *vdest, const void *vsrc, int n) {
	unsigned char *dest      =       (unsigned char *)vdest;
	const unsigned char *src = (const unsigned char *)vsrc;
	while(n--)
		*(dest++) = *(src++);
}

// assertions
#define ASS(cond) { if(!(cond)) { _ass(__FILE__,__LINE__,"##cond"); } }

void _ass(const char *file, int line, const char *cond);
void dprintf(const char *fmt, ...);

#endif // __UTIL_H_INCLUDED

