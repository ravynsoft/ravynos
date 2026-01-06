/*
 * Copyright (c) 2001-2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _SCSI_COMMAND_OPERATION_CODES_H_
#define _SCSI_COMMAND_OPERATION_CODES_H_

#pragma mark About this file
/* This file contains the operation code definitions for all commands defined
 * by the SCSI specifications.  The commands are listed in three formats:
 * 1) All commands are listed in alphabetical order.  This list is the live
 * enumeration for all of the command constants.
 * 2) The commands are listed in ascending numerical order.
 * 3) The commands are grouped by Peripheral Device Type.
 * 
 * In the command listings by Peripheral Device Type, there will be a comment
 * following each command.  This comment indentifies the section of the related
 * specification where the commands is defined and the requirement type of the
 * command, Mandatory or Optional.
 * If a specification redefines an optional command from SPC as mandatory,
 * the command will be relisted in the Peripheral Device Type command list with
 * the mandatory tag next to it.
 * All commands that are listed in SPC as Device Type Specifc will be relisted
 * as a comment in all specifications lists that support that command with the
 * appropriate Mandatory or Optional tag for that specification.
 *
 * The section number and the requirement type of the command are based on the 
 * version of the specification listed in the header comment for the Peripheral
 * Device Type.  This data is provided for informational purposes only.  The 
 * specification document and version that the device adheres to as indicated 
 * by the data returned in response to the INQUIRY command should be used as
 * the authorative source for supported and required behavior of the device.
 * 
 * The SPC set is listed before all other Peripheral Device Type commands as
 * this is the base document from which all of the other documents are derived.  
 *
 * The Peripheral Device Types and associated command sets as defined by SPC-2,
 * section 7.4.1 are as follows:
 *  Peripheral Device Type                  Associated Command Specification
 *  ------------------------------------    -----------------------------------
 *  0x00 Direct Access Device               SBC - SCSI-3 Block Commands
 *  0x01 Sequential Access Device           SSC - SCSI-3 Stream Commands
 *  0x02 Printer Device                     SSC - SCSI-3 Stream Commands
 *  0x03 Processor Device                   SPC - SCSI Primary Commands-2
 *  0x04 Write Once Device                  SBC - SCSI-3 Block Commands
 *  0x05 CD-ROM Device                      MMC - SCSI Multimedia Commands-2
 *  0x06 Scanner Device                     SGC - SCSI-3 Graphics Commands
 *  0x07 Optical Memory Device              SBC - SCSI-3 Block Commands
 *  0x08 Medium Changer Device              SMC - SCSI-3 Medium Changer Cmds
 *  0x09 Communications Device              SSC - SCSI-3 Stream Commands
 *  0x0A - 0x0B Graphic Arts Prepress Dev   ASC IT8
 *  0x0C Storage Array Controller Device    SCC-2 - SCSI Controller Commands-2
 *  0x0D Enclosure Services                 SES - SCSI-3 Enclosure Services
 *  0x0E Simplified Direct Access Device    RBC - SCSI Reduced Block Commands
 *  0x0F Optical Card Reader/Writer Device  OCRW - SCSI Specification for 
 *                                              Optical Card Reader/Writer
 *  0x10 Reserved                    		No command specification
 *	0x11 Object-Based Storage Device		OSD - SCSI Object Based Storage 
 *												Device Commands
 *  0x12 - 0x14 Reserved                    No command specification
 *  0x15 Multimedia Media Access Engine     RMC - Reduced Multimedia Commands
 *  0x16 - 0x1E Reserved                    No command specification
 *  0x1F Unknown or No Device               No command specification
 */ 

#pragma mark -
#pragma mark Command Definitions by Name
/* All SCSI Commands listed in alphabetical order.  These are the live
 * definitions of the commands.  All other command lists are informative.
 */
