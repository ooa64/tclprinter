#ifndef TCLPRINTER_H
#define TCLPRINTER_H

#include <windows.h>
#include <tchar.h>
#include "tclcmd.hpp"

class TclPrinterCmd : public TclCmd {

public:

    TclPrinterCmd(Tcl_Interp *interp, char *name):
            TclCmd(interp, name), lasterrorcode(0) {};

    virtual ~TclPrinterCmd() {};

    DWORD GetError() {
        return lasterrorcode;
    }

protected:

    enum interfaces {ifUnknown, ifGdi, ifRaw}; 
    enum selections {seUnknown, seDefault, seNamed, sePrintdialog, seSetupdialog};

    DWORD lasterrorcode;

    DWORD SetError(DWORD errorcode) {
        return (lasterrorcode = errorcode);
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
Tcl_Obj *NewObjFromExternal(CONST TCHAR *external);

int AppendSystemError (Tcl_Interp *interp, char *prefix, DWORD error);

#define SIZEOFARRAY(a) (sizeof(a)/sizeof(a[0]))

#endif
