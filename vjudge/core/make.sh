#!/bin/bash
sudo /etc/init.d/judged stop
cd judged
make
sudo rm -rf /usr/bin/judge_client
sudo rm -rf /usr/bin/judged
sudo rm -rf /usr/bin/sim.sh
sudo rm -rf /usr/bin/sim_c
sudo rm -rf /usr/bin/sim_cc
sudo rm -rf /usr/bin/sim_rb
sudo rm -rf /usr/bin/sim_sh
sudo rm -rf /usr/bin/sim_java
sudo rm -rf /usr/bin/sim_pas
sudo chmod +x judged
sudo cp judged /usr/bin
sudo /etc/init.d/judged start
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
