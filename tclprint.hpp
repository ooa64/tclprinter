#ifndef TCLPRINT_H
#define TCLPRINT_H

#include <windows.h>
#include "tclcmd.hpp"

class TclPrintCmd : public TclCmd {
    friend class TclPrinterCmd;

public:

    TclPrintCmd(Tcl_Interp *interp, CONST char *name): TclCmd(interp, name) {
        lasterrorcode = 0;
        pageno = 0;
        pageactive = FALSE;
        documentactive = FALSE;
        Tcl_DStringInit(&printername);
    };

    virtual ~TclPrintCmd() {
        Tcl_DStringFree(&printername);
    };

    virtual BOOL Opened() = 0;
    virtual BOOL Started();
    virtual BOOL PageStarted();

    virtual void Close();

#ifdef DIALOGS
    virtual int Select(BOOL setupdialog) = 0;
#endif
    virtual int Open(Tcl_Obj *printer) = 0;
    virtual int OpenDefault() = 0;
    virtual int StartDoc(Tcl_Obj *document, Tcl_Obj *output);
    virtual int AbortDoc();
    virtual int EndDoc();
    virtual int StartPage();
    virtual int EndPage();

    DWORD GetError() {
        return lasterrorcode;
    }

protected:

    int pageno;
    BOOL pageactive;
    BOOL documentactive;
    DWORD lasterrorcode;
    Tcl_DString printername;

    DWORD SetError(DWORD errorcode) {
        return (lasterrorcode = errorcode);
    }

    int ParseStartParams(int objc, Tcl_Obj *CONST objv[], int *obji, Tcl_Obj **document, Tcl_Obj **output);
};

#endif
