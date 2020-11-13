// $Id$

#include "tclprinter.hpp"
#include "tclprint.hpp"
#include "tclprintgdi.hpp"
#include "tclprintraw.hpp"

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

Tcl_Obj *NewObjFromExternal(CONST TCHAR *external) {
    if (external) {
        Tcl_Obj *obj;
        Tcl_DString ds;
        Tcl_DStringInit(&ds);
        ExternalToUtf(external, &ds);
        obj = Tcl_NewStringObj(Tcl_DStringValue(&ds), -1);
        Tcl_DStringFree(&ds);
        return obj;
    } else
        return NULL;
}

#ifdef GetDefaultPrinter

TCHAR *NewDefaultPrinterName1(TCHAR **name) {
    DWORD size = 0;
    _(::GetDefaultPrinter(NULL, &size));
    if (size > 0) {
        *name = (TCHAR *)ckalloc(size * sizeof(TCHAR));
        if (_(::GetDefaultPrinter(*name, &size))) {
            return *name;
        }
        ckfree((char *)(*name));
    }
    return NULL;
}

#else

#ifdef UNICODE
  #define GETDEFAULTPRINTER "GetDefaultPrinterW"
#else
  #define GETDEFAULTPRINTER "GetDefaultPrinterA"
#endif

typedef int (CALLBACK *GDP)(LPTSTR ,LPDWORD);

TCHAR *NewDefaultPrinterName1(TCHAR **name) {
    HMODULE hlib = LoadLibrary(_T("winspool.drv"));
    *name = NULL;
    if (hlib) {
        GDP fnGetDefaultPrinter = (GDP)_(::GetProcAddress(hlib, GETDEFAULTPRINTER));
        if (fnGetDefaultPrinter) {
            DWORD size = 0;
            fnGetDefaultPrinter(NULL, &size);
            if (size > 0) {
                *name = (TCHAR *)ckalloc(size * sizeof(TCHAR));
                if (!fnGetDefaultPrinter(*name, &size)) {
                    ckfree((char *)(*name));
                    *name = NULL;
                }
            }
        }
        FreeLibrary(hlib);
    }
    return *name;
}

#endif

TCHAR *NewDefaultPrinterName2(TCHAR **name) {
    DWORD size = 0;
    DWORD count = 0;
    *name = NULL;
    _(::EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, NULL, 0, &size, &count));
    if (size > 0) {
        PRINTER_INFO_2 *pi = (PRINTER_INFO_2 *)ckalloc(size);
        if (_(::EnumPrinters(PRINTER_ENUM_DEFAULT, NULL, 2, (LPBYTE)pi, size, &size, &count))) {
            size_t strsize = _tcsclen(pi->pPrinterName);
            (*name) = (TCHAR *)ckalloc((strsize + 1) * sizeof(TCHAR));
            _tcscpy(*name, pi->pPrinterName);
        }
        ckfree((char *)(pi));
    }
    return *name;
}

#define MAXBUFFERSIZE 256

TCHAR *NewDefaultPrinterName3(TCHAR **name) {
    TCHAR buffer[MAXBUFFERSIZE];
    DWORD size = 0;
    *name = NULL;

    if (_(::GetProfileString(_T("windows"), _T("device"), _T(",,,"), buffer, MAXBUFFERSIZE) <= 0))
        return *name;

    _tcstok(buffer, _T(",")); // Printer name precedes first "," character.
    size_t strsize = _tcslen(buffer);
    (*name) = (TCHAR *)ckalloc((strsize + 1) * sizeof(TCHAR));
    _tcscpy(*name, buffer);
    return *name;
}

TCHAR *NewDefaultPrinterName(TCHAR **name) {
    OSVERSIONINFO osv = {0};
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osv);

    if (osv.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        return NewDefaultPrinterName2(name);
    } else if (osv.dwPlatformId == VER_PLATFORM_WIN32_NT && osv.dwMajorVersion >= 5)
        return NewDefaultPrinterName1(name);
    return NewDefaultPrinterName3(name);
}

