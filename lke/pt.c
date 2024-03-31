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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xosstr.h>
#include <xos.h>
#include <xoserrmsg.h>
#include <xossvc.h>
#include <xoserr.h>


// NOTES:

//   $&bnW is used to send PML data (PML appears to use 1284.4 on EP3)
//   $&onW is the "Driver configure" command:
//               1st byte: Device ID
//               2nd byte: Function
//       additional bytes: Function arguments
//     This appears to set various parameters for specific printers.
//   $*bnW is the "new" configuration set up command (n = 20):
//     Offset  Size  Description
//        0      1   Format (= 6)
//        1      1   1F (KRGB ID)
//        2      2   Component count (= 2), MSB first
//                   First component:
//        4      2   Horizontal resolution, MSB first
//        6      2   Vertical resolution, MSB first
//        8      1   Compression method (= 9)
//        9      1   Orientation (= 0, pixel major mode)
//       10      1   Bits per component (= 1)
//       11      1   Planes per component (= 1)
//                   Second component:
//       12      2   Horizontal resolution, MSB first
//       14      2   Vertical resolution, MSB first
//       16      1   Compression method (= 10)
//       17      1   Orientation (=1, pixel major mode)
//       18      1   Bits per component (= 32)
//       19      1   Planes per component (= 1)
//     This specifies two "planes". The first is a 1-bit per pixel monochrome
//       plane which uses mode 9 compression. The second is a 32-bit per
//       pixel color plane which uses mode 10 compression.

// Compression modes: (Some of the HP printers require specific compression
//   modes!)

//   Counts: All count fields can be extended by specifying the maximum value
//     that fits in the fields. Each extention is an 8-bit byte. When the
//     length of a string of data bytes is extended, the extention bytes are
//     distributed thourgh out the data byte. Each field is followed by the
//     number of bytes it specifies followed by the next count extention byte,
//     etc. The last count extension byte must be less than 255. It may be 0.

//   Mode 9: Replacement delta row encoding for 1 bit per pixel planes

//     Command byte formats:

//       +---------------+  Uncompressed:
//       |0|C C C C|R R R|    C = Number of bytes to copy from seed row
//       +---------------+    R = Number of bytes - 1 to copy from data

//       +---------------+  Compressed (run length encoded):
//       |1|C C|R R R R R|    O = Number of bytes to copy from seed row
//       +---------------+    R = Number of times - 1 to repeat data byte

//   Mode 10: Modified replacement Delta Row Encoding (this appears to be
//              mode 9 extended for single plane color data)

//     Pixel formats:

//       +---------------+---------------+  Short delta pixel value
//       |1|R R R R R|G G G G G|B B B B B|    R = Red difference
//       +---------------+---------------+    G = Green difference
//                                            B = Blue difference / 2

//       +---------------+---------------+---------------+ Long pixel value
//       |0|R R R R R R R R|G G G G G G G G|B B B B B B B|   R = Red value
//       +---------------+---------------+---------------+   G = Green value
//                                                           B = Blue value / 2

//     Command byte formats:

//       +---------------+  Uncompressed:
//       |0|S S|C C|R R R|    S = Pixel source
//       +---------------+    C = Number of bytes to copy from seed row
//                            R = Number of bytes - 1 to copy from data

//       +---------------+  Compressed (run length encoded):
//       |1|S S|C C|R R R|    S = Pixel source
//       +---------------+    C = Number of bytes to copy from seed row
//                            R = Number of times - 1 to repeat data byte
//       Pixel source values:
//         00 = New pixel
//         01 = West pixel
//         10 = Northeast pixel
//         11 = Cached pixel (last new pixel specified)

// Currently only the monochorme plane with mode 9 compression is supported!!!

char *bfr;
char *pnt;
int   left;
int   amnt;
long  rtn;
long  ptr;

char prgname[] = "PT";

char *putline(char *pnt, int x1, int x2);


char buffer[20000];


char bgnpart[] =
	"\x1B""E"
	"\x1B%-12345X@PJL ENTER LANGUAGE=PCL3GUI\r\n"
	"\x1B""E"
///	"\x1B&b27WPML \x04\x00\x04\x01\x01\x02\x08\x10\x0E\x01\x15"
///			"200610250022\x00"
///	"\x1B&b16WPML \x04\x00\x06\x01\x04\x01\x04\x01\x06\x08\x01\x00"
	"\x1B*r4800SB"
	"\x1B&l0M"

	"\x1B*o5W\x0D\x03\x00\x00\x01"

	"\x1B&l1H"
	"\x1B&l2A"
	"\x1B&l0E"
	"\x1B&l0L"
	"\x1B&u600D"
	"\x1B*o0M"
	"\x1B&l-2H"
	"\x1B&l0O"

	"\x1B&o5W\x0E\x03\x00\x00\x00"

	"\x1B&o7W\x08\x09\x00\x00\x07\x00\x01"

	"\x1B*g20W\x06\x1F\x00\x02\x02\x58\x02\x58\x09\x00"
			 "\x01\x01\x02\x58\x02\x58\x0A\x01\x20\x01"
	"\x1B*r1A"

	"\x1B*b331y0v20w"
		 "\x9F\xFF\xFF\xFF\xFF\x17\x00\x00\x00\xFF"
		 "\xFF\x9F\xFF\xFF\xBD\xFF\x72\xFF\x90\x90"
	"0v17w\xDF\xFF\xFF\xFF\xFF\x16\xFF\xFF\xA1\xDF"
	     "\xFF\xBB\xFF\x74\xDF\x8E\x92"
	"0v0w0v0w0v27w"
	     "\x78\xFF\xFF\xFF\xFF\x16\xFF\xFF\xFF\xA6"
	     "\xDD\xB7\xFF\xFF\x78\x8A\xFF\x96\xFF\xFF"
	     "\xFF\xFF\xFF\xFF\xFF\xFF\x2E"