enum 
{
    kSCSICmd_ACCESS_CONTROL_IN              = 0x86,
    kSCSICmd_ACCESS_CONTROL_OUT             = 0x87,
    kSCSICmd_BLANK                          = 0xA1,
    kSCSICmd_CHANGE_DEFINITION              = 0x40,
    kSCSICmd_CLOSE_TRACK_SESSION            = 0x5B,
    kSCSICmd_COMPARE                        = 0x39,
    kSCSICmd_COPY                           = 0x18,
    kSCSICmd_COPY_AND_VERIFY                = 0x3A,
    kSCSICmd_ERASE_10						= 0x2C,
    kSCSICmd_ERASE_12						= 0xAC,
    kSCSICmd_EXTENDED_COPY                  = 0x83,
    kSCSICmd_FORMAT_UNIT                    = 0x04,
    kSCSICmd_GET_CONFIGURATION              = 0x46,
    kSCSICmd_GET_EVENT_STATUS_NOTIFICATION  = 0x4A,
    kSCSICmd_GET_PERFORMANCE                = 0xAC,
    kSCSICmd_INQUIRY                        = 0x12,
    kSCSICmd_LOAD_UNLOAD_MEDIUM             = 0xA6,
    kSCSICmd_LOCK_UNLOCK_CACHE              = 0x36,
    kSCSICmd_LOCK_UNLOCK_CACHE_16           = 0x92,
    kSCSICmd_LOG_SELECT                     = 0x4C,
    kSCSICmd_LOG_SENSE                      = 0x4D,
    kSCSICmd_MAINTENANCE_IN               	= 0xA3,
    kSCSICmd_MAINTENANCE_OUT                = 0xA4,
    kSCSICmd_MECHANISM_STATUS               = 0xBD,
    kSCSICmd_MEDIUM_SCAN                    = 0x38,
    kSCSICmd_MODE_SELECT_6                  = 0x15,
    kSCSICmd_MODE_SELECT_10                 = 0x55,
    kSCSICmd_MODE_SENSE_6                   = 0x1A,
    kSCSICmd_MODE_SENSE_10                  = 0x5A,
    kSCSICmd_MOVE_MEDIUM_ATTACHED           = 0xA7,
    kSCSICmd_PAUSE_RESUME                   = 0x4B,
    kSCSICmd_PERSISTENT_RESERVE_IN          = 0x5E,
    kSCSICmd_PERSISTENT_RESERVE_OUT         = 0x5F,
    kSCSICmd_PLAY_AUDIO_10                  = 0x45,
    kSCSICmd_PLAY_AUDIO_12                  = 0xA5,
    kSCSICmd_PLAY_AUDIO_MSF                 = 0x47,
    kSCSICmd_PLAY_AUDIO_TRACK_INDEX         = 0x48,
    kSCSICmd_PLAY_CD                        = 0xBC,
    kSCSICmd_PLAY_RELATIVE_10               = 0x49,
    kSCSICmd_PLAY_RELATIVE_12               = 0xA9,
    kSCSICmd_PREFETCH                       = 0x34,
    kSCSICmd_PREFETCH_16                    = 0x90,
    kSCSICmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E,
    kSCSICmd_READ_6                         = 0x08,
    kSCSICmd_READ_10                        = 0x28,
    kSCSICmd_READ_12                        = 0xA8,
    kSCSICmd_READ_16                        = 0x88,
    kSCSICmd_READ_ATTRIBUTE                 = 0x8C,
    kSCSICmd_READ_BUFFER                    = 0x3C,
    kSCSICmd_READ_BUFFER_CAPACITY           = 0x5C,
    kSCSICmd_READ_CAPACITY                  = 0x25,
    kSCSICmd_READ_CD                        = 0xBE,
    kSCSICmd_READ_CD_MSF                    = 0xB9,
    kSCSICmd_READ_DEFECT_DATA_10            = 0x37,
    kSCSICmd_READ_DEFECT_DATA_12            = 0xB7,
    kSCSICmd_READ_DISC_INFORMATION          = 0x51,
    kSCSICmd_READ_DVD_STRUCTURE             = 0xAD,
    kSCSICmd_READ_DISC_STRUCTURE            = 0xAD,
    kSCSICmd_READ_ELEMENT_STATUS_ATTACHED   = 0xB4,
    kSCSICmd_READ_FORMAT_CAPACITIES         = 0x23,
    kSCSICmd_READ_GENERATION				= 0x29,
    kSCSICmd_READ_HEADER                    = 0x44,
    kSCSICmd_READ_LONG                      = 0x3E,
    kSCSICmd_READ_MASTER_CUE                = 0x59,
    kSCSICmd_READ_SUB_CHANNEL               = 0x42,
    kSCSICmd_READ_TOC_PMA_ATIP              = 0x43,
    kSCSICmd_READ_TRACK_INFORMATION         = 0x52,
    kSCSICmd_READ_UPDATED_BLOCK_10			= 0x2D,
    kSCSICmd_REASSIGN_BLOCKS                = 0x07,
    kSCSICmd_REBUILD                        = 0x81,
    kSCSICmd_RECEIVE                        = 0x08,
    kSCSICmd_RECEIVE_COPY_RESULTS           = 0x84,
    kSCSICmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C,
    kSCSICmd_REDUNDANCY_GROUP_IN			= 0xBA,
    kSCSICmd_REDUNDANCY_GROUP_OUT			= 0xBB,
    kSCSICmd_REGENERATE                     = 0x82,
    kSCSICmd_RELEASE_6                      = 0x17,
    kSCSICmd_RELEASE_10                     = 0x57,
    kSCSICmd_REPAIR_TRACK                   = 0x58,
    kSCSICmd_REPORT_DEVICE_IDENTIFIER    	= 0xA3,
    kSCSICmd_REPORT_KEY                     = 0xA4,
    kSCSICmd_REPORT_LUNS                    = 0xA0,
    kSCSICmd_REQUEST_SENSE                  = 0x03,
    kSCSICmd_RESERVE_6                      = 0x16,
    kSCSICmd_RESERVE_10                     = 0x56,
    kSCSICmd_RESERVE_TRACK                  = 0x53,
    kSCSICmd_REZERO_UNIT                    = 0x01,
    kSCSICmd_SCAN_MMC                       = 0xBA,
    kSCSICmd_SEARCH_DATA_EQUAL_10           = 0x31,
    kSCSICmd_SEARCH_DATA_EQUAL_12           = 0xB1,
    kSCSICmd_SEARCH_DATA_HIGH_10            = 0x30,
    kSCSICmd_SEARCH_DATA_HIGH_12            = 0xB0,
    kSCSICmd_SEARCH_DATA_LOW_10             = 0x32,
    kSCSICmd_SEARCH_DATA_LOW_12             = 0xB2,
    kSCSICmd_SEEK_6                         = 0x0B,
    kSCSICmd_SEEK_10                        = 0x2B,
    kSCSICmd_SEND                           = 0x0A,
    kSCSICmd_SEND_CUE_SHEET                 = 0x5D,
    kSCSICmd_SEND_DIAGNOSTICS               = 0x1D,
    kSCSICmd_SEND_DVD_STRUCTURE             = 0xBF,
    kSCSICmd_SEND_EVENT                     = 0xA2,
    kSCSICmd_SEND_KEY                       = 0xA3,
    kSCSICmd_SEND_OPC_INFORMATION           = 0x54,
    kSCSICmd_SERVICE_ACTION_IN		        = 0x9E,
    kSCSICmd_SERVICE_ACTION_OUT		        = 0x9F,
    kSCSICmd_SET_CD_SPEED                   = 0xBB,
    kSCSICmd_SET_DEVICE_IDENTIFIER      	= 0xA4,
    kSCSICmd_SET_LIMITS_10                  = 0x33,
    kSCSICmd_SET_LIMITS_12                  = 0xB3,
    kSCSICmd_SET_READ_AHEAD                 = 0xA7,
    kSCSICmd_SET_STREAMING                  = 0xB6,
    kSCSICmd_SPARE_IN		                = 0xBC,
    kSCSICmd_SPARE_OUT		                = 0xBD,
    kSCSICmd_START_STOP_UNIT                = 0x1B,
    kSCSICmd_STOP_PLAY_SCAN                 = 0x4E,
    kSCSICmd_SYNCHRONIZE_CACHE              = 0x35,
    kSCSICmd_SYNCHRONIZE_CACHE_16           = 0x91,
    kSCSICmd_TEST_UNIT_READY                = 0x00,
    kSCSICmd_UPDATE_BLOCK					= 0x3D,
    kSCSICmd_UNMAP                          = 0x42,
    kSCSICmd_VERIFY_10                      = 0x2F,
    kSCSICmd_VERIFY_12                      = 0xAF,
    kSCSICmd_VERIFY_16                      = 0x8F,
    kSCSICmd_VOLUME_SET_IN                  = 0xBE,
    kSCSICmd_VOLUME_SET_OUT                 = 0xBF,
    kSCSICmd_WRITE_6                        = 0x0A,
    kSCSICmd_WRITE_10                       = 0x2A,
    kSCSICmd_WRITE_12                       = 0xAA,
    kSCSICmd_WRITE_16                       = 0x8A,
    kSCSICmd_WRITE_AND_VERIFY_10            = 0x2E,
	kSCSICmd_WRITE_AND_VERIFY_12            = 0xAE,
	kSCSICmd_WRITE_AND_VERIFY_16            = 0x8E,
	kSCSICmd_WRITE_ATTRIBUTE	            = 0x8D,
    kSCSICmd_WRITE_BUFFER                   = 0x3B,
    kSCSICmd_WRITE_LONG                     = 0x3F,
    kSCSICmd_WRITE_SAME                     = 0x41,
    kSCSICmd_WRITE_SAME_16                  = 0x93,
    kSCSICmd_XDREAD                         = 0x52,
    kSCSICmd_XDWRITE                        = 0x50,
    kSCSICmd_XDWRITE_EXTENDED               = 0x80,
    kSCSICmd_XDWRITEREAD_10           		= 0x53,
    kSCSICmd_XPWRITE                        = 0x51,
    
    kSCSICmdVariableLengthCDB				= 0x7F
};

/* Service Action Definitions for the Variable Length CDB (7Fh) command */
enum
{
	kSCSIServiceAction_READ_32				= 0x0009,
	kSCSIServiceAction_VERIFY_32			= 0x000A,
	kSCSIServiceAction_WRITE_32				= 0x000B,
	kSCSIServiceAction_WRITE_AND_VERIFY_32	= 0x000C,
	kSCSIServiceAction_WRITE_SAME_32		= 0x000D,
	kSCSIServiceAction_XDREAD_32			= 0x0003,
	kSCSIServiceAction_XDWRITE_32			= 0x0004,
	kSCSIServiceAction_XDWRITEREAD_32		= 0x0007,
	kSCSIServiceAction_XPWRITE_32			= 0x0006
};

/* Service Action Definitions for the MAINTENANCE IN (A3h) command */
enum
{
	kSCSIServiceAction_REPORT_ALIASES								= 0x0B,
	kSCSIServiceAction_REPORT_DEVICE_IDENTIFIER						= 0x05,
	kSCSIServiceAction_REPORT_PRIORITY								= 0x0E,
	kSCSIServiceAction_REPORT_PROVISIONING_INITIALIZATION_PATTERN   = 0x1D,
	kSCSIServiceAction_REPORT_SUPPORTED_OPERATION_CODES				= 0x0C,
	kSCSIServiceAction_REPORT_SUPPORTED_TASK_MANAGEMENT_FUNCTIONS	= 0x0D,
	kSCSIServiceAction_REPORT_TARGET_PORT_GROUPS					= 0x0A
};

