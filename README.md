# Task_Interview_Digital
Program made for replacing the strings in a file with their respective Unicode escape sequences.
It is a console program where you need to input the name of file (with .json in the end) or absolute path to it. 
I made several test files(input0.json, input1.json, input2.json).
This program might have some issues with symbols from other alphabets(cirilic for example) in case of creating of replacing map(it wil not transform into \x.. but will stay in \u... format). 
Important moment is that this program still ENCODE already ENCODED files.
