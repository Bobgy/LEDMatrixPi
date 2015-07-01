#ifndef _UTILS_H
#define _UTILS_H

typedef unsigned int INT32U;

struct GpioRegisters
{
    INT32U GPFSEL[6];
    INT32U Reserved1;
    INT32U GPSET[2];//7
    INT32U Reserved2;
    INT32U GPCLR[2];//10
	INT32U Reserved3;
	INT32U GPIN0;//13
	INT32U Reserved4[23];
	INT32U GPPUD;//37
	INT32U GPPUDCLK0;
};

static struct GpioRegisters* _pGpioRegisters;

#define HIGH 1
#define LOW  0

#define INPUT  0
#define OUTPUT 1

inline void pinMode(INT32U pin, INT32U st) {
	INT32U x, offset, val;
	x = pin / 10;
	offset = pin % 10;
	val = _pGpioRegisters->GPFSEL[x];
	val &= ~ (0x7 << offset * 3);
	val |= st << offset * 3;
	_pGpioRegisters->GPFSEL[x] = val;
}

int _not_used;
void delay(int time) {
	while(time--)_not_used++;
}

inline void pinPullUp(int pin) {
	_pGpioRegisters->GPPUD = 2;
	delay(100);
	_pGpioRegisters->GPPUDCLK0 = 1<<pin;
    delay(100);
	_pGpioRegisters->GPPUD = 2;
	_pGpioRegisters->GPPUDCLK0 = 0;
}

// pin is now a mask
inline void digitalWrite(INT32U pin, int v) {
	if (v) _pGpioRegisters->GPSET[0] = pin;
	else   _pGpioRegisters->GPCLR[0] = pin;
}

inline INT32U digitalRead(INT32U pin) {
	return (_pGpioRegisters->GPIN0 >> pin) & 1;
}
#endif
