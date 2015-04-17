/**
 *
 * dumpstate.c
 * A little debug tool for use as an NSIS extension DLL.
 *
 * Copyright (C) Andrew Francis, 2003. Portions Copyright (C) Nullsoft, Inc.
 * See README.TXT for copyright information relating to modification and
 * redistribution.
 *
 */

// we can't just dump data into the listbox as it gets narky about large strings.
// so define a limit at which we truncate it and append a warning, total length
// > 1024 seems to cause badness
#define CONF_BIGSTRING_MAX 200
#define CONF_BIGSTRING_SUBST " [ ... continues ...]"

#include <windows.h>
#include "util.h"
#include "exdll.h"
#include "resource.h"

BOOL CALLBACK DlgProcState(HWND,UINT,WPARAM,LPARAM);
BOOL CopyStateToDialog(HWND hwnd);
BOOL EditString(HWND parent, char *name, char *str);
int CALLBACK DlgProcEdit(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
int DoStackAction(int item, int index);

char *g_clipboardText = NULL;
HINSTANCE g_hInstance;
HWND g_hwndParent;

void __declspec(dllexport) debug(HWND hwndParent, int string_size, 
                                      char *variables, stack_t **stacktop)
{
  g_hwndParent=hwndParent;

  EXDLL_INIT();

  dprintf("string size: %0d", string_size);

  DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_STATE), g_hwndParent, DlgProcState);

  if(g_clipboardText)
	  GlobalFree(g_clipboardText);
}



BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = hInst;
	return TRUE;
}


int CALLBACK DlgProcState(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch(msg) {
	case WM_INITDIALOG:
		CopyStateToDialog(hwnd);
		return 0;
	
	case WM_COMMAND:
		// finishing up?
		if(LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
			EndDialog(hwnd, 0);
			return 1;
		}

		// copy all
		if(LOWORD(wp) == IDC_COPYALL) {
			HANDLE toClipboard;
			char *toClipboardPtr;
			int l;

			ASS(g_clipboardText);
			// GlobalAlloc a global memory region, get a pointer to
			// it and copy our stuff in
			l = lstrlen(g_clipboardText) + 1;
			toClipboard = GlobalAlloc(GMEM_DDESHARE, l);
			if(toClipboard) {
				toClipboardPtr = GlobalLock(toClipboard);
				CopyMemory(toClipboardPtr, g_clipboardText, l);
				GlobalUnlock(toClipboard);

				// open clipboard, empty it, throw our data in, close it again
				if(!OpenClipboard(hwnd)) {
					GlobalFree(toClipboard);
				} else {
					EmptyClipboard();
					SetClipboardData(CF_TEXT, toClipboard);
					CloseClipboard();
				}
			}
		}

		// editing
		if(LOWORD(wp) == IDC_VARS && HIWORD(wp) == LBN_DBLCLK) {
			HWND hVars = GetDlgItem(hwnd, IDC_VARS);
			int index = SendMessage(hVars, LB_GETCURSEL, 0, 0);
			if(index != LB_ERR) {
				if(EditString(hwnd, getuservarname(index), getuservariable(index))) {
					CopyStateToDialog(hwnd);
					SendMessage(hVars, LB_SETCURSEL, (WPARAM)index, 0);
				}
			}
			return 1;
		}

		if(LOWORD(wp) == IDC_STACK && HIWORD(wp) == LBN_DBLCLK) {
			HWND hStack = GetDlgItem(hwnd, IDC_STACK);
			int index = SendMessage(hStack, LB_GETCURSEL, 0, 0);
			if(index != LB_ERR) {
				int changeTo = index;
				stack_t *s;
				char name[32];
				
				wsprintf(name, "stack[%d]", index);

				s = *g_stacktop;
				while(s && s->next && index>0) {
					s = s->next;
					index--;
				}
				
				if(s && EditString(hwnd, name, s->text)) {
					CopyStateToDialog(hwnd);
					SendMessage(hStack, LB_SETCURSEL, (WPARAM)changeTo, 0);
								
				}
			}
			return 1;
		}

		
		// silliness with the stack
		if(LOWORD(wp) == IDC_REMOVE || LOWORD(wp) == IDC_INSERT 
				|| LOWORD(wp) == IDC_UP || LOWORD(wp) == IDC_DOWN) {
			HWND hStack = GetDlgItem(hwnd, IDC_STACK);
			int selection = SendMessage(hStack, LB_GETCURSEL, 0, 0);
			if(selection != LB_ERR) 
				selection = DoStackAction(LOWORD(wp), selection);
			
			if(selection != LB_ERR && selection >= 0) {
				CopyStateToDialog(hwnd);
				SendMessage(hStack, LB_SETCURSEL, (WPARAM)selection, 0);
			}

			return 1;
		}

		break;
	}

	return 0;
}

