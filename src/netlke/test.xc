

int x_getuser(long *loc, long *ua);

long getuser(long *loc, long *ua);
#pragma aux getuser =	\
	"CALL  getuser"	\
	"JC    short dn"	\
	"XOR   EAX, EAX"	\
"dn:"					\
	parm [EAX] [EDX] modify [EAX] value [EAX];


union uuu
{	long ll;
	char *cc;
};


long *uaddr;


long func(
	union uuu a)
{
	long value;

	if (getuser(&value, uaddr))
		return (-7);
	return (value + a.ll);
}


union uuu a1;



void qqq(void)
{
	a1.cc = "abc";
	func(a1);
}
