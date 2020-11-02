#ifndef TCLPRINTRAW_H
#define TCLPRINTRAW_H

#include <windows.h>
#include "tclprint.hpp"

class TclPrintRawCmd : public TclPrintCmd {
    friend class TclPrinterCmd;

public:

    TclPrintRawCmd(Tcl_Interp *interp, char *name):
          TclPrintCmd(interp, name),
          printerhandle(NULL),
          documenthandle(NULL) {};

    virtual ~TclPrintRawCmd() {
        Close();
    };

    virtual BOOL Opened();

    virtual void Close();

    virtual int Select(BOOL usedefault);
    virtual int Open(Tcl_Obj *printer);
    virtual int StartDoc(Tcl_Obj *document, Tcl_Obj *output);
    virtual int AbortDoc();
    virtual int EndDoc();
    virtual int StartPage();
    virtual int EndPage();

    int Write(Tcl_Obj *data);

protected:

    HANDLE printerhandle;
    DWORD documenthandle;

private:

    virtual int Command(int objc, struct Tcl_Obj *CONST objv[]);
};

#endif