/* Service Action Definitions for the MAINTENANCE OUT (A4h) command */
enum
{
	kSCSIServiceAction_CHANGE_ALIASES			= 0x0B,
	kSCSIServiceAction_SET_DEVICE_IDENTIFIER	= 0x06,
	kSCSIServiceAction_SET_PRIORITY				= 0x0E,
	kSCSIServiceAction_SET_TARGET_PORT_GROUPS	= 0x0A	
};

/* Service Action Definitions for the SERVICE ACTION IN (9Eh) command */
enum
{
	kSCSIServiceAction_GET_LBA_STATUS       = 0x12,
	kSCSIServiceAction_READ_CAPACITY_16		= 0x10,
	kSCSIServiceAction_READ_LONG_16			= 0x11,
};

/* Service Action Definitions for the SERVICE ACTION OUT (9Fh) command */
enum
{
	kSCSIServiceAction_WRITE_LONG_16		= 0x11	
};

#pragma mark -
#pragma mark Command Definitions by Number
#if 0
enum 
{
};
#endif


#pragma mark -
#pragma mark All Types SPC Commands
/* Commands defined by the T10:1236-D SCSI Primary Commands-2 (SPC-2) 
 * command specification.  The definitions and section numbers are based on
 * section 7 of the revision 18, 21 May 2000 version of the specification.
 *
 * These commands are defined for all devices.
 */
enum 
{
    kSPCCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kSPCCmd_COMPARE                        = 0x39, /* Sec. 7.2: Optional */
    kSPCCmd_COPY                           = 0x18, /* Sec. 7.3: Optional */
    kSPCCmd_COPY_AND_VERIFY                = 0x3A, /* Sec. 7.4: Optional */
    kSPCCmd_EXTENDED_COPY                  = 0x83, /* Sec. 7.5: Optional */
    kSPCCmd_INQUIRY                        = 0x12, /* Sec. 7.6: Mandatory */
    kSPCCmd_LOG_SELECT                     = 0x4C, /* Sec. 7.7: Optional */
    kSPCCmd_LOG_SENSE                      = 0x4D, /* Sec. 7.8: Optional */
    kSPCCmd_MODE_SELECT_6                  = 0x15, /* Sec. 7.9: Device Type
													* Specific */
    kSPCCmd_MODE_SELECT_10                 = 0x55, /* Sec. 7.10: Device Type
													* Specific */
    kSPCCmd_MODE_SENSE_6                   = 0x1A, /* Sec. 7.11: Device Type
													* Specific */
    kSPCCmd_MODE_SENSE_10                  = 0x5A, /* Sec. 7.12: Device Type
													* Specific */
    kSPCCmd_MOVE_MEDIUM_ATTACHED           = 0xA7, /* Defined in SMC */
    kSPCCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* Sec. 7.13: Device Type
													* Specific */
    kSPCCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* Sec. 7.14: Device Type
													* Specific */
    kSPCCmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E, /* Sec. 7.15: Device Type
													* Specific */
    kSPCCmd_READ_BUFFER                    = 0x3C, /* Sec. 7.16: Optional */
    kSPCCmd_READ_ELEMENT_STATUS_ATTACHED   = 0xB4, /* Defined in SMC */
    kSPCCmd_RECEIVE_COPY_RESULTS           = 0x84, /* Sec. 7.17: Optional */
    kSPCCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* Sec. 7.18: Optional */
    kSPCCmd_RELEASE_10                     = 0x57, /* Sec. 7.19: Device Type
													* Specific */
    kSPCCmd_RELEASE_6                      = 0x17, /* Sec. 7.20: Device Type
													* Specific */
    kSPCCmd_REPORT_DEVICE_IDENTIFIER       = 0xA3, /* Sec. 7.21: Optional */
    kSPCCmd_REPORT_LUNS                    = 0xA0, /* Sec. 7.22: Mandatory for
													* LUN Supporting devices*/
    kSPCCmd_REQUEST_SENSE                  = 0x03, /* Sec. 7.23: Device Type
													* Specific */
    kSPCCmd_RESERVE_10                     = 0x56, /* Sec. 7.24: Device Type
													* Specific */
    kSPCCmd_RESERVE_6                      = 0x16, /* Sec. 7.25: Device Type
													* Specific */
    kSPCCmd_SEND_DIAGNOSTICS               = 0x1D, /* Sec. 7.26: Optional */
    kSPCCmd_SET_DEVICE_IDENTIFIER          = 0xA4, /* Sec. 7.27: Optional */
    kSPCCmd_TEST_UNIT_READY                = 0x00, /* Sec. 7.28: Mandatory */
    kSPCCmd_WRITE_BUFFER                   = 0x3B  /* Sec. 7.29: Optional */
};

#pragma mark -
#pragma mark 0x00 SBC Direct Access Commands
/* Commands defined by the T10:990-D SCSI-3 Block Commands (SBC) command
 * specification.  The definitions and section numbers are based on section 6.1
 * of the revision 8c, 13 November 1997 version of the specification. 
 */
