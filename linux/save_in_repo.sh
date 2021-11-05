#!/bin/bash

cp ./kernel/{ptree.c,ptree.h} /home/ubuntu/Advanced-Operating-Systems-Homework/assignment1/ptree_excercise/kernel_related_changes/
cp ./ptree_module/{ptree_module_main.c,Kbuild,Makefile} /home/ubuntu/Advanced-Operating-Systems-Homework/assignment1/ptree_excercise/kernel_related_changes/ptree_module/
git diff > /home/ubuntu/Advanced-Operating-Systems-Homework/assignment1/ptree_excercise/kernel_related_changes/linux_kernel_diff.diff
