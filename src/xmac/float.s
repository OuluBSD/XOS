	.section .text

	FILDS	xxx
	FILDL	xxx
	FILDQ	xxx

	FISTS	xxx
	FISTL	xxx

	FISTPS	xxx
	FISTPL	xxx
	FISTPQ	xxx

	FLD	%st(3)
	FLDS	xxx
	FLDL	xxx
	FLDT	xxx

	FST	%st(3)
	FSTS	xxx
	FSTL	xxx

	FSTP	%st(3)
	FSTPS	xxx
	FSTPL	xxx
	FSTPT	xxx

	FSUBL	xxx
	FSUBRL	xxx

	FSUB

	FSUB	%st, %st(3)
	FSUB	%st(3), %st
	FSUBR	%st, %st(3)
	FSUBR	%st(3), %st

	FSUBP	%st, %st(3)
	FSUBRP	%st, %st(3)

	FSUB	%st(3)
	FSUBR	%st(3)

	FSUBP
	FSUBRP

	FISUBS	xxx
	FISUBS	4(%EAX)

	FISUBL	xxx
	FISUBL	4(%EAX)

	FDIVL	xxx
	FDIVRL	xxx

	FDIV

	FDIV	%st, %st(3)
	FDIV	%st(3), %st
	FDIVR	%st, %st(3)
	FDIVR	%st(3), %st

	FDIVP	%st, %st(3)
	FDIVRP	%st, %st(3)

	FDIV	%st(3)
	FDIVR	%st(3)

	FDIVP
	FDIVRP

	FIDIVS	xxx
	FIDIVS	4(%EAX)

	FIDIVL	xxx
	FIDIVL	4(%EAX)

	FADDL	xxx

	FADD

	FADD	%st, %st(3)
	FADD	%st(3), %st

	FADDP	%st, %st(3)

	FADD	%st(3)

	FADDP

	FIADDS	xxx
	FIADDS	4(%EAX)

	FIADDL	xxx
	FIADDL	4(%EAX)

	FMULL	xxx

	FMUL

	FMUL	%st, %st(3)
	FMUL	%st(3), %st

	FMULP	%st, %st(3)

	FMUL	%st(3)

	FMULP

	FIMULS	xxx
	FIMULS	4(%EAX)

	FIMULL	xxx
	FIMULL	4(%EAX)

xxx:	.LONG	123
