// Define device classes

#define CLASS_AUDIO    1
#define CLASS_CDCCNTL  2
#define CLASS_HID      3
#define CLASS_PHYSICAL 5
#define CLASS_IMAGE    6
#define CLASS_PRINTER  7
#define CLASS_MASS     8
#define CLASS_HUB      9
#define CLASS_CDCDATA  10
#define CLASS_CHIPCRD  11
#define CLASS_CONTENT  13
#define CLASS_WIRELESS 224


#define SUBCLASS_COMM_DLCM   1	// Direct line control model
#define SUBCLASS_COMM_ACM    2	// Abstract control model
#define SUBCLASS_COMM_TCM    3	// telephone control model
#define SUBCLASS_COMM_MCCM   4	// Multi-channel control model
#define SUBCLASS_COMM_CAPICM 5	// CAPI control model
#define SUBCLASS_COMM_ENNCM  6	// Ethernet networking control model
#define SUBCLASS_COMM_ATMNCM 7	// ATM networking control model

#define SUBCLASS_HID_BOOT 1

#define PROTOCOL_HID_KEYBRD 1
#define PROTOCOL_HID_MOUSE  2

#define SUBCLASS_MASS_RBC    1
#define SUBCLASS_MASS_ATAPI  2
#define SUBCLASS_MASS_QIC157 3
#define SUBCLASS_MASS_UFI    4
#define SUBCLASS_MASS_SFF    5
#define SUBCLASS_MASS_SCSI   6


// Define setup request type values

#define SETUP_INPUT     0x80
#define SETUP_OUTPUT    0x00
#define SETUP_DEVICE    0x00
#define SETUP_INTERFACE 0x01
#define SETUP_ENDPOINT  0x02
#define SETUP_OTHER     0x03
#define SETUP_STANDARD  0x00
#define SETUP_CLASS     0x20
#define SETUP_VENDOR    0x40

// Define the setup request values

#define TYPE_GET_STATUS    0
#define TYPE_CLR_FEATURE   1
#define TYPE_SET_FEATURE   3
#define TYPE_SET_ADDRESS   5
#define TYPE_GET_DESC      6
#define TYPE_SET_DESC      7
#define TYPE_GET_CONFIG    8
#define TYPE_SET_CONFIG    9
#define TYPE_GET_INTERFACE 10
#define TYPE_SET_INTERFACE 11
#define TYPE_SYNCH_FRAME   12

// Define descriptor types

#define DESC_DEVICE    1
#define DESC_CONFIG    2
#define DESC_STRING    3
#define DESC_INTERFACE 4
#define DESC_ENDPOINT  5
#define DESC_DEVQUAL   6
#define DESC_OTHERSPD  7
#define DESC_IFPOWER   8
#define DESC_REPORT    33

// Define the device descriptor

typedef _Packed struct
{	uchar  length;				// Descriptor length
	uchar  desctype;			// Descriptor type (1)
	ushort usbspec;				// USB version (BCD)
	uchar  class;				// Class
	uchar  subclass;			// Sub-class
	uchar  protocol;			// Protocol
	uchar  maxpktsz0;			// Maximum packet size for end-point 0
	ushort vendorid;			// Vendor ID
	ushort deviceid;			// Device ID
	ushort devrel;				// Device release
	uchar  manufacturer;		// Index for manufacturer string
	uchar  product;				// Index for product string
	uchar  serialnum;			// Index for serial number string
	uchar  numcfgs;				// Number of configurations
} DEVDESC;

// Define the configuration descriptor

typedef _Packed struct
{	uchar  length;				// Descriptor length
	uchar  desctype;			// Descriptor type (2)
	ushort ttllen;				// Total length of all configuration descriptors
	uchar  numifs;				// Number of interfaces
	uchar  cfgvalue;			// Configuration selectin value
	uchar  descindex;			// Index for description string
	uchar  attrib;				// Attributes
	uchar  maxpower;			// Maximum power taken from bus
} CFGDESC;

// Define the interface descriptor

typedef _Packed struct
{	uchar  length;				// Descriptor length
	uchar  desctype;			// Descriptor type (4)
	uchar  number;				// Interface number
	uchar  altsel;				// Alternate selection value
	uchar  numept;				// Number of end-points
	uchar  class;				// Class
	uchar  subclass;			// Sub-class
	uchar  protocol;			// Protocol
	uchar  descindex;			// Index for description string
} IFDESC;

// Define the end-point descriptor

typedef _Packed struct
{	uchar  length;				// Descriptor length
	uchar  desctype;			// Descriptor type (5)
	uchar  number;				// End-point number
	uchar  attrib;				// Attributes
	ushort maxpktsz;			// Maximum packet size
	uchar  pktrate;				// Packet rate (interval)
} EPDESC;

// Define the HID class descriptor

typedef _Packed struct			// Additional descriptor list element
{	uchar  type;				// Descriptor type
	ushort length;				// Descriptor length
} HIDLIST;

typedef _Packed struct
{	uchar   length;				// Descriptor length
	uchar   desctype;			// Descriptor type (0x21)
	ushort  hidver;				// HID version
	uchar   country;			// Country code
	uchar   numdesc;			// Number of additional descriptors
	HIDLIST desclist[1];		// Additional descriptor list
} HIDDESC;

// Define the hub class descriptor

typedef _Packed struct
{	uchar  length;				// Descriptor length
	uchar  desctype;			// Descriptor type (0x29)
	uchar  numports;			// Number of ports
	ushort attrib;				// Hub characteristics
	uchar  on2good;				// Time from power on to power good on a port
	uchar  contrcurrent;		// Maximum current for hub controller
	uchar  data[16];			// Data (devicermv and portpwrmask variable
								//   length values)
} HUBDESC;

// Define USB hub port status bits

#define USB_PS_LSD 0x0200		// Low speed device attached
#define USB_PS_PPS 0x0100		// Port power status
#define USB_PS_PRS 0x0010		// Port reset status
#define USB_PS_OCI 0x0008		// Over current indication
#define USB_PS_PSS 0x0004		// Port suspended status
#define USB_PS_PES 0x0002		// Port enabled status
#define USB_PS_PCS 0x0001		// Port connected status
