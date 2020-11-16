// $Id$

#include "tclprinter.hpp"
#include "tclprintraw.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif


int TclPrintRawCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static CONST char *commands[] = {
        "info",
        "status",
        "document",
        "start",
        "end",
        "startpage",
        "endpage",
        "write",
        "close",
        "abort",
        0L
    };
    enum commands {
        cmInfo,
        cmStatus,
        cmDocument,
        cmStart,
        cmEnd,
        cmStartPage,
        cmEndPage,
        cmWrite,
        cmClose,
        cmAbort,
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(tclInterp, 1, objv, "subcommand");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(tclInterp, objv[1], commands, "subcommand", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

//  Tcl_ResetResult(tclInterp);
//  Tcl_SetErrorCode(tclInterp, NULL);

    switch ((enum commands)(index)) {

    case cmInfo:
        if (objc == 3) {
            if (strcmp(Tcl_GetString(objv[2]), "protocol") == 0)
                Tcl_SetResult(tclInterp, (char *)"raw", NULL);
            else if (strcmp(Tcl_GetString(objv[2]), "name") == 0)
                Tcl_SetResult(tclInterp, Tcl_DStringValue(&printername), NULL);
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
        return TCL_ERROR;

    case cmDocument:
        if (objc < 3) {
            Tcl_WrongNumArgs(tclInterp, 2, objv, "select ?selectspec? | status ?id? | cancel ?id? | delete ?id? | pause ?id? | resume ?id? | restart ?id? | retain ?id? | release ?id?");
            return TCL_ERROR;
        }
        if (!Opened()) {
            Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
            return TCL_ERROR;
        }
        if (strcmp(Tcl_GetString(objv[2]), "names") == 0 && objc == 3) {
            Tcl_Obj *documents = Tcl_NewObj();
            if (DocumentSelect(documents, Tcl_NewStringObj("document", -1), NULL, NULL, NULL, NULL) == TCL_OK) {
                Tcl_SetObjResult(tclInterp, documents);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (strcmp(Tcl_GetString(objv[2]), "select") == 0) { 
            Tcl_Obj *selectlist = NULL;
            Tcl_Obj *printerpattern = NULL;
            Tcl_Obj *machinepattern = NULL;
            Tcl_Obj *userpattern = NULL;
            Tcl_Obj *documentpattern = NULL;
            int obji = 3;
            if (ParseDocumentSelectParams(objc, objv, &obji, &selectlist, &printerpattern, &machinepattern, &userpattern, &documentpattern) != TCL_OK) {
                return TCL_ERROR;
            }
            if (obji < objc) {
                Tcl_WrongNumArgs(tclInterp, 3, objv, "?outputlist? ?-printer pattern? ?-machine pattern? ?-user pattern? ?-document pattern?");
                return TCL_ERROR;
            }
            Tcl_Obj *documents = Tcl_NewObj();
            if (DocumentSelect(documents, selectlist, printerpattern, machinepattern, userpattern, documentpattern) == TCL_OK) {
                Tcl_SetObjResult(tclInterp, documents);
                return TCL_OK;
            }
            return TCL_ERROR;
        }
        if (objc < 5) {
            Tcl_Obj *answer = Tcl_NewObj();
            if (DocumentCommand(answer, objv[2], (objc < 4 ? NULL : objv[3])) == TCL_OK) {
                Tcl_SetObjResult(tclInterp, answer);
                return TCL_OK;
            } else
                AppendSystemError(tclInterp, "error processing document: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, "select ?selectspec? | id ?id? | status ?id? | cancel ?id? | delete ?id? | pause ?id? | resume ?id? | restart ?id? | retain ?id? | release ?id?");
        }
        return TCL_ERROR;

    case cmStart:
        {
            Tcl_Obj *document = NULL;
            Tcl_Obj *output = NULL;
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
            if (StartDoc(document, output) == TCL_OK)
                return TCL_OK;
            else 
                return AppendSystemError(tclInterp, "error starting document: ", GetError());
        }
        return TCL_ERROR;

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
            int written = Write(objv[objc-1]);
            if (written >= 0) {
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(written));
                return TCL_OK;
            } else
                return AppendSystemError(tclInterp, "error writing data: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    case cmStatus:
        if (!Opened()) {
            Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            Tcl_Obj *status = Tcl_NewObj();
            if (Status(status) == TCL_OK) {
               Tcl_SetObjResult(tclInterp, status);
               return TCL_OK;
            } else
               return AppendSystemError(tclInterp, "error quering status: ", GetError());
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

    case cmAbort:
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

    return TCL_ERROR;
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

void TclPrintRawCmd::Abort() {
    if (Opened())
        _(::AbortPrinter(printerhandle));
    printerhandle = NULL;
    TclPrintCmd::EndPage();
    TclPrintCmd::EndDoc();
    TclPrintCmd::Close();
}

#ifdef DIALOGS
int TclPrintRawCmd::Select (BOOL setupdialog) {
    int result = TCL_ERROR;
    PRINTDLG printdlg = {0};
    ZeroMemory(&printdlg, sizeof(printdlg));
    printdlg.lStructSize = sizeof(printdlg);
    printdlg.Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION;
    if (setupdialog)
        printdlg.Flags |= PD_PRINTSETUP;

    Close();

    if (_(::PrintDlg(&printdlg))) {
        if (printdlg.hDevMode != NULL && printdlg.hDevNames != NULL) {
            LPDEVNAMES devnames = (LPDEVNAMES)GlobalLock(printdlg.hDevNames) ;
            if (_(::OpenPrinter((TCHAR *)((LPCTSTR)devnames+devnames->wDeviceOffset), &printerhandle, NULL))) {
                ExternalToUtf((TCHAR *)((LPCTSTR)devnames+devnames->wDeviceOffset), &printername);
                result = TCL_OK;
            }
            GlobalUnlock(printdlg.hDevNames);
        }
    }
    if (result != TCL_OK)
        SetError(::GetLastError());
    return result;
}
#endif

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

int TclPrintRawCmd::OpenDefault() {
    int result = TCL_ERROR;
    TCHAR* name;

    Close();

    if (NewDefaultPrinterName(&name)) {
        if (_(::OpenPrinter(name, &printerhandle, NULL))) {
            ExternalToUtf(name, &printername);
            result = TCL_OK;
         }
         ckfree((char *)name);
    }

    if (result != TCL_OK)
        SetError(::GetLastError());
    return result;
}

BOOL TclPrintRawCmd::Opened() {
    return printerhandle != NULL;
}

int TclPrintRawCmd::StartDoc(Tcl_Obj *document, Tcl_Obj *output) {
    int result = TCL_ERROR;
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
    if (documenthandle == 0)
        SetError(::GetLastError());
    else
        result = TclPrintCmd::StartDoc(document, output);

    Tcl_DStringFree(&outputds);
    Tcl_DStringFree(&documentds);
    return result;
}

int TclPrintRawCmd::EndDoc() {
    if (pageactive) 
        EndPage(); // ignore errors
    if (!_(::EndDocPrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else
        return TclPrintCmd::EndDoc();
}

int TclPrintRawCmd::StartPage() {
    if (!_(::StartPagePrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else
        return TclPrintCmd::StartPage();
}

int TclPrintRawCmd::EndPage() {
    if (!_(::EndPagePrinter(printerhandle))) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else
        return TclPrintCmd::EndPage();
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

int TclPrintRawCmd::WriteDoc(Tcl_Obj *document, Tcl_Obj *output, Tcl_Obj *data) {
    int written = -1;
    if (StartDoc(document, output) == TCL_OK) {
        written = Write(data);
        if (written >= 0)
            EndDoc();
        else
            AbortDoc();
    } else
        AbortDoc();
    return written;
}

int TclPrintRawCmd::Status(Tcl_Obj *statuslist) {
    static DWORD status[] = {
        PRINTER_STATUS_BUSY,
        PRINTER_STATUS_DOOR_OPEN,	
        PRINTER_STATUS_ERROR,	
        PRINTER_STATUS_INITIALIZING,
        PRINTER_STATUS_IO_ACTIVE,
        PRINTER_STATUS_MANUAL_FEED,
        PRINTER_STATUS_NO_TONER,
        PRINTER_STATUS_NOT_AVAILABLE,
        PRINTER_STATUS_OFFLINE,
        PRINTER_STATUS_OUT_OF_MEMORY,
        PRINTER_STATUS_OUTPUT_BIN_FULL,
        PRINTER_STATUS_PAGE_PUNT,
        PRINTER_STATUS_PAPER_JAM,	
        PRINTER_STATUS_PAPER_OUT,	
        PRINTER_STATUS_PAPER_PROBLEM,
        PRINTER_STATUS_PAUSED,
        PRINTER_STATUS_PENDING_DELETION,
        PRINTER_STATUS_POWER_SAVE,
        PRINTER_STATUS_PRINTING,	
        PRINTER_STATUS_PROCESSING,
        PRINTER_STATUS_SERVER_UNKNOWN,
        PRINTER_STATUS_TONER_LOW,
        PRINTER_STATUS_USER_INTERVENTION,
        PRINTER_STATUS_WAITING,
        PRINTER_STATUS_WARMING_UP
    };
    static CONST char *statusMap[] = {
        "busy",
        "dooropen",
        "error",
        "initializing",
        "ioactive",
        "manualfeed",
        "notoner",
        "notavailable",
        "offline",
        "outofmemory",
        "outputbinfull",
        "pagepunt",
        "paperjam",
        "paperout",
        "paperproblem",
        "paused",
        "pendingdeletion",
        "powersave",
        "printing",
        "processing",
        "serverunknown",
        "tonerlow",
        "userintervention",
        "waiting",
        "warmingup"
    };
    int result = TCL_ERROR;
    PRINTER_INFO_2 *info = NULL;
    DWORD size = 0;

    _(::GetPrinter(printerhandle, 2, 0, 0, &size));
    if (size > 0) {
        info = (PRINTER_INFO_2 *)ckalloc(size);
//      this windows call does not work as expected (
        if (_(::GetPrinter(printerhandle, 2, (LPBYTE)info, size, &size))) {
            for (unsigned i = 0; i < SIZEOFARRAY(status); i++)
                if ((info->Status & status[i]))
                    Tcl_ListObjAppendElement(NULL, statuslist, Tcl_NewStringObj(statusMap[i], -1));
            if ((info->Attributes & PRINTER_ATTRIBUTE_WORK_OFFLINE))
                Tcl_ListObjAppendElement(NULL, statuslist, Tcl_NewStringObj("workoffline", -1));
            result = TCL_OK;
        }
        ckfree((char *)info);
    }
    SetError(result == TCL_OK ? 0 : ::GetLastError());
    return result;
}

int TclPrintRawCmd::DocumentSelect(Tcl_Obj *documentlist, Tcl_Obj *selectlist, Tcl_Obj *printerpattern, Tcl_Obj *machinepattern, Tcl_Obj *userpattern, Tcl_Obj *documentpattern) {
    int result = TCL_ERROR;
    DWORD size = 0;

    _(::GetPrinter(printerhandle, 2, 0, 0, &size));
    if (size > 0) {
        PRINTER_INFO_2 *printerinfo = (PRINTER_INFO_2 *)ckalloc(size);
        if (_(::GetPrinter(printerhandle, 2, (LPBYTE)printerinfo, size, &size))) {
            DWORD documentscount = printerinfo->cJobs;
            if (documentscount) {
                size = 0;
                _(::EnumJobs(printerhandle, 0, documentscount, 1, NULL, 0, &size, &documentscount));
                if (size > 0) {
                    JOB_INFO_1 *jobinfo = (JOB_INFO_1 *)ckalloc(size);
                    if (_(::EnumJobs(printerhandle, 0, 0xFFFFFFFF, 1, (LPBYTE)jobinfo, size, &size, &documentscount))) {
                        Tcl_Obj *documentelem;
                        Tcl_Obj *printer;
                        Tcl_Obj *machine;
                        Tcl_Obj *user;
                        Tcl_Obj *document;
                        int objc = 0;
                        Tcl_Obj **objv = NULL;
                        if (!selectlist || Tcl_ListObjGetElements(tclInterp, selectlist, &objc, &objv) == TCL_OK) {
                            for (unsigned i = 0; i < documentscount; i++) {
                                printer = NewObjFromExternal(jobinfo[i].pPrinterName);
                                machine = NewObjFromExternal(jobinfo[i].pMachineName);
                                user = NewObjFromExternal(jobinfo[i].pUserName);
                                document = NewObjFromExternal(jobinfo[i].pDocument);
                                if ((!printerpattern || Tcl_StringCaseMatch(Tcl_GetString(printer), Tcl_GetString(printerpattern), 1)) &&
                                        (!machinepattern || Tcl_StringCaseMatch(Tcl_GetString(machine), Tcl_GetString(machinepattern), 1)) &&
                                        (!userpattern || Tcl_StringCaseMatch(Tcl_GetString(user), Tcl_GetString(userpattern), 1)) &&
                                        (!documentpattern || Tcl_StringCaseMatch(Tcl_GetString(document), Tcl_GetString(documentpattern), 1))) {
                                    if (!selectlist) {
                                        documentelem = Tcl_NewObj();
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewStringObj("id", -1));
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewIntObj(jobinfo[i].JobId));
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewStringObj("printer", -1));
                                        Tcl_ListObjAppendElement(NULL, documentelem, printer);
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewStringObj("machine", -1));
                                        Tcl_ListObjAppendElement(NULL, documentelem, machine);
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewStringObj("user", -1));
                                        Tcl_ListObjAppendElement(NULL, documentelem, user);
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewStringObj("document", -1));
                                        Tcl_ListObjAppendElement(NULL, documentelem, document);
                                        Tcl_ListObjAppendElement(NULL, documentlist, documentelem);
                                    } else if (!objc) {
                                        documentelem = Tcl_NewObj();
                                        Tcl_ListObjAppendElement(NULL, documentelem, Tcl_NewIntObj(jobinfo[i].JobId));
                                        Tcl_ListObjAppendElement(NULL, documentelem, printer);
                                        Tcl_ListObjAppendElement(NULL, documentelem, machine);
                                        Tcl_ListObjAppendElement(NULL, documentelem, user);
                                        Tcl_ListObjAppendElement(NULL, documentelem, document);
                                        Tcl_ListObjAppendElement(NULL, documentlist, documentelem);
                                    } else {
                                        for (int obji = 0; obji < objc; obji++) {
                                            if (strcmp(Tcl_GetString(objv[obji]), "id") == 0)
                                               Tcl_ListObjAppendElement(NULL, documentlist, Tcl_NewIntObj(jobinfo[i].JobId));
                                            else if (strcmp(Tcl_GetString(objv[obji]), "printer") == 0)
                                               Tcl_ListObjAppendElement(NULL, documentlist, printer);
                                            else if (strcmp(Tcl_GetString(objv[obji]), "machine") == 0)
                                               Tcl_ListObjAppendElement(NULL, documentlist, machine);
                                            else if (strcmp(Tcl_GetString(objv[obji]), "user") == 0)
                                               Tcl_ListObjAppendElement(NULL, documentlist, user);
                                            else if (strcmp(Tcl_GetString(objv[obji]), "document") == 0)
                                               Tcl_ListObjAppendElement(NULL, documentlist, document);
                                            else 
                                               Tcl_ListObjAppendElement(NULL, documentlist, objv[obji]);
                                        }
                                    }
                                }
                            }
                            result = TCL_OK;
                        }
                    }
                    ckfree((char *)jobinfo);
                }
            } else // no documents at all
                result = TCL_OK;
        }
        ckfree((char *)printerinfo);
    }
    SetError(result == TCL_OK ? 0 : ::GetLastError());
    return result;
}

#ifndef JOB_CONTROL_RETAIN
#define JOB_CONTROL_RETAIN 8
#endif
#ifndef JOB_CONTROL_RELEASE
#define JOB_CONTROL_RELEASE 9
#endif

int TclPrintRawCmd::DocumentCommand(Tcl_Obj *answer, Tcl_Obj *command, Tcl_Obj *documentid) {
    static CONST char *commandsMap[] = {
        "id",
        "status",
        "cancel",
        "delete",
        "pause",
        "resume",
        "restart",
        "retain",
        "release",
        0L
    };
    static DWORD commands[] = {
         0,
         0,
         JOB_CONTROL_CANCEL,
         JOB_CONTROL_DELETE,
         JOB_CONTROL_PAUSE,
         JOB_CONTROL_RESUME,
         JOB_CONTROL_RESTART,
         JOB_CONTROL_RETAIN,
         JOB_CONTROL_RELEASE,
    };
    DWORD handle = 0;
    if (documentid) {
        if (Tcl_GetIntFromObj(tclInterp, documentid, (int *)&handle) != TCL_OK)
            return TCL_ERROR;
    } else
        handle = documenthandle;

    int index = 0;
    if (Tcl_GetIndexFromObj(tclInterp, command, commandsMap, "request", 0, &index) != TCL_OK)
        return TCL_ERROR;
    if (index == 0) {
        Tcl_SetIntObj(answer, handle);
        return TCL_OK;
    }
    if (index == 1)
        return DocumentStatus(answer, handle);
    if (_(::SetJob(printerhandle, handle, 0, NULL, commands[index])))
        return TCL_OK;

    SetError(::GetLastError());
    return TCL_ERROR;
}

#ifndef JOB_STATUS_COMPLETE
#define JOB_STATUS_COMPLETE 0x00001000
#endif
#ifndef JOB_STATUS_RETAINED
#define JOB_STATUS_RETAINED 0x00002000
#endif
#ifndef JOB_STATUS_RENDERING_LOCALLY
#define JOB_STATUS_RENDERING_LOCALLY 0x00004000
#endif

int TclPrintRawCmd::DocumentStatus(Tcl_Obj *statuslist, DWORD handle) {
    static DWORD status[] = {
        JOB_STATUS_PAUSED,
        JOB_STATUS_ERROR,
        JOB_STATUS_DELETING,
        JOB_STATUS_SPOOLING,
        JOB_STATUS_PRINTING,
        JOB_STATUS_OFFLINE,
        JOB_STATUS_PAPEROUT,
        JOB_STATUS_PRINTED,
        JOB_STATUS_DELETED,
        JOB_STATUS_BLOCKED_DEVQ,
        JOB_STATUS_USER_INTERVENTION,
        JOB_STATUS_RESTART,
        JOB_STATUS_COMPLETE,
        JOB_STATUS_RETAINED,
        JOB_STATUS_RENDERING_LOCALLY,
    };
    static char const *statusMap[] = {
        "paused",
        "error",
        "deleting",
        "spooling",
        "printing",
        "offline",
        "paperout",
        "printed",
        "deleted",
        "blockeddevq",
        "userintervention",
        "restart",
        "complete",
        "retained",
        "renderedlocally",
        0L
    };
    int result = TCL_ERROR;
    JOB_INFO_1 *info = NULL;
    DWORD size = 0;
    if (!handle)
        handle = documenthandle;
    _(::GetJob(printerhandle, handle, 1, NULL, size, &size));
    if (size > 0) {
        info = (JOB_INFO_1 *)ckalloc(size);
        if (_(::GetJob(printerhandle, handle, 1, (LPBYTE)info, size, &size))) {
            if (info->pStatus)
                Tcl_ListObjAppendElement(NULL, statuslist, NewObjFromExternal(info->pStatus));
            else
                for (unsigned i = 0; i < SIZEOFARRAY(status); i++) {
                    if ((info->Status & status[i]))
                        Tcl_ListObjAppendElement(NULL, statuslist, Tcl_NewStringObj(statusMap[i], -1));
                }
            result = TCL_OK;
        }
        ckfree((char *)info);
    }
    SetError(result == TCL_OK ? 0 : ::GetLastError());
    return result;
}

int TclPrintRawCmd::ParseDocumentSelectParams(int objc, Tcl_Obj *CONST objv[], int *obji, Tcl_Obj **selectlist, Tcl_Obj **printerpattern, Tcl_Obj **machinepattern, Tcl_Obj **userpattern, Tcl_Obj **documentpattern) {
    if (*obji < objc) {
        if (strncmp(Tcl_GetString(objv[*obji]), "-", 1) != 0) {
            *selectlist = objv[*obji];
            (*obji)++;
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-printer") == 0) {
            (*obji)++;
            if (*obji < objc) {
                *printerpattern = objv[*obji];
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-machine") == 0) {
            (*obji)++;
            if (*obji < objc) {
                *machinepattern = objv[*obji];
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-user") == 0) {
            (*obji)++;
            if (*obji < objc) {
                *userpattern = objv[*obji];
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-document") == 0) {
            (*obji)++;
            if (*obji < objc) {
                *documentpattern = objv[*obji];
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[*obji-1]), NULL);
                return TCL_ERROR;
            } 
        }
    }
    return TCL_OK;
}