int DoStackAction(int item, int index) {	
	stack_t **lastptr = g_stacktop;
	int oldIndex = index;
	
	// a copout.. moving item n up is the same as
	// moving item n-1 down
	if(item == IDC_UP) {
		if(index > 0) {
			if(DoStackAction(IDC_DOWN, index - 1)) {
				return index - 1;
			}
		}
		return -1;
	}
	
	while(index > 0 && (*lastptr) && (*lastptr)->next) {
		index--;
		lastptr = &( (*lastptr)->next );
	}
			
	if(item == IDC_INSERT) {
		stack_t *newptr = (stack_t*)GlobalAlloc(GPTR,sizeof(stack_t)+g_stringsize);
		lstrcpy(newptr->text, "<<NEW>>");
		newptr->next = *lastptr;
		*lastptr = newptr;
		return oldIndex;
	} else if(item == IDC_REMOVE) {
		stack_t *oldptr = *lastptr;
		*lastptr = (*lastptr)->next;
		GlobalFree(oldptr);
		
		// if we removed the last item, we need to go up one. otherwise
		// stay at the same place
		if(*lastptr)
			return oldIndex;
		else
			return oldIndex - 1;
	} else if(item == IDC_DOWN) {
		stack_t *one, *two, *three;
		
		one = *lastptr;
		if(one) 
			two = one->next;
		if(one && two) {
			three = two->next;

			*lastptr = two;
			two->next = one;
			one->next = three;
		
			return oldIndex+1;
		}
		
	} 

	return -1;
}	
	


/**
 * This function updates the list boxes with the current contents of variables/stack.
 * It also builds the big text dump of everything, which may be copied to the clipboard.
 */

BOOL CopyStateToDialog(HWND hwnd) {
	char *buf; // current line
	stack_t *stackWalker;
	int i;
	int lineMaxLength = 0, totalLength = 0;
	char *clipboardWalker; // points to the end of all currently appended text

	HWND hwndStack = GetDlgItem(hwnd, IDC_STACK);
	HWND hwndVars  = GetDlgItem(hwnd, IDC_VARS);

	// this doesn't quite belong here, but it's the best place - preserve
	// the state of the listboxes and restore it afterwards. 
	int selectionStack = SendMessage(hwndStack, LB_GETCURSEL, 0, 0);
	int selectionVars  = SendMessage(hwndVars,  LB_GETCURSEL, 0, 0);

	// first we figure out how big stuff will be. allow 24 characters for
	// before ("varname: ") and endline/termination.
	for(stackWalker = *g_stacktop; stackWalker; stackWalker = stackWalker->next) {
		int l = lstrlen(stackWalker->text) + 24;
		totalLength += l;
		if(l > lineMaxLength) lineMaxLength = l;
	}
	for(i = 0; i < __INST_LAST; i++) {
		int l = lstrlen(getuservariable(i)) + 24;
		totalLength += l;
		if(l > lineMaxLength) lineMaxLength = l;
	}
	dprintf("total length: %d\nmax line: %d\n", totalLength, lineMaxLength);	

	buf = GlobalAlloc(GPTR, lineMaxLength);
	if(!buf) return FALSE;
	if(g_clipboardText)
		GlobalFree(g_clipboardText);
	g_clipboardText = GlobalAlloc(GPTR, totalLength + 16);
	clipboardWalker = g_clipboardText;

	// clear existing contents
	SendMessage(hwndStack, LB_RESETCONTENT, 0, 0);
	SendMessage(hwndVars, LB_RESETCONTENT, 0, 0);

	// fill in stack
	i = 0;
	for(stackWalker = *g_stacktop; stackWalker; stackWalker = stackWalker->next) {
		int l;
		wsprintf(buf, "%3d: ", i);
		lstrcat(buf, stackWalker->text);

		l = lstrlen(buf);
		CopyMemory(clipboardWalker, buf, l);
		clipboardWalker += l;
		*(clipboardWalker++) = '\r'; *(clipboardWalker++) = '\n'; 

		if(l > CONF_BIGSTRING_MAX)
			lstrcpy(buf + CONF_BIGSTRING_MAX, CONF_BIGSTRING_SUBST);

		SendMessage(hwndStack, LB_ADDSTRING, 0, (LPARAM)buf);
		
		i++;
	}
	
	*(clipboardWalker++) = '\r'; *(clipboardWalker++) = '\n'; 

	// fill in user vars
	for(i = 0; i < __INST_LAST; i++) {
		int l;
		wsprintf(buf, "%s: ", getuservarname(i));
		lstrcat(buf, getuservariable(i));

		l = lstrlen(buf);
		CopyMemory(clipboardWalker, buf, l);
		clipboardWalker += l;
		*(clipboardWalker++) = '\r'; *(clipboardWalker++) = '\n'; 

		if(l > CONF_BIGSTRING_MAX)
			lstrcpy(buf + CONF_BIGSTRING_MAX, CONF_BIGSTRING_SUBST);

		SendMessage(hwndVars, LB_ADDSTRING, 0, (LPARAM)buf);
	}

	// restore state. if the indexes are no longer valid, no matter, it'll
	// just fail gracefully (sendmessage returning LB_ERR)
	SendMessage(hwndStack, LB_SETCURSEL, (WPARAM)selectionStack, 0);
	SendMessage(hwndVars,  LB_SETCURSEL, (WPARAM)selectionVars,  0);

	// we're done
	GlobalFree(buf);
	*clipboardWalker = '\0';
	return TRUE;
}


