// $Id$

#include "tclprinter.hpp"
#include "tclprint.hpp"

#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE 3
#endif


BOOL UnicodeOS() {
    int platform = VER_PLATFORM_WIN32_NT;
//  int platform = TclWinGetPlatformId();

    return ((platform == VER_PLATFORM_WIN32_NT) || (platform == VER_PLATFORM_WIN32_CE));
}

Tcl_Obj *NewObjFromExternalW (TCHAR *external) {
   Tcl_Obj *obj;
   Tcl_DString ds;

   Tcl_DStringInit(&ds);
   Tcl_WinTCharToUtf(external, -1, &ds);
   obj = Tcl_NewStringObj(Tcl_DStringValue(&ds), -1);
   Tcl_DStringFree(&ds);

   return obj;
}

Tcl_Obj *NewObjFromExternalA (TCHAR *external) {
   Tcl_Obj *obj;
   Tcl_DString ds;

   Tcl_DStringInit(&ds);
   Tcl_ExternalToUtfDString(NULL, (CONST char *)external, -1, &ds);
   obj = Tcl_NewStringObj(Tcl_DStringValue(&ds), -1);
   Tcl_DStringFree(&ds);

   return obj;
}

TCHAR *NewExternalFromObjW (Tcl_Obj *obj) {
    Tcl_DString ds;
    TCHAR *external;
    unsigned length;

    Tcl_DStringInit(&ds);
    Tcl_WinUtfToTChar(Tcl_GetString(obj), -1, &ds);
    length = Tcl_DStringLength(&ds);
    external = (TCHAR *)ckalloc(length + sizeof(TCHAR));
    memset(external, 0, length + sizeof(TCHAR));
    memcpy(external, Tcl_DStringValue(&ds), length);
    Tcl_DStringFree(&ds);

    return external;
}

TCHAR *NewExternalFromObjA (Tcl_Obj *obj) {
    Tcl_DString ds;
    TCHAR *external;
    unsigned length;

    Tcl_DStringInit(&ds);
    Tcl_UtfToExternalDString(NULL, Tcl_GetString(obj), -1, &ds);
    length = Tcl_DStringLength(&ds);
    external = (TCHAR *)ckalloc(length + sizeof(TCHAR));
    memset(external, 0, length + sizeof(TCHAR));
    memcpy(external, Tcl_DStringValue(&ds), length);
    Tcl_DStringFree(&ds);

    return (TCHAR *)external;
}

