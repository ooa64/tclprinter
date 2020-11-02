// $Id$

#include "tclprinter.hpp"
#include "tclprint.hpp"
#include "tclprintgdi.hpp"
#include "tclprintraw.hpp"

#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif

void UtfToExternal(CONST char *utf, Tcl_DString *external) {
#ifdef UNICODE
    Tcl_WinUtfToTChar(utf, -1, external);
#else
    Tcl_UtfToExternalDString(NULL, utf, -1, external);
#endif
}

void ExternalToUtf(CONST TCHAR *external, Tcl_DString *utf) {
#ifdef UNICODE
    Tcl_WinTCharToUtf(external, -1, utf);
#else
    Tcl_ExternalToUtfDString(NULL, external, -1, utf);
#endif
}

int AppendSystemError (Tcl_Interp *interp, char *prefix, DWORD error) {
    int length;
    char *msg;
    char id[TCL_INTEGER_SPACE], msgBuf[24 + TCL_INTEGER_SPACE];
    WCHAR *wMsgPtr, **wMsgPtrPtr = &wMsgPtr;
    Tcl_DString ds;

    length = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (WCHAR *) wMsgPtrPtr,
            0, NULL);
    if (length == 0) {
        char *msgPtr;

        length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM
                | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *) &msgPtr,
                0, NULL);
        if (length > 0) {
            wMsgPtr = (WCHAR *) LocalAlloc(LPTR, (length + 1) * sizeof(WCHAR));
            MultiByteToWideChar(CP_ACP, 0, msgPtr, length + 1, wMsgPtr,
                    length + 1);
            LocalFree(msgPtr);
        }
    }
    if (length == 0) {
        if (error == ERROR_CALL_NOT_IMPLEMENTED) {
            msg = "function not supported under Win32s";
        } else {
            sprintf(msgBuf, "unknown error: %ld", error);
            msg = msgBuf;
        }
    } else {
        Tcl_Encoding encoding;

        encoding = Tcl_GetEncoding(NULL, "unicode");
        Tcl_ExternalToUtfDString(encoding, (char *) wMsgPtr, -1, &ds);
        Tcl_FreeEncoding(encoding);
        LocalFree(wMsgPtr);

        msg = Tcl_DStringValue(&ds);
        length = Tcl_DStringLength(&ds);

        /*
         * Trim the trailing CR/LF from the system message.
         */

        if (msg[length-1] == '\n') {
            msg[--length] = 0;
        }
        if (msg[length-1] == '\r') {
            msg[--length] = 0;
        }
    }

    sprintf(id, "%ld", error);
    Tcl_SetErrorCode(interp, "WINDOWS", id, msg, NULL);
    Tcl_AppendResult(interp, prefix, msg, NULL);

    if (length != 0) {
        Tcl_DStringFree(&ds);
    }
    return TCL_ERROR;
}

/*
printer info default | names ?-verbose?
printer print ? -gdi | -raw ?
              ? -default | select  | -name printername?
              ? -docname document ?
              ? -wordwrap ?
              ? -margins {?left? ?top? ?right? ?bottom?} ?
              ? -font {name ?height? ?width? ?weight? ?style? ?charset? ?quality? ?pitch? ?family?} ?
              data
printer open  ? -gdi | -raw | xps ?
              ? -default | select  | -name printername?
              printcontextcommand
*/

int TclPrinterCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static char *commands[] = {
        "names",
        "print",
        "write",
        "open",
        0L
    };
    enum commands {
        cmNames,
        cmPrint,
        cmWrite,
        cmOpen
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

    case cmNames:
        if (Names(Tcl_GetObjResult(tclInterp)) == TCL_OK)
            return TCL_OK;
        else
            return AppendSystemError(tclInterp, "can't query names: ", lasterrorcode);

    case cmOpen:
    case cmPrint:
    case cmWrite:
        {
            interfaces printerinterface = ifUnknown;
            selections printerselection = seUnknown;
            Tcl_Obj *printername = NULL;
            Tcl_Obj *commandname;
            TclPrintCmd *command;
            int obji = 2;

            if (ParseOpenParams(objc, objv, &obji, &printerinterface, &printerselection, &printername) != TCL_OK)
                return TCL_ERROR;

            switch (printerinterface) {
                case ifUnknown:
                    switch ((enum commands)index) {
                        case cmPrint: printerinterface = ifGdi; break;
                        case cmWrite: printerinterface = ifRaw; break;
                        default:
                            {
                                printerinterface = ifGdi; break;
                                //Tcl_AppendResult(tclInterp, "printer interface not specified", NULL);
                                //return TCL_ERROR;
                            }
                    }
                    break;
                case ifGdi:
                    if ((enum commands)index != cmPrint && (enum commands)index != cmOpen) {
                        Tcl_AppendResult(tclInterp, "invalid printer interface specified", NULL);
                        return TCL_ERROR;
                    }
                    break;
                case ifRaw:
                    if ((enum commands)index != cmWrite && (enum commands)index != cmOpen) {
                        Tcl_AppendResult(tclInterp, "invalid printer interface specified", NULL);
                        return TCL_ERROR;
                    }
                    break;
                default:
                    return TCL_ERROR;
            }

            if ((enum commands)index != cmOpen)
                commandname = NewPrintNameObj();
            else if (obji == objc)
                commandname = NewPrintNameObj();
            else if (obji == objc-1)
                commandname = objv[objc-1];
            else {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi|-raw? ?-default|-select|-name printername? ?command?");
                return TCL_ERROR;
            }

            switch (printerinterface) {
                case ifGdi: command = new TclPrintGdiCmd(tclInterp, Tcl_GetString(commandname)); break;
                case ifRaw: command = new TclPrintRawCmd(tclInterp, Tcl_GetString(commandname)); break;
                default: return TCL_ERROR;
            }

            int result = TCL_OK;
            switch (printerselection) {
                case seUnknown:
                case seDefault: result = command->Select(TRUE); break;
                case seNamed: result = command->Open(printername); break;
                case seSelect: result = command->Select(FALSE); break;
                default: result = TCL_ERROR;
            }
            if (result != TCL_OK) {
                if (command->GetError() == 0)
                   Tcl_AppendResult(tclInterp, "cancelled", NULL);
                else
                   AppendSystemError(tclInterp, "can't open printer: ", command->GetError());
            } else if ((enum commands)index == cmOpen) {
                Tcl_SetObjResult(tclInterp, commandname);
                return TCL_OK;
            }

            Tcl_Obj *document = NULL;
            Tcl_Obj *output = NULL;

            if (result == TCL_OK)
                result = command->ParseStartParams(objc, objv, &obji, &document, &output);

            switch ((enum commands)index) {
                case cmPrint: 
                    {
                        int wrap = FALSE;
                        LOGFONT font = {};
                        RECT rect = {0, 0, _(::GetDeviceCaps(((TclPrintGdiCmd *)command)->GetDC(), HORZRES)), 
                                           _(::GetDeviceCaps(((TclPrintGdiCmd *)command)->GetDC(), VERTRES))};

                        if (result == TCL_OK)
                            result = ((TclPrintGdiCmd *)command)->ParsePrintParams(objc, objv, &obji, &rect, &font, &wrap);

                        if (result == TCL_OK)
                            if (obji == objc-1)
                                if ((result = ((TclPrintGdiCmd *)command)->Configure(&font)) == TCL_OK)
                                    if ((result = ((TclPrintGdiCmd *)command)->PrintDoc(document, output, &rect, wrap, objv[obji])) == TCL_OK)
                                        Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(result));
                                    else
                                        AppendSystemError(tclInterp, "error printing text: ", GetError());
                                else 
                                    AppendSystemError(tclInterp, "error configuring device context: ", command->GetError());
                            else {
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi? ?-default|-select|-name printername? ?-document documentname? ?-output filename? ?-margins rectspec? ?-font fontspec? ?-wordwrap? text");
                                result = TCL_ERROR;
                            }
                    }
                    break;
                case cmWrite:
                    {
                        DWORD written;
                        if (result == TCL_OK)
                            if (obji == objc-1)
                                if ((result = ((TclPrintRawCmd *)command)->StartDoc(document, output)) == TCL_OK)
                                    if ((written = ((TclPrintRawCmd *)command)->Write(objv[obji])) >= 0)
                                        if ((result = ((TclPrintRawCmd *)command)->EndDoc()) == TCL_OK)
                                            Tcl_SetObjResult(tclInterp, Tcl_NewLongObj(written));
                                        else {
                                            AppendSystemError(tclInterp, "error ending document: ", command->GetError());
                                            ((TclPrintRawCmd *)command)->AbortDoc();
                                        }
                                    else {
                                        AppendSystemError(tclInterp, "error printing text: ", command->GetError());
                                        ((TclPrintRawCmd *)command)->AbortDoc();
                                    }
                                else {
                                    AppendSystemError(tclInterp, "error starting document: ", command->GetError());
                                    ((TclPrintRawCmd *)command)->AbortDoc();
                                }
                            else {
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-raw? ?-default|-select|-name printername? ?-document documentname? ?-output filename data");
                                result = TCL_ERROR;
                            }
                    }
                    break;
            }
            delete command;
            return result;
        }

    } // switch index

    return TCL_ERROR;
}

