struc bootsector, 0x7c03
	iOEM:			resb	8				; OEM String
	iSectSize:		resw	1				; Bytes per sector
	iClustSize:		resb	1				; Sectors per cluster
	iResSect:		resw	1				; Number of reserved sectors
	iFatCnt:		resb	1				; Number of FAT count
	iRootSize:		resw	1				; Size of root directory
	iTotalSect:		resw	1				; Total number of sectors
	iMedia:			resb	1				; Media descriptor
	iFatSize:		resw	1				; Size of each FAT
	iTrackSect:		resw	1				; Sectors per track
	iHeadCnt:		resw	1				; Number of heads
	iHiddenSect:	resd	1				; Number of hidden sectors
	iSect32:		resd	1				; Number of sectors over 32MB
	iBootDrive:		resb	1				; Holds the boot drive
	iReserved:		resb	1				; Reserved, empty
	iBootSign:		resb	1				; Extended boot signature
	iVolID:			resw	1				; Disk serial number
	acVolumeLabel:	resb	11				; Volume label
	acFSType:		resb	8				; File system type
endstruc
