#!/bin/bash

workdir=$(mktemp -d)

cp autoload $workdir

cd $workdir
pwd

. ./autoload

funclist='ALTEST_func1 ALTEST_funcexport ALTEST_funcu'
for funcname in $funclist; do
    cat <<EOFFUNC > $funcname
$funcname ()
{
echo this is $funcname

}
EOFFUNC

done

export FPATH=$workdir

autoload ALTEST_func1 ALTEST_funcu
autoload -x ALTEST_funcexport

ok=0
failed=0

for funcname in $funclist; do

    testname="$funcname loaded"
    got=$(type $funcname 2>&1)
    if [[ $got =~ "$funcname: not found" ]]; then
        echo "## Failed $testname"
        ((failed+=1))
    else
        echo "ok - $testname"
        ((ok+=1))

        testname="$funcname is a shim"
        if [[ ! $got =~ "IS_SHIM" ]]; then
            echo "## Failed $testname"
            ((failed+=1))
        else
            echo "ok - $testname"
            ((ok+=1))

            testname="$funcname shim executed"
            $funcname > /dev/null
            got=$(type $funcname 2>&1)
            if [[ $got =~ "IS_SHIM" ]]; then
                echo "## Failed $testname"
                ((failed+=1))
            else
                echo "ok - $testname"
                ((ok+=1))
            fi
        fi
    fi
done

funcname=ALTEST_func1
testname="$funcname shim reloaded"
autoload -r $funcname
got=$(type $funcname 2>&1)
if [[ ! $got =~ "IS_SHIM" ]]; then
    echo "## Failed $testname"
    ((failed+=1))
else
    echo "ok - $testname"
    ((ok+=1))
fi

funcname=ALTEST_funcu
testname="$funcname shim unloaded"
autoload -u $funcname
got=$(type $funcname 2>&1)
if [[ ! $got =~ "$funcname: not found" ]]; then
    echo "## Failed $testname"
    ((failed+=1))
else
    echo "ok - $testname"
    ((ok+=1))
fi

testname="autoload -p"
got=$(autoload -p | grep ALTEST)
if [[ ! $got =~ "autoload ALTEST_func1" ]] || \
       [[ ! $got =~ "autoload ALTEST_funcexport" ]] ; then
echo "## Failed $testname"
    ((failed+=1))
else
    echo "ok - $testname"
    ((ok+=1))
fi

testname="autoload -s"
echo "Executing $testname, could take a long time..."
got=$(autoload -s | grep ALTEST)
if [[ ! $got =~ "ALTEST_func1 not exported not executed" ]] || \
       [[ ! $got =~ "ALTEST_funcexport exported executed" ]] ; then
    echo "## Failed $testname"
    echo "##    got: $got"
    ((failed+=1))
else
    echo "ok - $testname"
    ((ok+=1))
fi

testname="autoload -r -a $FPATH"
autoload -r -a $FPATH
localfailed=0
localok=0
for funcname in $funclist; do
    got=$(type $funcname 2>&1)
    if [[ $got =~ "$funcname: not found" ]]; then
        echo "## Failed $testname - $funcname"
        ((localfailed+=1))
    else
        ((localok+=1))
        if [[ ! $got =~ "IS_SHIM" ]]; then
            ((localfailed+=1))
        else
            ((localok+=1))
        fi
    fi
done
if ((localfailed==0)); then
    echo "ok - $testname"
    ((ok+=1))
else
    ((failed+=1))
fi

testname="autoload -u $funclist"
autoload -u $funclist
localfailed=0
localok=0
for funcname in $funclist; do
    got=$(type $funcname 2>&1)
    if [[ ! $got =~ "$funcname: not found" ]]; then
        echo "## Failed $testname - $funcname"
        ((localfailed+=1))
    else
        ((localok+=1))
    fi
done
if ((localfailed==0)); then
    echo "ok - $testname"
    ((ok+=1))
else
    ((failed+=1))
fi

testname="autoload -r -f"
autoload -r -f
localfailed=0
localok=0
for funcname in $funclist; do
    got=$(type $funcname 2>&1)
    if [[ $got =~ "$funcname: not found" ]]; then
        echo "## Failed $testname - $funcname"
        ((localfailed+=1))
    else
        ((localok+=1))
        if [[ ! $got =~ "IS_SHIM" ]]; then
            ((localfailed+=1))
        else
            ((localok+=1))
        fi
    fi
done
if ((localfailed==0)); then
    echo "ok - $testname"
    ((ok+=1))
else
    ((failed+=1))
fi

echo $ok passed, $failed failed
exit $failed
