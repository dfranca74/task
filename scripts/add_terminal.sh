#!/bin/sh

CARD=$1
TYPE=$2

curl -d "card=${CARD}&transaction=${TYPE}" -X POST http://localhost:9876/terminal/add; echo