/*

	"290y125v"
		"\x78\x45\x7F\x81\xFF\x08\x03\x80\xFF\x01"
		"\xFC\x7F\x81\xFF\x08\x03\x80\xFF\x01\xFC"
		"\x7F\x81\xFF\x08\x03\x80\xFF\x01\xFC\x7F"
		"\x81\xFF\x08\x03\x80\xFF\x01\xFC\x7F\x81"
		"\xFF\x08\x03\x80\xFF\x01\xFC\x7F\x81\xFF"
		"\x08\x03\x80\xFF\x01\xFC\x7F\x81\xFF\x08"
		"\x03\x80\xFF\x01\xFC\x7F\x81\xFF\x08\x03"
		"\x80\xFF\x00\xFC\xE1\x13\xFF\x00\xFE\x08"
		"\x07\x80\xFF\x00\xF8\x81\xFF\x00\xFE\x08"
		"\x07\x80\xFF\x00\xF8\x81\xFF\x00\xFE\x08"
		"\x07\x80\xFF\x00\xF8\x81\xFF\x00\xFE\x08"
		"\x07\x80\xFF\x00\xF8\x81\xFF\x00\xFE\x08"
		"\x07\x80\xFF\x00\xF8"
	"0w0v0w108v"
		"\x78\x45\x07\x10\xF0\x09\x00\x1F\x09\xC0"
		"\x07\x10\xF0\x09\x00\x1F\x09\xC0\x07\x10"
		"\xF0\x09\x00\x1F\x09\xC0\x07\x10\xF0\x09"
		"\x00\x1F\x09\xC0\x07\x10\xF0\x09\x00\x1F"
		"\x09\xC0\x07\x10\xF0\x09\x00\x1F\x09\xC0"
		"\x07\x10\xF0\x09\x00\x1F\x09\xC0\x07\x10"
		"\xF0\x09\x00\x1F\x08\xC0\x78\x07\x0F\x10"
		"\xE0\x09\x00\x3F\x09\x80\x0F\x10\xE0\x09"
		"\x00\x3F\x09\x80\x0F\x10\xE0\x09\x00\x3F"
		"\x09\x80\x0F\x10\xE0\x09\x00\x3F\x09\x80"
		"\x0F\x10\xE0\x09\x00\x3F\x08\x80"
	"0w90v"
		"\x78\x45\x00\x10\xC0\x10\x0F\xA0\x00\x10"
		"\xC0\x10\x0F\xA0\x00\x10\xC0\x10\x0F\xA0"
		"\x00\x10\xC0\x10\x0F\xA0\x00\x10\xC0\x10"
		"\x0F\xA0\x00\x10\xC0\x10\x0F\xA0\x00\x10"
		"\xC0\x10\x0F\xA0\x00\x10\xC0\x10\x0F\xB5"
		"\x00\x00\x01\x10\x80\x13\x1F\xFE\x00\x01"
		"\x10\x80\x13\x1F\xFE\x00\x01\x10\x80\x13"
		"\x1F\xFE\x00\x01\x10\x80\x13\x1F\xFE\x00"
		"\x01\x10\x80\x11\x1F\xFE\x9F\xFF\x6E\x00"
	"0w72v"
		"\x78\x46\x3F\x21\x07\xFC\x10\x3F\x21\x07"
		"\xFC\x10\x3F\x21\x07\xFC\x10\x3F\x21\x07"
		"\xFC\x10\x3F\x21\x07\xFC\x10\x3F\x21\x07"
		"\xFC\x10\x3F\x21\x07\xFC\x10\x3F\x21\x07"
		"\xFC\x79\x08\x00\x7F\x21\x0F\xF8\x09\x00"
		"\x7F\x21\x0F\xF8\x09\x00\x7F\x21\x0F\xF8"
		"\x09\x00\x7F\x21\x0F\xF8\x09\x00\x7F\x21"
		"\x0F\xF8"
	"0w80v"
		"\x78\x46\x1F\x08\x80\x18\xF8\x10\x1F\x08"
		"\x80\x18\xF8\x10\x1F\x08\x80\x18\xF8\x10"
		"\x1F\x08\x80\x18\xF8\x10\x1F\x08\x80\x18"
		"\xF8\x10\x1F\x08\x80\x18\xF8\x10\x1F\x08"
		"\x80\x18\xF8\x10\x1F\x08\x80\x18\xF8\x78"
		"\x09\x3F\xA1\x00\x08\xF0\x10\x3F\xA1\x00"
		"\x08\xF0\x10\x3F\xA1\x00\x08\xF0\x10\x3F"
		"\xA1\x00\x08\xF0\x10\x3F\xA1\x00\x08\xF0"
	"0w54v"
		"\x78\x46\x0F\x28\xF0\x10\x0F\x28\xF0\x10"
		"\x0F\x28\xF0\x10\x0F\x28\xF0\x10\x0F\x28"
		"\xF0\x10\x0F\x28\xF0\x10\x0F\x28\xF0\x10"
		"\x0F\x28\xF0\x78\x09\x1F\x28\xE0\x10\x1F"
		"\x28\xE0\x10\x1F\x28\xE0\x10\x1F\x28\xE0"
		"\x10\x1F\x28\xE0"
	"0w54v"
		"\x78\x46\x07\x28\xE0\x10\x07\x28\xE0\x10"
		"\x07\x28\xE0\x10\x07\x28\xE0\x10\x07\x28"
		"\xE0\x10\x07\x28\xE0\x10\x07\x28\xE0\x10"
		"\x07\x28\xE0\x78\x09\x0F\x28\xC0\x10\x0F"
		"\x28\xC0\x10\x0F\x28\xC0\x10\x0F\x28\xC0"
		"\x10\x0F\x28\xC0"
	"0w80v"
		"\x78\x46\x03\x08\xC0\x18\xC0\x10\x03\x08"
		"\xC0\x18\xC0\x10\x03\x08\xC0\x18\xC0\x10"
		"\x03\x08\xC0\x18\xC0\x10\x03\x08\xC0\x18"
		"\xC0\x10\x03\x08\xC0\x18\xC0\x10\x03\x08"
		"\xC0\x18\xC0\x10\x03\x08\xC0\x18\xC0\x78"
		"\x09\x07\x08\x80\x18\x80\x10\x07\x08\x80"
		"\x18\x80\x10\x07\x08\x80\x18\x80\x10\x07"
		"\x08\x80\x18\x80\x10\x07\x08\x80\x18\x80"
	"0w74v"
		"\x78\x46\x01\x21\x0F\x80\x10\x01\x21\x0F"
		"\x80\x10\x01\x21\x0F\x80\x10\x01\x21\x0F"
		"\x80\x10\x01\x21\x0F\x80\x10\x01\x21\x0F"
		"\x80\x10\x01\x21\x0F\x80\x10\x01\x21\x0F"
		"\x80\x78\x09\x03\x20\x1F\x81\x00\x00\x03"
		"\x20\x1F\x81\x00\x00\x03\x20\x1F\x81\x00"
		"\x00\x03\x20\x1F\x81\x00\x00\x03\x20\x1F"
		"\x9F\xFF\x6F\x00"
	"0w81v"
		"\x78\x46\x00\x08\xE0\x10\x1F\x82\x00\x08"
		"\xE0\x10\x1F\x82\x00\x08\xE0\x10\x1F\x82"
		"\x00\x08\xE0\x10\x1F\x82\x00\x08\xE0\x10"
		"\x1F\x82\x00\x08\xE0\x10\x1F\x82\x00\x08"
		"\xE0\x10\x1F\x82\x00\x08\xE0\x10\x1F\x97"
		"\x00\x00\x01\x08\xC0\x10\x3E\x18\x01\x08"
		"\xC0\x10\x3E\x18\x01\x08\xC0\x10\x3E\x18"
		"\x01\x08\xC0\x10\x3E\x18\x01\x08\xC0\x10"
		"\x3E"
	"0w28v"
		"\x78\x4B\x1E\x40\x1E\x40\x1E\x40\x1E\x40"
		"\x1E\x40\x1E\x40\x1E\x40\x1E\x78\x0F\x3C"
		"\x40\x3C\x40\x3C\x40\x3C\x40\x3C"
	"0w72v"
		"\x79\x47\x7F\xF0\x10\x3E\x21\x7F\xF0\x10"
		"\x3E\x21\x7F\xF0\x10\x3E\x21\x7F\xF0\x10"
		"\x3E\x21\x7F\xF0\x10\x3E\x21\x7F\xF0\x10"
		"\x3E\x21\x7F\xF0\x10\x3E\x21\x7F\xF0\x10"
		"\x3E\x78\x0A\x00\x08\xE0\x10\x7C\x18\x00"
		"\x08\xE0\x10\x7C\x18\x00\x08\xE0\x10\x7C"
		"\x18\x00\x08\xE0\x10\x7C\x18\x00\x08\xE0"
		"\x10\x7C"
	"0w54v"
		"\x78\x47\x3F\x18\x7C\x20\x3F\x18\x7C\x20"
		"\x3F\x18\x7C\x20\x3F\x18\x7C\x20\x3F\x18"
		"\x7C\x20\x3F\x18\x7C\x20\x3F\x18\x7C\x20"
		"\x3F\x18\x7C\x78\x0B\x7F\x18\xF8\x20\x7F"
		"\x18\xF8\x20\x7F\x18\xF8\x20\x7F\x18\xF8"
		"\x20\x7F\x18\xF8"
	"0w72v"
		"\x79\x47\x1F\xF8\x10\xF8\x21\x1F\xF8\x10"
		"\xF8\x21\x1F\xF8\x10\xF8\x21\x1F\xF8\x10"
		"\xF8\x21\x1F\xF8\x10\xF8\x21\x1F\xF8\x10"
		"\xF8\x21\x1F\xF8\x10\xF8\x21\x1F\xF8\x10"
		"\xF8\x79\x0B\x3F\xF0\x09\x01\xF0\x21\x3F"
		"\xF0\x09\x01\xF0\x21\x3F\xF0\x09\x01\xF0"
		"\x21\x3F\xF0\x09\x01\xF0\x21\x3F\xF0\x09"
		"\x01\xF0"
	"0w54v"
		"\x78\x48\xFC\x10\xF0\x28\xFC\x10\xF0\x28"
		"\xFC\x10\xF0\x28\xFC\x10\xF0\x28\xFC\x10"
		"\xF0\x28\xFC\x10\xF0\x28\xFC\x10\xF0\x28"
		"\xFC\x10\xF0\x78\x0C\xF8\x10\xE0\x28\xF8"
		"\x10\xE0\x28\xF8\x10\xE0\x28\xF8\x10\xE0"
		"\x28\xF8\x10\xE0"
	"0w67v"
		"\x79\x47\x0F\xFE\x08\x01\x29\x0F\xFE\x08"
		"\x01\x29\x0F\xFE\x08\x01\x29\x0F\xFE\x08"
		"\x01\x29\x0F\xFE\x08\x01\x29\x0F\xFE\x08"
		"\x01\x29\x0F\xFE\x08\x01\x29\x0F\xFE\x08"
		"\x01\x79\x0C\x1F\xFC\x08\x03\x29\x1F\xFC"
		"\x08\x03\x29\x1F\xFC\x08\x03\x29\x1F\xFC"
		"\x08\x03\x29\x1F\xFC\x08\x03"
	"0w67v"
		"\x78\x47\x07\x11\x03\xE0\x20\x07\x11\x03"
		"\xE0\x20\x07\x11\x03\xE0\x20\x07\x11\x03"
		"\xE0\x20\x07\x11\x03\xE0\x20\x07\x11\x03"
		"\xE0\x20\x07\x11\x03\xE0\x20\x07\x11\x03"
		"\xE0\x78\x0B\x0F\x11\x07\xC0\x20\x0F\x11"
		"\x07\xC0\x20\x0F\x11\x07\xC0\x20\x0F\x11"
		"\x07\xC0\x20\x0F\x11\x07\xC0"
	"0w67v"
		"\x78\x48\xFF\x09\x07\xC0\x28\xFF\x09\x07"
		"\xC0\x28\xFF\x09\x07\xC0\x28\xFF\x09\x07"
		"\xC0\x28\xFF\x09\x07\xC0\x28\xFF\x09\x07"
		"\xC0\x28\xFF\x09\x07\xC0\x28\xFF\x09\x07"
		"\xC0\x78\x0C\xFE\x09\x0F\x80\x28\xFE\x09"
		"\x0F\x80\x28\xFE\x09\x0F\x80\x28\xFE\x09"
		"\x0F\x80\x28\xFE\x09\x0F\x80"
	"0w87v"
		"\x78\x47\x03\x0A\x80\x0F\x80\x20\x03\x0A"
		"\x80\x0F\x80\x20\x03\x0A\x80\x0F\x80\x20"
		"\x03\x0A\x80\x0F\x80\x20\x03\x0A\x80\x0F"
		"\x80\x20\x03\x0A\x80\x0F\x80\x20\x03\x0A"
		"\x80\x0F\x80\x20\x03\x0A\x80\x0F\x80\x79"
		"\x0B\x07\xFF\x08\x1F\x83\x00\x01\x07\xFF"
		"\x08\x1F\x83\x00\x01\x07\xFF\x08\x1F\x83"
		"\x00\x01\x07\xFF\x08\x1F\x83\x00\x01\x07"
		"\xFF\x08\x1F\x9F\xFF\x70\x00"
	"0w61v"
		"\x78\x47\x01\xE3\x01\x01\x01\x01\xE3\x00"
		"\x00\x00\x01\xE3\x00\x00\x00\x01\xE3\x00"
		"\x00\x00\x01\xE3\x00\x00\x00\x01\xE3\x00"
		"\x00\x00\x01\xE3\x00\x00\x00\x01\xF9\x00"
		"\x00\x00\x03\x10\x1E\x28\x03\x10\x1E\x28"
		"\x03\x10\x1E\x28\x03\x10\x1E\x28\x03\x10"
		"\x1E"

	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"
	"0w0v0w0v0w0v0w0v"


	"0w41v"
		"\x79\x49\xC0\x1F\x39\xC0\x1F\x39\xC0\x1F"
		"\x39\xC0\x1F\x39\xC0\x1F\x39\xC0\x1F\x39"
		"\xC0\x1F\x39\xC0\x1F\x79\x0E\x80\x3E\x39"
		"\x80\x3E\x39\x80\x3E\x39\x80\x3E\x39\x80"
		"\x3E"
	"0w67v"
		"\x78\x47\x00\x09\xE0\x3E\x28\x00\x09\xE0"
		"\x3E\x28\x00\x09\xE0\x3E\x28\x00\x09\xE0"
		"\x3E\x28\x00\x09\xE0\x3E\x28\x00\x09\xE0"
		"\x3E\x28\x00\x09\xE0\x3E\x28\x00\x09\xE0"
		"\x3E\x78\x0C\x01\x09\xC0\x7C\x28\x01\x09"
		"\xC0\x7C\x28\x01\x09\xC0\x7C\x28\x01\x09"
		"\xC0\x7C\x28\x01\x09\xC0\x7C"
	"0w54v"
		"\x78\x48\x7F\x08\x7C\x30\x7F\x08\x7C\x30"
		"\x7F\x08\x7C\x30\x7F\x08\x7C\x30\x7F\x08"
		"\x7C\x30\x7F\x08\x7C\x30\x7F\x08\x7C\x30"
		"\x7F\x08\x7C\x78\x0C\x00\x10\xF8\x28\x00"
		"\x10\xF8\x28\x00\x10\xF8\x28\x00\x10\xF8"
		"\x28\x00\x10\xF8"
	"0w41v"
		"\x79\x49\xF0\xF8\x39\xF0\xF8\x39\xF0\xF8"
		"\x39\xF0\xF8\x39\xF0\xF8\x39\xF0\xF8\x39"
		"\xF0\xF8\x39\xF0\xF8\x79\x0E\xE1\xF0\x39"
		"\xE1\xF0\x39\xE1\xF0\x39\xE1\xF0\x39\xE1"
		"\xF0"
	"0w54v"
		"\x7A\x48\x3F\xF8\xF0\x32\x3F\xF8\xF0\x32"
		"\x3F\xF8\xF0\x32\x3F\xF8\xF0\x32\x3F\xF8"
		"\xF0\x32\x3F\xF8\xF0\x32\x3F\xF8\xF0\x32"
		"\x3F\xF8\xF0\x7A\x0D\x7F\xF1\xE0\x32\x7F"
		"\xF1\xE0\x32\x7F\xF1\xE0\x32\x7F\xF1\xE0"
		"\x32\x7F\xF1\xE0"
	"0w41v"
		"\x79\x48\x1F\xFD\x39\x1F\xFD\x39\x1F\xFD"
		"\x39\x1F\xFD\x39\x1F\xFD\x39\x1F\xFD\x39"
		"\x1F\xFD\x39\x1F\xFD\x79\x0E\x3F\xFB\x39"
		"\x3F\xFB\x39\x3F\xFB\x39\x3F\xFB\x39\x3F"
		"\xFB"
	"0w54v"
		"\x7A\x48\x0F\xFF\xE0\x32\x0F\xFF\xE0\x32"
		"\x0F\xFF\xE0\x32\x0F\xFF\xE0\x32\x0F\xFF"
		"\xE0\x32\x0F\xFF\xE0\x32\x0F\xFF\xE0\x32"
		"\x0F\xFF\xE0\x7A\x0D\x1F\xFF\xC0\x32\x1F"
		"\xFF\xC0\x32\x1F\xFF\xC0\x32\x1F\xFF\xC0"
		"\x32\x1F\xFF\xC0"
	"0w28v"
		"\x78\x4A\xC0\x40\xC0\x40\xC0\x40\xC0\x40"
		"\xC0\x40\xC0\x40\xC0\x40\xC0\x78\x0F\x80"
		"\x40\x80\x40\x80\x40\x80\x40\x80"
	"0w56v"
		"\x78\x48\x07\x08\x80\x30\x07\x08\x80\x30"
		"\x07\x08\x80\x30\x07\x08\x80\x30\x07\x08"
		"\x80\x30\x07\x08\x80\x30\x07\x08\x80\x30"
		"\x07\x08\x80\x78\x0D\x0F\xA5\x00\x00\x0F"
		"\xA5\x00\x00\x0F\xA5\x00\x00\x0F\xA5\x00"
		"\x00\x0F\xBF\xFF\x71\x00"
	"0w28v"
		"\x78\x48\x03\x40\x03\x40\x03\x40\x03\x40"
		"\x03\x40\x03\x40\x03\x40\x03\x78\x0F\x07"
		"\x40\x07\x40\x07\x40\x07\x40\x07"
	"0w0v"
	"0w54v"
		"\x78\x48\x01\x08\xC0\x30\x01\x08\xC0\x30"
		"\x01\x08\xC0\x30\x01\x08\xC0\x30\x01\x08"
		"\xC0\x30\x01\x08\xC0\x30\x01\x08\xC0\x30"
		"\x01\x08\xC0\x78\x0D\x03\x08\x80\x30\x03"
		"\x08\x80\x30\x03\x08\x80\x30\x03\x08\x80"
		"\x30\x03\x08\x80"
	"0w28v"
		"\x78\x48\x00\x40\x00\x40\x00\x40\x00\x40"
		"\x00\x40\x00\x40\x00\x40\x00\x78\x0F\x01"
		"\x40\x01\x40\x01\x40\x01\x40\x01"
	"0w28v"
		"\x78\x4A\xE0\x40\xE0\x40\xE0\x40\xE0\x40"
		"\xE0\x40\xE0\x40\xE0\x40\xE0\x78\x0F\xC0"
		"\x40\xC0\x40\xC0\x40\xC0\x40\xC0"
	"0w28v"
		"\x78\x4A\xF0\x40\xF0\x40\xF0\x40\xF0\x40"
		"\xF0\x40\xF0\x40\xF0\x40\xF0\x78\x0F\xE0"
		"\x40\xE0\x40\xE0\x40\xE0\x40\xE0"
	"0w28v"
		"\x78\x48\x01\x40\x01\x40\x01\x40\x01\x40"
		"\x01\x40\x01\x40\x01\x40\x01\x78\x0F\x03"
		"\x40\x03\x40\x03\x40\x03\x40\x03"
	"0w54v"
		"\x78\x48\x03\x08\xF8\x30\x03\x08\xF8\x30"
		"\x03\x08\xF8\x30\x03\x08\xF8\x30\x03\x08"
		"\xF8\x30\x03\x08\xF8\x30\x03\x08\xF8\x30"
		"\x03\x08\xF8\x78\x0D\x07\x08\xF0\x30\x07"
		"\x08\xF0\x30\x07\x08\xF0\x30\x07\x08\xF0"
		"\x30\x07\x08\xF0"
	"0w54v"
		"\x7A\x48\x07\xDF\xFC\x32\x07\xDF\xFC\x32"
		"\x07\xDF\xFC\x32\x07\xDF\xFC\x32\x07\xDF"
		"\xFC\x32\x07\xDF\xFC\x32\x07\xDF\xFC\x32"
		"\x07\xDF\xFC\x7A\x0D\x0F\xBF\xF8\x32\x0F"
		"\xBF\xF8\x32\x0F\xBF\xF8\x32\x0F\xBF\xF8"
		"\x32\x0F\xBF\xF8"
	"0w41v"
		"\x79\x49\x8F\xFE\x39\x8F\xFE\x39\x8F\xFE"
		"\x39\x8F\xFE\x39\x8F\xFE\x39\x8F\xFE\x39"
		"\x8F\xFE\x39\x8F\xFE\x79\x0E\x1F\xFC\x39"
		"\x1F\xFC\x39\x1F\xFC\x39\x1F\xFC\x39\x1F"
		"\xFC"
	"0w41v"
		"\x79\x48\x0F\x87\x39\x0F\x87\x39\x0F\x87"
		"\x39\x0F\x87\x39\x0F\x87\x39\x0F\x87\x39"
		"\x0F\x87\x39\x0F\x87\x79\x0E\x1F\x0F\x39"
		"\x1F\x0F\x39\x1F\x0F\x39\x1F\x0F\x39\x1F"
		"\x0F"
	"0w54v"
		"\x7A\x48\x1F\x07\xFF\x32\x1F\x07\xFF\x32"
		"\x1F\x07\xFF\x32\x1F\x07\xFF\x32\x1F\x07"
		"\xFF\x32\x1F\x07\xFF\x32\x1F\x07\xFF\x32"
		"\x1F\x07\xFF\x78\x0D\x3E\x08\xFE\x30\x3E"
		"\x08\xFE\x30\x3E\x08\xFE\x30\x3E\x08\xFE"
		"\x30\x3E\x08\xFE"
	"0w62v"
		"\x79\x48\x3E\x03\x08\x80\x29\x3E\x03\x08"
		"\x80\x29\x3E\x03\x08\x80\x29\x3E\x03\x08"
		"\x80\x29\x3E\x03\x08\x80\x29\x3E\x03\x08"
		"\x80\x29\x3E\x03\x08\x80\x29\x3E\x03\x08"
		"\x80\x7A\x0C\x7C\x07\xFF\x32\x7C\x07\xFF"
		"\x32\x7C\x07\xFF\x32\x7C\x07\xFF\x32\x7C"
		"\x07\xFF"
	"0w41v"
		"\x79\x48\x7C\x01\x39\x7C\x01\x39\x7C\x01"
		"\x39\x7C\x01\x39\x7C\x01\x39\x7C\x01\x39"
		"\x7C\x01\x39\x7C\x01\x79\x0E\xF8\x03\x39"
		"\xF8\x03\x39\xF8\x03\x39\xF8\x03\x39\xF8"
		"\x03"
	"0w54v"
		"\x78\x48\x78\x10\xC0\x28\x78\x10\xC0\x28"
		"\x78\x10\xC0\x28\x78\x10\xC0\x28\x78\x10"
		"\xC0\x28\x78\x10\xC0\x28\x78\x10\xC0\x28"
		"\x78\x10\xC0\x78\x0C\xF0\x10\x80\x28\xF0"
		"\x10\x80\x28\xF0\x10\x80\x28\xF0\x10\x80"
		"\x28\xF0\x10\x80"
	"0w72v"
		"\x79\x48\xF8\x00\x08\xE0\x29\xF8\x00\x08"
		"\xE0\x29\xF8\x00\x08\xE0\x29\xF8\x00\x08"
		"\xE0\x29\xF8\x00\x08\xE0\x29\xF8\x00\x08"
		"\xE0\x29\xF8\x00\x08\xE0\x29\xF8\x00\x08"
		"\xE0\x78\x0B\x01\x08\x01\x08\xC0\x20\x01"
		"\x08\x01\x08\xC0\x20\x01\x08\x01\x08\xC0"
		"\x20\x01\x08\x01\x08\xC0\x20\x01\x08\x01"
		"\x08\xC0"
	"0w62v"
		"\x79\x47\x01\xF0\x08\x7F\x29\x01\xF0\x08"
		"\x7F\x29\x01\xF0\x08\x7F\x29\x01\xF0\x08"
		"\x7F\x29\x01\xF0\x08\x7F\x29\x01\xF0\x08"
		"\x7F\x29\x01\xF0\x08\x7F\x29\x01\xF0\x08"
		"\x7F\x7A\x0C\x03\xE0\x00\x32\x03\xE0\x00"
		"\x32\x03\xE0\x00\x32\x03\xE0\x00\x32\x03"
		"\xE0\x00"
	"0w67v"
		"\x79\x47\x03\xE0\x10\xF0\x21\x03\xE0\x10"
		"\xF0\x21\x03\xE0\x10\xF0\x21\x03\xE0\x10"
		"\xF0\x21\x03\xE0\x10\xF0\x21\x03\xE0\x10"
		"\xF0\x21\x03\xE0\x10\xF0\x21\x03\xE0\x10"
		"\xF0\x79\x0B\x07\xC0\x10\xE0\x21\x07\xC0"
		"\x10\xE0\x21\x07\xC0\x10\xE0\x21\x07\xC0"
		"\x10\xE0\x21\x07\xC0\x10\xE0"
	"0w80v"
		"\x79\x47\x07\xC0\x09\x3F\xF8\x21\x07\xC0"
		"\x09\x3F\xF8\x21\x07\xC0\x09\x3F\xF8\x21"
		"\x07\xC0\x09\x3F\xF8\x21\x07\xC0\x09\x3F"
		"\xF8\x21\x07\xC0\x09\x3F\xF8\x21\x07\xC0"
		"\x09\x3F\xF8\x21\x07\xC0\x09\x3F\xF8\x79"
		"\x0B\x0F\x80\x09\x7F\xF0\x21\x0F\x80\x09"
		"\x7F\xF0\x21\x0F\x80\x09\x7F\xF0\x21\x0F"
		"\x80\x09\x7F\xF0\x21\x0F\x80\x09\x7F\xF0"
	"0w41v"
		"\x79\x4A\x1F\xFC\x39\x1F\xFC\x39\x1F\xFC"
		"\x39\x1F\xFC\x39\x1F\xFC\x39\x1F\xFC\x39"
		"\x1F\xFC\x39\x1F\xFC\x79\x0E\x3F\xF8\x39"
		"\x3F\xF8\x39\x3F\xF8\x39\x3F\xF8\x39\x3F"
		"\xF8"
	"0w72v"
		"\x79\x47\x0F\x80\x08\x0F\x29\x0F\x80\x08"
		"\x0F\x29\x0F\x80\x08\x0F\x29\x0F\x80\x08"
		"\x0F\x29\x0F\x80\x08\x0F\x29\x0F\x80\x08"
		"\x0F\x29\x0F\x80\x08\x0F\x29\x0F\x80\x08"
		"\x0F\x78\x0C\x1F\x80\x00\x00\x1F\x28\x1F"
		"\x80\x00\x00\x1F\x28\x1F\x80\x00\x00\x1F"
		"\x28\x1F\x80\x00\x00\x1F\x28\x1F\x80\x00"
		"\x00\x1F"
	"0w70v"
		"\x78\x47\x1F\x80\x00\x08\xFE\x20\x1F\x80"
		"\x00\x08\xFE\x20\x1F\x80\x00\x08\xFE\x20"
		"\x1F\x80\x00\x08\xFE\x20\x1F\x80\x00\x08"
		"\xFE\x20\x1F\x80\x00\x08\xFE\x20\x1F\x80"
		"\x00\x08\xFE\x20\x1F\x80\x00\x08\xFE\x78"
		"\x0B\x3E\x18\xFC\x20\x3E\x18\xFC\x20\x3E"
		"\x18\xFC\x20\x3E\x18\xFC\x20\x3E\x18\xFC"
	"0w67v"
		"\x78\x47\x3E\x11\x07\xFF\x20\x3E\x11\x07"
		"\xFF\x20\x3E\x11\x07\xFF\x20\x3E\x11\x07"
		"\xFF\x20\x3E\x11\x07\xFF\x20\x3E\x11\x07"
		"\xFF\x20\x3E\x11\x07\xFF\x20\x3E\x11\x07"
		"\xFF\x78\x0B\x7C\x11\x0F\xFE\x20\x7C\x11"
		"\x0F\xFE\x20\x7C\x11\x0F\xFE\x20\x7C\x11"
		"\x0F\xFE\x20\x7C\x11\x0F\xFE"
	"0w54v"
		"\x78\x47\x3C\x10\x03\x28\x3C\x10\x03\x28"
		"\x3C\x10\x03\x28\x3C\x10\x03\x28\x3C\x10"
		"\x03\x28\x3C\x10\x03\x28\x3C\x10\x03\x28"
		"\x3C\x10\x03\x78\x0C\x78\x10\x07\x28\x78"
		"\x10\x07\x28\x78\x10\x07\x28\x78\x10\x07"
		"\x28\x78\x10\x07"
	"0w54v"
		"\x78\x47\x7C\x20\x80\x18\x7C\x20\x80\x18"
		"\x7C\x20\x80\x18\x7C\x20\x80\x18\x7C\x20"
		"\x80\x18\x7C\x20\x80\x18\x7C\x20\x80\x18"
		"\x7C\x20\x80\x78\x0A\xF8\x18\xFF\x20\xF8"
		"\x18\xFF\x20\xF8\x18\xFF\x20\xF8\x18\xFF"
		"\x20\xF8\x18\xFF"
	"0w85v"
		"\x78\x47\xF8\x10\x01\x08\xC0\x18\xF8\x10"
		"\x01\x08\xC0\x18\xF8\x10\x01\x08\xC0\x18"
		"\xF8\x10\x01\x08\xC0\x18\xF8\x10\x01\x08"
		"\xC0\x18\xF8\x10\x01\x08\xC0\x18\xF8\x10"
		"\x01\x08\xC0\x18\xF8\x10\x01\x08\xC0\x79"
		"\x09\x01\xF0\x10\x03\x08\x80\x11\x01\xF0"
		"\x10\x03\x08\x80\x11\x01\xF0\x10\x03\x08"
		"\x80\x11\x01\xF0\x10\x03\x08\x80\x11\x01"
		"\xF0\x10\x03\x08\x80"
	"0w93v"
		"\x79\x46\x01\xF0\x10\x00\x08\xE0\x11\x01"
		"\xF0\x10\x00\x08\xE0\x11\x01\xF0\x10\x00"
		"\x08\xE0\x11\x01\xF0\x10\x00\x08\xE0\x11"
		"\x01\xF0\x10\x00\x08\xE0\x11\x01\xF0\x10"
		"\x00\x08\xE0\x11\x01\xF0\x10\x00\x08\xE0"
		"\x11\x01\xF0\x10\x00\x08\xE0\x79\x09\x03"
		"\xE0\x10\x01\x08\xC0\x11\x03\xE0\x10\x01"
		"\x08\xC0\x11\x03\xE0\x10\x01\x08\xC0\x11"
		"\x03\xE0\x10\x01\x08\xC0\x11\x03\xE0\x10"
		"\x01\x08\xC0"
	"0w41v"
		"\x79\x46\x03\xE0\x39\x03\xE0\x39\x03\xE0"
		"\x39\x03\xE0\x39\x03\xE0\x39\x03\xE0\x39"
		"\x03\xE0\x39\x03\xE0\x79\x0E\x07\xC0\x39"
		"\x07\xC0\x39\x07\xC0\x39\x07\xC0\x39\x07"
		"\xC0"
	"0w72v"
		"\x78\x46\x07\x21\x7F\xF0\x10\x07\x21\x7F"
		"\xF0\x10\x07\x21\x7F\xF0\x10\x07\x21\x7F"
		"\xF0\x10\x07\x21\x7F\xF0\x10\x07\x21\x7F"
		"\xF0\x10\x07\x21\x7F\xF0\x10\x07\x21\x7F"
		"\xF0\x78\x09\x0F\x18\x00\x08\xE0\x10\x0F"
		"\x18\x00\x08\xE0\x10\x0F\x18\x00\x08\xE0"
		"\x10\x0F\x18\x00\x08\xE0\x10\x0F\x18\x00"
		"\x08\xE0"
	"0w67v"
		"\x79\x46\x0F\xC0\x20\xF8\x11\x0F\xC0\x20"
		"\xF8\x11\x0F\xC0\x20\xF8\x11\x0F\xC0\x20"
		"\xF8\x11\x0F\xC0\x20\xF8\x11\x0F\xC0\x20"
		"\xF8\x11\x0F\xC0\x20\xF8\x11\x0F\xC0\x20"
		"\xF8\x79\x09\x1F\x80\x20\xF0\x11\x1F\x80"
		"\x20\xF0\x11\x1F\x80\x20\xF0\x11\x1F\x80"
		"\x20\xF0\x11\x1F\x80\x20\xF0"
	"0w67v"
		"\x78\x46\x1F\x21\x3F\xFC\x10\x1F\x21\x3F"
		"\xFC\x10\x1F\x21\x3F\xFC\x10\x1F\x21\x3F"
		"\xFC\x10\x1F\x21\x3F\xFC\x10\x1F\x21\x3F"
		"\xFC\x10\x1F\x21\x3F\xFC\x10\x1F\x21\x3F"
		"\xFC\x78\x09\x3F\x21\x7F\xF8\x10\x3F\x21"
		"\x7F\xF8\x10\x3F\x21\x7F\xF8\x10\x3F\x21"
		"\x7F\xF8\x10\x3F\x21\x7F\xF8"
	"0w54v"
		"\x78\x46\x3F\x28\xFE\x10\x3F\x28\xFE\x10"
		"\x3F\x28\xFE\x10\x3F\x28\xFE\x10\x3F\x28"
		"\xFE\x10\x3F\x28\xFE\x10\x3F\x28\xFE\x10"
		"\x3F\x28\xFE\x78\x09\x7F\x28\xFC\x10\x7F"
		"\x28\xFC\x10\x7F\x28\xFC\x10\x7F\x28\xFC"
		"\x10\x7F\x28\xFC"
	"0w72v"
		"\x78\x46\xFF\x21\x7F\xFF\x10\xFF\x21\x7F"
		"\xFF\x10\xFF\x21\x7F\xFF\x10\xFF\x21\x7F"
		"\xFF\x10\xFF\x21\x7F\xFF\x10\xFF\x21\x7F"
		"\xFF\x10\xFF\x21\x7F\xFF\x10\xFF\x21\x7F"
		"\xFF\x79\x08\x01\xFF\x21\xFF\xFE\x09\x01"
		"\xFF\x21\xFF\xFE\x09\x01\xFF\x21\xFF\xFE"
		"\x09\x01\xFF\x21\xFF\xFE\x09\x01\xFF\x21"
		"\xFF\xFE"
	"0w73v"
		"\x78\x45\x03\x08\xE0\x29\x80\x03\x08\xE0"
		"\x29\x80\x03\x08\xE0\x29\x80\x03\x08\xE0"
		"\x29\x80\x03\x08\xE0\x29\x80\x03\x08\xE0"
		"\x29\x80\x03\x08\xE0\x29\x80\x03\x08\xE0"
		"\x28\x80\x78\x07\x07\x08\xC0\x20\xFF\x08"
		"\x07\x08\xC0\x20\xFF\x08\x07\x08\xC0\x20"
		"\xFF\x08\x07\x08\xC0\x20\xFF\x08\x07\x08"
		"\xC0\x20\xFF"
	"0w111v"
		"\x78\x45\x0F\x08\xF0\x10\x01\x80\xFF\x01"
		"\xF0\x0F\x08\xF0\x10\x01\x80\xFF\x01\xF0"
		"\x0F\x08\xF0\x10\x01\x80\xFF\x01\xF0\x0F"
		"\x08\xF0\x10\x01\x80\xFF\x01\xF0\x0F\x08"
		"\xF0\x10\x01\x80\xFF\x01\xF0\x0F\x08\xF0"
		"\x10\x01\x80\xFF\x01\xF0\x0F\x08\xF0\x10"
		"\x01\x80\xFF\x01\xF0\x0F\x08\xF0\x10\x01"
		"\x80\xFF\x00\xF0\x78\x07\x1F\x08\xE0\x10"
		"\x03\x11\xE0\x1F\x08\xE0\x10\x03\x11\xE0"
		"\x1F\x08\xE0\x10\x03\x11\xE0\x1F\x08\xE0"
		"\x10\x03\x11\xE0\x1F\x08\xE0\x10\x03\x10"
		"\xE0"
	"0w99v"
		"\x78\x45\x7F\x08\xFF\x10\x1F\x11\xFE\x7F"
		"\x08\xFF\x10\x1F\x11\xFE\x7F\x08\xFF\x10"
		"\x1F\x11\xFE\x7F\x08\xFF\x10\x1F\x11\xFE"
		"\x7F\x08\xFF\x10\x1F\x11\xFE\x7F\x08\xFF"
		"\x10\x1F\x11\xFE\x7F\x08\xFF\x10\x1F\x11"
		"\xFE\x7F\x08\xFF\x10\x1F\x10\xFE\xE0\x13"
		"\xFF\x00\xFE\x10\x3F\x10\xFC\x80\xFF\x00"
		"\xFE\x10\x3F\x10\xFC\x80\xFF\x00\xFE\x10"
		"\x3F\x10\xFC\x80\xFF\x00\xFE\x10\x3F\x10"
		"\xFC\x80\xFF\x00\xFE\x10\x3F\x10\xFC\x"

*/

	"249y9v"
