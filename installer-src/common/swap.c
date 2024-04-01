// Note that even though these functions are defined here as returning "void",
//   they really do return values!


void sll(
	long lowval,
	long highval)

{
	_asm
	{
		mov		EAX, highval
		xchg	AL, AH
		ror		EAX, 16
		xchg	AL, AH
		mov		EDX, lowval
		xchg	DL, DH
		ror		EDX, 16
		xchg	DL, DH
	}
}


//****************************************************
// Function: sl - Reverse byte order in a 4-byte value
// Returned: Resulting value
//****************************************************

void sl(
	long value)

{
	__asm
	{
		mov		EAX, value
		xchg	AL, AH
		ror		EAX, 16
		xchg	AL, AH
	}
}

//****************************************************
// Function: sw - Reverse byte order in a 2-byte value
// Returned: Resulting value
//****************************************************

void sw(
	long value)

{
	__asm
	{
		mov		EAX, value
		xchg	AL, AH
	}
}