int AppendSystemError (Tcl_Interp *interp, const char *prefix, DWORD error) {
    int length;
    char *msg;
    char id[TCL_INTEGER_SPACE], msgBuf[24 + TCL_INTEGER_SPACE];
    WCHAR *wMsgPtr, **wMsgPtrPtr = &wMsgPtr;
    Tcl_DString ds;

    if (!error)
        return TCL_ERROR;

    length = FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (WCHAR *) wMsgPtrPtr,
            0, NULL);
    if (length == 0) {
        char *msgPtr;
        length = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char *) &msgPtr, 0, NULL);
        if (length > 0) {
            wMsgPtr = (WCHAR *) LocalAlloc(LPTR, (length + 1) * sizeof(WCHAR));
            MultiByteToWideChar(CP_ACP, 0, msgPtr, length + 1, wMsgPtr, length + 1);
            LocalFree(msgPtr);
        }
    }
    if (length == 0) {
        if (error == ERROR_CALL_NOT_IMPLEMENTED) {
            msg = (char *)"function not supported under Win32s";
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

int TclPrinterCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static CONST char *commands[] = {
        "default",
        "names",
        "print",
        "write",
        "open",
        0L
    };
    enum commands {
        cmDefault,
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

    if (Tcl_GetIndexFromObj(tclInterp, objv[1], commands, "subcommand", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch ((enum commands)(index)) {

    case cmDefault:
        {
            TCHAR *name = NULL;
            if (NewDefaultPrinterName(&name)) {
                Tcl_SetObjResult(tclInterp, NewObjFromExternal(name));
                ckfree((char *)name);
                return TCL_OK;
            } else if (SetError(GetLastError()) == 0)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "can't query default: ", GetError());
        }

    case cmNames:
        if (Names(Tcl_GetObjResult(tclInterp)) == TCL_OK)
            return TCL_OK;
        else
            return AppendSystemError(tclInterp, "can't query names: ", GetError());

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
//                              Tcl_AppendResult(tclInterp, "unknown printer protocol", NULL);
//                              return TCL_ERROR;
                            }
                    }
                    break;
                case ifGdi:
                    if ((enum commands)index != cmPrint && (enum commands)index != cmOpen) {
                        Tcl_AppendResult(tclInterp, "invalid printer protocol for command", NULL);
                        return TCL_ERROR;
                    }
                    break;
                case ifRaw:
                    if ((enum commands)index != cmWrite && (enum commands)index != cmOpen) {
                        Tcl_AppendResult(tclInterp, "invalid printer protocol for command", NULL);
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
            else if (obji == objc-1 && strncmp(Tcl_GetString(objv[objc-1]), "-", 1) != 0)
                commandname = objv[objc-1];
            else {
#ifdef DIALOGS
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi|-raw? ?-default|-printdialog|-setupdialog|-name printername? ?command?");
#else
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi|-raw? ?-default|-name printername? ?command?");
#endif
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
                case seDefault: result = command->OpenDefault(); break;
                case seNamed: result = command->Open(printername); break;
#ifdef DIALOGS
                case sePrintdialog: result = command->Select(FALSE); break;
                case seSetupdialog: result = command->Select(TRUE); break;
#endif
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
                        LOGFONT font = {0};
                        RECT rect = {0, 0, _(::GetDeviceCaps(((TclPrintGdiCmd *)command)->GetDC(), HORZRES)), 
                                           _(::GetDeviceCaps(((TclPrintGdiCmd *)command)->GetDC(), VERTRES))};

                        if (result == TCL_OK)
                            result = ((TclPrintGdiCmd *)command)->ParsePrintParams(objc, objv, &obji, &rect, &font, &wrap);

                        if (result == TCL_OK) {
                            if (obji == objc-1) {
                                result = ((TclPrintGdiCmd *)command)->Configure(&font);
                                if (result == TCL_OK) {
                                    int printed = ((TclPrintGdiCmd *)command)->PrintDoc(document, output, &rect, wrap, objv[obji]);
                                    if (printed >= 0)
                                        Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(printed));
                                    else
                                        result = AppendSystemError(tclInterp, "error printing text: ", command->GetError());
                                } else 
                                    result = AppendSystemError(tclInterp, "error configuring device context: ", command->GetError());
                            } else {
#ifdef DIALOGS
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi? ?-default|-printdialog|-setupdialog|-name printername? ?-document documentname? ?-output filename? ?-margins rectspec? ?-font fontspec? ?-wordwrap? text");
#else
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi? ?-default|-name printername? ?-document documentname? ?-output filename? ?-margins rectspec? ?-font fontspec? ?-wordwrap? text");
#endif
                                result = TCL_ERROR;
                            }
                        }
                    }
                    break;
                case cmWrite:
                    {
                        if (result == TCL_OK) {
                            if (obji == objc-1) {
                                int written = ((TclPrintRawCmd *)command)->WriteDoc(document, output, objv[obji]);
                                if (written >= 0)
                                    Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(written));
                                else
                                    result =  AppendSystemError(tclInterp, "error writing text: ", command->GetError());
                            } else {
#ifdef DIALOGS
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-raw? ?-default|-printdialog|-setupdialog|-name printername? ?-document documentname? ?-output filename data");
#else
                                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-raw? ?-default|-name printername? ?-document documentname? ?-output filename data");
#endif
                                result = TCL_ERROR;
                            }
                        }
                    }
                    break;
                default:
                    result = TCL_ERROR;
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
        if (SetError(::GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
            SetError(0);
        else
            return TCL_ERROR;
    }
    if (buffersize > 0) {
        PRINTER_INFO_2 *pi = (PRINTER_INFO_2 *)ckalloc(buffersize);
        if (!_(::EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 2, (LPBYTE)pi, buffersize, &buffersize, &printerscount))) {
            SetError(::GetLastError());
            ckfree((char *)pi);
            return TCL_ERROR;
        } else {
            for (DWORD i = 0; i < printerscount; i++)
                Tcl_ListObjAppendElement(NULL, result, NewObjFromExternal(pi[i].pPrinterName));
            ckfree((char *)pi);
            return TCL_OK;
        }
    } else // anybody here ?
        return TCL_OK;
}

int TclPrinterCmd::ParseOpenParams(int objc, Tcl_Obj *CONST objv[], int *obji, 
        interfaces *printerinterface, selections *printerselection, Tcl_Obj **printername) {
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
#ifdef DIALOGS
        } else if (strcmp(Tcl_GetString(objv[*obji]), "-printdialog") == 0) {
            *printerselection = sePrintdialog;
            (*obji)++;
        } else if (strcmp(Tcl_GetString(objv[*obji]), "-setupdialog") == 0) {
            *printerselection = seSetupdialog;
            (*obji)++;
#endif
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
                         
