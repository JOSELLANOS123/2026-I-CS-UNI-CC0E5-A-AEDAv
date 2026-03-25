#ifndef __VECTOR_H__
#define __VECTOR_H__

class Vector{
private:
    int  m_capacity;
    int  m_size;
    int* m_data;
public:
    Vector(int capacity);
    virtual ~Vector();
    virtual void push_back(int value);
    virtual int  get(int index);
    virtual int  size();
};

void DemoVector();

#endif // __VECTOR_H__