enum
{
    kSBCCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kSBCCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kSBCCmd_COPY                           = 0x18, /* SPC: Optional */
    kSBCCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional*/
    kSBCCmd_FORMAT_UNIT                    = 0x04, /* Sec. 6.1.1: Mandatory */
    kSBCCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kSBCCmd_LOCK_UNLOCK_CACHE              = 0x36, /* Sec. 6.1.2: Optional */
    kSBCCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kSBCCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kSBCCmd_MODE_SELECT_6                  = 0x15, /* SPC: Optional */
    kSBCCmd_MODE_SELECT_10                 = 0x55, /* SPC: Optional */
    kSBCCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Optional */
    kSBCCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Optional */
    kSBCCmd_MOVE_MEDIUM_ATTACHED           = 0xA7, /* SMC: Optional */
    kSBCCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* SPC: Optional */
    kSBCCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* SPC: Optional */
    kSBCCmd_PREFETCH                       = 0x34, /* Sec. 6.1.3: Optional */
    kSBCCmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E, /* SPC: Optional */
    kSBCCmd_READ_6                         = 0x08, /* Sec. 6.1.4: Mandatory */
    kSBCCmd_READ_10                        = 0x28, /* Sec. 6.1.5: Mandatory */
    kSBCCmd_READ_12                        = 0xA8, /* Sec. 6.2.4: Optional */
    kSBCCmd_READ_BUFFER                    = 0x3C, /* SPC: Optional */
    kSBCCmd_READ_CAPACITY                  = 0x25, /* Sec. 6.1.6: Mandatory */
    kSBCCmd_READ_DEFECT_DATA_10            = 0x37, /* Sec. 6.1.7: Optional */
    kSBCCmd_READ_DEFECT_DATA_12            = 0xB7, /* Sec. 6.2.5: Optional */
    kSBCCmd_READ_ELEMENT_STATUS_ATTACHED   = 0xB4, /* SMC: Optional */
    kSBCCmd_READ_GENERATION			       = 0x29, /* Sec. 6.2.6: Optional */
    kSBCCmd_READ_LONG                      = 0x3E, /* Sec. 6.1.8: Optional */
    kSBCCmd_READ_UPDATED_BLOCK_10		   = 0x2D, /* Sec. 6.2.7: Optional */
    kSBCCmd_REASSIGN_BLOCKS                = 0x07, /* Sec. 6.1.9: Optional */
    kSBCCmd_REBUILD                        = 0x81, /* Sec. 6.1.10: Optional */
    kSBCCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* SPC: Optional */
    kSBCCmd_REGENERATE                     = 0x82, /* Sec. 6.1.11: Optional */
    kSBCCmd_RELEASE_6                      = 0x17, /* SPC: Optional */
    kSBCCmd_RELEASE_10                     = 0x57, /* SPC: Mandatory */
    kSBCCmd_REPORT_LUNS                    = 0xA0, /* SPC: Optional */
    kSBCCmd_REQUEST_SENSE                  = 0x03, /* SPC: Mandatory */
    kSBCCmd_RESERVE_6                      = 0x16, /* SPC: Optional */
    kSBCCmd_RESERVE_10                     = 0x56, /* SPC: Mandatory */
    kSBCCmd_REZERO_UNIT                    = 0x01, /* Obsolete */
    kSBCCmd_SEARCH_DATA_EQUAL_10           = 0x31, /* Obsolete */
    kSBCCmd_SEARCH_DATA_HIGH_10            = 0x30, /* Obsolete */
    kSBCCmd_SEARCH_DATA_LOW_10             = 0x32, /* Obsolete */
    kSBCCmd_SEEK_6                         = 0x0B, /* Obsolete */
    kSBCCmd_SEEK_10                        = 0x2B, /* Sec. 6.1.12: Optional */
    kSBCCmd_SEND_DIAGNOSTICS               = 0x1D, /* SPC: Mandatory */
    kSBCCmd_SET_LIMITS_10                  = 0x33, /* Sec. 6.1.13: Optional */
    kSBCCmd_SET_LIMITS_12                  = 0xB3, /* Sec. 6.2.8: Optional */
    kSBCCmd_START_STOP_UNIT                = 0x1B, /* Sec. 6.1.14: Optional */
    kSBCCmd_SYNCHRONIZE_CACHE              = 0x35, /* Sec. 6.1.15: Optional */
    kSBCCmd_TEST_UNIT_READY                = 0x00, /* SPC: Mandatory */
	kSBCCmd_UPDATE_BLOCK                   = 0x3D, /* Sec. 6.2.9: Optional */
    kSBCCmd_VERIFY_10                      = 0x2F, /* Sec. 6.1.16: Optional */
    kSBCCmd_WRITE_6                        = 0x0A, /* Sec. 6.1.17: Optional */
    kSBCCmd_WRITE_10                       = 0x2A, /* Sec. 6.1.18: Optional */
    kSBCCmd_WRITE_12                       = 0xAA, /* Sec. 6.2.13: Optional */
    kSBCCmd_WRITE_AND_VERIFY_10            = 0x2E, /* Sec. 6.1.19: Optional */
    kSBCCmd_WRITE_AND_VERIFY_12            = 0xAE, /* Sec. 6.2.15: Optional */
    kSBCCmd_WRITE_BUFFER                   = 0x3B, /* SPC: Optional */
    kSBCCmd_WRITE_LONG                     = 0x3F, /* Sec. 6.1.20: Optional */
    kSBCCmd_WRITE_SAME                     = 0x41, /* Sec. 6.1.21: Optional */
    kSBCCmd_XDREAD                         = 0x52, /* Sec. 6.1.22: Optional */
    kSBCCmd_XDWRITE                        = 0x50, /* Sec. 6.1.23: Optional */
    kSBCCmd_XDWRITE_EXTENDED               = 0x80, /* Sec. 6.1.24: Optional */
    kSBCCmd_XPWRITE                        = 0x51  /* Sec. 6.1.25: Optional */
};

#pragma mark -
#pragma mark 0x01 SSC Sequential Access Commands
/* Commands defined by the T10:997-D SCSI-3 Stream Commands (SSC) command
 * specification.  The definitions and section numbers are based on section 5
 * of the revision 22, January 1, 2000 version of the specification. 
 */
enum
{
    kSSCSeqCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kSSCSeqCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kSSCSeqCmd_COPY                           = 0x18, /* SPC: Optional */
    kSSCSeqCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional */
    kSSCSeqCmd_ERASE                          = 0x19, /* Sec. 5.3.1: Mandatory */
    kSSCSeqCmd_FORMAT_MEDIUM                  = 0x04, /* Sec. 5.3.2: Optional */
    kSSCSeqCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kSSCSeqCmd_LOAD_UNLOAD                    = 0x1B, /* Sec. 5.3.3: Optional */
    kSSCSeqCmd_LOCATE                         = 0x2B, /* Sec. 5.3.4: Optional */
    kSSCSeqCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kSSCSeqCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kSSCSeqCmd_MODE_SELECT_6                  = 0x15, /* SPC: Mandatory */
    kSSCSeqCmd_MODE_SELECT_10                 = 0x55, /* SPC: Optional */
    kSSCSeqCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Mandatory */
    kSSCSeqCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Optional */
    kSSCSeqCmd_MOVE_MEDIUM                    = 0xA5, /* SMC: Optional */
    kSSCSeqCmd_MOVE_MEDIUM_ATTACHED           = 0xA7, /* SMC: Optional */
    kSSCSeqCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* SPC: Optional */
    kSSCSeqCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* SPC: Optional */
    kSSCSeqCmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E, /* SPC: Optional */
    kSSCSeqCmd_READ_6                         = 0x08, /* Sec. 5.3.5: Mandatory */
    kSSCSeqCmd_READ_BLOCK_LIMITS              = 0x05, /* Sec. 5.3.6: Mandatory */
    kSSCSeqCmd_READ_BUFFER                    = 0x3C, /* SPC: Optional */
    kSSCSeqCmd_READ_ELEMENT_STATUS            = 0xB8, /* SMC: Optional */
    kSSCSeqCmd_READ_ELEMENT_STATUS_ATTACHED   = 0xB4, /* SMC: Optional */
    kSSCSeqCmd_READ_POSITION                  = 0x34, /* Sec. 5.3.7: Mandatory */
    kSSCSeqCmd_READ_REVERSE                   = 0x0F, /* Sec. 5.3.8: Optional */
    kSSCSeqCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* SPC: Optional */
    kSSCSeqCmd_RECOVER_BUFFERED_DATA          = 0x14, /* Sec. 5.3.9: Optional */
    kSSCSeqCmd_RELEASE_6                      = 0x17, /* SPC: Mandatory */
    kSSCSeqCmd_RELEASE_10                     = 0x57, /* SPC: Mandatory */
    kSSCSeqCmd_REPORT_DENSITY_SUPPORT         = 0x44, /* Sec. 5.3.10: Mandatory*/
    kSSCSeqCmd_REPORT_LUNS                    = 0xA0, /* SPC: Mandatory */
    kSSCSeqCmd_REQUEST_SENSE                  = 0x03, /* SPC: Mandatory */
    kSSCSeqCmd_RESERVE_6                      = 0x16, /* SPC: Mandatory */
    kSSCSeqCmd_RESERVE_10                     = 0x56, /* SPC: Mandatory */
    kSSCSeqCmd_REWIND                         = 0x01, /* Sec. 5.3.11: Mandatory*/
    kSSCSeqCmd_SEND_DIAGNOSTICS               = 0x1D, /* SPC: Mandatory */
    kSSCSeqCmd_SPACE                          = 0x11, /* Sec. 5.3.12: Mandatory*/
    kSSCSeqCmd_TEST_UNIT_READY                = 0x00, /* SPC: Mandatory */
    kSSCSeqCmd_VERIFY_6                       = 0x13, /* Sec. 5.3.13: Optional */
    kSSCSeqCmd_WRITE_6                        = 0x0A, /* Sec. 5.3.14: Mandatory*/
    kSSCSeqCmd_WRITE_BUFFER                   = 0x3B, /* SPC: Optional */
    kSSCSeqCmd_WRITE_FILEMARKS                = 0x10  /* Sec. 5.3.15: Mandatory*/
};