///		"\x78\x49\x08\x79\x01\x7F\xF0\x08\x18"
		"\x78\x49\x08\x79\x01\x7F\xF0\x08\x18"
	"0w15v"
		"\x78\x49\x1C\x20\x3F\x81\xFF\x00\xFE\x30"
		"\x07\x80\xFF\x08\x38"
	"0w8v"
		"\x79\x52\xFF\xF0\x28\x3F\x10\xE0"
	"0w13v"
		"\x78\x49\x3C\x20\x01\x20\xFC\x2C\xFF\xE0"
		"\x0F\xF8\x78"
	"0w17v"
		"\x78\x49\x3E\x25\x00\x7F\xFE\x00\x3F\xFF"
		"\x20\x03\xA0\x00\x01\xFF\xF8"
	"0w15v"
		"\x78\x49\x7E\x29\x3F\xF0\x08\x07\x08\xC0"
		"\x19\x07\xFC\x10\x3F"
	"0w13v"
		"\x78\x49\x7F\x40\x01\x08\xE0\x19\x1F\xF0"
		"\x11\x1F\xFC"
	"0w12v"
		"\x78\x4F\x1F\x12\x00\x7F\xF0\x19\x3F\xE0"
		"\x10\x0F"
	"0w11v"
		"\x78\x49\xFF\x49\x3F\xF8\x19\x7F\x80\x10"
		"\x03"
	"0w10v"
		"\x78\x4A\x80\x41\x1F\xFC\x18\xFF\x81\x00"
	"0w10v"
		"\x78\x48\x01\x51\x0F\xFE\x10\x01\x20\x01"
	"0w8v"
		"\x78\x4A\xC0\x61\x03\xFE\x18\x00"
	"0w11v"
		"\x78\x48\x03\x51\x07\xFF\x11\x07\xFC\x20"
		"\x7C"
	"0w3v"
		"\x78\x4A\xE0"
	"0w8v"
		"\x78\x48\x07\x71\x0F\xF8\x20\x3C"
	"0w9v"
		"\x78\x49\xBF\x48\x03\x08\x80\x08\x1F"
	"0w7v"
		"\x78\x4A\xF0\x68\xF0\x20\x1C"
	"0w8v"
		"\x79\x48\x0F\x1F\x68\x3F\x28\x1E"
	"0w7v"
		"\x78\x4A\xF8\x68\xE0\x20\x0E"
	"0w5v"
		"\x78\x48\x1E\x70\x7F"
	"0w3v"
		"\x78\x49\x0F"
	"0w9v"
		"\x78\x48\x3E\x08\xFC\x60\xFF\x28\x06"
	"0w6v"
		"\x79\x48\x3C\x07\x70\xC0"
	"0w14v"
		"\x78\x48\x7C\x08\xFE\x40\x07\xA0\x00\xFF"
		"\x03\xFF\xE0\x00"
	"0w6v"
		"\x79\x48\x78\x03\x60\x01"
	"0w3v"
		"\x78\x4A\xFF"
	"0w6v"
		"\x78\x48\xF0\x51\x0F\xFE"
	"0w7v"
		"\x78\x49\x01\x50\xFC\x18\x80"
	"0w8v"
		"\x79\x47\x01\xE0\x10\x80\x38\x1F"
	"0w8v"
		"\x78\x49\x00\x49\x3F\xF8\x08\x03"
	"0w8v"
		"\x78\x47\x03\x18\xC0\x39\xFF\xF0"
	"0w9v"
		"\x78\x48\xC0\x38\xF8\x08\x0F\x08\xC0"
	"0w10v"
		"\x78\x47\x07\x10\x7F\xE2\x02\xFF\x00\x80"
	"0w9v"
		"\x78\x48\x80\x10\xE0\x38\xFC\x80\x00"
	"0w8v"
		"\x78\x47\x0F\x10\x3F\x41\xFF\x80"
	"0w11v"
		"\xE0\x54\x00\x08\xF0\x22\xF0\x00\x1F\x08"
		"\xF0"
	"0w5v"
		"\x78\x52\x01\x08\xFC"
	"0w10v"
		"\x78\x47\x1E\x11\x1F\xF8\x32\x00\x7F\xFE"
	"0w4v"
		"\x79\x53\x1F\xFF"
	"0w9v"
		"\x78\x47\x3E\x10\x0F\x40\x07\x08\x80"
	"0w9v"
		"\x78\x47\x3C\x18\xFC\x38\x03\x08\xC0"
	"0w9v"
		"\x78\x47\x7C\x10\x07\x40\x01\x08\xE0"
	"0w7v"
		"\x78\x47\x78\x18\xFE\x50\x01"
	"0w9v"
		"\xE2\x53\xFF\x40\x00\x08\xF0\x10\xC0"
	"0w3v"
		"\x78\x54\x7F"
	"0w3v"
		"\x78\x4B\xFF"
	"0w11v"
		"\x79\x46\x01\xE0\x80\x00\x00\x01\x51\xF8"
		"\x00"
	"0w7v"
		"\x78\x4C\x80\x38\x3F\x18\xE0"
	"0w4v"
		"\x79\x46\x03\xC0"
	"0w7v"
		"\x78\x4A\x00\x08\xC0\x50\x7F"
	"0w6v"
		"\x78\x46\x07\x78\x02\xF0"
	"0w5v"
		"\x78\x47\x80\x18\x7F"
	"0w9v"
		"\x78\x46\x0F\x28\xE0\x50\x3F\x28\x01"
	"0w11v"
		"\xE2\x53\x00\x00\x3F\x59\x1F\xF8\x21\x03"
		"\x80"
	"0w13v"
		"\x78\x4C\xF0\x39\x7F\xF0\x10\xFC\xFF\x02"
		"\xFF\xDF\x00"
	"0w8v"
		"\x78\x46\x1E\x78\x01\x0F\x28\x06"
	"0w9v"
		"\x78\x4B\x1F\x48\xE0\x10\xFE\x20\x0E"
	"0w12v"
		"\x78\x46\x3C\x28\xF8\x38\xFF\x11\x07\xFF"
		"\x20\x1C"
	"0w11v"
		"\x78\x53\x01\x08\xC0\x08\x03\x08\x80\x18"
		"\x38"
	"0w12v"
		"\x78\x46\x7C\x21\x0F\xFC\x50\x01\x08\xC0"
		"\x18\x70"
	"0w13v"
		"\x78\x4C\xFE\x30\x03\x08\x80\x08\x00\x08"
		"\xE0\x18\xE0"
	"0w13v"
		"\x78\x46\xF8\x60\x0F\xA1\x00\x01\x7F\xF8"
		"\x11\x03\xC0"
	"0w16v"
		"\x78\x45\x01\x30\xFF\x10\x3F\x19\x1F\xFE"
		"\x19\x3F\xFC\x11\x07\x80"
	"0w18v"
		"\x78\x46\xFC\x30\x80\x29\x7F\xFC\x1A\x1F"
		"\xFF\x80\x08\x3F\x9F\xFF\xE0\x00"
	"0w21v"
		"\x79\x45\x07\xFE\x20\x1F\x08\xC0\x0D\x7F"
		"\xFF\x80\x07\xFF\xF0\x18\x07\x0A\xF0\x01"
		"\xFE"
	"0w20v"
		"\x7A\x45\x1F\xFF\x80\x18\x7F\x09\xF0\x01"
		"\x83\xFF\x00\xC0\x18\x01\xA0\xFF\x00\xF8"
	"0w19v"
		"\x78\x45\x7F\x08\xF8\x10\x07\x80\xFF\x01"
		"\xFE\x3F\xE3\x02\x00\x00\x7F\x10\xE0"
	"0w9v"
		"\x78\x53\xC0\x28\x1F\xDF\xFF\xE1\x00"
	"0w5v"
		"\xF3\x51\x00\x08\xF0"

	"0w8v"
		"\x04\x0F\x0F\x0F\x0F\x0F\x08\x33"
	"0w0v"
	"0w0v"
	"0w0v"
	"0w0v"
	"0w0v"
	"0w0v"
	"0w0v"


