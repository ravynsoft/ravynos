# Select an appropriate endinaness based on the value of target.  When
# building for all targets we select little endian, which is what the
# '*' pattern is for.
case ${target} in
    arceb-*)
	ARC_ENDIAN="big"
	;;
    arc-* | * )
	ARC_ENDIAN="little"
	;;
esac
