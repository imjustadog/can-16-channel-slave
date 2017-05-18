# MPLAB IDE generated this makefile for use with GNU make.
# Project: String.mcp
# Date: Wed Jul 16 16:26:59 2014

AS = pic30-as.exe
CC = pic30-gcc.exe
LD = pic30-ld.exe
AR = pic30-ar.exe
HX = pic30-bin2hex.exe
RM = rm

String.hex : String.cof
	$(HX) "String.cof"

String.cof : main.o timer.o adc.o sci.o IC.o DS1302.o FRAM.o ecan.o
	$(CC) -mcpu=33FJ64GP706 "main.o" "timer.o" "adc.o" "sci.o" "IC.o" "DS1302.o" "FRAM.o" "ecan.o" -o"String.cof" -Wl,-Tp33FJ64GP706.gld,--defsym=__MPLAB_BUILD=1,-Map="String.map",--report-mem

main.o : ecan.h FRAM.h DS1302.h IC.h sci.h adc.h timer.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h main.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "main.c" -o"main.o" -g -Wall

timer.o : c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h timer.h timer.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "timer.c" -o"timer.o" -g -Wall

adc.o : c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h adc.h adc.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "adc.c" -o"adc.o" -g -Wall

sci.o : sci.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h sci.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "sci.c" -o"sci.o" -g -Wall

IC.o : IC.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h IC.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "IC.c" -o"IC.o" -g -Wall

DS1302.o : c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h DS1302.h DS1302.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "DS1302.c" -o"DS1302.o" -g -Wall

FRAM.o : FRAM.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h FRAM.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "FRAM.c" -o"FRAM.o" -g -Wall

ecan.o : ecan.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33FJ64GP706.h c:/program\ files/microchip/mplab\ c30/support/dsPIC33F/h/p33Fxxxx.h ecan.c
	$(CC) -mcpu=33FJ64GP706 -x c -c "ecan.c" -o"ecan.o" -g -Wall

clean : 
	$(RM) "main.o" "timer.o" "adc.o" "sci.o" "IC.o" "DS1302.o" "FRAM.o" "ecan.o" "String.cof" "String.hex"

