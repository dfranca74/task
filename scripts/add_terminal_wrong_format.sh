#!/bin/sh

CARD=Visa
TYPE=Savings

curl -d "obladi=${CARD}&transaction=${TYPE}" -X POST http://localhost:9876/terminal/add; echo
sleep 1
curl -d "card=${CARD}&oblada=${TYPE}" -X POST http://localhost:9876/terminal/add; echo
sleep 1
curl -d "transaction=${TYPE}" -X POST http://localhost:9876/terminal/add; echo
sleep 1
curl -d "card=${CARD}" -X POST http://localhost:9876/terminal/add; echo