///		"\xE0\x54\x00\xFF\xF0\xFF\xF0\x00\x1F\x08"
///		"\xF0"

// Second line

	"0w46y10v"
		"\x78\x49\x08\x7A\x13\x01\xFF\xC0\x08\x60"
	"0w16v"
		"\x78\x49\x1C\x68\x07\x82\xFF\x00\xC0\x78"
		"\x00\x1F\x08\xFC\x08\xE0"
	"0w8v"
		"\x78\x5C\xFE\xE1\x0C\xFF\x00\x80"
	"0w15v"
		"\x78\x49\x3C\x69\x00\x3F\x19\xFF\x80\x68"
		"\x03\x0A\x80\x3F\xE1"
	"0w16v"
		"\x78\x49\x3E\x70\x0F\x09\xC0\x07\x08\xE0"
		"\x6C\x0F\xFC\x00\x03\xFF"
	"0w15v"
		"\x78\x49\x7E\x71\x07\xFE\x80\x00\x08\xF8"
		"\x69\x1F\xF0\x08\x00"
	"0w13v"
		"\x78\x49\x7F\x79\x03\x3F\xFC\x69\x7F\xC0"
		"\x11\x7F\xF0"
	"0w11v"
		"\x78\x58\x03\x19\x0F\xFE\x69\xFF\x80\x10"
		"\x3F"
	"0w14v"
		"\x78\x49\xFF\x79\x03\x07\xFF\x61\x01\xFE"
		"\x81\x00\x00\x0F"
	"0w11v"
		"\x78\x4A\x80\x78\x02\x03\x08\x80\x59\x03"
		"\xFC"
	"0w12v"
		"\x78\x48\x01\x78\x04\x01\x08\xC0\x58\x07"
		"\x20\x07"
	"0w9v"
		"\x78\x4A\xC0\x79\x10\x0F\xF8\x18\x03"
	"0w13v"
		"\x78\x48\x03\x78\x04\x00\x08\xE0\x59\x1F"
		"\xF0\x18\x01"
	"0w3v"
		"\x78\x4A\xE0"
	"0w9v"
		"\x78\x48\x07\x79\x12\x3F\xE0\x18\x00"
	"0w9v"
		"\x78\x49\xBF\x79\x04\x7F\xF0\x58\x7F"
	"0w8v"
		"\x78\x4A\xF0\x78\x11\xC0\x20\x70"
	"0w9v"
		"\x79\x48\x0F\x1F\x78\x11\xFF\x28\x78"
	"0w8v"
		"\x78\x4A\xF8\x78\x11\x80\x20\x38"
	"0w6v"
		"\x78\x48\x1E\x78\x11\x01"
	"0w3v"
		"\x78\x49\x0F"
	"0w10v"
		"\x78\x48\x3E\x08\xFC\x78\x0F\x03\x30\x18"
	"0w7v"
		"\x79\x48\x3C\x07\xE3\x1E\x00"
	"0w14v"
		"\x78\x48\x7C\x08\xFE\x79\x03\xFF\xE0\xFF"
		"\x0E\xFF\xCD\x00"
	"0w7v"
		"\x79\x48\x78\x03\x78\x10\x07"
	"0w3v"
		"\x78\x4A\xFF"
	"0w8v"
		"\x78\x48\xF0\x78\x04\x01\x08\xC0"
	"0w8v"
		"\x78\x49\x01\x78\x05\x80\x58\xFE"
	"0w9v"
		"\x79\x47\x01\xE0\x10\x80\x78\x01\x03"
	"0w10v"
		"\x78\x49\x00\x78\x03\x07\xA9\x00\x00\x0F"
	"0w9v"
		"\x78\x47\x03\x18\xC0\x79\x01\x1F\xFE"
	"0w10v"
		"\x78\x48\xC0\x78\x01\xFF\x0A\x01\xFF\xF8"
	"0w10v"
		"\x78\x47\x07\x10\x7F\xE1\x0C\xFF\x00\xF0"
	"0w8v"
		"\x78\x48\x80\x10\xE0\x78\x02\x80"
	"0w8v"
		"\x78\x47\x0F\x10\x3F\x78\x03\xF0"
	"0w11v"
		"\xE0\x54\x00\x08\xF0\x6A\xFE\x00\x03\x08"
		"\xFE"
	"0w6v"
		"\x7B\x5B\x00\x3F\xFF\x80"
	"0w11v"
		"\x78\x47\x1E\x11\x1F\xF8\x78\x01\x0F\x08"
		"\xC0"
	"0w5v"
		"\x78\x5C\x03\x08\xE0"
	"0w10v"
		"\x78\x47\x3E\x10\x0F\x78\x02\x00\x08\xF0"
	"0w9v"
		"\x78\x47\x3C\x18\xFC\x79\x02\x7F\xF8"
	"0w9v"
		"\x78\x47\x7C\x10\x07\x79\x03\x3F\xFC"
	"0w8v"
		"\x78\x47\x78\x18\xFE\x78\x0E\x07"
	"0w9v"
		"\xE2\x53\xFF\x79\x03\x1F\xFE\x58\xFF"
	"0w3v"
		"\x78\x5D\x0F"
	"0w3v"
		"\x78\x4B\xFF"
	"0w13v"
		"\x79\x46\x01\xE0\x80\x00\x00\x01\x78\x04"
		"\xFF\x50\x03"
	"0w8v"
		"\x78\x4C\x80\x78\x01\x07\x68\x80"
	"0w4v"
		"\x79\x46\x03\xC0"
	"0w8v"
		"\x78\x4A\x00\x08\xC0\x78\x0D\x01"
	"0w6v"
		"\x78\x46\x07\x78\x15\xC0"
	"0w5v"
		"\x78\x47\x80\x18\x7F"
	"0w10v"
		"\x78\x46\x0F\x28\xE0\x78\x0D\x00\x30\x04"
	"0w11v"
		"\xE2\x53\x00\x00\x3F\x79\x0F\x7F\xE0\x20"
		"\x0E"
	"0w11v"
		"\x78\x4C\xF0\x79\x01\x0F\xFE\x60\xF0\x20"
		"\x0C"
	"0w8v"
		"\x78\x46\x1E\x78\x14\x3F\x28\x18"
	"0w10v"
		"\x78\x4B\x1F\x78\x03\xFC\x60\xF8\x20\x38"
	"0w13v"
		"\x78\x46\x3C\x28\xF8\x78\x01\x1F\x61\x1F"
		"\xFC\x20\x70"
	"0w9v"
		"\x79\x5D\x3F\xF8\x59\x0F\xFE\x20\xE0"
	"0w13v"
		"\x78\x46\x7C\x21\x0F\xFC\x79\x0E\x07\xFF"
		"\x19\x01\xC0"
	"0w14v"
		"\x78\x4C\xFE\x79\x01\x7F\xF0\x58\x03\x08"
		"\x80\x11\x03\x80"
	"0w18v"
		"\x78\x46\xF8\x7A\x06\x01\xFF\xE0\x58\x01"
		"\x08\xE0\x10\x0F\x9F\xFF\xCD\x00"
	"0w17v"
		"\x78\x45\x01\x30\xFF\x58\x07\x18\x03\x08"
		"\xC0\x58\x00\x08\xF0\x10\x1E"
	"0w14v"
		"\x78\x46\xFC\x30\x80\x70\x0F\x08\x80\x61"
		"\x7F\xFE\x10\xFC"
	"0w23v"
		"\x79\x45\x07\xFE\x20\x1F\x08\xC0\x52\x0F"
		"\xFF\xF0\x09\xFF\xFE\x8B\x00\x04\x1F\xFF"
		"\xC0\x07\xF8"
	"0w21v"
		"\x7A\x45\x1F\xFF\x80\x18\x7F\x08\xF0\x50"
		"\x3F\xA1\xFF\x00\xF8\x68\x07\xA0\xFF\x00"
		"\xE0"
	"0w21v"
		"\x78\x45\x7F\x08\xF8\x10\x07\x80\xFF\x00"
		"\xFE\x48\x07\x83\xFF\x00\xE0\x68\x01\x18"
		"\x80"
	"0w13v"
		"\x78\x5C\xF8\x8D\x00\x00\x7F\x08\xFC\x9F"
		"\xFF\xCE\x00"
	"0w8v"
		"\xFF\x51\x06\x00\x00\x03\x08\xC0"
	"0w0M"




	"0w0v0w0M"
	"\x1b*b0M"
	"\x1b*b0M"
	"\x1b*b0M"
	"\x1b*b0M"
	"\x1b*b0M"
	"\x1b*b0M"
	"\x1b*b0M";


