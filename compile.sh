#!/bin/bash
designs=("rocket20" "boom21")
tools=("repcut-sim" "essent-sim")

for design in "${designs[@]}"; do
    for tool in "${tools[@]}"; do
        make DESIGN=$design -C $tool all
    done
done

wait