#ifndef SHAPE_H
#define SHAPE_H

#include <itkMacro.h>
#include <itkObject.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

namespace mitk
{
  class MITKCORE_EXPORT Shape: public itk::Object
  {
  public:
    mitkClassMacroItkParent(Shape, itk::Object);
    //static int nshapes;
    Shape() = default;// { nshapes++; }
    virtual ~Shape() /*= default;*/{ MITK_INFO << "shape orignal destructor"; }
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
    mitkClassMacro(Circle, Shape);
    itkCloneMacro(Self);
    Circle(double r);// : radius(r) {}
    double area() override;
    double perimeter() override;
    
  };

  class MITKCORE_EXPORT Square : public Shape
  {
  private:
    double width;

  public:
    mitkClassMacro(Square, Shape);
    mitkNewMacro1Param(Self,double);
    
    double area() override;
    double perimeter() override;
  protected:
    Square(double w);
    ~Square(){MITK_INFO << "Square destructor";};

  };

}
#endif
