//============================================
// cmdactive.c
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

// This command reports decimal two values: The first is the current number of
//   active root threads; The second is the total of root thread events.

// This command is intended to be used during startup to determine when USBCTL
//   is finished setting up existing USB devices. The intent is that it will
//   be polled every N milliseconds (probably on the order of 200) until it is
//   observed to be inactive (first value is 0) and unchanged for M periods
//   (probably on the order of 3 or 4). At this point it is reasonable to
//   assume that all available devices have been configured.

void cmdactive(
    char *cmd,
	int   len)
{
	(void)cmd;
	(void)len;

	sfwResponse(0, SFWRSP_SEND|SFWRSP_FINAL, "%d %d", numactive, ttlactive);
}
