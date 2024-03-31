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

;Define offsets in the PDA - the PDA is an allocated memory block which
;  contains the data which define a process, including its address space.

;The minimum PDA is one page (4KB). It is expanded if more space is needed
;  for the device handle table (pdaHndlTbl).

	.MACRO	pdadef
SET0 $$$, 0
DEF Label     , 4		;Label (PDA&)
DEF Spy       , 4		;Addres of process spy data
DEF Addr      , 4		;Actual address for this PDA
DEF Pid       , 4		;Process ID
DEF Status1   , 1		;First process status byte
DEF Status2   , 1		;Second process status byte
DEF Status3   , 1		;Third process status byte
SKIP            1
DEF CR3       , 4
DEF WSLimit   , 4		;Working set size limit
DEF WSAllow   , 4		;Working set size allowed
DEF TMLimit   , 4		;Total user memory limit
DEF TMAllow   , 4		;Total user memory allowed
DEF UMLimit   , 4		;Protected mode memory limit
DEF UMAllow   , 4		;Protected mode memory allowed
DEF OMLimit   , 4		;Overhead memory limit
DEF OMAllow   , 4		;Overhead memory allowed

DEF AvlPriv   , 8		;Available privilage bits
DEF ActPriv   , 8		;Active privilege bits

DEF FileProt  , 4		;Default file protection value
DEF DirProt   , 4		;Default directory protection value

DEF WPHits    , 4		;Watchpoint hit bits
DEF WPEnable  , 4		;Enable mask for local watchpoints

DEF Country   , 4		;Country code

DEF TdaHead   , 4		;Actual address of first (base) TDA for process
DEF TdaTail   , 4		;Actual address of last TDA for process

DEF TdaNum    , 4		;Number of TDAs
DEF TdaLimit  , 4		;Maximum number of TDAs

DEF Section   , 4		;Address of section list data block
DEF Junk      , 4

DEF Lnb       , 4		;Address of first local logical name block
DEF LnbRmb    , rmb_SIZE	;RMB for the LNB list (must follow pdaLnb)
DEF PhyPath   , 4		;Address of first physical device path name
				;  block
DEF PhyPRmb   , rmb_SIZE	;RMB for the physical device path list (must
				;  follow pdaPhyPath)
DEF LogPath   , 4		;Address of first logical device path name block
DEF LogPRmb   , rmb_SIZE	;RMB for the logical device path list (must
				;  follow pdaLogPath)

DEF SigVTbl   , 256t*8		;Signal vector table
DEF SesData   , 4		;Address of session data block

DEF PLevel    , 1		;Process level
SKIP            3

DEF AlrmCnt   , 4		;Number of active alarms

DEF CDateTime , 8		;Date/time process was created
DEF SesPda    , 4		;Actual address of PDA for session process
DEF ParPda    , 4		;Actual address of PDA for parent process
DEF SibPda    , 4		;Actual address of PDA for next sibling of this
				;  process
DEF ChildPda  , 4		;Actual address of PDA for last child of this
				;  process
DEF Top       , 4		;Highest allocated address for PDA + 1

DEF Events    , 4		;Address of 1st event cluster definition block
DEF FirstMdb  , 4		;Address of first direct allocation MDB

DEF LdtSel    , 4		;Selector for the LDT
DEF LdtPnt    , 4
DEF LdtSize   , 4

DEF HdlbList  , 4		;Pointer to first HDLB for process
DEF HndlNum   , 4
DEF HndlTbl   , IHNDLNUM*8+8
DEF End       , 0

	.ENDM
.PAGE
;Define offsets in the TDAs (Task Data Area). Each TDA consists of a minimum
;  of 2 pages (8KB). This includes the thread's exec stack, the thread's TSS,
;  and all thread local exec data. Up to an additional 8KB may be used if it
;  is needed for an IO permission bit map. There is always one selector
;  associated with each TDA. This is a TSS selector that specifies the
;  thread's TSS. Since XOS does not use the hardware task switch mechanism,
;  this TSS is almost unused but is still needed to specify the initial level
;  0 stack pointer and an IO bit map (if one is used).

