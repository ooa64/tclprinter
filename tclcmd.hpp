// $Id$

#ifndef TCLCMD_H
#define TCLCMD_H

#include <tcl.h>

#if defined(DEBUG)
#   include <iostream>
#   define DEBUGLOG(_x_) (::std::cerr << "DEBUG: " << _x_ << "\n")
#   define _(_p_) (::std::cerr << "DEBUG: " #_p_ << "\n", _p_)
#else
#   define DEBUGLOG(_x_)
#   define _(_p_) _p_
#endif

#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION == 0
#   define Tcl_GetString(a) \
           Tcl_GetStringFromObj(a,NULL)
#   define Tcl_AppendObjToObj(a,b) \
           Tcl_AppendToObj(a,Tcl_GetStringFromObj(b,NULL),-1)
#endif


// Common TclCmd ancestor to all objects. This defines
// common destructing and command dispatching methods.

class TclCmd {

public:

  TclCmd();

  TclCmd(Tcl_Interp * interp, CONST char * name);
  TclCmd(Tcl_Interp * interp, CONST char * name, TclCmd * parent);

  virtual ~TclCmd();
  // virtual destructor for all objects

  void Rename(Tcl_Interp * interp, CONST char * name);
  int IsNamed() {return (tclInterp && tclToken);};
  CONST char * Name() {return IsNamed() ?
           Tcl_GetCommandName(tclInterp, tclToken) : NULL;};
  // create, delete, query tcl command name

  TclCmd * Parent() {return pParent;};
  TclCmd * Next() {return pNext;};
  TclCmd * Prev() {return pPrev;};
  TclCmd * Children() {return pChildren;};
  void SetParent(TclCmd * parent);
  void AddChild(TclCmd * child);
  void RemoveChild(TclCmd * child);

  static void Destroy(ClientData);
  // Static function for destroying objects when command deleted

  static void Delete(ClientData clientData, Tcl_Interp *interp);
  // Static function for destroying objects when interp deleted

  static int Dispatch(ClientData clientData, Tcl_Interp * interp,
              int objc, Tcl_Obj * const objv[]);
  // Static function for dispatching commands to objects

protected:

  void Unname () {
    tclInterp = NULL;
    tclToken = NULL;
  };

  Tcl_Interp * tclInterp;
  Tcl_Command tclToken;
  // token for tcl command in the interpreter interp

  TclCmd * pParent;
  TclCmd * pNext;
  TclCmd * pPrev;
  TclCmd * pChildren;
  // links for linking certain objects to each other

private:

  virtual int Command(int objc, Tcl_Obj * const objv[]) = 0;
  // virtual function for class dependent implementation of static Dispatch().

  virtual void Cleanup() {};
  // virtual function for class dependent cleanup.

};

#endif
