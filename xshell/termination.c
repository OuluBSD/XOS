//------------------------------------------------------------------------------
//
//  child.c
//
//  Written by John Goltz
//
//  Edit History:
//  -------------
//------------------------------------------------------------------------------

#include "xshell.h"


// Function to report reason child died

void childreport(
	CDATA *pnt)
{
    char *str1;
    char *str2;
    long  status;
    long  intnum;
	int   len;
	char  buffer[256];

    status = pnt->status;
    switch ((short)pnt->trm)
    {
	 case TC_EXIT:						// Process terminated with exit
	 case TC_CTRLC:						// Process terminated by ^C
		break;

	 case TC_KILL:						// Process killed
		len = sprintf(buffer, "\n\x1b[KProcess %u.%u killed: status = 06X\n",
				pnt->pid>>16, pnt->pid & 0x0FFFF, status);
		goto display;

	 case TC_PIPE:						// Process terminated because pipe
										//   had no more input
		len = sprintf(buffer, "\n\x1B[KProcess %u.%u out of input data: "
				"status = %6.6X\n\X1B[K", pnt->pid>>16,
				pnt->pid & 0xFFFF, status);
		goto display;

	 case TC_BDUSTK:					// Bad user stack address
		len = sprintf(buffer, "\n\x1B[K? %s: Illegal user stack pointer in "
				"process %u.%u\n\x1B[K          "
				"CS:EIP = %4.4X:%8.8X, EFR = %8.8X, "
				"SS:ESP = %4.4X:%8.8X\n\x1B[K", prgname, pnt->pid >> 16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR,
				pnt->segment, pnt->offset);
		goto display;

	 case TC_NOMEM:						// No memory available
		len = sprintf(buffer, "\n\x1B[K? %s: No memory available for process "
				"%u.%u\n\x1B[K          CS:EIP = %4.4X:%8.8X, "
				"EFR = %8.8X\n\x1B[K", prgname, pnt->pid>>16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR);
		goto display;

	 case TC_UNIMOP:					// Unimplemented operation
		len = sprintf(buffer, "\n\x1B[K? %s: Unimplemented operation in "
				"process %u.%u\n          CS:EIP = %4.4X:%8.8X, EFR = "
				"%8.8X\n\x1B[K         Data = %2.2X, %2.2X, %2.2X, "
				"%2.2X\n\x1B[K", prgname, pnt->pid>>16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR,
				pnt->data & 0xFF, (pnt->data/0x100) & 0xFF,
				(pnt->data/0x10000) & 0xFF, (pnt->data/0x1000000) & 0xFF);
		goto display;

	 case TC_RUNFAIL:					// Run svc failed
		len = sprintf(buffer, "\n\x1B[K? %s: Run SVC failed in process "
				"%u.%u\n\x1B[K          CS:EIP = %4.4X:%8.8X, "
				"EFR = %8.8X\n\x1B[K", prgname, pnt->pid >> 16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR);
		goto display;

	 case TC_NOVECT:					// Uninitialized user vector
		switch ((int)status)
		{
		 case VECT_ILLINS:				// Illegal instruction
			str1 = "Illegal instruction";
			goto novect;

		 case VECT_DIVERR:				// Divide error
			str1 = "Divide error";
			goto novect;

		 case VECT_BRKPNT:				// goto displaypoint interrupt
			str1 = "Breakpoint trap";
			goto novect;

		 case VECT_FPPNAVL:				// Coprocessor not available
			str1 = "Coprocessor not available";
			goto novect;

		 case VECT_FPUERR:				// Coprocessor error
			str1 = "Coprocessor error";
			goto novect;

		 case VECT_BOUND:				// BOUND instruction
			str1 = "BOUND instruction trap";
			goto novect;

		 case VECT_INTO:				// INTO instruction
			str1 = "INTO instruction trap";
			goto novect;

		 case VECT_PTERM:				// Child died
			str1 = "Process terminated signal";
			goto novect;

		 case VECT_CNTC:				// Control-C
			str1 = "Control-C signal";
		 novect:
			len = sprintf(buffer, "\n\x1B[K? %s: %s in process %u.%u"
					"\n\x1B[K          CS:EIP = %4.4X:%8.8X, "
					"EFR = %8.8X\n\x1B[K", prgname, str1,
					pnt->pid>>16, pnt->pid & 0xFFFF, pnt->pCS,
					pnt->pEIP, pnt->pEFR);
			goto display;

		 case VECT_PROT:				// Protection fault
			str1 = "Protection";
			goto segerr;

		 case VECT_SEGNP:				// Segment not present
			str1 = "Segment not present";
		 segerr:
			len = sprintf(buffer, "\n\x1B[K? %s: %s exception in process "
					"%u.%u\n\x1B[K          CS:EIP = %4.4X:%8.8X, "
					"EFR = %8.8X, Selector = %4.4X\x1B[K",
					prgname, str1, pnt->pid >> 16, pnt->pid & 0xFFF,
					pnt->pCS, pnt->pEIP, pnt->pEFR,
					pnt->segment & 0xFFFC);
			goto display;

		 case VECT_PAGEFLT:				// Page fault
			str1 = "Page fault";
			goto addrerr;

		 case VECT_ALNCHK:				// Alignment error
			str1 = "Alignment";
		 addrerr:
			len = sprintf(buffer, "\n\x1B[K? %s: %s exception in process "
					"%u.%u\n\x1B[K          CS:EIP = %4.4X:%8.8X, "
					"EFR = %8.8X, Memory = %4.4X:%8.8X\n\x1B[K",
					prgname, str1, pnt->pid >> 16, pnt->pid & 0xFFF, pnt->pCS,
					pnt->pEIP, pnt->pEFR, pnt->segment, pnt->offset);
			break;

		 default:
			intnum = status;
			if (status < 0x100)
			{
				str1 = "protected mode ";
				str2 = "interrupt";
			}
			else if (status < 0x120)
			{
				str1 = "protected mode ";
				str2 = "processor exception";
				intnum -= 0x100;
			}
			else if (status < 0x140)
			{
				str1 = "V86 mode ";
				str2 = "processor exception";
				intnum -= 0x120;
			}
			else if (status < 0x160)
			{
				str1 = "";
				str2 = "XOS exception";
				intnum -= 0x140;
			}
			else if (status >= 0x200 && status < 0x300)
			{
				str1 = "V86 mode ";
				str2 = "processor exception";
				intnum -= 0x200;
			}
			else
			{
				len = sprintf(buffer, "\n\x1B[K? %s: Uninitialized signal "
						"0x%X\n\x1B{K          in process %u.%u, "
						"CS:EIP = %4.4X:%8.8X, EFR = %8.8X\n",
						prgname, status, pnt->pid >> 16,
						pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR);
				break;
			}
			len = sprintf(buffer, "\n\x1B[K? %s: Uninitialized %s%s 0x%X "
					"(signal 0x%X)\n\x1B[K          in process "
					"%u.%u, CS:EIP = %4.4X:%8.8X, EFR = %8.8X\n",
					prgname, str1, str2, intnum, status, pnt->pid >> 16,
					pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR);
			break;
		}
		goto display;

	 case TC_ACCT:
		len = sprintf(buffer, "\n\x1B[K? %s: Accounting violation in process "
				"%u.%u\n\x1B[K         "
				"CS:EIP = %4.4X:%8.8X, EFR = %8.8X, "
				"SS:ESP = %4.4X:%8.8X\n\x1B[K", prgname, pnt->pid >> 16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR,
				pnt->segment, pnt->offset);
		goto display;

	 case TC_BDCRIT:					// Invalid critical region
		len = sprintf(buffer, "\n\x1B[K? %s: Invalid critical region in "
				"process %u.%u\n\x1B[K         "
				"CS:EIP = %4.4X:%8.8X, EFR = %8.8X, "
				"SS:ESP = %4.4X:%8.8X\n\x1B[K", prgname, pnt->pid >> 16,
				pnt->pid & 0xFFFF, pnt->pCS, pnt->pEIP, pnt->pEFR,
				pnt->segment, pnt->offset);
		goto display;

	 default:
		len = sprintf(buffer, "\n\x1B[K? %s: Process %u.%u terminated "
				"with unknown termination type = 0x%2.2X\n\x1B[K", prgname,
				pnt->pid >> 16, pnt->pid & 0xFFFF, pnt->trm);
	 display:
		if (flags & FLG_HAVECC)
		{
			histcurblk = NULL;
			svcSchCtlCDone();
		}
		svcIoOutBlock(ccs->cmderr, buffer, len);
		break;
	}
}
