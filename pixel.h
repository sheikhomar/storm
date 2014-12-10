/**
 * @file pixel.h
 * @author Kurt Normark
 * @brief PIXELS - A pixel represents a RGB color.
 * @see http://people.cs.aau.dk/~normark/impr-c/more-functions-slide-ppm-lib.html
 */

/** @brief A new type that represents a single RGB pixel */
typedef unsigned int pixel;

/** @brief The constructor of a pixel in terms of red, green and blue (between 0 and 255) */
pixel make_pixel(unsigned int red, unsigned int green, unsigned int blue);

/** @brief Access and return the red component of the pixel p */
unsigned int get_red(pixel p);

/** @brief Access and return the green component of the pixel p */
unsigned int get_green(pixel p);

/** @brief Access and return the blue component of the pixel p */
unsigned int get_blue(pixel p);
