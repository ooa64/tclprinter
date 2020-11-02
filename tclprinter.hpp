#ifndef TCLPRINTER_H
#define TCLPRINTER_H

#include <windows.h>
#include <tchar.h>
#include "tclcmd.hpp"

class TclPrinterCmd : public TclCmd {

public:

    TclPrinterCmd(Tcl_Interp *interp, char *name):
            TclCmd(interp, name), lasterrorcode(0) {
        DEBUGLOG("TclPrinterCmd::Construct *" << this);
    };

    virtual ~TclPrinterCmd() {
        DEBUGLOG("TclPrinterCmd::Destruct *" << this);
    };

    DWORD GetError() {
        return lasterrorcode;
    }

protected:

    enum interfaces {ifUnknown, ifGdi, ifRaw, ifXps}; 
    enum selections {seUnknown, seDefault, seNamed, seSelect};

    DWORD lasterrorcode;

    void SetError(DWORD errorcode) {
        lasterrorcode = errorcode;
    }

    int Names(Tcl_Obj *result);
    int ParseOpenParams(int objc, Tcl_Obj *CONST objv[], int *obji,
        interfaces *printerinterface, selections *printerselection, Tcl_Obj **printername);

private:

    virtual int Command(int objc, struct Tcl_Obj *CONST objv[]);

    Tcl_Obj *NewPrintNameObj();
};

void UtfToExternal(CONST CHAR *utf, Tcl_DString *external);
void ExternalToUtf(CONST TCHAR *external, Tcl_DString *utf);

int AppendSystemError (Tcl_Interp *interp, char *prefix, DWORD error);

#endif