;TDAs are used with both user and IO threads. (In previous versions of XOS
;  Extended Fork Frames (XFFs) took the place of IO threads.)

;Since IO threads are created and destroyed often, a small number of idle TDAs
;  are kept in a free list to minimize the cost of creating and destroying
;  a thread.

tda_SZ_STK==!0x1A00		;Size of stack area

	.MACRO	tdadef
SET0 $$$, tda_SZ_STK-84t
DEF XStkBegin, 0
DEF URegGS   , 4		;Saved user GS
DEF URegFS   , 4		;Saved user FS
DEF URegES   , 4		;Saved user ES
DEF URegDS   , 4		;Saved user DS
DEF URegEDI  , 4		;Saved user EDI
DEF URegESI  , 4		;Saved user ESI
DEF URegEBP  , 4		;Saved user EBP
DEF URegxxx  , 4		;Saved user dummy ESP
DEF URegEBX  , 4		;Saved user EBX
DEF URegEDX  , 4		;Saved user EDX
DEF URegECX  , 4		;Saved user ECX
DEF URegEAX  , 4		;Saved user EAX
DEF URegEIP  , 4		;Saved user EIP
DEF URegCS   , 4		;Saved user CS
DEF URegEFR  , 4		;Saved user EFR
DEF URegESP  , 4		;Saved user ESP
DEF URegSS   , 4		;Saved user SS
DEF XStackP  , 0		;Process exec stack top for protected mode
DEF URegVES  , 4		;Saved user ES (V86 mode only)
DEF URegVDS  , 4		;Saved user DS (V86 mode only)
DEF URegVFS  , 4		;Saved user FS (V86 mode only)
DEF URegVGS  , 4		;Saved user GS (V86 mode only)e
DEF XStackV  , 0		;Process exec stack top for real mode

;Following items are preserved across putting the TDA on the free list.

DEF Label    , 4		;Label (TDA&)

DEF Addr     , 4		;Actual address of this TDA
DEF Tid      , 4		;Slot number and sequence number for the thread
				;  (Thread ID)
DEF Spy      , 4		;Addres of thread spy data
DEF PtaPTP0  , 4		;First per-thread area page table pointer
DEF PtaPTP1  , 4		;Second per-thread area page table pointer
DEF PtaPTPT  , 4		;Per-thread area user page page table pointer
DEF TssSel   , 4		;Selector for the TSS

;End of items preserved across putting the TDA on the freelist.

;The following items are used in all TDAs

DEF Pda      , 4		;Actual address of the PDA
DEF Type     , 1		;TDA type
SKIP           3
DEF Next     , 4		;Actual address of next TDA for process
DEF Prev     , 4		;Actual address of previous TDA for process

DEF TSts1    , 1		;First thread status byte
DEF TSts2    , 1		;Second thread status byte
DEF TSts3    , 1		;Third thread status byte
DEF SSigType , 1		;System signal type bits

DEF Tss      , 0		;Task state segment for process
SET0 $$$, tdaTss+tss_CR3	;The following items occupy unused TSS
				;  locations
DEF SRegESP  , 4		;ESP value while thread not running
DEF QueNext  , 4		;Actual address of next TDA in scheduler queue
DEF QuePrev  , 4		;Actual address of previous PDA in scheduler
				;  queue
DEF State    , 4		;Current thread state

DEF SigReq   , 1		;Signal request bits
DEF SigMask  , 1		;Signal request mask
DEF SigLvl   , 1		;Current signal level
DEF ResWType , 1		;Resource wait type

DEF RunTime  , 8		;Total run-time (clock ticks)
DEF InQueTime, 4		;In-queue run time remaining (clock ticks)
DEF WaitIndex, 4		;Thread wait index
DEF WaitNext , 4		;Next TDA in the wait hash list
SET0 $$$, tdaTss+104t		;Reserve space for the rest of the TSS (Note
				;  that the IO bitmap pointer at offset 0x66
				;  in the TSS is used even though no symbol
				;  is defined for it here.)

