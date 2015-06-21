#!/bin/bash 

for f in *Fixed.xml ; do rm $f ; done #Delete old files so we don't run this on them multiple times.

for f in *.xml ; do
	./XMLFix $f > /dev/null  2> /dev/null #I don't want to see much output.
done

pass=0
fail=0

#Clear our pass/fail logs
echo "" > pass.txt
echo "" > fail.txt 

for f in *Fixed.xml ; do 
	result=$(xmllint $f |& tail -n 1)
	if [ $result = "</Root>" ] ; then #Check the last element to see if it ran correctly.
		echo "$f Passed" >> pass.txt
		pass=$(( $pass + 1 ))
	else
		echo "$f Failed" >> fail.txt
		fail=$(( $fail + 1 ))
	fi
done

echo "$pass,$fail" >> output.csv
echo "Pass = $pass"
echo "Fail = $fail"
