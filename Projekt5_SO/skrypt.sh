#!/bin/bash
n=$1
for (( i=1; i <= n; i++ ))
do
echo "Message" | ./k &
done
exit 0
