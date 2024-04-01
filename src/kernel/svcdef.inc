	.SBTTL	Parameter file SVCDEF.INC

;++++
; This software is in the public domain.  It may be freely copied and used
; for whatever purpose you see fit, including commerical uses.  Anyone
; modifying this software may claim ownership of the modifications, but not
; the complete derived code.  It would be appreciated if the authors were
; told what this software is being used for, but this is not a requirement.

;   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
;   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
;   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
;   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
;   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
;   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
;   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
;   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
;   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
;   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;----

	.NLBEX
	.PARM

;While the protected mode addresses are currently fixed at the values given
;  here these values may change and should not be used in user programs. User
;  programs should import the value of the entry name. The real mode addresses
;  are fixed at the values specified

	.MACRO	SVCDEFSYS

 SD  0, SchExit0	;Terminate current thread with 0 status
 SD  2, SchTerminate	;Terminate any thread in current process
 SD  1, SchExit		;Terminate current process
 SD  2, SchKill		;Terminate any thread in any process
 SD  7, SchExitX	;Extended terminate any thread in any process
 SD  3, SchSuspend	;Suspend (time in taicks)
 SD  0, SchSleep	;Sleep
 SD  1, SchClrAlarm	;Clear alarm
 SD  4, SchSetAlarm	;Set alarm
 SD  3, SchMakEvent	;Make event cluster
 SD  2, SchResEvent	;Reserve event
 SD  2, SchRelEvent	;Release event
 SD  4, SchSetEvent	;Declare event
 SD  2, SchClrEvent	;Clear event(s)
 SD  4, SchWaitSEvent	;Wait for single event
 SD  3, SchWaitMEvent	;Wait for multiple events
 SD  3, SchWaitProc	;Wait for process to terminate
 SD  1, SchSetLevel	;Set signal level
 SD  3, SchSetVector	;Set signal vector
 SD  2, SchGetVector	;Get signal vector
 SD  1, SchInitVector
 SD  1, SchClrSignal	;Clear signals for vector
 SD  4, SchReqSignal	;Request signal
 SD  4, SchDisSignal	;Discard signals
 SD  0, SchCtlCDone	;Report cntl-C processing done
 SD  1, SchCritWait	;Wait for busy critical region
 SD  1, SchCritDone	;Critical region done
 SD  1, SchSpawn	;Create process
 SD  6, SchThread	;Create user thread
 SD  1, SchWake		;Wake up a suspended thread

 SD  1, MemRmvMult	;Remove multiple msects
 SD  3, MemChange	;Allocate or deallocate memory pages
 SD  2, MemMove		;Move msect
 SD  2, MemSpy		;Map exec spy areas
 SD  4, MemMapPhys	;Map physical memory
 SD  4, MemMapSys	;Map system memory
 SD  6, MemConvShr	;Convert msect to shared section
 SD  3, MemLinkShr	;Link to shared section
 SD  4, MemPageType	;Change page(s) type
 SD  3, MemDebug	;Memory debug functions
 SD  4, MemWPSet	;Set or clear watchpoint
 SD  1, MemWPCntl	;Get memory debug status bits (DR6)

 SD  2, SysGetPName	;Get process name
 SD  1, SysSetPName	;Set process name
 SD  1, SysGetPLevel	;Get process level
 SD  2, SysGetPData	;Get process data
 SD  0, SysGetPid	;Get process ID
 SD  4, SysGetInfo	;Get system information
 SD  2, SysSetBillLvl	;Set billing level
 SD  2, SysDateTime	;Get or set date and time of day
 SD  2, SysLog		;Place message in system log
 SD  2, SysCmos		;Read or write CMOS memory data
 SD  1, SysLoadLke	;Load loadable kernel extension

 SD  3, SysGetRandom	;Get random bytes

 SD  1, IoQueue		;Queue IO function
 SD  2, IoCancel	;Cancel IO request(s)
 SD  1, IoWait		;Wait for IO operation to finish
 SD  3, IoControl	;Queued IO control functions
 SD  0, IoMakePipe	;Create pipe
 SD  2, IoDupHandle	;Duplicate device handle
 SD  2, IoGetHndlMap	;Get device handle map
 SD  2, IoPorts		;Allocate IO ports for direct access

 SD  3, LogDefine	;Define logical name
 SD  4, LogGet		;Get logical name
 SD  3, LogGetAll	;Get all logical names
 SD  4, LogExpand	;Expand logical name

 SD  2, TrmFunction	;General terminal functions
 SD  3, TrmDspMode	;Get or set display mode
 SD  4, TrmMapScrn	;Map screen buffer
 SD  2, TrmCurType	;Get or set cursor type
 SD  4, TrmCurPos	;Get or set cursor  position
 SD  2, TrmDspPos	;Get or set display page
 SD  6, TrmScroll	;Scroll wnidow
 SD  2, TrmGetAtChr	;Get attribute and character
 SD  5, TrmSetAtChr	;Set attribute and character
 SD  5, TrmSetChr	;Set character only
 SD  5, TrmLdStdFont	;Load standard display font
 SD  6, TrmLdCusFont	;Load custom display font
 SD  3, TrmSelFont	;Select font toable and mode
 SD  3, TrmAttrib	;Get or set screen attributes
 SD  3, TrmWrtInB	;Write to input buffer
 SD  1, TrmDebugIn	;Debug input
 SD  2, TrmSwScreen	;Switch screens
 SD  2, TrmInsertChr	;Insert char in input buffer
 SD  6, TrmGSetCurPat	;Set graphic cursor pattern
 SD  3, TrmGSetCurCol	;Set graphic cursor colors
 SD  4, TrmGSetCurPos	;Set graphic cursor position
 SD  2, TrmGGetCurPos	;Get graphic cursor position

 SD  6, GphDone		;Graphic support function complete

 SD  2, ScnInsertChr	;Write char to input buffer
 SD  3, ScnMapBufr	;Map physical screen buffer
 SD  6, ScnTrans	;Transfer data for screen symbiont
 SD  7, ScnMaskWrt	;Masked write to disp memory
 SD  2, ScnUtil		;Screen symbiont util functions

	.ENDM

;Following defines old style names for user mode system calls. These will
;  go away once the transistion to the new names is complete.

	.MACRO	SVCDEFUSER
 SD	IoOpen		;Open file/device
 SD	IoClose		;Close file/device
 SD	IoDevParm	;Get IO parameter values
 SD	IoPath		;Set path
 SD	IoSpecial	;Special device functions
 SD	IoSetPos	;Set position for IO
 SD	IoDevChar	;Device characteristics functions
 SD	IoInSingle	;Input single byte
 SD	IoInSingleP	;Input single byte/parms
 SD	IoInBlock	;Input block
 SD	IoInBlockP	;Input block/parms
 SD	IoOutSingle	;Output single byte
 SD	IoOutSingleP	;Output single byte/parms
 SD	IoOutBlock	;Output block
 SD	IoOutBlockP	;Output block/parms
 SD	IoOutString	;Output string
 SD	IoOutStringP	;Output string/parms
 SD	IoDelete	;Delete file
 SD	IoRename	;Rename file
 SD	IoCommit	;Commit output data
 SD	IoClsChar	;Class characteristics functions
 SD	IoGetDosDevice	;Get DOS device name
 SD	IoRun		;Run program
 SD	SysGetAllEnv
 SD	SysGetEnv
 SD	SysFindEnv
 SD	SysDefEnv
 SD	SysErrMsg

	.ENDM
