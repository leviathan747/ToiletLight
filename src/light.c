#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

/* Set Pin Numbers */
#define R_OUT 29
#define G_OUT 25
#define B_OUT 28

#define P_IN 21

#define PERC_INIT 10000
#define INCR 50

/* create color data structure */
typedef struct COLOR {
    char R; 
    char G; 
    char B;
} Color;

/* color functions */
char getRed(Color* c) {
    if (!c) return -1;
    return c->R;
}

char getGreen(Color* c) {
    if (!c) return -1;
    return c->G;
}

char getBlue(Color* c) {
    if (!c) return -1;
    return c->B;
}

char setRed(Color* c, char red) {
    if (!c || red >= 256) return -1;
    c->R = red;
    return 0;
}

char setGreen(Color* c, char green) {
    if (!c || green >= 256) return -1;
    c->G = green;
    return 0;
}

char setBlue(Color* c, char blue) {
    if (!c || blue >= 256) return -1;
    c->B = blue;
    return 0;
}

char setColor(Color* c, unsigned int amt) {
    if (!c) return -1;
    amt = amt % 0x1000000;
    setRed(c, (char) (amt & 0xFF0000) >> 16);
    setGreen(c, (char) ((amt & 0x00FF00) >> 8));
    setBlue(c, (char) amt & 0x0000FF);
    return 0;
}

char addColor(Color* c, unsigned int amt) {
    if (!c) return -1;
    unsigned int cur = 0x100*0x100*getRed(c) + 0x100*getGreen(c) + getBlue(c);
    cur = cur + amt % 0x1000000;
    setRed(c, (char) (cur & 0xFF0000) >> 16);
    setGreen(c, (char) ((cur & 0x00FF00) >> 8));
    setBlue(c, (char) cur & 0x0000FF);
    return 0;
}

unsigned int getColor(Color* c) {
    if (!c) return -1;
    return 0x100*0x100*getRed(c) + 0x100*getGreen(c) + getBlue(c);
}

char initColor(Color* c) {
    if (!c) return -1;
    setRed(c, 0);
    setGreen(c, 0);
    setBlue(c, 0);
    return 0;
}

int main(void) {
  
    // Initialize GPIO pins to map to wiringPi numbers
    if(wiringPiSetup() == -1) return 1; // return with error status (initialization failed)
    
    // Set modes for the GPIO pins
    pinMode(R_OUT, OUTPUT);
    pinMode(G_OUT, OUTPUT);
    pinMode(B_OUT, OUTPUT);

    pinMode(P_IN, INPUT);

    // set up pwm
    int pwm_err = 0;
    if (pwm_err = softPwmCreate(R_OUT, 0, 255)) return pwm_err;
    if (pwm_err = softPwmCreate(G_OUT, 0, 255)) return pwm_err;
    if (pwm_err = softPwmCreate(B_OUT, 0, 255)) return pwm_err;
    
    // Setup color object
    Color col;
    initColor(&col);

    // control vars
    int perc_red = PERC_INIT;
    int perc_green = 0;
    int perc_blue = 0;

    int paused = 0;

    while(1) {

        // read photo sensor
	if (digitalRead(P_IN)) {
            //printf("off\n");
            softPwmWrite(R_OUT, 0);
            softPwmWrite(G_OUT, 0);
            softPwmWrite(B_OUT, 0);
            paused = 1;
        }
        else {
            //printf("on\n");
            paused = 0;
        }

        if (!paused) {
            // change intensities for a sweep
            if (perc_red > 0 && perc_blue <= 0) {
                perc_blue = 0;
                perc_red -= INCR;
                perc_green += INCR;
            }
            else if (perc_green > 0 && perc_red <= 0) {
                perc_red = 0;
                perc_green -= INCR;
                perc_blue += INCR;
            }
            else if (perc_blue > 0 && perc_green <= 0) {
                perc_green = 0;
                perc_blue -= INCR;
                perc_red += INCR;
            }
            else {printf("fail\n");}

            setRed(&col, 255 * perc_red / PERC_INIT);
            setGreen(&col, 255 * perc_green / PERC_INIT);
            setBlue(&col, 255 * perc_blue / PERC_INIT);

            //printf("red: %3d, green: %3d, blue: %3d, total: %u\n", (int) getRed(&col), (int) getGreen(&col), (int) getBlue(&col), (unsigned int) getColor(&col));

            softPwmWrite(R_OUT, getRed(&col));
            softPwmWrite(G_OUT, getGreen(&col));
            softPwmWrite(B_OUT, getBlue(&col));
        }
    
        // Use of delay is important as there is a small lag in reading/writing values each time
        delay(10);
    }

    return 0;
}
