/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// NSColorHSBToRGB, NSColorRGBToHSB
// Algorithms derived from: http://en.wikipedia.org/wiki/HSL_and_HSV

static inline void NSColorHSBToRGB(float hue, float saturation, float brightness, float *redp, float *greenp, float *bluep) {
    float red = brightness, green = brightness, blue = brightness;

    float H = hue * 360.f; // convert to degrees
    float S = saturation;
    float V = brightness;
    float m = 0; // the brightness delta.

    if(V != 0) {
        // We have enough color information to do a conversion

        float C = V * S;

        H = fmod(H, 360.f);
        H /= 60.f; // convert to hexagonal segments

        int hueFace = H; // cube face index

        float X = C * (1 - fabs(fmod(H, 2) - 1));

        switch(hueFace) {
            case 0:
                red = C;
                green = X;
                blue = 0;
                break;
            case 1:
                red = X;
                green = C;
                blue = 0;
                break;
            case 2:
                red = 0;
                green = C;
                blue = X;
                break;
            case 3:
                red = 0;
                green = X;
                blue = C;
                break;
            case 4:
                red = X;
                green = 0;
                blue = C;
                break;
            case 5:
                red = C;
                green = 0;
                blue = X;
                break;
        }

        m = V - C;
    }

    // Finally add in the brightness delta
    *redp = red + m;
    *greenp = green + m;
    *bluep = blue + m;

    //	NSLog(@"RGB(%f, %f, %f) <- HSB(%f, %f, %f)", *redp, *greenp, *bluep, hue, saturation, brightness);
}

static inline void NSColorRGBToHSB(float red, float green, float blue, float *huep, float *saturationp, float *brightnessp) {

    float M = MAX(red, MAX(green, blue));
    float m = MIN(red, MIN(green, blue));

    float H = 0;
    float S = 0;
    float V = M;

    if(V > 0) {
        float C = M - m;
        S = C / V;
        if(C == 0) {
            H = 0;
        } else if(red == M) {
            H = (green - blue) / C;
            H = fmod(H, 6);
        } else if(green == M) {
            H = 2 + (blue - red) / C;
        } else if(blue == M) {
            H = 4 + (red - green) / C;
        }

        H *= 60.f;
        if(H < 0) {
            H += 360.f;
        }
    }

    if(huep != NULL) {
        *huep = H / 360.0;
    }
    if(saturationp != NULL) {
        *saturationp = S;
    }
    if(brightnessp != NULL) {
        *brightnessp = V;
    }
    //	NSLog(@"RGB(%f, %f, %f) -> HSB(%f, %f, %f)", red, green, blue, H/360.f, S, V);
}