///////////////////////////////////////////////// string edit dialog box stuff

typedef struct {
	char *name;
	char *buffer;
	BOOL changed;
} editstring_t;

BOOL EditString(HWND parent, char *name, char *buffer) {
	editstring_t e;
	e.name = name;
	e.buffer = buffer;
	e.changed = FALSE;

	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_EDITSTRING), parent, DlgProcEdit, (LPARAM)&e);
	
	return e.changed;
}

int CALLBACK DlgProcEdit(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	// we have some information in this structure, passed to us with initdialog.
	// we save the pointer as arbitrary data attached to the dialog window, and
	// recover this reference each time around
	editstring_t *e;
	if(msg == WM_INITDIALOG) {
		SetWindowLong(hwnd, DWL_USER, lp);
		
		// this ludicrous hack is required for the above to take effect. see msdn
		// documentation for details
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	} 
	e = (editstring_t *)GetWindowLong(hwnd, DWL_USER);
	if(!e) return 0;
	
	switch(msg) {
	case WM_INITDIALOG: 
		{
		HWND hName  = GetDlgItem(hwnd, IDC_EDITNAME);
		HWND hValue = GetDlgItem(hwnd, IDC_EDITVALUE);

		SendMessage(hName, WM_SETTEXT, 0, (LPARAM)(e->name));
		SendMessage(hValue, WM_SETTEXT, 0, (LPARAM)(e->buffer));
		SendMessage(hValue, EM_SETSEL, (WPARAM)0, (LPARAM)(-1));
		SetFocus(hValue);

		return 0;
		}

	case WM_COMMAND:
		if(LOWORD(wp) == IDOK) {
			// make sure our string isn't too long
			int n = SendMessage(GetDlgItem(hwnd, IDC_EDITVALUE), WM_GETTEXTLENGTH, 0, 0);
			if(n + 1 >= g_stringsize) {
				MessageBox(hwnd, "String too long!", "Error", MB_OK);
				return 1;
			}

			// actually get it back
			if(SendMessage(GetDlgItem(hwnd, IDC_EDITVALUE), WM_GETTEXT, (WPARAM)(n+1), (LPARAM)(e->buffer)) != n) {
				MessageBox(hwnd, "couldn't get string", "panic!", MB_OK);
				return 1;
			}

			// mark as changed and return
			e->changed = TRUE;
			EndDialog(hwnd, 0);
			return 1;
		}
		
		if(LOWORD(wp) == IDCANCEL) {
			e->changed = FALSE;
			EndDialog(hwnd, 0);
			return 1;
		}
		break;
	}

	return 0;
}	
	