TCHAR *NewStringFromExternal (void *external) {
    TCHAR *result;
    size_t bytelength;

    if (UnicodeOS())
        bytelength = wcslen((wchar_t *)external)*sizeof(wchar_t)+sizeof(wchar_t);
    else
        bytelength = strlen((char *)external)*sizeof(char)+sizeof(char);

    result = (TCHAR *)ckalloc((unsigned)bytelength);
    memset(result, 0, (unsigned)bytelength);
    memcpy(result, external, (unsigned)bytelength);

    return result;
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
printer print ? -gdi | -raw | -xps ?
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
        "open",
        0L
    };
    enum commands {
        cmNames,
        cmPrint,
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
        {
            enum interfaces {ifUnknown, ifGdi, ifRaw, ifXps}; 
            enum selections {seUnknown, seDefault, seNamed, seSelect};
            interfaces printerinterface = ifUnknown;
            selections printerselection = seUnknown;
            TclPrintCmd *command;
            Tcl_Obj *commandname;
            TCHAR *document = NULL;
            TCHAR *output = NULL;
            int result = TCL_OK;
            int printerindex;
            int obji = 2;

            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-gdi") == 0) {
                    printerinterface = ifGdi;
                    obji++;
                } else // default
                    printerinterface = ifGdi;
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-default") == 0) {
                    printerselection = seDefault;
                    obji++;
                } else if (strcmp(Tcl_GetString(objv[obji]), "-select") == 0) {
                    printerselection = seSelect;
                    obji++;
                } else if (strcmp(Tcl_GetString(objv[obji]), "-name") == 0) {
                    printerselection = seNamed;
                    obji++;
                    if (obji < objc) {
                        printerindex = obji;
                        obji++;
                    } else {
                        Tcl_AppendResult(tclInterp, "printer name required for -name", NULL);
                        return TCL_ERROR;
                    }
                }
            }

            if ((enum commands)(index) == cmOpen) {
                if (obji == objc)
                    commandname = NewPrintNameObj();
                else if (obji == objc-1)
                    commandname = objv[objc-1];
                else {
                    Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi? ?-default|-select|-name printername? ?command?");
                    return TCL_ERROR;
                }
                command = new TclPrintCmd(tclInterp, Tcl_GetString(commandname));
            } else { // cmPrint
	            if (result == TCL_OK && obji < objc && strcmp(Tcl_GetString(objv[obji]), "-document") == 0) {
                    obji++;
                    if (obji < objc) {
                        document = NewExternalFromObj(objv[obji]);
                        obji++;
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -document", NULL);
                        result = TCL_ERROR;
                    }
                }
                if (result == TCL_OK && obji < objc && strcmp(Tcl_GetString(objv[obji]), "-output") == 0) {
                    obji++;
                    if (obji < objc) {
                        output = NewExternalFromObj(objv[obji]);
                        obji++;
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -output", NULL);
                        result = TCL_ERROR;
                    }
                }
                if (result == TCL_ERROR) {
                    if (document) ckfree((char *)document);
                    if (output) ckfree((char *)output);
                    return TCL_ERROR;
                }
                command = new TclPrintCmd(tclInterp, Tcl_GetString(NewPrintNameObj()));
            }
 
            switch (printerselection) {
            case seUnknown:
            case seDefault:
                result = command->Select(TRUE);
                break;
            case seNamed:
                result = command->Open(NewExternalFromObj(objv[printerindex]));
                break;
            case seSelect:
                result = command->Select(FALSE);
                break;
            }
            if (result == TCL_ERROR) {
                if (command->GetError() == 0) {
                   Tcl_AppendResult(tclInterp, "cancelled", NULL);
                } else {
                   AppendSystemError(tclInterp, "can't open printer: ", command->GetError());
                }
                delete command;
                if (document) ckfree((char *)document);
                if (output) ckfree((char *)output);
                return TCL_ERROR;
            }
            if ((enum commands)(index) == cmOpen) {
                Tcl_SetObjResult(tclInterp, commandname);
                return TCL_OK;
            }
            // cmPrint
            LOGFONT font = {};
            RECT rect = {0, 0, _(::GetDeviceCaps(command->GetDC(), HORZRES)), _(::GetDeviceCaps(command->GetDC(), VERTRES))};
            int wrap = FALSE;

            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-margins") == 0) {
                    obji++;
                    if (obji < objc) {
                        RECT margins = {0, 0, 0, 0};
                        if (command->ParseRectArg(objv[obji], &margins) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -margins", NULL);
                            return TCL_ERROR;
                        }
                        rect.left += margins.left;
                        rect.top += margins.top;
                        rect.right -= margins.right;
                        rect.bottom -= margins.bottom;
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -margins", NULL);
                        result = TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-font") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (command->ParseFontArg(objv[obji], &font) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -font", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -font", NULL);
                        result = TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-wordwrap") == 0) {
                    wrap = TRUE;
                    obji++;
                }
            }
            if (result == TCL_OK)
                if (obji == objc-1) {
                    if (command->Configure(&font) == TCL_OK)
                        if (command->PrintDocObj(document, output, &rect, wrap, objv[obji]) == TCL_OK)
                            Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(result));
                        else
                            result = AppendSystemError(tclInterp, "error printing text: ", GetError());
                    else
                        result = AppendSystemError(tclInterp, "error configuring device context: ", GetError());
                } else {
                    Tcl_WrongNumArgs(tclInterp, 2, objv, "?-gdi? ?-default|-select|-name printername? ?-document documentname? ?-output filename? ?-margins rectspec? ?-font fontspec? ?-wordwrap? text");
                    result = TCL_ERROR;
                }
            delete command;
            if (document) ckfree((char *)document);
            if (output) ckfree((char *)output);
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
            Tcl_ListObjAppendElement(tclInterp, result, NewObjFromExternal(pi[i].pPrinterName));
        }
        ckfree((char *)pi);
        return TCL_OK;
    } else
        return TCL_ERROR;
}

Tcl_Obj *TclPrinterCmd::NewPrintNameObj() {
    static int suffix = 0;
    char buffer[8 + TCL_INTEGER_SPACE];
    Tcl_CmdInfo info;

    do {
        sprintf(buffer, "printer%d", suffix);
        suffix++;
    } while (Tcl_GetCommandInfo(tclInterp, buffer, &info) != NULL);

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
                         
