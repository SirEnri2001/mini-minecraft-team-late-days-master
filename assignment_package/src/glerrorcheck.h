#ifndef GLERRORCHECK_H
#define GLERRORCHECK_H
#include <cassert>
#include <iostream>
#include <QDebug>

#define GLCall(x) {x;}checkAndPrint(glGetError(),#x,__FILE__,__LINE__)

inline void checkAndPrint(int errorCode, const char* func, const char* file, int line){
    if (errorCode != 0) {
        qDebug()
                <<"\033[31m"<<"[Opengl Error "
                <<errorCode<<" ] at "
                <<func<<" in "
                <<file<<" line "
                <<line<<"\033[0m";
        assert(false);
    }
}

#endif // GLERRORCHECK_H
