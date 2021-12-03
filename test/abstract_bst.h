#ifndef ABST_H
#define ABST_H
class ABST {
    public:
        virtual ~ABST();
        virtual bool contains(int key) = 0;
        virtual bool add(int key) = 0;
        virtual bool remove(int key) = 0;
};

#endif