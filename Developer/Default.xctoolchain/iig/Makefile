CXX := ${HOST_CXX}
SRCS = iig.cpp
OBJS = iig.o
CXXFLAGS += -std=gnu++17

all: ${TOOLS}/iig

${TOOLS}/iig: ${OBJS}
	${CXX} -o ${.TARGET} ${OBJS}

