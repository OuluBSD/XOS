//============================================
// VENDOR.C
// Written by John Goltz
//============================================

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

#include "usbctl.h"


static void endpoints(char *pnt, int *min, int *max);
static int  getdec(char **ppnt, ulong *val);
static int  gethex(char **ppnt, ulong *val);
static void getstr(char *pnt, char *bfr, int len, int num);


int findvendor(
	ulong ven,
	ulong dev,
	DRV  *drv)

{
	FILE *file;
	char *pnt;
	ulong val1;
	ulong val2;
	int   stopper;
	char  bufr[256];

	sprintf(bufr, "XOSUSB:VEN%04X.USB", ven);
	if ((file = fopen(bufr, "r")) == NULL)
		return (FALSE);
	while (fgets(bufr, sizeof(bufr), file) != NULL)
	{
		if (strnicmp(bufr, "DEV:", 4) == NULL)
		{
			pnt = bufr + 4;
			while (*pnt != '\n')
			{
				if ((stopper = gethex(&pnt, &val1)) == '-')
					stopper = gethex(&pnt, &val2);
				else
					val2 = val1;
				if (stopper == ',' || stopper == '\n' &&
						dev >= val1 && dev <= val2)
				{
					drv->name[0] = 0;
					drv->func[0]= 0;
					drv->drvr[0][0] = 0;
					drv->drvr[1][0] = 0;
					drv->drvr[2][0] = 0;
					drv->drvr[3][0] = 0;
					drv->clss[0] = 0;
					drv->dtype[0] = 0;
					drv->bimin = 0;
					drv->bimax = 16;
					drv->bomin = 0;
					drv->bomax = 16;
					drv->iimin = 0;
					drv->iimax = 16;
					drv->iomin = 0;
					drv->iomax = 16;
					drv->unbase = 0;
					drv->uninc = 1;
					while (fgets(bufr, sizeof(bufr), file) != NULL)
					{
						if (strnicmp(bufr, "NAME:", 5) ==0)
							getstr(bufr + 5, drv->name, sizeof(drv->name), 1);
						if (strnicmp(bufr, "FNC:", 4) == 0)
							getstr(bufr + 4, drv->func, sizeof(drv->func), 1);
						else if (strnicmp(bufr, "CLS:", 4) == 0)
							getstr(bufr + 4, drv->clss, sizeof(drv->clss), 1);
						else if (strnicmp(bufr, "DRV:", 4) == NULL)
							getstr(bufr + 4, drv->drvr[0], 32, 4);
						else if (strnicmp(bufr, "DTP:", 4) == 0)
							getstr(bufr + 4, drv->dtype, sizeof(drv->dtype), 1);
						else if (strnicmp(bufr, "BI:", 3) == 0)
							endpoints(bufr + 3, &drv->bimin, &drv->bimax);
						else if (strnicmp(bufr, "BO:", 3) == 0)
							endpoints(bufr + 3, &drv->bomin, &drv->bomax);
						else if (strnicmp(bufr, "II:", 3) == 0)
							endpoints(bufr + 3, &drv->iimin, &drv->iimax);
						else if (strnicmp(bufr, "II:", 3) == 0)
							endpoints(bufr + 3, &drv->iomin, &drv->iomax);
					}
					fclose(file);
					return (drv->func[0] != 0 && drv->drvr[0][0] != 0 &&
							drv->clss[0] != 0);
				}
			}
		}
	}
	fclose(file);
	return (FALSE);
}


static void getstr(
	char *pnt,
	char *bfr,
	int   len,
	int   num)

{
	char *dst;
	int   cnt;
	char  chr;

	do
	{
		dst = bfr;
		cnt = len;
		while ((chr = *pnt++) != 0 && chr != ',' && chr != '\n')
		{
			if (--cnt > 0)
				*dst++ = toupper(chr);
		}
		*dst = 0;
		bfr += len;
	} while (chr == ',' && --num > 0);
}


static void endpoints(
	char *pnt,
	int  *min,
	int  *max)

{
	ulong val;
	int   chr;

	if (((chr = getdec(&pnt, &val)) == ',' || chr == '\n') && val <= 16)
	{
		*min = val;
		if (chr == ',' && getdec(&pnt, &val) == '\n' && val <= 16 &&
				val > *min)
			*max = val;
	}
}


static int gethex(
	char **ppnt,
	ulong *pval)

{
	char *pnt;
	ulong value;
	int   chr;

	value = 0;
	pnt = *ppnt;
	while ((chr = *pnt++) != 0 && isxdigit(chr))
	{
		if (chr > '9')
			chr += 9;
		value = (value << 4) + (chr & 0x0F);
	}
	*pval = value;
	*ppnt = pnt;
	return (chr);
}


static int getdec(
	char **ppnt,
	ulong *pval)

{
	char *pnt;
	ulong value;
	int   chr;

	value = 0;
	pnt = *ppnt;
	while ((chr = *pnt++) != 0 && isdigit(chr))
		value = value * 10 + (chr & 0x0F);
	*pval = value;
	*ppnt = pnt;
	return (chr);
}
