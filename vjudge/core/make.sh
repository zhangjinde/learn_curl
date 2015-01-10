#!/bin/bash
make
sudo chmod +x judged
sudo cp judged /usr/bin
sudo chmod +x judge_client
sudo cp judge_client /usr/bin
make clean