#pragma mark -
#pragma mark 0x02 SSC Printer Commands
/* Commands defined by the T10:997-D SCSI-3 Stream Commands (SSC) command
 * specification.  The definitions and section numbers are based on section 6
 * of the revision 22, January 1, 2000 version of the specification. 
 */
enum 
{
    kSSCPrinterCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kSSCPrinterCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kSSCPrinterCmd_COPY                           = 0x18, /* SPC: Optional */
    kSSCPrinterCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional */
    kSSCPrinterCmd_FORMAT                         = 0x04, /* Sec. 6.2.1: Optional */
    kSSCPrinterCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kSSCPrinterCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kSSCPrinterCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kSSCPrinterCmd_MODE_SELECT_6                  = 0x15, /* SPC: Mandatory */
    kSSCPrinterCmd_MODE_SELECT_10                 = 0x55, /* SPC: Optional */
    kSSCPrinterCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Mandatory */
    kSSCPrinterCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Optional */
    kSSCPrinterCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* SPC: Optional */
    kSSCPrinterCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* SPC: Optional */
    kSSCPrinterCmd_PRINT                          = 0x0A, /* Sec. 6.2.2: Mandatory */
    kSSCPrinterCmd_READ_BUFFER                    = 0x3C, /* SPC: Optional */
    kSSCPrinterCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* SPC: Optional */
    kSSCPrinterCmd_RECOVER_BUFFERED_DATA          = 0x14, /* Sec. 6.2.3: Optional */
    kSSCPrinterCmd_RELEASE_6                      = 0x17, /* SPC: Mandatory */
    kSSCPrinterCmd_RELEASE_10                     = 0x57, /* SPC: Mandatory */
    kSSCPrinterCmd_REPORT_LUNS                    = 0xA0, /* SPC: Mandatory */
    kSSCPrinterCmd_REQUEST_SENSE                  = 0x03, /* SPC: Mandatory */
    kSSCPrinterCmd_RESERVE_6                      = 0x16, /* SPC: Mandatory */
    kSSCPrinterCmd_RESERVE_10                     = 0x56, /* SPC: Mandatory */
    kSSCPrinterCmd_SEND_DIAGNOSTICS               = 0x1D, /* SPC: Mandatory */
    kSSCPrinterCmd_SLEW_AND_PRINT                 = 0x0B, /* Sec. 6.2.4: Optional */
    kSSCPrinterCmd_STOP_PRINT                     = 0x1B, /* Sec. 6.2.5: Optional */
    kSSCPrinterCmd_SYNCHRONIZE_BUFFER             = 0x10, /* Sec. 6.2.6: Optional */
    kSSCPrinterCmd_TEST_UNIT_READY                = 0x00, /* SPC: Mandatory */
    kSSCPrinterCmd_WRITE_BUFFER                   = 0x3B  /* SPC: Optional */
};
  
#pragma mark -
#pragma mark 0x03 SPC Processor Commands
/* Commands defined by the T10:1236-D SCSI Primary Commands-2 (SPC-2) 
 * command specification.  The definitions and section numbers are based on
 * section 9 of the revision 18, 21 May 2000 version of the specification.
 */
enum 
{
    kSPCProcCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kSPCProcCmd_COMPARE                        = 0x39, /* Sec. 7.2: Optional */
    kSPCProcCmd_COPY                           = 0x18, /* Sec. 7.3: Optional */
    kSPCProcCmd_COPY_AND_VERIFY                = 0x3A, /* Sec. 7.4: Optional */
    kSPCProcCmd_EXTENDED_COPY                  = 0x83, /* Sec. 7.5: Optional */
    kSPCProcCmd_INQUIRY                        = 0x12, /* Sec. 7.6: Mandatory */
    kSPCProcCmd_LOG_SELECT                     = 0x4C, /* Sec. 7.7: Optional */
    kSPCProcCmd_LOG_SENSE                      = 0x4D, /* Sec. 7.8: Optional */
    kSPCProcCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* Sec. 7.13: Optional */
    kSPCProcCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* Sec. 7.14: Optional */
    kSPCProcCmd_READ_BUFFER                    = 0x3C, /* Sec. 7.16: Optional */
    kSPCProcCmd_RECEIVE                        = 0x08, /* Sec. 9.2: Optional */
    kSPCProcCmd_RECEIVE_COPY_RESULTS           = 0x84, /* Sec. 7.17: Optional */
    kSPCProcCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* Sec. 7.18: Optional */
    kSPCProcCmd_RELEASE_10                     = 0x57, /* Sec. 7.19: Optional */
    kSPCProcCmd_RELEASE_6                      = 0x17, /* Sec. 7.20: Optional */
    kSPCProcCmd_REPORT_LUNS                    = 0xA0, /* Sec. 7.22: Optional */
    kSPCProcCmd_REQUEST_SENSE                  = 0x03, /* Sec. 7.23: Mandatory */
    kSPCProcCmd_RESERVE_10                     = 0x56, /* Sec. 7.24: Optional */
    kSPCProcCmd_RESERVE_6                      = 0x16, /* Sec. 7.25: Optional */
    kSPCProcCmd_SEND                           = 0x0A, /* Sec. 9.3: Optional */
    kSPCProcCmd_SEND_DIAGNOSTICS               = 0x1D, /* Sec. 7.26: Mandatory */
    kSPCProcCmd_TEST_UNIT_READY                = 0x00, /* Sec. 7.27: Mandatory */
    kSPCProcCmd_WRITE_BUFFER                   = 0x3B  /* Sec. 7.29: Optional */
};

#pragma mark -
#pragma mark 0x04 SBC Write Once Commands
/* Commands defined by the T10:990-D SCSI-3 Block Commands (SBC) command
 * specification.  The definitions and section numbers are based on section 6.3
 * of the revision 8c, 13 November 1997 version of the specification. 
 */
