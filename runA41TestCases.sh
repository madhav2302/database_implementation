#!/bin/bash

echo "" > Statistics.txt
./a4-1.out 1
cat Statistics.txt > output41.txt
echo "***************************************************************************************************************************************" >> output41.txt

./a4-1.out 2
cat Statistics.txt >> output41.txt
echo "***************************************************************************************************************************************" >> output41.txt

./a4-1.out 5
cat Statistics.txt >> output41.txt
echo "***************************************************************************************************************************************" >> output41.txt

./a4-1.out 10
cat Statistics.txt >> output41.txt
echo "***************************************************************************************************************************************" >> output41.txt

./a4-1.out 11
cat Statistics.txt >> output41.txt
echo "***************************************************************************************************************************************" >> output41.txt
