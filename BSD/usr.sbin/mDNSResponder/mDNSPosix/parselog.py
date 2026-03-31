#!/usr/bin/python
# Emacs settings: -*- tab-width: 4 -*-
#
# Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# parselog.py, written and contributed by Kevin Marks
#
# Requires OS X 10.3 Panther or later, for Python and Core Graphics Python APIs
# Invoke from the command line with "parselog.py fname" where fname is a log file made by mDNSNetMonitor
#
# Caveats:
# It expects plain ASCII, and doesn't handle spaces in record names very well right now
# There's a procedure you can follow to 'sanitize' an mDNSNetMonitor log file to make it more paletable to parselog.py:
# 1. Run mDNSNetMonitor in a terminal window.
#    When you have enough traffic, type Ctrl-C and save the content of the terminal window to disk.
#    Alternatively, you can use "mDNSNetMonitor > logfile" to write the text directly to a file.
#    You now have a UTF-8 text file.
# 2. Open the UTF-8 text file using BBEdit or some other text editor.
#    (These instructions are for BBEdit, which I highly recommend you use when doing this.)
# 3. Make sure BBEdit correctly interprets the file as UTF-8.
#    Either set your "Text Files Opening" preference to "UTF-8 no BOM", and drop the file onto BBEdit,
#    or manually open the File using "File -> Open" and make sure the "Read As" setting is set to "UTF-8 no BOM"
#    Check in the document pulldown menu in the window toolbar to make sure that it says "Encoding: UTF-8 no BOM"
# 4. Use "Tools -> Convert to ASCII" to replace all special characters with their seven-bit ascii equivalents.
#    (e.g. curly quotes are converted to straight quotes)
# 5. Do a grep search and replace. (Cmd-F; make sure Grep checkbox is turned on.)
#    Enter this search text     : ^(.................\(................\S*) (.* -> .*)$
#    Enter this replacement text: \1-\2
#    Click "Replace All"
#    Press Cmd-Opt-= repeatedly until there are no more instances to be replaced.
#    You now have text file with all spaces in names changed to hyphens
# 6. Save the new file. You can save it as "UTF-8 no BOM", or as "Mac Roman". It really doesn't matter which --
#    the file now contains only seven-bit ascii, so it's all the same no matter how you save it.
# 7. Run "parselog.py fname"
# 8. Open the resulting fname.pdf file with a PDF viewer like Preview on OS X
#
# Key to what you see:
# Time is on the horizontal axis
# Individual machines are shown on the vertical axis
# Filled red    circle: Normal query              Hollow red    circle: Query requesting unicast reply
# Filled orange circle: Probe (service starting)  Hollow orange circle: First probe (requesting unicast reply)
# Filled green  circle: Normal answer             Hollow green  circle: Goodbye message (record going away)
#                                                 Hollow blue   circle: Legacy query (from old client)

from CoreGraphics import *
import math   # for pi

import string
import sys, os
import re

def parselog(inFile):
	f = open(inFile)
	hunt = 'getTime'
	ipList = {}
	querySource = {}
	plotPoints = []
	maxTime=0
	minTime = 36*60*60
	spaceExp = re.compile(r'\s+')
	print "Reading " + inFile
	while 1:
		lines = f.readlines(100000)
		if not lines:
			break
		for line in lines:
			if (hunt == 'skip'):
				if (line == '\n' or line == '\r' or line ==''): 
					hunt = 'getTime'
