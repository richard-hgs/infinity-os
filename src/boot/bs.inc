bootsector:
	iOEM:			db	"PhilOS  "		; OEM String
	iSectSize:		dw	0x200			; Bytes per sector
	iClustSize:		db	1				; Sectors per cluster
	iResSect:		dw	1				; Number of reserved sectors
	iFatCnt:		db	1				; Number of FAT count
	iRootSize:		dw	224				; Size of root directory
	iTotalSect:		dw	2880			; Total number of sectors
	iMedia:			db	0xf0			; Media descriptor
	iFatSize:		dw	7				; Size of each FAT
	iTrackSect:		dw	18				; Sectors per track
	iHeadCnt:		dw	2				; Number of heads
	iHiddenSect:	dd	0				; Number of hidden sectors
	iSect32:		dd	0				; Number of sectors over 32MB
	iBootDrive:		db	0				; Holds the boot drive
	iReserved:		db	0				; Reserved, empty
	iBootSign:		db	0x29			; Extended boot signature
	iVolID:			dw	0xface			; Disk serial number
	acVolumeLabel:	db	"PRS-FLOPPY "	; Volume label
	acFSType:		db	"PRSFS   "		; File system type
