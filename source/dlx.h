#ifndef DLX_H
#define DLX_H

extern int solution[9][9], givens[9][9], correct[9][9];
extern int ind[46656];
void setupDLX();
int setSolution();
void setGivens(int d);

#endif
