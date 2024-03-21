unset CDPATH

MYDIR=$(pwd -P)
FULLDIR=/tmp/bash-dir-a
DIR=${FULLDIR##*/}

mkdir $FULLDIR
CDPATH=.:/tmp
cd $DIR
pwd
echo $PWD

cd "$MYDIR"
rmdir $FULLDIR
