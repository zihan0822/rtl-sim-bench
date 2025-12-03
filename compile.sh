#!/bin/bash
designs=("rocket20" "boom21")
tools=("repcut-sim" "essent-sim" "patronus-sim" "verilator")

for design in "${designs[@]}"; do
    for tool in "${tools[@]}"; do
        nohup make DESIGN=$design -C $tool all &
    done
done

wait