enum
{
    kSBCWOCmd_CHANGE_DEFINITION              = 0x40, /* SPC: Optional */
    kSBCWOCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kSBCWOCmd_COPY                           = 0x18, /* SPC: Optional */
    kSBCWOCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional*/
    kSBCWOCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kSBCWOCmd_LOCK_UNLOCK_CACHE              = 0x36, /* Sec. 6.1.2: Optional */
    kSBCWOCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kSBCWOCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kSBCWOCmd_MEDIUM_SCAN                    = 0x38, /* Sec. 6.2.3: Optional */
    kSBCWOCmd_MODE_SELECT_6                  = 0x15, /* SPC: Optional */
    kSBCWOCmd_MODE_SELECT_10                 = 0x55, /* SPC: Optional */
    kSBCWOCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Optional */
    kSBCWOCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Optional */
    kSBCWOCmd_MOVE_MEDIUM                    = 0xA5, /* SMC: Optional */
    kSBCWOCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* SPC: Optional */
    kSBCWOCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* SPC: Optional */
    kSBCWOCmd_PREFETCH                       = 0x34, /* Sec. 6.1.3: Optional */
    kSBCWOCmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E, /* SPC: Optional */
    kSBCWOCmd_READ_6                         = 0x08, /* Sec. 6.1.4: Optional */
    kSBCWOCmd_READ_10                        = 0x28, /* Sec. 6.1.5: Mandatory */
    kSBCWOCmd_READ_12                        = 0xA8, /* Sec. 6.2.4: Optional */
    kSBCWOCmd_READ_BUFFER                    = 0x3C, /* SPC: Optional */
    kSBCWOCmd_READ_CAPACITY                  = 0x25, /* Sec. 6.1.6: Mandatory */
    kSBCWOCmd_READ_ELEMENT_STATUS            = 0xB8, /* SMC: Optional */
    kSBCWOCmd_READ_LONG                      = 0x3E, /* Sec. 6.1.8: Optional */
    kSBCWOCmd_REASSIGN_BLOCKS                = 0x07, /* Sec. 6.1.9: Optional */
    kSBCWOCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* SPC: Optional */
    kSBCWOCmd_RELEASE_6                      = 0x17, /* SPC: Optional */
    kSBCWOCmd_RELEASE_10                     = 0x57, /* SPC: Mandatory */
    kSBCWOCmd_REQUEST_SENSE                  = 0x03, /* SPC: Mandatory */
    kSBCWOCmd_RESERVE_6                      = 0x16, /* SPC: Optional */
    kSBCWOCmd_RESERVE_10                     = 0x56, /* SPC: Mandatory */
    kSBCWOCmd_REZERO_UNIT                    = 0x01, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_EQUAL_10           = 0x31, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_EQUAL_12           = 0xB1, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_HIGH_10            = 0x30, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_HIGH_12            = 0xB0, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_LOW_10             = 0x32, /* Obsolete */
    kSBCWOCmd_SEARCH_DATA_LOW_12             = 0xB2, /* Obsolete */
    kSBCWOCmd_SEEK_6                         = 0x0B, /* Obsolete */
    kSBCWOCmd_SEEK_10                        = 0x2B, /* Sec. 6.1.12: Optional */
    kSBCWOCmd_SEND_DIAGNOSTICS               = 0x1D, /* SPC: Mandatory */
    kSBCWOCmd_SET_LIMITS_10                  = 0x33, /* Sec. 6.1.13: Optional */
    kSBCWOCmd_SET_LIMITS_12                  = 0xB3, /* Sec. 6.2.8: Optional */
    kSBCWOCmd_START_STOP_UNIT                = 0x1B, /* Sec. 6.1.14: Optional */
    kSBCWOCmd_SYNCHRONIZE_CACHE              = 0x35, /* Sec. 6.1.15: Optional */
    kSBCWOCmd_TEST_UNIT_READY                = 0x00, /* SPC: Mandatory */
    kSBCWOCmd_VERIFY_10                      = 0x2F, /* Sec. 6.2.10: Optional */
    kSBCWOCmd_VERIFY_12                      = 0xAF, /* Sec. 6.2.11: Optional */
    kSBCWOCmd_WRITE_6                        = 0x0A, /* Sec. 6.1.17: Optional */
    kSBCWOCmd_WRITE_10                       = 0x2A, /* Sec. 6.2.10: Mandatory*/
    kSBCWOCmd_WRITE_12                       = 0xAA, /* Sec. 6.2.13: Optional */
    kSBCWOCmd_WRITE_AND_VERIFY_10            = 0x2E, /* Sec. 6.2.14: Optional */
    kSBCWOCmd_WRITE_AND_VERIFY_12            = 0xAE, /* Sec. 6.2.15: Optional */
    kSBCWOCmd_WRITE_BUFFER                   = 0x3B, /* SPC: Optional */
    kSBCWOCmd_WRITE_LONG                     = 0x3F  /* Sec. 6.1.20: Optional */
};

#pragma mark -
#pragma mark 0x05 MMC CD-ROM Commands
/* Commands defined by the T10:1363-D SCSI Multimedia Commands-3 (MMC-3) 
 * specification.  The definitions and section numbers are based on section 6.1
 * of the revision 01, March 03, 2000 version of the specification. 
 *
 * NOTE: The comments following each command may not be accurate.  These are
 * not from the MMC-3 specification, but have been derived from the SCSI-2 and
 * original MMC specifications.  Unlike the other SCSI command specifications,
 * MMC-2 and MMC-3 do not provide a command requirement type and therefore does
 * not relist the SPC commands with these requirements as they apply to MMC
 * devices.  The MMC-2 and MMC-3 specifications also refer back to the SBC
 * specification which seems invalid since MMC devices do not represent a
 * Peripheral Device Type defined by SBC.  It is assumed that the SBC
 * references refer to the Peripheral Device Type 0x00 - Direct Access Commands
 * definitions from that specification.
 */