;The following items are used during IO operations and may be used by either
;  user or IO threads

DEF TmOutSub , 4		;Offset of subroutine to call on time-out
DEF Cmd      , 4		;Device command bits
DEF Delay    , 8		;IO delay value (system ticks)
DEF TimeOut  , 8		;IO time-out value (system ticks)

DEF Adb      , 4		;Address of additional data block for device

DEF CLockCnt , 4		;Number of locked disk cache buffers

;The following locations are an image of the first part of an IOQB. These
;  item are copied to or from the IOQB and must be in the same order as the
;  corresponding items in the IOQB

DEF Ioqb     , 0
DEF IoqbLabel, 4		;'IOQB'
DEF Qab      , 4		;Address of user's QAB

;The following items match both the IOQB and the QAB

DEF Func     , 2		;IO function bits
DEF Status   , 2		;IO status bits
DEF Error    , 4
DEF Amount   , 4		;Amount transfered
DEF Handle   , 4		;Device handle
DEF Vector   , 1
SKIP           3
DEF Option   , 4		;Options or command
DEF Count    , 4		;Buffer 1 size
DEF Buffer1  , 4		;Address of user's first buffer
DEF Buffer2  , 4		;Address of user's second buffer
DEF Parm     , 4		;Address of user's parameter list

;End of QAB items

DEF Frame    , 1		;Offset of dcb_inpframe or dcb_outframe relative
				;  to DCB
DEF QSts1    , 1		;Status bits
SKIP           2
DEF Dcb      , 4		;Address of DCB for device
DEF UserTda  , 4		;Address of TDA for user thread that started IO
DEF Routine  , 4

;End of items copied to or from the IORB

DEF RtnValue0, 4		;Returned value for IO operation
DEF RtnValue1, 4
DEF RtnValue2, 4
DEF RtnValue3, 4
DEF RtnValue4, 4

DEF DevAmnt  , 4		;Amount transfered by device

DEF DevName  , 16t		;Device name buffer

DEF BgnClr   , 0		;Start of item to clear when starting IO
DEF CloseSlb , 4		;Address of close SLB (Used when opening a
				;  spooled defice - We should find a better
				;  place to put this!)
DEF PVFileOptn, 4		;File option bits
DEF PPFileSpec, 4		;File specification buffer address
DEF PLFileSpec, 4		;File specification length word address
DEF PCFileSpec, 4		;File specification byte count
DEF PXFileSpec, 4		;File specification saved byte count
DEF PSFileSpec, 4		;File specification buffer size
DEF HvValue1  , 4		;Value flag bits
DEF HvValue2  , 4
SKIP            8

DEF FileAttr  , 1		;Value of IOPAR_FILATTR
DEF SrcAttr  , 1		;Value of IOPAR_SRCATTR
DEF CurAttr  , 1		;Current file attribute bits
SKIP           1
DEF DirHndl  , 4
DEF DirOfs   , 4
DEF IoPos    , 4
DEF GrpSize  , 4
DEF ReqAlloc , 4
DEF RqrAlloc , 4
DEF Length   , 4
DEF ADate    , 8
DEF CDate    , 8
DEF MDate    , 8

DEF OpenDisp , 4		;Dispatch for doopen
DEF Vect1Num , 2
DEF Vect2Num , 2
DEF Vect1Pda , 4
DEF Vect2Pda , 4
DEF ConLimit , 4		;Value of IOPAR_NETCONLIMIT
DEF ConHndl  , 4		;Value of IOPAR_NETCONHNDL
DEF Prot     , 4		;Value of IOPAR_PROT
DEF LockVal  , 8		;Value of IOPAR_SETLOCK or IOPAR_CLRLOCK
DEF ShrParms , 4		;Value of IOPAR_SHRPARMS
DEF AvlSpace , 4		;Value of IOPAR_AVLSPACE
DEF TtlSpace , 4		;Value of IOPAR_TTLSPACE
DEF ClsSize  , 1		;Value of IOPAR_CLSSIZE
DEF FSType   , 1		;Value of IOPAR_FSTYPE
SKIP           2
DEF SectSize , 2		;Value of IOPAR_SECTSIZE
DEF NumSect  , 2		;Value of IOPAR_NUMSECT
DEF NumHead  , 2		;Value of IOPAR_NUMHEAD
DEF NumCyln  , 2		;Value of IOPAR_NUMCYLN

