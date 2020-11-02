// $Id$

#include "tclcmd.hpp"

#if defined(DEBUG) && !defined(TCLCMD_DEBUG)
#   undef DEBUGLOG
#   define DEBUGLOG(_x_)
#endif

TclCmd::TclCmd() {
  DEBUGLOG("TclCmd::Construct *" << this);
  tclInterp = NULL;
  tclToken = NULL;
  pParent = NULL;
  pNext = NULL;
  pPrev = NULL;
  pChildren = NULL;
  Tcl_CreateExitHandler(TclCmd::Destroy, this);
}

TclCmd::TclCmd(Tcl_Interp * interp, CONST char * name) {
  DEBUGLOG("TclCmd::Construct *" << this << " " << interp << " " << name);
  tclInterp = NULL;
  tclToken = NULL;
  pParent = NULL;
  pNext = NULL;
  pPrev = NULL;
  pChildren = NULL;
  Rename(interp, name);
  Tcl_CreateExitHandler(TclCmd::Destroy, this);
};

TclCmd::TclCmd(Tcl_Interp * interp, CONST char * name, TclCmd * parent) {
  DEBUGLOG("TclCmd::Construct *" << this << " " << interp << " " << name << " " << parent);
  tclInterp = NULL;
  tclToken = NULL;
  pParent = NULL;
  pNext = NULL;
  pPrev = NULL;
  pChildren = NULL;
  Rename(interp, name);
  SetParent(parent);
  Tcl_CreateExitHandler(TclCmd::Destroy, this);
};

TclCmd::~TclCmd() {
  DEBUGLOG("TclCmd::Destruct *" << this);
  Tcl_DeleteExitHandler(TclCmd::Destroy, this);
  SetParent(NULL);
  Rename(NULL, NULL);
}

void TclCmd::Rename(Tcl_Interp * interp, CONST char * name) {
  DEBUGLOG("TclCmd::Rename *" << this << " '" << \
       Tcl_GetCommandName(tclInterp, tclToken) << \
       "' to '" << (name ? name : "") << "'");
  if (tclInterp && tclToken) {
    Tcl_Interp * oldInterp = tclInterp;
    Tcl_Command oldToken = tclToken;
    tclInterp = NULL;
    tclToken = NULL;
//  Tcl_DontCallWhenDeleted(oldInterp, TclCmd::Delete, this);
    Tcl_DeleteCommandFromToken(oldInterp, oldToken);
  }
  if (interp && name) {
    tclInterp = interp;
    tclToken = Tcl_CreateObjCommand(interp, name,
                    &TclCmd::Dispatch, this, TclCmd::Destroy);
//  Tcl_CallWhenDeleted(interp, TclCmd::Delete, this);
  }
}

void TclCmd::SetParent(TclCmd * parent) {
  DEBUGLOG("TclCmd::SetParent *" << this << " " << parent);
  if (pParent)
    pParent->RemoveChild(this);
  if (parent)
    parent->AddChild(this);
  pParent = parent;
};

void TclCmd::AddChild(TclCmd * child) {
  DEBUGLOG("TclCmd::AddChild *" << this << " " << child);
  if (pChildren)
    pChildren->pPrev = child;
  child->pNext = pChildren;
  child->pPrev = NULL;
  pChildren = child;
}

void TclCmd::RemoveChild(TclCmd * child) {
  DEBUGLOG("TclCmd::RemoveChild *" << this << " " << child);
  if (child->pPrev)
    child->pPrev->pNext = child->pNext;
  if (child->pNext)
    child->pNext->pPrev = child->pPrev;
  if (pChildren == child)
    pChildren = child->pNext;
}

int TclCmd::Dispatch (ClientData clientData, Tcl_Interp * interp,
             int objc, Tcl_Obj * const objv[]) {
  TclCmd *o = (TclCmd *) clientData;

#ifdef TCLCMD_DEBUG
  if (interp != o->tclInterp) {
    DEBUGLOG("TclCmd::Dispatch *" << o << " mismatched '" << \
         Tcl_GetString(objv[0]) << " " << (objc > 1 ? Tcl_GetString(objv[1]) : "NULL") << "'");
  } else {
    DEBUGLOG("TclCmd::Dispatch *" << o << " '" << \
         Tcl_GetString(objv[0]) << " " << (objc > 1 ? Tcl_GetString(objv[1]) : "NULL") << "'");
  }
#endif
  return o->Command(objc, objv);
}

void TclCmd::Destroy(ClientData clientData) {
  TclCmd *o = (TclCmd *) clientData;

  DEBUGLOG("TclCmd::Destroy *" << o);
  if (o->IsNamed()) {
    o->Cleanup();
    o->Unname();
    delete o;
  }
}

void TclCmd::Delete(ClientData clientData, Tcl_Interp *interp) {
  TclCmd *o = (TclCmd *) clientData;

  DEBUGLOG("TclCmd::Delete *" << o);
  if (o->IsNamed()) {
    o->Cleanup();
    o->Unname();
    delete o;
  }
}
