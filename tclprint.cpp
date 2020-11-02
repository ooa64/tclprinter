#include "tclprint.hpp"

BOOL TclPrintCmd::Started() {
    return documentactive;
}

BOOL TclPrintCmd::PageStarted() {
    return pageactive;
}

void TclPrintCmd::Close() {
    SetError(0);
    Tcl_DStringFree(&printername);
    Tcl_DStringInit(&printername);
}

int TclPrintCmd::StartDoc(Tcl_Obj *document, Tcl_Obj *output) {
    SetError(0);
    documentactive = TRUE;
    return TCL_OK;
}

int TclPrintCmd::AbortDoc() {
    SetError(0);
    pageno = 0;
    pageactive = FALSE;
    documentactive = FALSE;
    return TCL_OK;
}

int TclPrintCmd::EndDoc() {
    SetError(0);
    pageno = 0;
    pageactive = FALSE;
    documentactive = FALSE;
    return TCL_OK;
}

int TclPrintCmd::StartPage() {
    SetError(0);
    pageno++;
    pageactive = TRUE;
    return TCL_OK;
}

int TclPrintCmd::EndPage() {
    SetError(0);
    pageactive = FALSE;
    return TCL_OK;
}

int TclPrintCmd::ParseStartParams(int objc, Tcl_Obj *CONST objv[], int *obji, Tcl_Obj **document, Tcl_Obj **output) {
    if (*obji < objc && strcmp(Tcl_GetString(objv[*obji]), "-document") == 0) {
        (*obji)++;
        if (*obji < objc) {
            *document = objv[*obji];
            (*obji)++;
        } else {
            Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
            return TCL_ERROR;
        }
    }
    if (*obji < objc && strcmp(Tcl_GetString(objv[*obji]), "-output") == 0) {
        (*obji)++;
        if (*obji < objc) {
            *output = objv[*obji];
            (*obji)++;
        } else {
            Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
            return TCL_ERROR;
        }
    }
    return TCL_OK;
}
