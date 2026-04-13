//
//  lf_hfs_file_extent_mapping.h
//  hfs
//
//  Created by Yakov Ben Zaken on 22/03/2018.
//

#ifndef lf_hfs_file_extent_mapping_h
#define lf_hfs_file_extent_mapping_h

/*    File Extent Mapping routines*/
OSErr FlushExtentFile( ExtendedVCB *vcb );

int32_t CompareExtentKeysPlus( const HFSPlusExtentKey *searchKey, const HFSPlusExtentKey *trialKey );

OSErr SearchExtentFile( ExtendedVCB             *vcb,
                       const FCB               *fcb,
                       int64_t                 filePosition,
                       HFSPlusExtentKey        *foundExtentKey,
                       HFSPlusExtentRecord     foundExtentData,
                       u_int32_t               *foundExtentDataIndex,
                       u_int32_t               *extentBTreeHint,
                       u_int32_t               *endingFABNPlusOne );

OSErr TruncateFileC(    ExtendedVCB         *vcb,
                    FCB                 *fcb,
                    int64_t             peof,
                    int                 deleted,
                    int                 rsrc,
                    uint32_t            fileid,
                    Boolean             truncateToExtent );

OSErr ExtendFileC(  ExtendedVCB     *vcb,
                  FCB             *fcb,
                  int64_t         bytesToAdd,
                  u_int32_t       blockHint,
                  u_int32_t       flags,
                  int64_t         *actualBytesAdded );

OSErr MapFileBlockC(    ExtendedVCB     *vcb,
                    FCB             *fcb,
                    size_t          numberOfBytes,
                    off_t           offset,
                    daddr64_t       *startBlock,
                    size_t          *availableBytes );

OSErr HeadTruncateFile( ExtendedVCB     *vcb,
                       FCB             *fcb,
                       u_int32_t       headblks );

int AddFileExtent(  ExtendedVCB     *vcb,
                  FCB             *fcb,
                  u_int32_t       startBlock,
                  u_int32_t       blockCount );

Boolean NodesAreContiguous( ExtendedVCB     *vcb,
                           FCB             *fcb,
                           u_int32_t       nodeSize );

#endif /* lf_hfs_file_extent_mapping_h */