enum 
{
    kMMCCmd_BLANK                          = 0xA1, /* Sec. 6.1.1: */
    kMMCCmd_CHANGE_DEFINITION              = 0x40, /* Obsolete */
    kMMCCmd_CLOSE_TRACK_SESSION            = 0x5B, /* Sec. 6.1.2: */
    kMMCCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kMMCCmd_COPY                           = 0x18, /* SPC: Optional */
    kMMCCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional */
    kMMCCmd_ERASE                          = 0x2C, /* SBC: */
    kMMCCmd_FORMAT_UNIT                    = 0x04, /* Sec. 6.1.3: */
    kMMCCmd_GET_CONFIGURATION              = 0x46, /* Sec. 6.1.4: */
    kMMCCmd_GET_EVENT_STATUS_NOTIFICATION  = 0x4A, /* Sec. 6.1.5: */
    kMMCCmd_GET_PERFORMANCE                = 0xAC, /* Sec. 6.1.6: */
    kMMCCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kMMCCmd_LOAD_UNLOAD_MEDIUM             = 0xA6, /* Sec. 6.1.7: */
    kMMCCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kMMCCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kMMCCmd_MECHANISM_STATUS               = 0xBD, /* Sec. 6.1.8: */
    kMMCCmd_MODE_SELECT_6                  = 0x15, /* SPC: Mandatory */
    kMMCCmd_MODE_SELECT_10                 = 0x55, /* SPC: Mandatory */
    kMMCCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Mandatory */
    kMMCCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Mandatory */
    kMMCCmd_PAUSE_RESUME                   = 0x4B, /* Sec. 6.1.9: */
    kMMCCmd_PLAY_AUDIO_10                  = 0x45, /* Sec. 6.1.10: */
    kMMCCmd_PLAY_AUDIO_12                  = 0xA5, /* Sec. 6.1.11: */
    kMMCCmd_PLAY_AUDIO_MSF                 = 0x47, /* Sec. 6.1.12: */
    kMMCCmd_PLAY_AUDIO_TRACK_INDEX         = 0x48, /* Obsolete */
    kMMCCmd_PLAY_CD                        = 0xBC, /* Sec. 6.1.13: */
    kMMCCmd_PLAY_RELATIVE_10               = 0x49, /* Obsolete */
    kMMCCmd_PLAY_RELATIVE_12               = 0xA9, /* Obsolete */
    kMMCCmd_PREFETCH                       = 0x34, /* Optional */
    kMMCCmd_PREVENT_ALLOW_MEDIUM_REMOVAL   = 0x1E, /* Optional */
    kMMCCmd_READ_6                         = 0x08, /* Optional */
    kMMCCmd_READ_10                        = 0x28, /* Mandatory */
    kMMCCmd_READ_12                        = 0xA8, /* Optional */
    kMMCCmd_READ_BUFFER                    = 0x3C, /* Optional */
    kMMCCmd_READ_BUFFER_CAPACITY           = 0x5C, /* Sec. 6.1.15: */
    kMMCCmd_READ_CD                        = 0xBE, /* Sec. 6.1.16: */
    kMMCCmd_READ_CD_MSF                    = 0xB9, /* Sec. 6.1.17: */
    kMMCCmd_READ_CAPACITY                  = 0x25, /* Sec. 6.1.18: */
    kMMCCmd_READ_DISC_INFORMATION          = 0x51, /* Sec. 6.1.19: */
    kMMCCmd_READ_DVD_STRUCTURE             = 0xAD, /* Sec. 6.1.20: */
    kMMCCmd_READ_DISC_STRUCTURE            = 0xAD, /* Sec. 6.1.20: */
    kMMCCmd_READ_FORMAT_CAPACITIES         = 0x23, /* Sec. 6.1.21: */
    kMMCCmd_READ_HEADER                    = 0x44, /* Sec. 6.1.22: */
    kMMCCmd_READ_LONG                      = 0x3E, /* Optional */
    kMMCCmd_READ_MASTER_CUE                = 0x59, /* Sec. 6.1.23: */
    kMMCCmd_READ_SUB_CHANNEL               = 0x42, /* Sec. 6.1.24: */
    kMMCCmd_READ_TOC_PMA_ATIP              = 0x43, /* Sec. 6.1.25: */
    kMMCCmd_READ_TRACK_INFORMATION         = 0x52, /* Sec. 6.1.27: */
    kMMCCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* Optional */
    kMMCCmd_RELEASE_6                      = 0x17, /* Mandatory */
    kMMCCmd_RELEASE_10                     = 0x57, /* Optional */
    kMMCCmd_REPAIR_TRACK                   = 0x58, /* Sec. 6.1.28: */
    kMMCCmd_REPORT_KEY                     = 0xA4, /* Sec. 6.1.29: */
    kMMCCmd_REQUEST_SENSE                  = 0x03, /* Mandatory */
    kMMCCmd_RESERVE_6                      = 0x16, /* Mandatory */
    kMMCCmd_RESERVE_10                     = 0x56, /* Optional */
    kMMCCmd_RESERVE_TRACK                  = 0x53, /* Sec. 6.1.30: */
    kMMCCmd_SCAN_MMC                       = 0xBA, /* Sec. 6.1.31: */
    kMMCCmd_SEARCH_DATA_EQUAL_10           = 0x31, /* Obsolete */
    kMMCCmd_SEARCH_DATA_EQUAL_12           = 0xB1, /* Obsolete */
    kMMCCmd_SEARCH_DATA_HIGH_10            = 0x30, /* Obsolete */
    kMMCCmd_SEARCH_DATA_HIGH_12            = 0xB0, /* Obsolete */
    kMMCCmd_SEARCH_DATA_LOW_10             = 0x32, /* Obsolete */
    kMMCCmd_SEARCH_DATA_LOW_12             = 0xB2, /* Obsolete */
    kMMCCmd_SEEK_6                         = 0x0B, /* Obsolete */
    kMMCCmd_SEEK_10                        = 0x2B, /* SBC: */
    kMMCCmd_SEND_CUE_SHEET                 = 0x5D, /* Sec. 6.1.32: */
    kMMCCmd_SEND_DIAGNOSTICS               = 0x1D, /* Mandatory */
    kMMCCmd_SEND_DVD_STRUCTURE             = 0xBF, /* Sec. 6.1.33: */
    kMMCCmd_SEND_EVENT                     = 0xA2, /* Sec. 6.1.34: */
    kMMCCmd_SEND_KEY                       = 0xA3, /* Sec. 6.1.35: */
    kMMCCmd_SEND_OPC_INFORMATION           = 0x54, /* Sec. 6.1.36: */
    kMMCCmd_SET_CD_SPEED                   = 0xBB, /* Sec. 6.1.37: */
    kMMCCmd_SET_LIMITS_10                  = 0x33, /* Optional */
    kMMCCmd_SET_LIMITS_12                  = 0xB3, /* Optional */
    kMMCCmd_SET_READ_AHEAD                 = 0xA7, /* Sec. 6.1.38: */
    kMMCCmd_SET_STREAMING                  = 0xB6, /* Sec. 6.1.39: */
    kMMCCmd_START_STOP_UNIT                = 0x1B, /* Optional */
    kMMCCmd_STOP_PLAY_SCAN                 = 0x4E, /* Sec. 6.1.40: */
    kMMCCmd_SYNCHRONIZE_CACHE              = 0x35, /* Sec. 6.1.41: */
    kMMCCmd_TEST_UNIT_READY                = 0x00, /* Mandatory */
    kMMCCmd_VERIFY_10                      = 0x2F, /* Optional */
    kMMCCmd_VERIFY_12                      = 0xAF, /* Optional */
    kMMCCmd_WRITE_10                       = 0x2A, /* Sec. 6.1.42: */
    kMMCCmd_WRITE_12                       = 0xAA, /* Sec. 6.1.43: */
    kMMCCmd_WRITE_AND_VERIFY_10            = 0x2E, /* Sec. 6.1.44: */
    kMMCCmd_WRITE_BUFFER                   = 0x3B  /* Optional */
};

#pragma mark -
#pragma mark 0x06 SGC Scanner Commands
/* Commands defined by the T10:998-D SCSI-3 Graphics Commands (SGC)
 * specification.  The definitions and section numbers are based on section 6
 * of the revision 0, April 1995 version of the specification. 
 */
enum 
{
    kSGCCmd_CHANGE_DEFINITION              = 0x40, /* SPC: Optional */
    kSGCCmd_COMPARE                        = 0x39, /* SPC: Optional */
    kSGCCmd_COPY                           = 0x18, /* SPC: Optional */
    kSGCCmd_COPY_AND_VERIFY                = 0x3A, /* SPC: Optional */
    kSGCCmd_GET_DATA_BUFFER_STATUS         = 0x34, /* Sec. 6.1.1: Optional */
    kSGCCmd_GET_WINDOW                     = 0x25, /* Sec. 6.1.2: Optional */
    kSGCCmd_INQUIRY                        = 0x12, /* SPC: Mandatory */
    kSGCCmd_LOG_SELECT                     = 0x4C, /* SPC: Optional */
    kSGCCmd_LOG_SENSE                      = 0x4D, /* SPC: Optional */
    kSGCCmd_MODE_SELECT_6                  = 0x15, /* SPC: Optional */
    kSGCCmd_MODE_SELECT_10                 = 0x55, /* SPC: Optional */
    kSGCCmd_MODE_SENSE_6                   = 0x1A, /* SPC: Optional */
    kSGCCmd_MODE_SENSE_10                  = 0x5A, /* SPC: Optional */
    kSGCCmd_OBJECT_POSITION                = 0x31, /* Sec. 6.1.3: Optional */
    kSGCCmd_PORT_STATUS                    = 0x11, /* SPC (??): Mandatory
													* for dual port devices */
    kSGCCmd_READ                           = 0x28, /* Sec. 6.1.4: Mandatory */
    kSGCCmd_READ_BUFFER                    = 0x3C, /* SPC: Optional */
    kSGCCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x1C, /* SPC: Optional */
    kSGCCmd_RELEASE_6                      = 0x17, /* SPC: Mandatory */
    kSGCCmd_REQUEST_SENSE                  = 0x03, /* SPC: Mandatory */
    kSGCCmd_RESERVE_6                      = 0x16, /* SPC: Mandatory */
    kSGCCmd_SCAN                           = 0x1B, /* Sec. 6.1.5: Optional */
    kSGCCmd_SEND                           = 0x1B, /* Sec. 6.1.6: Optional */
    kSGCCmd_SEND_DIAGNOSTICS               = 0x1D, /* SPC: Mandatory */
    kSGCCmd_SET_WINDOW                     = 0x24, /* Sec. 6.1.7: Mandatory */
    kSGCCmd_TEST_UNIT_READY                = 0x00, /* SPC: Mandatory */
    kSGCCmd_WRITE_BUFFER                   = 0x3B  /* SPC: Optional */
};

