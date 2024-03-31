// ++++
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

#ifndef _XOSSYS_H_
#define _XOSSYS_H_

#include <_prolog.h>

// DPMI system calls

long svcDpmiSetup(long flag, long regs) _XC_;

// IO system calls
                
long svcIoCancel(QAB *qab,long bits) _XC_;
long svcIoClear(long dev, long bits) _XC_;
long svcIoClose(long dev, long bits);
long svcIoClsChar(char *name, void *chrlst) _XC_;
long svcIoCommit(long dev) _XC_;
long svcIoControl(QAB *qab, long func,long data) _XC_;
long svcIoDefLog(long level, long type, char *name, char *defin, long type,
		long level) _XC_;
long svcIoDelete(long cmd, char *name, void * parm) _XC_;
long sysIoDevChar(long hndl, void *chrlst) _XC_;
long sysIoDevParm(long bits, char *name, void *parm) _XC_;
long sysIoDstName(char *rs, char *ms,char *ds, long dl) _XC_;
long sysIODupHandle(long oldh, long newh, long cmd) _XC_;
long sysIoFindLog(long lvl, char *str, char *name, char *defin, long len,
		long *skip) _XC_;
long svcIoGetHndlMap(unsigned char *map, long size) _XC_;
long svcIoInBlock(long dev, char *data, long size) _XC_;
long svcIoInBlockP(long dev, char *data, long size, void *parm) _XC_;
long svcIoInSingle_F_(long dev) _XC_;
long svcIoInSingleP(long dev, void *parm) _XC_;
long svcIoMakePipe(void) _XC_;
long svcIoOpen(long bits, char *name, void *parm) _XC_;
long svcIoOutBlock(long dev, char *data, long size) _XC_;
long svcIoOutBlockP(long dev, char *data, long size, void *parm) _XC_;
long svcIoOutSingle(long dev,long byte) _XC_;
long svcIoOutSingleP(Long dev, long byte, void *parm) _XC_;
long svcIoOutString(long dev, char *data, long size) _XC_;
long svcIoOutStringP(long dev, char *data, long size,void *parm) _XC_;
long svcIoPath(char *name, long opt, char *bfr, long size) _XC_;
long svcIoPorts(long func, long base, long num) _XC_;
long svcIoQueue(QAB *qab) _XC_;
long svcIoRename(long cmd, char *on, char *nn, void *parm) _XC_;
long svcIoRun(QAB *qab) _XC_;
long svcIoSetPos(long dev, long pos, long mode) _XC_;
long svcIoSpecial(long dev, long func, char *addr, long cnt, void *parm) _XC_;
long svcIoTransName(long fnc, char *name, char *bfr, long size) _XC_;
		name,0,fnc)
long svcIoWait(QAB *qab) _XC_;

// Memory management system calls

long svcMemChange(void *base, long bits, long size) _XC_;
long svcMemConvShr(void *addr, char *name, long type, long level, long prot,
		void *acl) _XC_;
long svcMemCreate(long sel, long type) _XC_;
long svcMemDebug(long func, void *addr, long *value) _XC_;
long svcMemLink(long newsel, long oldsel, long type) _XC_;
long svcMemLinkShr(void *addr, char *name, long lvl) _XC_;
long svcMemMap(void *base, long phys, long bits, long size) _XC_;
long svcMemNull(void *base, long offset, long pg, long size) _XC_;
long svcMemPageType(void *base, long btm, long top, long bits) _XC_;
long svcMemSegType(long sel, long type) _XC_;
long vcMemWPCntl(long mask) _XC_;
long svcMemWPSet(long num, long type, long size, void *addr) _XC_;
long svcSchAlarm(long func, long hndl, long vect, long data, long date,
		long time) _XC_;

// Scheduler system calls

long svcSchClrEvent(char *name, long mask) _XC_;
long svcSchClrSignal(long vector) _XC_;
long svcSchCtlCDone(void) _XC_
long sysSchGetVecotr(long vect, void **addr) _XC_;
long svcSchInitVector(long type) _XC_;
long svcSchIntrProc(long pid, long func, long sts) _XC_;
long svcSchKill(long sts, long pid) _XC_;
long svcSchKillX(long data, long efr, long cseip, long mema, longe proc,
		long sts) _XC_;
long svcSchMakEvent(char *name, long size, long intr) _XC_;
long svcSchRelEvent(char *name, long mask) _XC_;
long svcSchResEvent(long name, long event) _XC_;
long svcSchSetEvent(char *name, long event, long val, long pid) _XC_;
long svcSchSetLevel(long level) _XC_;
long svcSchSetVector(long vect, long lvl, void *addr) _XC_;
long svcSchSleep(void) _XC_;
long svcSchSuspend(long *flag, long time) _XC_;
long svcSchWaitProc(long pid, long time) _XC_;
long svcSchWaitMEvent(char * name, long mask, long to) _XC_;
long svcSchWaitSEvent(char * name, long event, long to) _XC_;
long svcSchWaitSignal(long *flag, long time) _XC_;

// General system calls

long svcSysCmos(long addr, long data) _XC_;
long svcSysDateTime(long func, long data) _XC_;
long svcSysDefEnv(long lvl, char *str, char *def) _XC_;
long svcSysErrMsg(long code, long fmt, char *str) _XC_;
long svcSysFindEnv(long lvl,char *str, char *fnd, char *defin, long len,
		long *skip) _XC_;
long svcSysGetEnv(long lvl, char *bufr, long size) _XC_;
long svcSysGetInfo(long func, long data, char *bfr, long size) _XC_;
long svcSysGetPData(long pid, void *data) _XC_;
long svcSysGetPid(void) _XC_;
long svcSysGetPLevel(long pid) _XC_;
long svcSysGetPName(void *pid, long str) _XC_;
long vcSysLog(char *msg, long size) _XC_;
long svcSysSetPName(char *name) _XC_;

// Terminal system calls

long svcTrmAttrib(long dev,long func, void *data) _XC_;
long svcTrmCurPos(long dev, long page,long chr, long line) _XC_;
long svcTrmCurType(long dev, long curtype) _XC_;
long svcTrmDspMode(long dev, long func,void *data) _XC_;
long svcTrmDspPage(long dev, long page) _XC_;
long svcTrmEgaPalet(long dev, long list) _XC_;
long svcTrmFunction(long dev, long func) _XC_;
long svcTrmGetAtChr(long dev, long page) _XC_;
long svcTrmGetPixel(long dev, long page, long xc, long yc) _XC_;
long svcTrmLdCusFont(long dev, long tbl, long size, void *data, long bgn,
		long cnt) _XC_;
long svcTrmLdStdFont(long dev, long tbl, long font, long bgn, long cnt) _XC_;
long svcTrmMapScrn(long dev, void *bufr, long size, long ofs) _XC_;
long svcTrmScroll(long dev,long page, long xul, long yul,long xlr, long ylr,
		long cnt, long attr) _XC_;
long svcTrmSelFont(long desc, long fonta, long fontb) _XC_;
long svcTrmSetAtChr(long dev, long page, longe chr, long attr, long cnt) _XC_;
long svcTrmSetChr(long dev, long page, long chr, long color, long cnt) _XC_;
long svcTrmSetPixel(long dev, long page, long xc, long yc, long val) _XC_;
long svcTrmWrtLnB(long dev, long str, long cnt) _XC_;

#include <_epilog.h>

#endif							// _XOSSYS_H_
