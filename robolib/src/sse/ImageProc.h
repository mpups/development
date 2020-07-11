/*
**  Copyright (c) Mark Pupilli 2007, all rights reserved.
**
**  Standard, non-optimised, image processing operations.
*/
#ifndef IMAGE_PROC_H
#define IMAGE_PROC_H

void zero(unsigned int n, unsigned char* a); ///< Zeros an array of n unsigned chars

void sdiff(unsigned int n, unsigned char* a, unsigned char* b, unsigned char* c);

unsigned int sad(unsigned int n, unsigned char* a, unsigned char* b);

void invert(unsigned int n, unsigned char* a, unsigned char* b);

void threshold(unsigned int n, unsigned char threshold, unsigned char* a, unsigned char* b);

void minmax(unsigned int n, unsigned char* a, unsigned char& min, unsigned char& max); ///< find the minimum and maximum values in the array

#endif
