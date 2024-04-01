#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>

static char *msgtbl[] =
{	"NOERRNo error indicated",								//  0
	"EOF  End of file",										// -1
	"SVC  Illegal SVC function",							// -2
	"FUNC Illegal function",								// -3
	"FUNCMIllegal function for current mode",				// -4
	"VALUEIllegal value specified",							// -5
	"PARMIIllegal parameter index",							// -6
	"PARMVIllegal parameter value",							// -7
	"PARMSIllegal parameter value size",					// -8
	"PARMTIllegal parameter type",							// -9
	"PARMFIllegal parameter function",						// -10
	"PARMMRequired parameter missing",						// -11
	"CHARNIllegal characteristic name",						// -12
	"CHARVIllegal characteristic value",					// -13
	"CHARSIllegal characteristic value size",				// -14
	"CHARTIllegal characteristic type",						// -15
	"CHARFIllegal characteristic function",					// -16
	"CHARMRequired characteristic missing",					// -17
	"BDNAMIllegal process or segment name",					// -18
	"BDPIDIllegal process descriptor",						// -19
	"NSP  No such process",									// -20
	"PRIV Privilege failure",								// -21
	"NSEGANo segment available",							// -22
	"NEMA Not enough memory available",						// -23
	"MACFTMemory allocation conflict",						// -24
	"MAERRMemory allocation error",							// -25
	"NODCBNo disk cache buffer available",					// -26
	"NOBUFNo system buffer available",						// -27
	"ACT  Device is active",								// -28
	"BDSPCIllegal device/file specification",				// -29
	"NSDEVNo such device",									// -30
	"DEVIUDevice or file in use",							// -31
	"DEVIODevice is open",									// -32
	"DEVNODevice is not open",								// -33
	"LASNALinear address space not available",				// -34
	"DEVFLDevice full",										// -35
	"TMDVPToo many devices open for process",				// -36
	"DFDEVDifferent device for rename",						// -37
	"FILNFFile not found",									// -38
	"FILEXFile exists",										// -39
	"BUSY Device or file is busy",							// -40
	"FILADFile access denied",								// -41
	"DIRNFDirectory not found",								// -42
	"DIRFLDirectory full",									// -43
	"DIRNEDirectory not empty",								// -44
	"DIRTDDirectory level too deep",						// -45
	"DATERData error",										// -46
	"IDFERRecord ID field error",							// -47
	"SEKERSeek error",										// -48
	"RNFERRecord not found",								// -49
	"LSTERLost data error",									// -50
	"WRTERWrite error",										// -51
	"WPRERWrite protect error",								// -52
	"DEVERDevice error",									// -53
	"DATTRData truncated",									// -54
	"NORSPDevice did not respond",							// -55
	"BDDBKBad disk block number",							// -56
	"BDDVHBad device handle",								// -57
	"NOOUTOutput not allowed",								// -58
	"NOIN Input not allowed",								// -59
	"ADRERAddress out of bounds",							// -60
	"IRFT Illegal RUN file type",							// -61
	"IRFF Illegal RUN file format",							// -62
	"IRFRDIllegal relocation in RUN file",					// -63
	"RELTRRelocation truncation in RUN file",				// -64
	"NOSADNo starting address in RUN file",					// -65
	"NOSTKNo stack specified in RUN file",					// -66
	"IFDEVIllegal function for device",						// -67
	"ICDEVIllegal byte count for device",					// -68
	"IADEVIllegal buffer address for device",				// -69
	"MDCHGMedia changed",									// -70
	"RTOBGRecord too big",									// -71
	"NACT Device or request is not active",					// -72
	"FMTERFormat error",									// -73
	"NTRDYDevice not ready",								// -74
	"NTDIRFile is not a directory",							// -75
	"ISDIRFile is a directory",								// -76
	"NTTRMDevice is not a terminal",						// -77
	"ILSEKSeek function illegal for device",				// -78
	"BPIPEPipe error",										// -79
	"DLOCKDeadlock condition",								// -80
	"FBFERFIB format error",								// -81
	"FBPERFIB pointer error",								// -82
	"FBRERError reading FIB",								// -83
	"FBWERError writing FIB",								// -84
	"HMFERHome block format error",							// -85
	"HMRERError reading home block",						// -86
	"STFERStorage allocation table format error",			// -87
	"STRERError reading storage allocation table",			// -88
	"STWERError writing storage allocation table",			// -89
	"DRFERDirectory block format error",					// -90
	"DRRERDirectory block read error",						// -91
	"DRWERDirectory block write error",						// -92
	"NTFILDevice is not file structured",					// -93
	"IATTRIllegal file attribute change",					// -94
	"NTDSKDevice is not a disk",							// -95
	"DQUOTDisk quota exceeded",								// -96
	"FSINCFile system is inconsistent",						// -97
	"NTDEFNot defined",										// -98
	"BDLNMExpanded logical name too long",					// -99
	"WLDNAWild-card name not allowed",						// -100
	"NTLNGName is too long",								// -101
	"TMUSRToo many users",									// -102
	"TMPSSToo many processes or shared segments in system",	// -103
	"PDTYPPhysical device is illegal type",					// -104
	"PDNAVPhysical device not available",					// -105
	"PDADFPhysical device already defined",					// -106
	"DUADFDevice unit already defined",						// -107
	"NSCLSNo such device class",							// -108
	"CLSADDevice class already defined",					// -109
	"XFRBKData transfer blocked",							// -110
	"TMDVCToo many devices open for class",					// -111
	"NPERRNetwork protocol error",							// -112
	"NPRNONetwork port not open",							// -113
	"NPRIUNetwork port in use",								// -114
	"NILPRIllegal network port number",						// -115
	"NILADIllegal network address",							// -116
	"NILRFIllegal network request format",					// -117
	"NILPCIllegal network protocol type",					// -118
	"NPCIUNetwork protocol type in use",					// -119
	"NCONGNetwork congestion",								// -120
	"NRTERNetwork routing error",							// -121
	"NSNODNo such network node",							// -122
	"NTTIMNetwork time-out",								// -123
	"NCLSTNetwork connection lost",							// -124
	"NHSNANetwork host not available",						// -125
	"NCCLRNetwork connection cleared",						// -126
	"NCRFSNetwork connection refused",						// -127
	"NNNDFNetwork name is not defined",						// -128
	"NNSNCNetwork name server not capable",					// -129
	"NNSRFNetwork name server refused request",				// -130
	"NNSNANetwork name server not available",				// -131
	"NNSRQNetwork name server bad request format",			// -132
	"NNSRSNetwork name server bad response format",			// -133
	"NNSERNetwork name server error",						// -134
	"NNMTDNetwork name mapping is too deep",				// -135
	"NRTNANetwork router not available",					// -136
	"NNCONNo connection established",						// -137
	"NDRTLNetwork data rejected - too long",				// -138
	"NPSQENetwork protocol sequencing error",				// -139
	"NOMEMMemory not allocated",							// -140
	"ALDEFAlready defined",									// -141
	"NCOMPNot compatible",									// -142
	"NOPAPPrinter is out of paper",							// -143
	"IMEMAIllegal memory address",							// -144
	"NSTYPNo such device type",								// -145
	"CHNNADMA channel not available",						// -146
	"BDLA Bad linear address",								// -147
	"TMRNCToo many requests for network connection",		// -148
	"DKRMVDisk removed",									// -149
	"ABORTIO operation aborted",							// -150
	"CANCLIO operation canceled",							// -151
	"SELNASegment selector not allocated",					// -152
	"BDSELBad segment selector value",						// -153
	"DOSMCDOS memory allocation data corrupted",			// -154
	"NDOSDNo DOS IO data block available",					// -155
	"IDEVCIncorrect device class",							// -156
	"DTINTData transfer interrupted",						// -157
	"IOSATIO saturation",									// -158
	"IDRENInvalid directory rename operation",				// -159
	"LKEALLKE already loaded",								// -160
	"CDAADLKE common data area already defined",			// -161
	"CDANDLKE common data area not defined",				// -162
	"ININUInterrupt number in use",							// -163
	"DIFERDevice interface error",							// -164
	"DVDERDevice driver error",								// -165
	"FINTRFunction interrupted",							// -166
	"NTIMPNot implemented",									// -167
	"ERRORUnspecified general error",						// -168
	"IOINUIO register block in use",						// -169
	"NACONNetwork - already connected",						// -170
	"NAUNRNetwork address unreachable",						// -171
	"NAINUNetwork address is in use",						// -172
	"TMIOMToo many IO requests for memory page",			// -173
	"TMIOPToo many IO request pointers",					// -174
	"MPILKMemory page is locked",							// -175
	"TMIOQToo many IO requests queued",						// -176
	"TMUDVToo many users for device",						// -177
	"TMDDVToo many device units for device",				// -178
	"NTLCLNot local",										// -179
	"DOSPBPermanent DOS process is busy",					// -180
	"INCIOIncomplete input-output operation",				// -181
	"NSLP Not a session level process",						// -182
	"LOCK File record lock violation",						// -183
	"CAASPClose action already specified",					// -184
	"CAERRClose action error",								// -185
	"FTPERFAT block pointer error",							// -186
	"FTRERError reading FAT block",							// -187
	"FTWERError writing FAT block",							// -188
	"TMRQBToo many requests for buffer",					// -189
	"CCMSSCannot change memory section size",				// -190
	"NNOPCNo network protocol specified",					// -191
	"IPDIR Illegal pointer in directory",					// -192
	"MSNPRMsect is not private",							// -193
	"INVST Invalid segment type",							// -194
	"NLKNANetwork link not available",						// -195
	"EVRESEvent is reserved",								// -196
	"EVNRSEvent is not reserved",							// -197
	"EVSETEvent is set",									// -198
	"CPDNR Child process did not respond",					// -199
	"STKERStack error",										// -200
	"DIVERDivide error",									// -201
	"ILLINIllegal instruction",								// -202
	"UNXSIUnexpected software interrupt",					// -203
	"NWPA No watchpoint available",							// -204
	"BDALMBad alarm handle",								// -205
	"TMALMToo many alarms for process",						// -206
	"DPMICDPMI environment corrupted}",						// -207
	"MEMLXMemory limit exceeded",							// -208
	"VECNSSignal vector not set up",						// -209
	"TRMNATerminal is not attached",						// -210
	"STIIUSCSI target ID is in use",						// -211
	"SLKCESCSI linked command error",						// -212
	"STDNRSCSI target did not respond",						// -213
	"SDLNESCSI data length error",							// -214
	"SUXBFSCSI unexpected bus free state",					// -215
	"STBPSSCSI target bus phase sequence failure",			// -216
	"STARISCSI target number is illegal",					// -217
	"SLUNISCSI logical unit number is illegal",				// -218
	"SSUNISCSI sub-unit number is illegal",					// -219
	"SDVTISCSI device type is incompatible",				// -220
	"BLANKMedia is blank",									// -221
	"NBLNKMedia is not blank",								// -222
	"EOS  End of set",										// -223
	"EOM  End of media",									// -224
	"IRFSUIllegal RUN file symbol - undefined",				// -225
	"IRFSLIllegal RUN file symbol - name too long",			// -226
	"IFXSTIllegal format in exported symbol table",			// -227
	"OUTNEOutput is not enabled",							// -228
	"NOACKOutput has not been acknowleged",					// -229
	"TMORQToo many output requests",						// -230
	"NMBTSName buffer is too small",						// -231
	"IINUMIllegal interrupt number",						// -232
	"IDSPCIllegal destination file specification",			// -233
	"TYPADDevice type already defined",						// -234
	"NEDMANot enough device memory available",				// -235
	"PWUSRIncorrect user name or password",					// -236
	"NNPA No network port available",						// -237
	"SCTNASection not available",							// -238
	"TMDVSToo many devices open for system",				// -239
	"IONTCIO operation not complete",						// -240
	"MDICNModem is connected",								// -241
	"MDNCNModem not connected",								// -242
	"NSSRVNo such server",									// -243
	"ISREQIllegal server request",							// -244
	"ISRSPIllegal server response",							// -245
	"SIOERServer IO error",									// -246
	"IDFMTIllegal data format",								// -247
	"NAPERNetwork application protocol error",				// -248
	"MSGNFMessage not found",								// -249
	"MSGFEMessage format error",							// -250
	"NPNIUNetwork port not in use",							// -251
	"NNDFPNo destination for network protocol",				// -252
	"TMMBKToo many physical memory blocks",					// -253
	"NTINSNot installed",									// -254
	"NANAVNetwork Address is not available",				// -255
	"NIYT Not implemented yet",								// -256
	"MATH Math library routine error",						// -257
	"RANGEMath library argument out of range",				// -258
	"TMTHDToo many threads",								// -259
	"THDNSThread is not suspended",							// -260
	"NODATNo data in file",									// -261
	"BDFMTBad file format}",								// -262
	"BDPFXBad prefixed value",								// -262
	"REC2LRecord is too long",								// -263
	"REC2SRecord is too short",								// -264
	"BLK2SBlock is too short}",								// -265
	"BLK2SBlock is too short",								// -266
	"BDFNTBad font file",									// -267
	"FIL2LFile is too long"									// -267
};

//*****************************************************************
// Function: xoserrmsg - Get error messages text for XOS error code
// Returned: Nothing
//*****************************************************************

void xoserrmsg(
	long  code,					// XOS error code
	char *test)					// Buffer to receive string, must be at
								//   least 80 bytes in length

{
    char *pnt;

	if (code > 0 || code < -(int)(sizeof(msgtbl)/sizeof(char *)) ||
			(pnt = msgtbl[-code]) == NULL)
		sprintf(test, "{%05.5d} Undefined error code", code);
	else
		sprintf(test, "{%5.5s} %s", pnt, pnt + 5);
}
