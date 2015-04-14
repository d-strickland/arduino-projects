#ifndef SPECREND_H
#define SPECREND_H

#include <Arduino.h>

/*  Pass in a temperature in Kelvin and three pointers to rgb values.
    for a pretty spectrum, use a minimum temperature of 1,000K and a maximum
    temperature of 10,000K.
*/
void spectrum_to_rgb(double t, double *r, double *g, double *b);

#endif
