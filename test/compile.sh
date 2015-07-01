#/bin/sh
gcc main.cpp -o main
gcc -std=c99 app.c -o app -lwiringPi
