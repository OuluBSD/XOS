//------------------------------------------------------------------------------
//
//  history.c - Functions to manage the command history
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"

// Command recall uses a doublely linked list of blocks. Each block contains
//   the text for one command. Three pointers are used:
//     histfirstblk - Contains the address of the oldest history block.
//     histcurblk   - Contains the address of the "current" history block.
//                      This is the block that contains the command that has
//                      been last recalled. If no command has been recalled
//                      it is NULL.
//     histlastblk  - Contains the address of the most recent history block.


HIST *histfirstblk;
HIST *histcurblk;
HIST *histlastblk;

int   histmax = 200;

static int  findinhistory(char *txt);
static int  getfromhistory(long num);
static void putinkbbuffer(void);


int putinhistory(
	char *txt,
	int   len)
{
	HIST *hpnt;
	char *cpnt;
	char  chr;

	// If same as last command, don't store it

	if (histlastblk != NULL && len == histlastblk->len &&
			strcmp(txt, histlastblk->txt) == 0)
		return (TRUE);

	cpnt = txt;
	while ((chr = *cpnt) != 0 && !isspace(chr))
		cpnt++;
	if (strnicmp(txt, "history", cpnt - txt) == 0)
		return (TRUE);

	// If the history is full, delete the oldest entry

	if (histfirstblk != NULL &&
			(histlastblk->num - histfirstblk->num) >= histmax)
	{
		hpnt = histfirstblk->next;
		free(histfirstblk);
		histfirstblk = hpnt;
		hpnt->prev = NULL;
	}

	// Allocate a block, fill it in, and link it to the end of the list

	if ((hpnt = (HIST *)malloc(sizeof(HIST) + len)) == NULL)
	{
		cmnderror(-errno, "Error allocating memory for command history");
		return (FALSE);
	}
	memcpy(hpnt->txt, txt, len + 1);
	hpnt->num = (histlastblk != NULL) ? (histlastblk->num + 1) : 1;
	hpnt->len = (ushort)len;
	hpnt->next = NULL;
	hpnt->prev = histlastblk;
	if (histlastblk == NULL)
		histfirstblk = hpnt;
	else
		histlastblk->next = hpnt;
	histlastblk = hpnt;
	histcurblk = NULL;
	return (TRUE);
}



void retrievefromhistory(
	char *cmd)
{
	long  num;
	char  text[128];
	char  chr;

	while ((chr = *cmd) != 0 && isspace(chr)) // Skip leading whitespace
		cmd++;
	if (isdigit(*cmd))					// Was a number specified?
	{
		if (!getonenum(cmd, &num) || !getfromhistory(num))
		{
			num = sprintf(text, "? Command #%d not found in the command "
					"history\n", num);
			svcIoOutBlock(ccs->cmderr, text, num);
			return;
		}
	}
	else
	{
		if (!findinhistory(cmd))
		{
			num = sprintf(text, "? Command beginning with \"%s\" not found "
					"in the command history\n", cmd);
			svcIoOutBlock(ccs->cmderr, text, num);
			return;
		}
	}
	putinkbbuffer();
}



void prevfromhistory(void)
{
	if (histcurblk == NULL)
	{
		if (histlastblk == NULL)
			return;
		histcurblk = histlastblk;
	}
	else if (histcurblk->prev != NULL)
		histcurblk = histcurblk->prev;
	putinkbbuffer();
}



void nextfromhistory(void)
{
	if (histcurblk != NULL && (histcurblk = histcurblk->next) != NULL)
		putinkbbuffer();
}



int findinhistory(
	char *txt)
{
	HIST *hpnt;
	int   len;

	len = strlen(txt);
	hpnt = (histcurblk != NULL) ? histcurblk->prev : histlastblk;
	while (hpnt != NULL)
	{
		if (strnicmp(txt, hpnt->txt, len) == 0)
		{
			histcurblk = hpnt;
			return (TRUE);
		}
		hpnt = hpnt->prev;
	}
	return (FALSE);
}



int getfromhistory(
	long num)
{
	HIST *hpnt;

	if (num >= histfirstblk->num && num <= histlastblk->num)
	{
		hpnt = histlastblk;
		while (hpnt != NULL)
		{
			if (num == hpnt->num)
			{
				histcurblk = hpnt;
				return (TRUE);
			}
			hpnt = hpnt->prev;
		}
	}
	return (FALSE);
}


static void putinkbbuffer(void)
{
	long rtn;

    if ((rtn = svcTrmWrtInB(STDTRM, histcurblk->txt, histcurblk->len)) < 0)
	{
		cmnderror(rtn, "Error writing to terminal buffer");
		showprompt();
	}
}
