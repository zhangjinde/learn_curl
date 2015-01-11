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
cd ../sim/sim_2_67
make clean
make sim_c
make sim_java
make sim_pasc
sudo chmod +x sim*
sudo cp sim_c /usr/bin
sudo cp sim_java /usr/bin/sim_java
sudo cp sim_pasc /usr/bin/sim_pas
make clean
cd ..
sudo cp sim.sh /usr/bin
sudo chmod +x /usr/bin/sim.sh
sudo ln -s /usr/bin/sim_c /usr/bin/sim_cc
sudo ln -s /usr/bin/sim_c /usr/bin/sim_rb
sudo ln -s /usr/bin/sim_c /usr/bin/sim_sh
