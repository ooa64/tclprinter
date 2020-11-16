#ifndef TCLPRINTRAW_H
#define TCLPRINTRAW_H

#include <windows.h>
#include "tclprint.hpp"

class TclPrintRawCmd : public TclPrintCmd {
    friend class TclPrinterCmd;

public:

    TclPrintRawCmd(Tcl_Interp *interp, CONST char *name) : TclPrintCmd(interp, name) {
        printerhandle = NULL;
        documenthandle = 0;
    };

    virtual ~TclPrintRawCmd() {
        Close();
    };

    BOOL Opened() override;

    void Close() override;

#ifdef DIALOGS
    int Select(BOOL setupdialog) override;
#endif
    int Open(Tcl_Obj *printer) override;
    int OpenDefault() override;
    int StartDoc(Tcl_Obj *document, Tcl_Obj *output) override;
    int EndDoc() override;
    int StartPage() override;
    int EndPage() override;

    void Abort();
    int Write(Tcl_Obj *data);
    int WriteDoc(Tcl_Obj *document, Tcl_Obj *output, Tcl_Obj *data);
    int Status(Tcl_Obj *statuslist);

    int DocumentCommand(Tcl_Obj *answer, Tcl_Obj *command, Tcl_Obj *documentid);
    int DocumentSelect(Tcl_Obj *documentlist, Tcl_Obj *selectlist, Tcl_Obj *printerpattern, Tcl_Obj *machinepattern, Tcl_Obj *userpattern, Tcl_Obj *documentpattern);

protected:

    HANDLE printerhandle;
    DWORD documenthandle;

    int ParseDocumentSelectParams(int objc, Tcl_Obj *CONST objv[], int *obji, Tcl_Obj **selectlist, Tcl_Obj **printerpattern, Tcl_Obj **machinepattern, Tcl_Obj **userpattern, Tcl_Obj **documentpattern);

    int DocumentStatus(Tcl_Obj *statuslist, DWORD handle);

private:

    int Command(int objc, struct Tcl_Obj *CONST objv[]) override;
};

#endif
