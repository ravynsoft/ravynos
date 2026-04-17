def HexByte(byte):
	byte = ord(byte)
	digits = "0123456789ABCDEF"
	return digits[byte >> 4] + digits[byte & 0x0F]

def HexDump(bytes, offset=0, bytesPerLine=32, offsetFormat="%08X: ", verbose=False):
	printable = "." * 32 + "".join(map(chr, range(32,127))) + "." * 129
	if offsetFormat is None or offset is None:
		offsetFormat = ""
	length = len(bytes)
	index = 0
	lastLine = ""
	skipping = False
	while index < length:
		if "%" in offsetFormat:
			offStr = offsetFormat % offset
		else:
			offStr = ""
		line = bytes[index:index+bytesPerLine]
		if line == lastLine and not verbose:
			if not skipping:
				print "*"
				skipping = True
		else:
			hex = " ".join(map(HexByte, line))
			ascii = line.translate(printable)
			print "%s%-*s |%s|" % (offStr, 3*bytesPerLine, hex, ascii)
			lastLine = line
			skipping = False
		index += bytesPerLine
		offset += bytesPerLine
	if skipping and ("%" in offsetFormat):
		print offsetFormat % offset
