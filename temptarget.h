/*
TempTarget Library holds temperature measurements and
by using the controlTemp() and testTemp() methods
 provide the calculated temperatures in a specific point
 in the day cycle.
*/

#ifndef TEMPTARGET_H
#define TEMPTARGET_H


class TempTarget
{
    public:
        TempTarget();
        virtual ~TempTarget();
        float controlTemp(int , int );
        float testTemp(int, int);

    protected:

    private:
};

#endif // TEMPTARGET_H
