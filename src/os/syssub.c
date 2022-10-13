#include "os/syssub.h"

#include "os/system.h"

// Syssub functions
void SetBackColor(int r, int g, int b)
{
	// Set db clear color
	DBufDc.clear0.rgbaq.R = r;
	DBufDc.clear0.rgbaq.G = g;
	DBufDc.clear0.rgbaq.B = b;

	DBufDc.clear1.rgbaq = DBufDc.clear0.rgbaq;
}