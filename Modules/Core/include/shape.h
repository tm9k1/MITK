#ifndef SHAPE_H
#define SHAPE_H
#include <MitkCoreExports.h>
namespace mitk
{
  class MITKCORE_EXPORT Shape
  {
  public:
    //static int nshapes;
    Shape() = default;// { nshapes++; }
    virtual ~Shape() = default; //{ nshapes--; }
    double x, y;
    void move(double dx, double dy);
    virtual double area() = 0;
    virtual double perimeter() = 0;
  };

  class MITKCORE_EXPORT Circle : public Shape
  {
  private:
    double radius;

  public:
    Circle(double r);// : radius(r) {}
    double area() override;
    double perimeter() override;
  };

  class MITKCORE_EXPORT Square : public Shape
  {
  private:
    double width;

  public:
    Square(double w);// : width(w) {}
    double area() override;
    double perimeter() override;
  };

}
#endif
