#!/bin/bash

c++ -I ./ -std=c++17 -O3 querysa.cpp -o querysa

c++ -I ./ -std=c++17 -O3 buildsa.cpp -o buildsa

c++ -I ./ -std=c++17 -O3 inspectsa.cpp -o inspectsa
