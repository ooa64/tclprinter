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

    DWORD lasterrorcode;

    void SetError(DWORD errorcode) {
        lasterrorcode = errorcode;
    }

    int Names(Tcl_Obj *result);

private:

    virtual int Command(int objc, struct Tcl_Obj *CONST objv[]);

    Tcl_Obj *NewPrintNameObj();
};

Tcl_Obj *NewObjFromExternalW (TCHAR *external);
Tcl_Obj *NewObjFromExternalA (TCHAR *external);
TCHAR *NewExternalFromObjW (Tcl_Obj *obj);
TCHAR *NewExternalFromObjA (Tcl_Obj *obj);

#ifdef UNICODE
#define NewObjFromExternal NewObjFromExternalW
#define NewExternalFromObj NewExternalFromObjW
#else
#define NewObjFromExternal NewObjFromExternalA
#define NewExternalFromObj NewExternalFromObjA
#endif

TCHAR *NewStringFromExternal(void *external);

BOOL UnicodeOS();

int AppendSystemError (Tcl_Interp *interp, char *prefix, DWORD error);

#endif
