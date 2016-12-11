#ifndef CODES_H
#define CODES_H

class Codes{
public:
    static const quint8 authRequest = 1;
    static const quint8 authSuccess = 2;
    static const quint8 authIncorrectPair = 3;
    static const quint8 authProblem = 4;
    static const quint8 messagePublic = 5;
    static const quint8 messagePrivate = 6;
    static const quint8 signUpRequest = 7;
    static const quint8 signUpNameNotVacant = 8;
    static const quint8 authAlreadyOnline = 9;
    static const quint8 signUpProblem = 10;
    static const quint8 signUpSuccess = 11;
};



#endif // CODES_H
