.section .text

.globl _main
_main:
LFB3:
	FSUBL	%st, %st(3)
	FSUBL	%st(3), %st

	testb	$4, __sch_istable+1(%eax,%eax)

	.ASCII	"abc\12\0"

	jmp	*L320(,%eax,4)

	leal	14(%eax,%edx), %edi

	shrl	$19, %eax

	imull	$66, zzzz, %ebx

	imull	_numcolumns, %eax
	imull	$55, %ebx
	imull	$55, %ebx, %ebx

	jmp	*%eax
	jmp	*%ax
	jmp	*kkk

	pushl	$5
	subl	$5, %esp
	subl	$200, %esp

	movl	$0, 5(%eax)
	.loc 1 481 0
LBB2:
	subl	$12, %esp
	pushl	$_pib
LCFI3:
	call	_reg_pib
	addl	$16, %esp

L320:	.LONG	1234
