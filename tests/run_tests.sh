#!/bin/bash

# Crema Unit Test Framework
# (C) Assured Information Security, Inc. 2014
# 10/22/2014
#
# Runs all the tests in fail that are supposed to fail either parsing or semantic analysis
# and all the tests in success which should succeed and collates that information into a single,
# easy-to-read display.

TOTALTESTS=0
PASSEDTESTS=0

if [ ! -f ../src/cremacc ]
then
    echo "cremacc not found! Please build and try again!"
    exit -1
fi

echo "Running failure tests:"
for FILE in $(ls fail/)
do
    echo -n "Running test" $FILE "... "
    ((TOTALTESTS++))
    CREMA=$(../src/cremacc < "fail/"$FILE &> /dev/null)
    if [[ $? -ne 0 ]]
    then
	echo "passed!"
	((PASSEDTESTS++))
    else
	echo "failed!"
    fi
done

echo ""
echo "Running success tests:"
for FILE in $(ls success/)
do
    echo -n "Running test" $FILE "... "
    ((TOTALTESTS++))
    CREMA=$(../src/cremacc < "success/"$FILE &> /dev/null)
    if [[ $? -eq 0 ]]
    then
	echo "passed!"
	((PASSEDTESTS++))
    else
	echo "failed!"
    fi
done

echo ""
echo "Passed " $PASSEDTESTS "/" $TOTALTESTS "!"
