#include <iostream>


// runtime polymorphism 
class RuntimeExample
{
    virtual void placeOrder()
    {
        printf("RuntimeExample::placeOrder()\n");
    }
};

class SpecificRuntimeExample : public RuntimeExample
{
public:
    void placeOrder() override
    {
        printf("SpecificRuntimeExample::placeOrder()\n");
    }
};

// compile time polymorphism
template <typename T>
class CRTPExample
{
public:
    void placeOrder()
    {
        static_cast<T*>(this)->actualPlaceOrder();
    }
    
    void actualPlaceOrder()
    {
        printf("CRTPExample::actualPlaceOrder()\n");
    }
};

class SpecificCRTPExample : public CRTPExample<SpecificCRTPExample>
{
public:
    void actualPlaceOrder()
    {
        printf("SpecificCRTPExample::actualPlaceOrder()\n");
    }
};

int main()
{
    CRTPExample<SpecificCRTPExample> baseClass;
    baseClass.placeOrder();
    return 0;
}