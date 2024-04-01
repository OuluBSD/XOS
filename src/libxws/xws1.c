//++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <xosxxws.h>

XWSWIN *xwstravhead[8];			// Head pointers for the traversal lists
XWSWIN *xwstravtail[8];

///extern char win_baszorder[];
extern char win_basdrawnext[];
extern char win_bastbhead[];
extern char win_basevent[];

// EDB used for windows created by XWS

static char *fontnames[] =
{	"DejaVu_Serif|Book",
	"DejaVu_Serif|Bold",
	"DejaVu_Sans|Book",
	"DejaVu_Sans|Bold",
	"DejaVu_Sans_Mono|Book",
	"DejaVu_Sans_Mono|Bold"
};
static long colors[] = { -1, -2, 0xC00000, 0x808080};
XWSEDB xwsedb = { 6, fontnames, 4, colors};


void xwscheck(void)
{
///	if (offsetof(XWSWIN, zorder) != (long)win_baszorder)
///		xwsFail(0, "XWSWIN.zorder is wrong!");

	if (offsetof(XWSWIN, drawnext) != (long)win_basdrawnext)
		xwsFail(0, "XWSWIN.drawnext is wrong!");
	if (offsetof(XWSWIN, tbhead) != (long)win_bastbhead)
		xwsFail(0, "XWSWIN.tbhead is wrong!");
	if (offsetof(XWSWIN, usrevent) != (long)win_basevent)
		xwsFail(0, "XWSWIN.event is wrong!");
}


//***************************************
// Function: xwsFail - Report fatal error
// Returnes: Never returns
//***************************************

// This function is intended to report serious errors (such as no memory
//   available) that are likely to prevent XWS from displaying an error
//   message. It is also used to report serious internal errors or impossible
//   internal states. The terminal is put in text mode, the message is
//   displayed and the program is terminated.

// NOTE: Although this function does not return, it is NOT defined that
//       way so we can see where it was called from if running under GECKO.

void xwsFail(
	long  code,
	char *fmt, ...)
{
	va_list pi;
	char    text[300];

	if (humArea.debugger == 'DEBG')		// If loaded under a debugger, go to
		INT3;							//   the debugger before dieing.
	svcTrmDspMode(DH_STDTRM, DM_USEBASE, 0); // Set to normal text mode
    va_start(pi, fmt);					// Display the message
    vsprintf(text, fmt, pi);
	fprintf(stderr, "====\n");
	fprintf(stderr, "==== ? XWS Error: %s\n", text);
	if (code < 0)
	{
		svcSysErrMsg(code, 0x03, text);
		fprintf(stderr, "====              %s\n", text);
	}
	fprintf(stderr, "====\n");
	exit(1);							// That's all!
}


//*******************************************
// Function: xwsAssert - assert hook function
// Returned: Never returns
//*******************************************

// This function is never called directly. It is only called as a result of
//   a call to _assert from user code. All XWS code calls xwsFail instead of
//   _assert.

void xwsAssert(
	char *msg)
{
	int len;

	if (msg[0] == '?')
		msg++;
	while(msg[0] == ' ')
		msg++;
	len = strlen(msg);
	if (msg[len - 1] == '\n')
		msg[len - 1] = 0;
	xwsFail(0, msg);
}
