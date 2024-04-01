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

#include "xosmkinst.h"

// XOS install set files (XIS) have a format identical to XID files except
//   that they contain only items that are to be copied to the target disk.
//   The IF_ISET and IF_COPY item header flags are nit ysed. Also there is no
//   IT_BOOT item. Each XIS file contains only one application or operating
//   system version. The base directory name and the application name are
//   specified by the name of the XIS file (see below) so there are no IT_DIR
//   items.

// The name of an XIS file always has the following format:
//		XOSINSx-base-name-mj-mn-ed.xis
//   Where:
//		xxx  = "a" if application or "o" if operating system.
//		base = Bendor name if application or base directory name (usually
//				 xossys) if operating system.
//		name = Application name if application or name of kernel file (usually
//				 xos) if operating system.
//      mj   = Major version number.
//		mn   = Minor version number
//		ed   = Edit number

// Version number in the name MUST match the version number of the main
//   executable file.

// The directory tree for the kernel is as follows:
//   xossys\				// Base directory
//     xos.run				// The kernel executable
//     sys\					// System files
//     cmd\					// Command files
//     etc.					// Other system directories
// The directory trees for an application is as follows:
//   xosapp\				// Application executables base (not changeable)
//     vendor\				// Base directory for application executables
//                               (vendor name)
//       name.run			// Main executable for application
//       etc.				// Any additional directly runable execuables
//							//   that are part of the application
//   xosapd\				// Application data base (not changeable)
//     vendor\				// Base durectory for application data (vendor
//							//   name)
//       name\				// Application data directory
//         etc.				// Any data file or subdirectories associated
//							//   with the application

//// 0x701CE1722770000 --> 1-1-1601


