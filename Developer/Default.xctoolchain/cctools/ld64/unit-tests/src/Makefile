
all: ../bin/results-to-xml ../bin/xmlparser

../bin/results-to-xml: results-to-xml.cpp
	g++ -g -O -Wall $< -o ../bin/results-to-xml

../bin/xmlparser:
	cd xmlparser; xcodebuild -alltargets
	cp -p xmlparser/build/Release/xmlparser ../bin/.