DEF InpQLmt  , 1		;Value of IOPAR_INPQLMT
DEF OutQLmt  , 1		;Value of IOPAR_OUTQLMT
DEF BufrLmt  , 1		;Value of IOPAR_BUFRLMT
SKIP           1
DEF Protocol , 4		;Value of IOPAR_NETPROTOCOL (network only)
DEF LclPort  , 4		;Value of IOPAR_NETLCLPORT (network only)
DEF RmtPortS , 4		;Value of IOPAR_NETRMTPORTS (network only)
DEF RmtNetAS , 4		;Value of IOPAR_NETRMTNETAS (network only)
DEF KATime   , 2		;Value of IOPAR_NETKATIME (network only)
DEF PktMax   , 2		;Value of IOPAR_NETPKTMAX (network only)
DEF LRmtAddrS, 2		;Length of IOPAR_NETRMTADDRS string
SKIP           2        
DEF VRmtAddrS, 4		;Pointer to IOPAR_NETRMTADDRS string
DEF SRmtAddrS, 4		;Pointer to IOPAR_NETRMTADDRS size value

DEF EndClr   , 0		;End of items to clear when starting IO
SET1 Amnt2Clr, {{tdaEndClr-tdaBgnClr}/4}

DEF Owner    , 36t

DEF WaitList , 4		;Address of next TDA waiting for a buffer
DEF WaitBufr , 4		;Address of buffer we are waiting for
DEF WakeTmr  , 4		;Offset of time-out timer block

;End of items used in all TDAs

;The following items should only be used with user threads

DEF FpuUsed  , 1		;Non-zero if FPU has been used
SKIP           3

;SVC arguments are copied to the following items. Some of them are also
;  overloaded to hold values for some routines that do not use all of the
;  arguments.

DEF Ccb      , 0		;Address of Ccb (4 bytes)
DEF Arg10    , 4		;Tenth SVC argument
DEF CharData , 0		;Global data item used when processing
				;  characteristics (4 bytes)