#pragma mark -
#pragma mark 0x07 SBC Optical Media Commands
/* Commands defined by the T10:990-D SCSI-3 Block Commands (SBC) 
 * (revision 8c, 13 November 1998) command specification.  
 */

#pragma mark -
#pragma mark 0x08 SMC Medium Changer Commands
/* Commands defined by the T10:1228-D SCSI-3 Medium Changer Commands-2 (SMC-2)
 * (revision 0, March 16, 2000) command specification.  
 */
enum
{
    /* Commands For Independent Medium Changers */
    kSMCCmd_EXCHANGE_MEDIUM                = 0xA6, /* Optional */
    kSMCCmd_INITIALIZE_ELEMENT_STATUS      = 0x07, /* Optional */
    kSMCCmd_MODE_SELECT_6                  = 0x15, /* Optional */
    kSMCCmd_MODE_SELECT_10                 = 0x55, /* Optional */
    kSMCCmd_MODE_SENSE_6                   = 0x1A, /* Optional */
    kSMCCmd_MODE_SENSE_10                  = 0x5A, /* Optional */
    kSMCCmd_MOVE_MEDIUM                    = 0xA5, /* Mandatory */
    kSMCCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* Optional */
    kSMCCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* Optional */
    kSMCCmd_POSITION_TO_ELEMENT            = 0x2B, /* Optional */
    kSMCCmd_READ_ELEMENT_STATUS            = 0xB8, /* Mandatory */
    kSMCCmd_RELEASE_ELEMENT_6              = 0x16, /* Optional */
    kSMCCmd_RELEASE_ELEMENT_10             = 0x56, /* Optional */
    kSMCCmd_REQUEST_VOLUME_ELEMENT_ADDRESS = 0xB5, /* Optional */
    kSMCCmd_REQUEST_SENSE                  = 0x03, /* Mandatory */
    kSMCCmd_RESERVE_ELEMENT_6              = 0x16, /* Optional */
    kSMCCmd_RESERVE_ELEMENT_10             = 0x56  /* Optional */
};

#pragma mark -
#pragma mark 0x09 SSC Communications Commands
/* Commands defined by the T10:997-D SCSI-3 Stream Commands (SSC) 
 * (revision 22, January 1, 2000) command specification. 
 */
  
#pragma mark -
#pragma mark 0x0A ASC IT8 Prepress Commands
#pragma mark 0x0B ASC IT8 Prepress Commands
/* Commands defined by the ASC IT8 <title goes here> specification
 * (revision xx, month day, year) command specification.  
 */
#if 0
enum
{
};
#endif

#pragma mark -
#pragma mark 0x0C SCC Array Controller Commands
/* Commands defined by the ANSI NCITS.318-199x SCSI Controller
 * Commands (SCC-2) ratified command specification.  
 */
enum
{
    kSCCCmd_MAINTENANCE_IN                 = 0xA3, /* Mandatory */
    kSCCCmd_MAINTENANCE_OUT                = 0xA4, /* Optional */
    kSCCCmd_MODE_SELECT_6                  = 0x15, /* Optional */
    kSCCCmd_MODE_SELECT_10                 = 0x55, /* Optional */
    kSCCCmd_MODE_SENSE_6                   = 0x1A, /* Optional */
    kSCCCmd_MODE_SENSE_10                  = 0x5A, /* Optional */
    kSCCCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* Optional */
    kSCCCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* Optional */
    kSCCCmd_PORT_STATUS                    = 0x1F, /* Optional */
    kSCCCmd_REDUNDANCY_GROUP_IN            = 0xBA, /* Mandatory */
    kSCCCmd_REDUNDANCY_GROUP_OUT           = 0xBB, /* Optional */
    kSCCCmd_RELEASE_6                      = 0x17, /* Optional */
    kSCCCmd_RELEASE_10                     = 0x57, /* Optional */
    kSCCCmd_REPORT_LUNS                    = 0xA0, /* Mandatory */
    kSCCCmd_REQUEST_SENSE                  = 0x03, /* Mandatory */
    kSCCCmd_RESERVE_6                      = 0x16, /* Optional */
    kSCCCmd_RESERVE_10                     = 0x56, /* Optional*/
    kSCCCmd_SEND_DIAGNOSTICS               = 0x1D, /* Optional */
    kSCCCmd_SPARE_IN                       = 0xBC, /* Mandatory */
    kSCCCmd_SPARE_OUT                      = 0xBD  /* Optional */
};

#pragma mark -
#pragma mark 0x0D SES Enclosure Services Commands
/* Commands defined by the T10:1212-D SCSI-3 Enclosure Services (SES) 
 * (revision 8b, February 11, 1998) command specification.  
 */
enum
{
    kSESCmd_MODE_SELECT_6                  = 0x15, /* Optional */
    kSESCmd_MODE_SELECT_10                 = 0x55, /* Optional */
    kSESCmd_MODE_SENSE_6                   = 0x1A, /* Optional */
    kSESCmd_MODE_SENSE_10                  = 0x5A, /* Optional */
    kSESCmd_PERSISTENT_RESERVE_IN          = 0x5E, /* Optional */
    kSESCmd_PERSISTENT_RESERVE_OUT         = 0x5F, /* Optional */
    kSESCmd_RECEIVE_DIAGNOSTICS_RESULTS    = 0x17, /* Mandatory */
    kSESCmd_RELEASE_6                      = 0x17, /* Optional */
    kSESCmd_RELEASE_10                     = 0x57, /* Optional */
    kSESCmd_REQUEST_SENSE                  = 0x03, /* Mandatory */
    kSESCmd_RESERVE_6                      = 0x16, /* Optional */
    kSESCmd_RESERVE_10                     = 0x56, /* Optional */
    kSESCmd_SEND_DIAGNOSTICS               = 0x1D  /* Mandatory */
};

#pragma mark -
#pragma mark 0x0E RBC Reduced Block Commands
/* Commands defined by the T10:1240-D Reduced Block Commands (RBC) 
 * (revision 10a, August 18, 1999) command specification.  
 */
enum
{
    kRBCCmd_FORMAT_UNIT                    = 0x04, /* Optional */
    kRBCCmd_READ_10                        = 0x28, /* Mandatory */
    kRBCCmd_READ_CAPACITY                  = 0x25, /* Mandatory */
    kRBCCmd_START_STOP_UNIT                = 0x1B, /* Mandatory */
    kRBCCmd_SYNCHRONIZE_CACHE              = 0x35, /* Optional */
    kRBCCmd_VERIFY_10                      = 0x2F, /* Mandatory */
    kRBCCmd_WRITE_10                       = 0x2A, /* Mandatory */
    kRBCCmd_WRITE_BUFFER                   = 0x3B  /* Mandatory for fixed media
												    * Optional for removable */
};

#pragma mark -
#pragma mark 0x0F OCRW Optical Card Commands
/* Commands defined by the ISO/IEC 14776-381 SCSI Specification for
 * Optical Card Reader/Writer (OCRW) ratified command specification.  
 */
#if 0
enum
{
};
#endif

#pragma mark -
#pragma mark 0x11 OSD Object-based Storage Commands
/* Commands defined by the T10:1355-D Object-based Storage Commands (OSD) 
 * (revision 1, 18 May 2000) command specification.  
 */
#if 0
enum
{
};
#endif

#pragma mark -
#pragma mark 0x15 RMC Simplified Multimedia Commands
/* Commands defined by the T10:1364-D Reduced Multimedia Commands (RMC) 
 * (revision 1, November 11, 1999) command specification.  
 */
#if 0
enum
{
};
#endif

#endif	/* _SCSI_COMMAND_OPERATION_CODES_H_ */
