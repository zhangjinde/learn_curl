#!/bin/bash
cd judged
make
sudo chmod +x judged
sudo cp judged /usr/bin
make clean
cd ../judge_client
make
sudo chmod +x judge_client
sudo cp judge_client /usr/bin
make clean