DEF Arg9     , 4		;Ninth SVC argument
DEF CharValue, 0		;Characteristics value buffer (16 bytes)
DEF Arg8     , 4		;Eight SVC argument
DEF Arg7     , 4		;Seventh SVC argument
DEF Arg6     , 4		;Sixth SVC argument
DEF Arg5     , 4		;Fifth SVC argument
DEF Arg4     , 4		;Fourth SVC argument
DEF Arg3     , 4		;Third SVC argument
DEF Arg2     , 4		;Second SVC argument
DEF Arg1     , 4		;First SVC argument
DEF ArgCnt   , 4		;Arguement count for SVC
DEF ArgPnt   , 4		;Arguement pointer for SVC (points to first
				;  argument on user's stack)

;The following items are the head and tail pointers for the signal queues

DEF L1HeadP  , 4		;Level 1 signal list head pointer
DEF L2HeadP  , 4		;Level 2 signal list head pointer
DEF L3HeadP  , 4		;Level 3 signal list head pointer
DEF L4HeadP  , 4		;Level 4 signal list head pointer
DEF L5HeadP  , 4		;Level 5 signal list head pointer
DEF L6HeadP  , 4		;Level 6 signal list head pointer
DEF L7HeadP  , 4		;Level 7 signal list head pointer
DEF L8HeadP  , 4		;Level 8 signal list head pointer
DEF L1TailP  , 4		;Level 1 signal list tail pointer
DEF L2TailP  , 4		;Level 2 signal list tail pointer
DEF L3TailP  , 4		;Level 3 signal list tail pointer
DEF L4TailP  , 4		;Level 4 signal list tail pointer
DEF L5TailP  , 4		;Level 5 signal list tail pointer
DEF L6TailP  , 4		;Level 6 signal list tail pointer
DEF L7TailP  , 4		;Level 7 signal list tail pointer
DEF L8TailP  , 4		;Level 8 signal list tail pointer
SET1 xxTP, tdaL1TailP-tdaL1HeadP

DEF URegDR0   , 4		;Virtual DR0
DEF URegDR1   , 4		;Virtual DR1
DEF URegDR2   , 4		;Virtual DR2
DEF URegDR3   , 4		;Virtual DR3
DEF URegDR6   , 4		;Virtual DR6
DEF URegDR7   , 4		;Virtual DR7
DEF URegCR2   , 4
DEF TrapErr   , 4

DEF VRegEFR   , 4		;Virtual EFR value

DEF TrapSlb   , slb_SIZE	;Partial SLB used when granting a trap
DEF TrapVect  , 4
DEF TrapData  , 4
DEF TrapCode  , 4

DEF TermEvNum , 1		;Event number for process termination
SKIP            2
DEF CritState , 1		;Thread's critical region state
DEF CritNext  , 4		;Actual address of TDA for next thread in
				;  critical region wait list

DEF TermEvPid , 4		;PID for termination event
DEF TermSigPid, 4		;PID for termination signal
DEF TermSts   , 4		;Termination status value
DEF TermData  , 4		;Termination data value
;;;;DEF TermWkTda , 4		;Address of TDA for thread to wake up when
				;  termination is complete - This is only set
				;  when a thread is being forcefully terminated
				;  by another thread.

DEF Response  , 4		;Response value
DEF ErrCode   , 4		;Hardware error code value
DEF ThrdData  , 4		;User thread data (stored in hum_thrddata when
				;  thread is running

;End of items used with user threads.

;The following items should be used with IO threads only

DEF IoQHead   , 4		;IO queue head pointer
DEF IoQTail   , 4		;IO queue tail pointer
DEF IoQCnt    , 1		;IO queue count
SKIP            3
DEF SpoolUnit , 4

DEF IoBMTop   , 4		;Hightest available IO bitmap address + 1

DEF End       , 0		;This symbol is used to verify that the
				;  size of the TDA structure matches these
				;  definitions!

SET0 $$$, 0x1E00		;The FPU state save area is allocated at tne
				;  end of the 2nd TDA page. Putting it here
				;  is the easiest way to ensure the necessary
				;  16-byte alignment.
DEF FpuState , 512t		;FPU state save area

;The IO bit map starts at the beginning of the third page. One or two
;  additional pages are allocated when an IO bitmap is needed. Note that
;  the IO bitmap pages are only mapped at their actual addresses. Given how
;  seldom they are accessed, it is not worth the overhead to doubly map them.
;  (The overhead is significant since the second mapping has to be modified
;  for  each page on every context switch!)

SET0 $$$, 0x2000
DEF IoBitMap , 0		;IO permission bitmap

	.ENDM
.PAGE
	.MACRO	sdadef
SET0 $$$, 0x1000
DEF Magic    , 4		;Magic number which identifies an SDA
DEF Sid      , 4		;Segment ID
DEF Next     , 4		;Selector for next SDA
DEF Type     , 1		;Segment type
SKIP           3
DEF UseCnt   , 4		;Use count
DEF Level    , 4		;Compatibility level
DEF Name     , 48t		;Name of this segment
DEF MdbHead  , 4		;Offset of first MDA for this segment
DEF MdbTail  , 4		;Offset of last MDA for this segment
DEF TotalSize, 4		;Total length of the segment (pages)
DEF SdaSize  , 4		;Length of this SDA (pages)
DEF Prot     , 4		;Protection value
DEF ACList   , 4		;Offset of start of access control list
DEF FPnt     , 0		;First memory pointer for segment
	.ENDM
