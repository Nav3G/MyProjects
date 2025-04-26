#include "core/Color.h"

// Constructors
Color::Color() : r(30), g(30), b(30) {}

Color::Color(uint8_t red, uint8_t green, uint8_t blue) : r(red), g(green), b(blue) {}

// Operators
Color Color::operator*(float f) const
{
	return Color(r * f, g * f, b * f);
}
Color Color::operator+(const Color& color) const
{
	return Color(r + color.r, g + color.g, b + color.b);
}