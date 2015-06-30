#ifndef _UTILS_H
#define _UTILS_H

#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPSET0  0x2020001C
#define GPCLR0  0x20200028
#define GPIN0	0x20200034
#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

#define HIGH 1
#define LOW  0

#define INPUT  0
#define OUTPUT 1

typedef unsigned int INT32U;

#define GET32(addr) (*(INT32U*)(addr))
#define PUT32(addr, val) *(INT32U*)(addr)=(val)

inline void pinMode(INT32U pin, INT32U st) {
	INT32U x, offset, gpfsel, val;
	x = pin / 10;
	offset = pin % 10;
	gpfsel = GPFSEL0 + x * 4;
	val = GET32(gpfsel);
	val &= ~ (0x7 << offset * 3);
	val |= st << offset * 3;
	PUT32(gpfsel, val);
}

int _not_used;
void delay(int time) {
	while(time--)_not_used++;
}

inline void pinPullUp(int pin) {
	PUT32(GPPUD, 2);
	delay(100);
	PUT32(GPPUDCLK0, 1<<pin);
    delay(100);
	PUT32(GPPUD, 0);
	PUT32(GPPUDCLK0, 0);
}

inline void digitalWrite(int pin, int v) {
	if (v) PUT32(GPSET0, 1<<pin);
	else   PUT32(GPCLR0, 1<<pin);
}

inline INT32U digitalRead(int pin) {
	return (GET32(GPIN0) >> pin) & 1;
}
#endif