int TclPrinterCmd::Names (Tcl_Obj *result) {
    DWORD buffersize = 0;
    DWORD printerscount = 0;
    if (!_(::EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, NULL, 0, &buffersize, &printerscount))) {
        SetError(::GetLastError());
        if (lasterrorcode == ERROR_INSUFFICIENT_BUFFER)
            lasterrorcode = 0;
        else
            return TCL_ERROR;
    }
    if (buffersize != 0) {
        PRINTER_INFO_2 *pi = (PRINTER_INFO_2 *)ckalloc(buffersize);
        if (!_(::EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE)pi, buffersize, &buffersize, &printerscount))) {
            SetError(::GetLastError());
            ckfree((char *)pi);
            return TCL_ERROR;
        }
        for (DWORD i = 0; i < printerscount; i++) {
            Tcl_DString ds;
            Tcl_DStringInit(&ds);
            ExternalToUtf(pi[i].pPrinterName, &ds);
            Tcl_ListObjAppendElement(tclInterp, result, Tcl_NewStringObj(Tcl_DStringValue(&ds), -1));
            Tcl_DStringFree(&ds);
        }
        ckfree((char *)pi);
        return TCL_OK;
    } else
        return TCL_ERROR;
}

int TclPrinterCmd::ParseOpenParams(int objc, Tcl_Obj *CONST objv[], int *obji, 
                                   interfaces *printerinterface, 
                                   selections *printerselection, 
                                   Tcl_Obj **printername) {
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-gdi") == 0) {
            *printerinterface = ifGdi;
            (*obji)++;
        } else if (strcmp(Tcl_GetString(objv[*obji]), "-raw") == 0) {
            *printerinterface = ifRaw;
            (*obji)++;
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-default") == 0) {
            *printerselection = seDefault;
            (*obji)++;
        } else if (strcmp(Tcl_GetString(objv[*obji]), "-select") == 0) {
            *printerselection = seSelect;
            (*obji)++;
        } else if (strcmp(Tcl_GetString(objv[*obji]), "-name") == 0) {
            *printerselection = seNamed;
            (*obji)++;
            if (*obji < objc) {
                *printername = objv[*obji];
                (*obji)++;
            } else {
                Tcl_AppendResult(tclInterp, "printer name required for -name", NULL);
                return TCL_ERROR;
            }
        }
    }
    return TCL_OK;
}


Tcl_Obj *TclPrinterCmd::NewPrintNameObj() {
    static int suffix = 0;
    char buffer[8 + TCL_INTEGER_SPACE];
    Tcl_CmdInfo info;

    do {
        sprintf(buffer, "printer%d", suffix);
        suffix++;
    } while (Tcl_GetCommandInfo(tclInterp, buffer, &info) != 0);

    return Tcl_NewStringObj(buffer, -1);
}

#undef TCL_STORAGE_CLASS
#define TCL_STORAGE_CLASS DLLEXPORT

EXTERN int Tclprinter_Init (Tcl_Interp *interp) {
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8.1", 0) == 0L) {
    return TCL_ERROR;
  }
#endif

  new TclPrinterCmd(interp, "printer");

  Tcl_PkgProvide(interp, "tclprinter", "1.0");

  return TCL_OK;
}
                         