long domonorow(ushort *rowpnt);
long ptrout(uchar *buffer, long len);



void main(void)

{
	bfr = (char *)((((long)buffer) + 0xFFF) & 0xFFFFF000);


	printf("Mem = 0x%08X, len = %d\n", bfr, sizeof(bgnpart));

	pnt = buffer + strlen(bgnpart);

	if ((ptr = svcIoOpen(XO_OUT, "PTR50:", NULL)) < 0)
		femsg2(prgname, "Error opening printer", ptr, NULL);

	if ((rtn = svcIoOutBlock(ptr, pnt, 1000)) < 0)
		femsg2(prgname, "Error writing to printer", rtn, NULL);

	memcpy(bfr , bgnpart, sizeof(bgnpart));
	pnt = bfr + sizeof(bgnpart);

///	pnt = putline(pnt, 200, 200);
///	pnt = putline(pnt, 200, 200);
///	pnt = putline(pnt, 200, 200);
///	pnt = putline(pnt, 200, 200);

	pnt = strmov(pnt, "\x1B*rC\x1B""E\x1B%-12345X@PJL EOJ\r\n\x1B""E");

	amnt = 100;
	while (--amnt >= 0)
		*pnt++ = 0;
	left = pnt - bfr;
	pnt = bfr;


	while (left > 0)
	{
		if ((amnt = left) > 1024)
			amnt = 1024;
		printf("### out = %d\n", amnt);
		if ((rtn = svcIoOutBlock(ptr, pnt, amnt)) < 0)
			femsg2(prgname, "Error writing to printer", rtn, NULL);
		pnt += amnt;
		left -= amnt;
	}


///	printf("### out = %d\n", left);
///	if ((rtn = svcIoOutBlock(ptr, pnt, left)) < 0)
///		femsg2(prgname, "Error writing to printer", rtn, NULL);

	svcIoClose(ptr, 0);
}


