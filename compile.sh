#!/bin/bash
designs=("rocket20" "boom21")
tools=("repcut-sim" "essent-sim" "patronus-sim" "verilator")

for design in "${designs[@]}"; do
    for tool in "${tools[@]}"; do
        nohup make DESIGN=$design -C $tool all &
    done
done

nohup make DESIGN=rocket20 -C ksim all &
# boom21 compilation of ksim is very time consuming (takes hours to compile), uncomment the following line to run it
# nohup make DESIGN=boom21 -C ksim all &
wait