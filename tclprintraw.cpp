// $Id$

#include "tclprinter.hpp"
#include "tclprintraw.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif


int TclPrintRawCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static char *commands[] = {
        "info",
        "start",
        "abort",
        "end",
        "startpage",
        "endpage",
        "write",
        "close",
        0L
    };
    enum commands {
        cmInfo,
        cmStart,
        cmAbort,
        cmEnd,
        cmStartPage,
        cmEndPage,
        cmWrite,
        cmClose
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(tclInterp, 1, objv, "command");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(tclInterp, objv[1], (CONST char **)commands, "command", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch ((enum commands)(index)) {
    case cmInfo:
        if (objc == 3) {
            if (strcmp(Tcl_GetString(objv[2]), "protocol") == 0)
                Tcl_SetResult(tclInterp, "raw", NULL);
            else if (strcmp(Tcl_GetString(objv[2]), "name") == 0)
                Tcl_DStringResult(tclInterp, &printername);
            else if (strcmp(Tcl_GetString(objv[2]), "page") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(pageno));
            else {
                Tcl_AppendResult(tclInterp, "bad option ", Tcl_GetString(objv[2]), NULL);
                return TCL_ERROR;
            }
            return TCL_OK;
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, "name");
            return TCL_ERROR;
        }

    case cmStart:
        {
            Tcl_Obj *document = NULL;
            Tcl_Obj *output = NULL;
            int result = TCL_OK;
            int obji = 2;

            if (!Opened()) {
                Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
                return TCL_ERROR;
            }
            if (Started()) {
                Tcl_AppendResult(tclInterp, "document is already started", NULL);
                return TCL_ERROR;
            }
            if (ParseStartParams(objc, objv, &obji, &document, &output) != TCL_OK) {
                return TCL_ERROR;
            }
            if (obji < objc) {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-document name? ?-output name?");
                return TCL_ERROR;
            }
            result = StartDoc(document, output);
            if (result == TCL_ERROR) {
                return AppendSystemError(tclInterp, "error starting document: ", GetError());
            }
            return result;
        }

    case cmAbort:
    case cmEnd:
        if (!Started()) {
            Tcl_AppendResult(tclInterp, "document is not started", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            if (EndDoc() == TCL_OK)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "error ending document: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    case cmStartPage:
        if (!Started()) {
            Tcl_AppendResult(tclInterp, "document is not started", NULL);
            return TCL_ERROR;
        }
        if (PageStarted()) {
            Tcl_AppendResult(tclInterp, "page is already started", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            if (StartPage() == TCL_OK)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "error starting page: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    case cmEndPage:
        if (!PageStarted()) {
            Tcl_AppendResult(tclInterp, "page is not started", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            if (EndPage() == TCL_OK)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "error ending page: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    case cmWrite:
        if (!Opened()) {
            Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
            return TCL_ERROR;
        }
        if (objc == 3) {
            if (Write(objv[objc-1]) >= 0)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "error writing data: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    case cmClose:
        if (!Opened()) {
            Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            delete this;
            return TCL_OK;
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    } // switch index

    return TCL_OK;
}

void TclPrintRawCmd::Close() {
    if (PageStarted()) 
        EndPage();
    if (Started()) 
        EndDoc();
    if (Opened())
        _(::ClosePrinter(printerhandle));
    printerhandle = NULL;
    TclPrintCmd::Close();
}

int TclPrintRawCmd::Select (BOOL usedefault) {
    Close();
    SetError(0);

    if (usedefault) {
        DWORD size;
        TCHAR* buffer = NULL;
        _(::GetDefaultPrinter(NULL, &size));
        if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            buffer = (TCHAR *)ckalloc(size*sizeof(TCHAR));
            if (_(::GetDefaultPrinter(buffer, &size))) {
                if (_(::OpenPrinter(buffer, &printerhandle, NULL))) {
                    ExternalToUtf(buffer, &printername);
                    return TCL_OK;
                }
                printerhandle = NULL;
            }
            ckfree((char *)buffer);
        }
        SetError(::GetLastError());
        return TCL_ERROR;
    }
    return TCL_ERROR;
}

int TclPrintRawCmd::Open(Tcl_Obj *printer) {
    Tcl_DString ds;
    Tcl_DStringInit(&ds);
    UtfToExternal(Tcl_GetString(printer), &ds);

    Close();

    if (!_(::OpenPrinter((TCHAR *)Tcl_DStringValue(&ds), &printerhandle, NULL)))
        printerhandle = NULL;

    Tcl_DStringFree(&ds);

    if (printerhandle) {
        Tcl_DStringAppend(&printername, Tcl_GetString(printer), -1);
        return TCL_OK;
    } else {
        SetError(::GetLastError());
        return TCL_ERROR;
    }
}

int TclPrintRawCmd::StartDoc(Tcl_Obj *document, Tcl_Obj *output) {
    int result;
    Tcl_DString documentds, outputds;
    Tcl_DStringInit(&documentds);
    Tcl_DStringInit(&outputds);

    DOC_INFO_1 docinfo;
    ZeroMemory(&docinfo, sizeof(DOC_INFO_1));
    docinfo.pDatatype = _T("RAW");
    if (document) {
        UtfToExternal(Tcl_GetString(document), &documentds);
        docinfo.pDocName = (TCHAR *)Tcl_DStringValue(&documentds);
    }
    if (output) {
        UtfToExternal(Tcl_GetString(output), &outputds);
        docinfo.pOutputFile = (TCHAR *)Tcl_DStringValue(&outputds);
    }
    documenthandle = _(::StartDocPrinter(printerhandle, 1, (LPBYTE)&docinfo));
    if (documenthandle == 0) {
        SetError(::GetLastError());
//      pageno = 0;
        result = TCL_ERROR;
    } else
        result = TclPrintCmd::StartDoc(document, output);

    Tcl_DStringFree(&outputds);
    Tcl_DStringFree(&documentds);
    return result;
}

int TclPrintRawCmd::AbortDoc() {
    return TCL_ERROR;
}

int TclPrintRawCmd::EndDoc() {
    if (pageactive) 
        EndPage(); // ignore errors
    if (!_(::EndDocPrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        return TclPrintCmd::EndDoc();
    }
}

int TclPrintRawCmd::StartPage() {
    if (!_(::StartPagePrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        return TclPrintCmd::StartPage();
    }
}

int TclPrintRawCmd::EndPage() {
    if (!_(::EndPagePrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        return TclPrintCmd::EndPage();
    }
}

int TclPrintRawCmd::Write(Tcl_Obj *data) {
    int length;
    void *bytes = Tcl_GetByteArrayFromObj(data, &length);
    DWORD written;

    if (_(::WritePrinter(printerhandle, bytes, length, &written))) {
        return written;
    } else {
        SetError(::GetLastError());
        return -1;
    }
}

BOOL TclPrintRawCmd::Opened() {
    return printerhandle != NULL;
}
