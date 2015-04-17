// this file is from the nsisv2 distribution, copyrighted by nullsoft

#ifndef __EXDLL_H
#define __EXDLL_H

#include "util.h" // assertions

// only include this file from one place in your DLL.
// (it is all static, if you use it in two places it will fail)

#define EXDLL_INIT()           {  \
        g_stringsize=string_size; \
        g_stacktop=stacktop;      \
        g_variables=variables; }

// For page showing plug-ins
#define WM_NOTIFY_OUTER_NEXT (WM_USER+0x8)
#define WM_NOTIFY_CUSTOM_READY (WM_USER+0xd)
#define NOTIFY_BYE_BYE 'x'

typedef struct _stack_t {
  struct _stack_t *next;
  char text[1]; // this should be the length of string_size
} stack_t;


static unsigned int g_stringsize;
static stack_t **g_stacktop;
static char *g_variables;

static int popstring(char *str); // 0 on success, 1 on empty stack
static void pushstring(char *str);

enum
{
INST_0,         // $0
INST_1,         // $1
INST_2,         // $2
INST_3,         // $3
INST_4,         // $4
INST_5,         // $5
INST_6,         // $6
INST_7,         // $7
INST_8,         // $8
INST_9,         // $9
INST_R0,        // $R0
INST_R1,        // $R1
INST_R2,        // $R2
INST_R3,        // $R3
INST_R4,        // $R4
INST_R5,        // $R5
INST_R6,        // $R6
INST_R7,        // $R7
INST_R8,        // $R8
INST_R9,        // $R9
INST_CMDLINE,   // $CMDLINE
INST_INSTDIR,   // $INSTDIR
INST_OUTDIR,    // $OUTDIR
INST_EXEDIR,    // $EXEDIR
INST_LANG,      // $LANGUAGE
__INST_LAST
};


// utility functions (not required but often useful)
static int popstring(char *str) {
	stack_t *th;
	if (!g_stacktop || !*g_stacktop) return 1;
	th=(*g_stacktop);
	lstrcpy(str,th->text);
	*g_stacktop = th->next;
	GlobalFree((HGLOBAL)th);
	return 0;
}

static void pushstring(char *str) {
	stack_t *th;
	if (!g_stacktop) return;
	th=(stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
	lstrcpyn(th->text,str,g_stringsize);
	th->next=*g_stacktop;
	*g_stacktop=th;
}

static char *getuservariable(int varnum) {
	ASS(varnum >= 0 && varnum < __INST_LAST);
	return g_variables+varnum*g_stringsize;
}

static void setuservariable(int varnum, char *var)
{
	ASS(var != NULL && varnum >= 0 && varnum < __INST_LAST);
	lstrcpy(g_variables + varnum*g_stringsize, var);
}

static char *g_regNames[] = {
"$0",
"$1",
"$2",
"$3",
"$4",
"$5",
"$6",
"$7",
"$8",
"$9",
"$R0",
"$R1",
"$R2",
"$R3",
"$R4",
"$R5",
"$R6",
"$R7",
"$R8",
"$R9",
"$CMDLINE",
"$INSTDIR",
"$OUTDIR",
"$EXEDIR",
"$LANGUAGE"
};

static char *getuservarname(int n) {
	ASS(n < sizeof(g_regNames) / sizeof(char*));
	return g_regNames[n];
}

#endif // __EXDLL_H