char *putline(
	char *pnt,
	int   x1,
	int   x2)

{
	pnt += sprintf(pnt, "\x1B*b0m0v%dW", x1 + x2);
	while (--x1 >= 0)
		*pnt++ = 0;
	while (--x2 >= 0)
		*pnt++ = -1;
	return (pnt);
}


#define ROWLEN (600*8/8)


uchar seedrow[ROWLEN];		// Seek row (1 bit/pixel)
uchar thisrow[ROWLEN];
uchar genrow[ROWLEN + (ROWLEN+254)/255 + 2];




long  width;				// Display width (pixels)


long   inx;
long   max;
long   bgn;
long   lenm1;
long   cnt;
long   offset;
long   bytecnt;
long   prv;
uchar *genpnt;
uchar *bytepnt;

int    pixcnt;

uchar  byteval;
uchar  pixbit;
uchar  seednotwhite;		// TRUE if seedrow does not contain all white (0)
							//   pixels


//***************************************************************
// Function: printpage - Print one page from an XWS memory region
// Returned:
//***************************************************************

long printpage(
	ushort *buffer,			// Pixel buffer (16-bits per pixel)
	long    bwidth,			// Page width in pixels
	long    bheight)			// Page height in poxels

{
	static char bgnpage[] = "\x1B*r0A\x1B*b0Y";
	static char endpage[] = "\x1B*rC\x0c";

	int  whitecnt;
	int  len;
	char cmdtxt[64];

	memset(seedrow, 0, sizeof(seedrow));
	width = bwidth;
	whitecnt = 0;
	if ((rtn = ptrout(bgnpage, sizeof(bgnpage))) < 0)
		return (rtn);
	while (--bheight >= 0)
	{
		if ((len = domonorow(buffer)) < 0)
			whitecnt++;
		else
		{
			if (whitecnt > 0)
			{
				rtn = sprintf(cmdtxt, "%dy", whitecnt);
				if ((rtn = ptrout(cmdtxt, rtn)) < 0)
					return (rtn);
				whitecnt = 0;
			}
			rtn = sprintf(cmdtxt, (bheight > 0) ? "%dw" : "%dW", len);
			if (len > 0)
			{
				if ((rtn = ptrout(cmdtxt, rtn)) < 0 ||
						(rtn = ptrout(genrow, len)) < 0)
					return (rtn);
			}
		}
		buffer += width;
	}
	if (whitecnt > 0)
	{
		if ((rtn = ptrout("0Y", 2)) < 0)
			return (rtn);
	}
	if ((rtn = ptrout(endpage, sizeof(endpage))) < 0)
		return (rtn);					// Leave raster mode, print the page
	return (0);
}


