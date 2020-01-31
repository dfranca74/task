#!/bin/sh

for card in Visa MasterCard EFTPOS
do
  for type in Cheque Savings Credit
    do
       sh add_terminal.sh ${card} ${type}
       sleep 1
    done
done
