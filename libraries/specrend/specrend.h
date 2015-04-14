#ifndef SPECREND_H
#define SPECREND_H

#include <Arduino.h>

/*  Pass in a wavelength in nm and three pointers to rgb values.
    for a pretty spectrum, use a minimum wavelength of 380nm and a maximum
    temperature of 780nm.
*/
void wavelength_to_rgb(int wavelength, float *r, float *g, float *b);

#endif
