#!/bin/sh

ID=$1

curl http://localhost:9876/terminal/read/${ID}