#				else:
#					msg = ("skipped" , line)
#					print msg
			elif (hunt == 'getTime'):
				if (line == "^C\n" ):
					break
				time = line.split(' ')[0].split(':')
				if (len(time)<3):
					#print "bad time, skipping",time
					hunt = 'skip'
				else:
					hunt = 'getIP'
				#print (("getTime:%s" % (line)), time)
			elif (hunt == 'getIP'):
				ip = line.split(' ',1)
				ip = ip[0]
				secs=0
				for t in time:
					secs = secs*60 +float(t)
				if (secs>maxTime):
					maxTime=secs
				if (secs<minTime):
					minTime=secs
				if (not ip in ipList):
					ipList[ip] = [len(ipList), "", ""]
				#print (("getIP:%s" % (line)), time, secs)
				hunt = 'getQA'
			elif (hunt == 'getQA'):
				qaList = spaceExp.split(line)
				# qaList[0] Source Address
				# qaList[1] Operation type (PU/PM/QU/QM/AN etc.)
				# qaList[2] Record type (PTR/SRV/TXT etc.)
				# For QU/QM/LQ:
				# qaList[3] RR name
				# For PU/PM/AN/AN+/AD/AD+/KA:
				# qaList[3] TTL
				# qaList[4] RR name
				# qaList[5...] "->" symbol and following rdata
				#print qaList
				if (qaList[0] == ip):
					if (qaList[1] == '(QU)' or qaList[1] == '(LQ)' or qaList[1] == '(PU)'):
						plotPoints.append([secs, ipList[ip][0], (qaList[1])[1:-1]])
					elif (qaList[1] == '(QM)'):
						plotPoints.append([secs, ipList[ip][0], (qaList[1])[1:-1]])
						querySource[qaList[3]] = len(plotPoints)-1
					elif (qaList[1] == '(PM)'):
						plotPoints.append([secs, ipList[ip][0], (qaList[1])[1:-1]])
						querySource[qaList[4]] = len(plotPoints)-1
					elif (qaList[1] == '(AN)' or qaList[1] == '(AN+)' or qaList[1] == '(DE)'):
						plotPoints.append([secs, ipList[ip][0], (qaList[1])[1:-1]])
						try:
							theQuery = querySource[qaList[4]]
							theDelta = secs - plotPoints[theQuery][0]
							if (theDelta < 1.0):
								plotPoints[-1].append(querySource[qaList[4]])
							#print "Answer AN+ %s points to %d" % (qaList[4],querySource[qaList[4]])
						except:
							#print "Couldn't find any preceeding question for", qaList
							pass
					elif (qaList[1] != '(KA)' and qaList[1] != '(AD)' and qaList[1] != '(AD+)'):
						print "Operation unknown", qaList

					if (qaList[1] == '(AN)' or qaList[1] == '(AN+)' or qaList[1] == '(AD)' or qaList[1] == '(AD+)'):
						if (qaList[2] == 'HINFO'):
							ipList[ip][1] = qaList[4]
							ipList[ip][2] = string.join(qaList[6:])
							#print ipList[ip][1]
						elif (qaList[2] == 'AAAA'):
							if (ipList[ip][1] == ""):
								ipList[ip][1] = qaList[4]
								ipList[ip][2] = "Panther"
						elif (qaList[2] == 'Addr'):
							if (ipList[ip][1] == ""):
								ipList[ip][1] = qaList[4]
								ipList[ip][2] = "Jaguar"
				else:
					if (line == '\n'): 
						hunt = 'getTime'
					else:
						hunt = 'skip'
	f.close()
	#print plotPoints
	#print querySource
	#width=20.0*(maxTime-minTime)
	if (maxTime < minTime + 10.0):
		maxTime = minTime + 10.0
	typesize = 12
	width=20.0*(maxTime-minTime)
	pageHeight=(len(ipList)+1) * typesize
	scale = width/(maxTime-minTime)
	leftMargin = typesize * 60
	bottomMargin = typesize
	pageRect = CGRectMake (-leftMargin, -bottomMargin,  leftMargin + width, bottomMargin + pageHeight)   #  landscape
	outFile = "%s.pdf" % (".".join(inFile.split('.')[:-1]))
	c = CGPDFContextCreateWithFilename (outFile, pageRect)
	print "Writing " + outFile
	ourColourSpace = c.getColorSpace()
	# QM/QU red    solid/hollow
	# PM/PU orange solid/hollow
	# LQ    blue         hollow
	# AN/DA green  solid/hollow
	#colourLookup = {"L":(0.0,0.0,.75), "Q":(.75,0.0,0.0), "P":(.75,0.5,0.0), "A":(0.0,0.75,0.0), "D":(0.0,0.75,0.0), "?":(.25,0.25,0.25)}
	colourLookup = {"L":(0.0,0.0,1.0), "Q":(1.0,0.0,0.0), "P":(1.0,0.8,0.0), "A":(0.0,1.0,0.0), "D":(0.0,1.0,0.0), "?":(1.0,1.0,1.0)}
	c.beginPage (pageRect)
	c.setRGBFillColor(.75,0.0,0.0,1.0)
	c.setRGBStrokeColor(.25,0.75,0.25,1.0)
	c.setLineWidth(0.25)
	for point in plotPoints:
		#c.addArc((point[0]-minTime)*scale,point[1]*typesize+6,5,0,2*math.pi,1)
		c.addArc((point[0]-minTime)*scale,point[1]*typesize+6,typesize/4,0,2*math.pi,1)
		theColour = colourLookup[(point[2])[0]]
		if (((point[2])[0]) != "L") and (((point[2])[0]) != "Q") and (((point[2])[0]) != "P") and (((point[2])[0]) != "A") and (((point[2])[0]) != "D"):
			print "Unknown", point
		if ((point[2])[-1] == "M" or (point[2])[0]== "A"):
			c.setRGBFillColor(theColour[0],theColour[1],theColour[2],.5)
			c.fillPath()
		else:
			c.setRGBStrokeColor(theColour[0],theColour[1],theColour[2],.5)
			c.setLineWidth(1.0)
			c.strokePath()
		c.setRGBStrokeColor(.25,0.75,0.25,1.0)
		c.setLineWidth(0.25)
		for index in point[3:]:
			c.beginPath()
			c.moveToPoint((point[0]-minTime)*scale,point[1]*typesize+6)
			c.addLineToPoint(((plotPoints[index])[0]-minTime)*scale,(plotPoints[index])[1]*typesize+6)
			c.closePath()
			c.strokePath()
	c.setRGBFillColor (0,0,0, 1)
	c.setTextDrawingMode (kCGTextFill)
	c.setTextMatrix (CGAffineTransformIdentity)
	c.selectFont ('Gill Sans', typesize, kCGEncodingMacRoman)
	c.setRGBStrokeColor(0.25,0.0,0.0,1.0)
	c.setLineWidth(0.1)
	for ip,[height,hname,hinfo] in ipList.items():
		c.beginPath()
		c.moveToPoint(pageRect.origin.x,height*typesize+6)
		c.addLineToPoint(width,height*typesize+6)
		c.closePath()
		c.strokePath()
		c.showTextAtPoint(pageRect.origin.x + 2,               height*typesize + 2, ip,    len(ip))
		c.showTextAtPoint(pageRect.origin.x + 2 + typesize*8,  height*typesize + 2, hname, len(hname))
		c.showTextAtPoint(pageRect.origin.x + 2 + typesize*25, height*typesize + 2, hinfo, len(hinfo))
	for time in range (int(minTime),int(maxTime)+1):
		c.beginPath()
		c.moveToPoint((time-minTime)*scale,pageRect.origin.y)
		c.addLineToPoint((time-minTime)*scale,pageHeight)
		c.closePath()
		if (int(time) % 10 == 0):
			theHours = time/3600
			theMinutes = time/60 % 60
			theSeconds = time % 60
			theTimeString = '%d:%02d:%02d' % (theHours, theMinutes, theSeconds)
			# Should measure string width, but don't know how to do that
			theStringWidth = typesize * 3.5
			c.showTextAtPoint((time-minTime)*scale - theStringWidth/2, pageRect.origin.y + 2, theTimeString, len(theTimeString))
			c.setLineWidth(0.3)
		else:
			c.setLineWidth(0.1)
		c.strokePath()
	c.endPage()
	c.finish()

			
for arg in sys.argv[1:]:
	parselog(arg)
