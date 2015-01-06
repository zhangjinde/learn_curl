#!/bin/bash
make
chmod +x judged
cp judged /usr/bin
chmod +x judge_client
cp judge_client /usr/bin
make clean
