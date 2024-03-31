// Define offsets in the display controller register area. This is a 512K
//   block of memory. It would be "nicer" to define a structure for this,
//   but the usage of the area is extremely sparse, so it seems easier to
//   simply decleare the area as an array of longs (virually all of the
//   register require 32-bit accesses) and use these values as an index
//   into the array.

// 0x00000 - 0x00FFF - VGA registers (these values are for the color
//   configuration)

#define VGA_ATRXW 0x3C0
#define VGA_ATRR  0x3C1
#define VGA_ST00  0x3C2
#define VGA_MSRW  0x3C2
#define VGA_SEQX  0x3C4
#define VGA_SEQD  0x3C5
#define VGA_MSRR  0x3CC
#define VGA_GCX   0x3CE
#define VGA_GCD   0x3CF
#define VGA_ST01  0x3DA

// 0x01000 - 0x04FFF - ???

#define MM_PGTBL      (0x02020/4)	// Physical address of the page table

// 0x05000 - 0x05FFF - GMBUS and I/O control

// 0x06000 - 0x06FFF - Display clocks and clock gating

#define MM_DPLLVGA0   (0x06000/4)
#define MM_DPLLVGA1   (0x06004/4)
#define MM_VGAPD      (0x06010/4)
#define MM_ADPLLCTRL  (0x06014/4)
#define MM_BDPLLCTRL  (0x06018/4)
#define MM_A0DPLLDIV  (0x06040/4)
#define MM_A1DPLLDIV  (0x06044/4)
#define MM_B0DPLLDIV  (0x06048/4)
#define MM_B1DPLLDIV  (0x0604C/4)

// 0x0A000 = 0x0AFFF - Display palette A/B registers

#define MM_APALETTE   (0x0A000/4)
#define MM_BPALETTE   (0x0A800/4)

// 0x30000 - 0x3FFFF - Overlay registers

// 0x60000 - 0x600FF - Display pipeline A

#define MM_AHTOTAL    (0x60000/4)
#define MM_AHBLANK    (0x60004/4)
#define MM_AHSYNC     (0x60008/4)
#define MM_AVTOTAL    (0x6000C/4)
#define MM_AVBLANK    (0x60010/4)
#define MM_AVSYNC     (0x60014/4)
#define MM_ASRCSIZE   (0x6001C/4)
#define MM_APBDRCOL   (0x60020/4)

// 0x61000 - 0x610FF - Display pipeline B

#define MM_BHTOTAL    (0x61000/4)
#define MM_BHBLANK    (0x61004/4)
#define MM_BHSYNC     (0x61008/4)
#define MM_BVTOTAL    (0x6100C/4)
#define MM_BVBLANK    (0x61010/4)
#define MM_BVSYNC     (0x61014/4)
#define MM_BSRCSIZE   (0x6101C/4)
#define MM_BPBDRCOL   (0x61020/4)

// 0x61100 - 0x611FF - Display port control registers

#define MM_APORTCTRL  (0x61100/4)	// Analog port control
#define MM_LPORTCTRL  (0x60080/4)	// LVDS port control

// 0x61200 - 0x612FF - Panel fitting/power sequencing/LVDS control

// 0x64000 - 0x64FFF - Display port registers

// 0x70000 - 0x7FFFF - Display pipeline, display planes, cursor planes, and
//						 VGA control registers

#define MM_APIPECFG   (0x70008/4)	// Pipe A configuration register

#define MM_ACURCTRL   (0x70080/4)	// Cursor A control
#define MM_ACURBASE   (0x70084/4)	// Cursor A base address
#define MM_ACURPOS    (0x70088/4)	// Cursor A position
#define MM_ACURPALET0 (0x70090/4}	// Cursor A palette 0
#define MM_ACURPALET1 (0x70094/4}	// Cursor A palette 1
#define MM_ACURPALET2 (0x70098/4}	// Cursor A palette 2
#define MM_ACURPALET3 (0x7009C/4}	// Cursor A palette 3

#define MM_BCURCTRL   (0x700C0/4)	// Cursor B control
#define MM_BCURBASE   (0x700C4/4)	// Cursor B base address
#define MM_BCURPOS    (0x700C8/4)	// Cursor B position
#define MM_BCURPALET0 (0x700D0/4}	// Cursor B palette 0
#define MM_BCURPALET1 (0x700D4/4}	// Cursor B palette 1
#define MM_BCURPALET2 (0x700D8/4}	// Cursor B palette 2
#define MM_BCURPALET3 (0x700DC/4}	// Cursor B palette 3

#define MM_ADSPCTRL   (0x70180/4)

#define MM_ADSPLINOFF (0x70184/4)
#define MM_ADSPSTRIDE (0x70188/4)
#define MM_ADSPBASE   (0x7019C/4)	// Base address for plane (Writing this
									//   triggers updates of all other plane
									//   registers!)

#define MM_BPIPECONF  (0x71008/4)	// Pipe B configuration register

#define MM_VGACTRL    (0x71400/4)	// VGA mode control
