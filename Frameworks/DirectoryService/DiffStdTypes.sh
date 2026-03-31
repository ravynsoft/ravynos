#!/bin/sh

# Get the standard types in the API headers
grep "kDSStdRecordType.*dsRecTypeStandard:" APIFramework/DirServicesConst* | grep -v kDSStdRecordTypePrefix | grep -v kDSStdRecordTypeAll | sed 's/.*\"dsRecTypeStandard:/dsRecTypeStandard:/;s/\"//' | sort > /tmp/DSAPIStdTypes.txt

# Get the standard types that are in the DS Local plug-in's Mappings file
grep "dsRecTypeStandard:" Plugins/Local/RecordMappings.plist | grep -v "<string>" | sed 's/.*<key>//;s/<\/key>//' | sort > /tmp/DSLocalStdTypes.txt

# helpful text
echo ""
echo "--- == The Record Type is not in the DS Local RecordMappings.plist file"
echo ""

# diff
diff -uw /tmp/DSAPIStdTypes.txt /tmp/DSLocalStdTypes.txt | grep -e "[+|-]"

# clean up
rm /tmp/DSAPIStdTypes.txt /tmp/DSLocalStdTypes.txt