//*********************************************************
// Function: domonorow - Generate data for one pixel row
// Returned: Length of the generated output (0 if there are
//				no differences, -1 if new row is all white
//*********************************************************


long domonorow(
	ushort *rowpnt)

{
	genpnt = genrow;

	// First convert the input row to a 1-bit per pixel row (same format as
	//   the seed row buffer)

	bytepnt = thisrow;
	bytecnt = width/8;
	seednotwhite = FALSE;
	while (--bytecnt >= 0)
	{
		byteval = 0;					// Construct 1 byte of pixel values
		pixcnt = 0;
		pixbit = 0x80;
		do
		{
			if (*rowpnt++ == 0)
				byteval |= pixbit;
			pixbit >>= 1;
		} while (--pixcnt > 0);
		*bytepnt++ = byteval;
		seednotwhite |= byteval;
	}
	if (!seednotwhite)					// Is the entire new row white?
	{
		memset(seedrow, 0, sizeof(seedrow)); // Yes - this is easy!
		return (-1);
	}

	// Here with the new row constructed in thisrow. Now compare it to the
	//   seed row, generate any required output, and update the seed row.

	inx = 0;
	max = width/8;

	while (inx < max)
	{
		if (thisrow[inx] != seedrow[inx])
		{
			// Here with a difference between the new row and the seed row.
			//   Find the end of the difference. Note that we require two
			//   matching bytes to end the difference. The actual amount
			//   that gives maximum is not necessarily two but depends on
			//   the length of the next difference region. Its very complex
			//   to calulate this and it does not really make much difference
			//   so always using 2 is reasonable.

			bgn = ++inx;
			while (inx < (max - 1) && (thisrow[inx] != seedrow[inx] ||
					thisrow[inx+1] != seedrow[inx + 1]))
				inx++;

			lenm1 = inx - bgn - 1;

			// Note that this version does not use RLE encoding!

			// Generate output for this difference region

			offset = bgn - prv;

			// Store the command byte

			*genpnt++ = (((offset > 15) ? 15 : offset) << 3) +
					((lenm1 >= 7) ? 7 : lenm1);

			// Store any excess offset bytes

			if ((offset -= 15) >= 0)
			{
				while (offset >= 255)
				{
					*genpnt++ = 255;
					offset -= 255;
				}
				*genpnt++ = offset;
			}

			// Store any excess count bytes

			cnt = lenm1;
			if ((lenm1 -= 7) >= 0)
			{
				while (lenm1 >= 255)
				{
					*genpnt++ = 255;
					lenm1 -= 255;
				}
				*genpnt++ = lenm1;
			}

			// Store the data bytes and update seedrow

			do
			{
				byteval = thisrow[bgn];
				*genpnt++ = byteval;
				seedrow[bgn++] = byteval;
			} while (--cnt >= 0);
			prv = inx;					// Remember where this region ended


		}
		inx++;
	}
	return (genpnt - genrow);
}


//**********************************************************
// Function: ptrout - Output data to the printer
// Returned: 0 if normal or negative XOS error code if error
//**********************************************************

long ptrout(
	uchar *buffer,
	long   len)

{
	buffer = buffer;
	len = len;




	return (0);
}