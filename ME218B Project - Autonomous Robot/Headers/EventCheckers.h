/****************************************************************************
 Module
     EventCheckers.h
 Description
     header file for the event checking functions
 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 08/06/13 14:37 jec      started coding
*****************************************************************************/

#ifndef EventCheckers_H
#define EventCheckers_H

// prototypes for event checkers

bool Check4Keystroke(void);
bool CheckTapeStatus(void);
void setCheckTapeFound(bool eventCheckerStatus);

#endif /* EventCheckers_H */
