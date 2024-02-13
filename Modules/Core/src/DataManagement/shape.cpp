#include <shape.h>
#define M_PI 3.14159265358979323846

/* Move the shape to a new location */
void mitk::Shape::move(double dx, double dy)
{
  x += dx;
  y += dy;
}

/* namespace mitk
{
  int Shape::nshapes = 0;
}*/

mitk::Circle::Circle(double r): radius(r) {}

double mitk::Circle::area()
{
  return M_PI*radius*radius;
}

double mitk::Circle::perimeter()
{
  return 2*M_PI*radius;
}

mitk::Square::Square(double w) : width(w) {}

double mitk::Square::area()
{
  return width*width;
}

double mitk::Square::perimeter()
{
  return 4*width;
}
