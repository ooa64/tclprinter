// $Id$

#include "tclprinter.hpp"
#include "tclprint.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif

/*
command start ?-document documentname? ?-output filename?
command abort
command end
command close

command startpage - must be called after start
command endpage

command info protocol | printer | document | page | status | width | height

command place 
              ? -rect {?left? ?top? ?right? ?bottom?} ?
              ? -align {flag1 flag2 ...} ?
              ? -font {name ?height? ?width? ?weight? ?style? ?charset? ?quality? ?pitch? ?family?} ?
              ? -calcrect ?
              data

command print 
              ? -margins {?left? ?top? ?right? ?bottom?} ?
              ? -font {name ?height? ?width? ?weight? ?style? ?charset? ?quality? ?pitch? ?family?} ?
              ? -wordwrap ?
              data

*/

int TclPrintCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static char *commands[] = {
        "info",
        "start",
        "abort",
        "end",
        "startpage",
        "endpage",
        "place",
        "print",
        "frame",
        "fill",
        "rectangle",
        "round",
        "ellipse",
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
        cmPlace,
        cmPrint,
        cmFrame,
        cmFill,
        cmRectangle,
        cmRoundrect,
        cmEllipse,
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
            if (strcmp(Tcl_GetString(objv[2]), "name") == 0)
                Tcl_SetObjResult(tclInterp, NewObjFromExternal(printername));
            else if (strcmp(Tcl_GetString(objv[2]), "height") == 0) 
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, HORZRES))));
            else if (strcmp(Tcl_GetString(objv[2]), "width") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, VERTRES))));
            else if (strcmp(Tcl_GetString(objv[2]), "pheight") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, HORZSIZE))));
            else if (strcmp(Tcl_GetString(objv[2]), "pwidth") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, VERTSIZE))));
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
            TCHAR *document = NULL;
            TCHAR *output = NULL;
            int result = TCL_OK;
            int obji = 2;

            if (!Opened()) {
                Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
                return TCL_ERROR;
            }
            if (obji < objc && strcmp(Tcl_GetString(objv[obji]), "-document") == 0) {
                obji++;
                if (obji < objc) {
                    document = NewExternalFromObj(objv[obji]);
                    obji++;
                } else {
                    Tcl_AppendResult(tclInterp, "missing option value for -document", NULL);
                    return TCL_ERROR;
                }
            }
            if (obji < objc && strcmp(Tcl_GetString(objv[obji]), "-output") == 0) {
                obji++;
                if (obji < objc) {
                    output = NewExternalFromObj(objv[obji]);
                    obji++;
                } else {
                    Tcl_AppendResult(tclInterp, "missing option value for -output", NULL);
                    return TCL_ERROR;
                }
            }
            if (obji < objc) {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-document name? ?-output name?");
                return TCL_ERROR;
            }
            result = StartDoc(document, output);
            if (document) 
                ckfree((char *)document);
            if (output) 
                ckfree((char *)output);
            if (result == TCL_ERROR) {
                Tcl_SetResult(tclInterp, "error starting document", NULL);
//              StartDoc hasn't set last error 
//              return AppendSystemError(tclInterp, "error starting document: ", GetError());
            }
            return result;
        }

    case cmAbort:
        if (!Started()) {
            Tcl_AppendResult(tclInterp, "document is not started", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            if (AbortDoc() == TCL_OK)
                return TCL_OK;
            else
                return AppendSystemError(tclInterp, "error aborting document: ", GetError());
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

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

    case cmPlace:
        {
            LOGFONT font = {};
            RECT rect = {0, 0, _(::GetDeviceCaps(printerdc, HORZRES)), _(::GetDeviceCaps(printerdc, VERTRES))};
            UINT align = 0;
            int calc = FALSE;
            int obji = 2;

            if (!PageStarted()) {
                Tcl_AppendResult(tclInterp, "page is not started", NULL);
                return TCL_ERROR;
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-rect") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseRectArg(objv[obji], &rect) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -rect", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -rect", NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-align") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseAlignArg(objv[obji], &align) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -align", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -align", NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-font") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseFontArg(objv[obji], &font) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -font", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -font", NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-calcrect") == 0) {
                    calc = TRUE;
                    obji++;
                }
            }
            if (obji == objc-1) {
                if (Configure(&font) == TCL_OK) {
                    TCHAR *text = NewExternalFromObj(objv[obji]);
                    int result = Place(&rect, calc, align, text);
                    ckfree((char *)text);
                    if (result > 0) {
                        if (calc) {
                           Tcl_ListObjAppendElement(tclInterp, Tcl_GetObjResult(tclInterp), Tcl_NewIntObj(rect.left));
                           Tcl_ListObjAppendElement(tclInterp, Tcl_GetObjResult(tclInterp), Tcl_NewIntObj(rect.top));
                           Tcl_ListObjAppendElement(tclInterp, Tcl_GetObjResult(tclInterp), Tcl_NewIntObj(rect.right));
                           Tcl_ListObjAppendElement(tclInterp, Tcl_GetObjResult(tclInterp), Tcl_NewIntObj(rect.bottom));
                        } else
                           Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(result));
                        return TCL_OK;
                    } else
                        return AppendSystemError(tclInterp, "error placing text: ", GetError());
                } else 
                    return AppendSystemError(tclInterp, "error configuring device context: ", GetError());
            } else {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-rect rectspec? ? ?-align flags? ?-font fontspec? ?-calcrect? text");
                return TCL_ERROR;
            }
        }
        break;

    case cmPrint:
        {
            LOGFONT font = {};
            RECT rect = {0, 0, _(::GetDeviceCaps(printerdc, HORZRES)), _(::GetDeviceCaps(printerdc, VERTRES))};
            int wrap = FALSE;
            int obji = 2;

            if (!PageStarted()) {
                Tcl_AppendResult(tclInterp, "page is not started", NULL);
                return TCL_ERROR;
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-margins") == 0) {
                    obji++;
                    if (obji < objc) {
                        RECT margins = {0, 0, 0, 0};
                        if (ParseRectArg(objv[obji], &margins) == TCL_ERROR) {
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
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-font") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseFontArg(objv[obji], &font) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -font", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -font", NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-wordwrap") == 0) {
                    wrap = TRUE;
                    obji++;
                }
            }
            if (obji == objc-1) {
                if (Configure(&font) == TCL_OK) {
                    TCHAR *text = NewExternalFromObj(objv[obji]);
                    int printed = Print(&rect, wrap, text);
                    ckfree((char *)text);
                    if (printed >= 0) {
                        Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(printed));
                        return TCL_OK;
                    } else
                        return AppendSystemError(tclInterp, "error printing text: ", GetError());
                } else 
                    return AppendSystemError(tclInterp, "error configuring device context: ", GetError());
            } else {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-margins rectspec? ?-font fontspec? ?-wordwrap? text");
                return TCL_ERROR;
            }
        }

    case cmFrame:
    case cmFill:
    case cmRectangle:
    case cmRoundrect:
    case cmEllipse:
        {
            RECT rect = {0, 0, _(::GetDeviceCaps(printerdc, HORZRES)), _(::GetDeviceCaps(printerdc, VERTRES))};
            int obji = 2;
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-rect") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseRectArg(objv[obji], &rect) == TCL_ERROR) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for -rect", NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for -rect", NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji == objc) {
                if (SelectObject(printerdc, GetStockObject(WHITE_BRUSH)) != NULL &&
                        SelectObject(printerdc, GetStockObject(BLACK_PEN)) != NULL) {
                    int result = 0;
                    switch ((enum commands)(index)) {
                    case cmFrame:
                        result = FrameRect(printerdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
                        break;
                    case cmFill:
                        result = FillRect(printerdc, &rect, (HBRUSH)GetStockObject(GRAY_BRUSH));
                        break;
                    case cmRectangle:
                        result = Rectangle(printerdc, rect.left, rect.top, rect.right, rect.bottom);
                        break;
                    case cmRoundrect:
                        result = RoundRect(printerdc, rect.left, rect.top, rect.right, rect.bottom, 20, 20);
                        break;
                    case cmEllipse:
                        result = Ellipse(printerdc, rect.left, rect.top, rect.right, rect.bottom);
                        break;
                    }
                    if (result) 
                        return TCL_OK;
                    else
                        return AppendSystemError(tclInterp, "drawing error: ", GetError());
                } else
                    return AppendSystemError(tclInterp, "drawing error: ", GetError());
            } else {
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-rect place?");
                return TCL_ERROR;
            }
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

void TclPrintCmd::Close() {
    if (PageStarted()) 
        EndPage();
    if (Started()) 
        EndDoc();
    if (Opened())
        _(::DeleteDC(printerdc));
    if (printerfont) 
        _(::DeleteObject(printerfont));
    if (printername) 
        ckfree((char *)printername);

    lasterrorcode = 0;
    printerdc = NULL;
    printerfont = NULL;
    printername = NULL;

    ZeroMemory(&logfont, sizeof(logfont));
}

int TclPrintCmd::Select (BOOL usedefault) {
    PRINTDLG printdlg = {0};
    ZeroMemory(&printdlg, sizeof(printdlg));
    printdlg.lStructSize = sizeof(printdlg);

    Close();

    printdlg.Flags = (usedefault)?
        (PD_USEDEVMODECOPIESANDCOLLATE | PD_RETURNDEFAULT | PD_RETURNDC):
        (PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION | PD_RETURNDC );

    if (_(::PrintDlg(&printdlg))) {
        printerdc = printdlg.hDC;
        if (printdlg.hDevMode != NULL && printdlg.hDevNames != NULL) {
            LPDEVNAMES devnames = (LPDEVNAMES)GlobalLock(printdlg.hDevNames) ;
            printername = NewStringFromExternal((void *)((LPCTSTR)devnames+devnames->wDeviceOffset));
            GlobalUnlock(printdlg.hDevNames);
         }
        return TCL_OK;
    } else {
        SetError(::GetLastError());
        return TCL_ERROR;
    }
}

int TclPrintCmd::Open(TCHAR *name) {
    if (printerdc) 
        _(::DeleteDC(printerdc));
    if (printername) 
        ckfree((char *)printername);
    printerdc = _(::CreateDC(_T("WINSPOOL"), name, NULL, NULL));
    printername = NULL;
    if (printerdc) {
        size_t namelength = name ? _tcsclen(name) * sizeof(TCHAR) : 0;
        if (namelength) {
            printername = (TCHAR *)ckalloc((unsigned)namelength + sizeof(TCHAR));
            memset((void *)printername, 0, (unsigned)namelength + sizeof(TCHAR));
            memcpy((void *)printername, name, (unsigned)namelength);
        }
        return TCL_OK;
    } else {
        SetError(::GetLastError());
        return TCL_ERROR;
    }
}

int TclPrintCmd::Configure(LOGFONT *lf) {
    int result = TCL_OK;
    SetError(0);

    if (memcmp(&logfont, lf, sizeof(logfont)) != 0) {
        LONG userheight = lf->lfHeight;
        LONG userwidth = lf->lfWidth;
        lf->lfHeight = MulDiv(userheight, _(GetDeviceCaps(printerdc, LOGPIXELSY)), 72);
        lf->lfWidth = MulDiv(userwidth, _(::GetDeviceCaps(printerdc, LOGPIXELSX)), 72);

        printerfont = _(::CreateFontIndirect(lf));

        lf->lfHeight = userheight;
        lf->lfWidth = userwidth;

        if (printerfont && _(::SelectObject(printerdc, printerfont)) != NULL) {
            logfont = *lf;
        } else {
            SetError(::GetLastError());
            result = TCL_ERROR;
        }
    }
    return result;
}

int TclPrintCmd::StartDoc(TCHAR *docname, TCHAR *output) {
     docinfo.cbSize = sizeof(DOCINFO);
     if (docname) {
        size_t bytelength = _tcsclen(docname) * sizeof(TCHAR);
        docinfo.lpszDocName = (TCHAR *)ckalloc((unsigned)bytelength + sizeof(TCHAR));
        memset((void *)docinfo.lpszDocName, 0, (unsigned)bytelength + sizeof(TCHAR));
        memcpy((void *)docinfo.lpszDocName, docname, (unsigned)bytelength);
    }
     if (output) {
        size_t bytelength = _tcsclen(output) * sizeof(TCHAR);
        docinfo.lpszOutput = (TCHAR *)ckalloc((unsigned)bytelength + sizeof(TCHAR));
        memset((void *)docinfo.lpszOutput, 0, (unsigned)bytelength + sizeof(TCHAR));
        memcpy((void *)docinfo.lpszOutput, output, (unsigned)bytelength);
    }

    if (_(::StartDoc(printerdc, &docinfo) <= 0)) {
//      StartDoc hasn't set last error 
//      SetError(::GetLastError());
        _(::AbortDoc(printerdc));
        pageno = 0;
        pageposy = 0;
        if (docinfo.lpszDocName) 
            ckfree((char *)docinfo.lpszDocName);
        if (docinfo.lpszOutput) 
            ckfree((char *)docinfo.lpszOutput);
        ZeroMemory(&docinfo, sizeof(docinfo));
        return TCL_ERROR;
    } else {
        SetError(0);
        return TCL_OK;
    }
}

int TclPrintCmd::AbortDoc() {
    if (_(::AbortDoc(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageno = 0;
        pageposy = 0;
        pageactive = FALSE;
        if (docinfo.lpszDocName) 
            ckfree((char *)docinfo.lpszDocName);
        if (docinfo.lpszOutput) 
            ckfree((char *)docinfo.lpszOutput);
        ZeroMemory(&docinfo, sizeof(docinfo));
        SetError(0);
        return TCL_OK;
    }
}

int TclPrintCmd::EndDoc() {
    if (pageactive) 
        EndPage(); // ignore errors
    if (_(::EndDoc(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageno = 0;
        pageposy = 0;
        if (docinfo.lpszDocName) 
            ckfree((char *)docinfo.lpszDocName);
        if (docinfo.lpszOutput) 
            ckfree((char *)docinfo.lpszOutput);
        ZeroMemory(&docinfo, sizeof(docinfo));
        SetError(0);
        return TCL_OK;
    }
}

int TclPrintCmd::StartPage() {
    if (_(::StartPage(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageno++;
        pageposy = 0;
        pageactive = TRUE;
        _(::MoveToEx(printerdc, 0, 0, NULL));
        SetError(0);
        return TCL_OK;
    }
}

int TclPrintCmd::EndPage() {
    if (_(::EndPage(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageactive = FALSE;
        SetError(0);
        return TCL_OK;
    }
}

int TclPrintCmd::Place(RECT *rect, BOOL calc, UINT align, TCHAR *text) {
    UINT format = align | DT_NOPREFIX;

    _(::SetTextAlign(printerdc, TA_TOP | TA_LEFT | TA_NOUPDATECP));

    int result = _(::DrawText(printerdc, text, -1, rect, (calc ? (format | DT_CALCRECT) : format)));
    if (result == 0)
        SetError(::GetLastError());
    else 
        SetError(0);
    return result;
}

int TclPrintCmd::Print(RECT *rect, BOOL wrap, TCHAR *text) {
    UINT format = DT_NOPREFIX | DT_EXPANDTABS | DT_SINGLELINE;
    TCHAR *start = text;
    TCHAR *stop = text;
    SIZE outsize;
    int drawlength;
    int maxheight = rect->bottom;

    if (pageposy < rect->top) {
        pageposy = rect->top;
       _(::MoveToEx(printerdc, rect->left, pageposy, NULL));
    }

    _(::SetTextAlign(printerdc, TA_UPDATECP));

    SetError(0);
    if (wrap) {
        TCHAR *nextstop;
        POINT p;
        _(::MoveToEx(printerdc, 0, 0, &p));
        _(::MoveToEx(printerdc, p.x, p.y, NULL));
        int pageposy = p.y;
        int pageposx = p.x;
        int linewidth = 0;
        while (*start) {
#ifdef _DEBUG
            MoveToEx(printerdc, 0, 0, &p);
            MoveToEx(printerdc, p.x, p.y, NULL);
            assert(pageposx == p.x && pageposy == p.y);
#endif
            outsize.cx = 0;
            outsize.cy = 0;
            stop = start;
            while (*stop != '\0' && *stop != '\n') {
                nextstop = _tcsspnp(stop, _T(" \t"));
                if (nextstop == NULL)
                    nextstop = _tcsrchr(stop, '\0');

                stop = nextstop;

                nextstop = _tcspbrk(stop, _T(" \t\n"));
                if (nextstop == NULL)
                    nextstop = _tcsrchr(stop, 0);

                _(::GetTextExtentPoint32(printerdc, start, nextstop - start, &outsize));

                if (pageposx + outsize.cx < rect->right) {
                    stop = nextstop;
                } else {
                    if (start == stop) // very long word
                        stop = nextstop;
                    break;
                }
            }
            drawlength = stop - start;
            if (drawlength && *stop == '\n' && *(stop-1) == '\r')
                drawlength--;

            if (drawlength) {
                if (pageposy + outsize.cy < maxheight) {
                    _(::GetTextExtentPoint32(printerdc, start, drawlength, &outsize));
                    _(::TextOut(printerdc, 0, pageposy, start, drawlength));
                    pageposx += outsize.cx;
                } else {
                    stop = start;
                    break;
                }
            }
            if (*stop != '\0') {
                if (outsize.cy == 0)
                    _(::GetTextExtentPoint32(printerdc, _T("A"), 1, &outsize));
                if (pageposy + outsize.cy < maxheight) {
                    pageposy += outsize.cy;
                    pageposx = rect->left;
                    _(::MoveToEx(printerdc, pageposx, pageposy, NULL));
                    if (*stop == '\n')
                        stop++;
                } else
                    break;
            }
            start = stop;
        }
    } else {
        while (*start) {
            stop = _tcschr(start,'\n');
            if (!stop) {
                stop = start + _tcsclen(start);
            }
            drawlength = (int)(stop-start);
            if (drawlength && *(stop-1) == '\r')
                drawlength --;
            if (drawlength) {
                _(::GetTextExtentPoint32(printerdc, start, drawlength, &outsize));
                if (pageposy + outsize.cy < maxheight) {
                    _(::TextOut(printerdc, 0, pageposy, start, drawlength));
                } else {
                    stop = start;
                    break;
                }
            } else if (*stop == '\n') {
                _(::GetTextExtentPoint32(printerdc, _T("A"), 1, &outsize));
            } 
            if (*stop == '\n') {
                if (pageposy + outsize.cy < maxheight) {
                    _(::MoveToEx(printerdc, rect->left, pageposy + outsize.cy, NULL));
                    pageposy += outsize.cy;
                    stop++;
                } else
                    break;
            } 
            start = stop;
        }
    }

    return (int)(stop - text);
}

int TclPrintCmd::PrintDoc(TCHAR *document, TCHAR *output, RECT *rect, BOOL wrap, TCHAR *text) {
    int result = TCL_OK;

    result = StartDoc(document, output);
    if (result == TCL_OK) {
        result = StartPage();
        if (result == TCL_OK) {
            TCHAR *start = text;
            int restlength = _tcsclen(text);
            int printed;
            while (restlength > 0) {
                printed = Print(rect, wrap, start);
                if (printed <= 0) {
                    SetError(::GetLastError());
                    result = TCL_ERROR;
                    break;
                } else if (printed < restlength) {
                    result = EndPage();
                    if (result == TCL_OK) {
                        result = StartPage();
                    }
                    if (result == TCL_ERROR)
                        break;
                    restlength -= printed;
                    start += printed;
                } else {
#ifdef _DEBUG
                    assert(printed == restlength);
#endif
                    break;
                }
            }
            result = EndPage();
        }
        result = EndDoc();
    }
    return result;
}

int TclPrintCmd::PrintDocObj(TCHAR *document, TCHAR *output, RECT *rect, BOOL wrap, Tcl_Obj *textobj) {
    int result;
#ifdef UNICODE
    if (UnicodeOS())
        result = PrintDoc(document, output, rect, wrap, (TCHAR *)Tcl_GetUnicode(textobj));
    else {
        Tcl_DString ds;
        Tcl_UtfToExternalDString(NULL, Tcl_GetString(textobj), -1, &ds);
        result = PrintDoc(document, output, rect, wrap, (TCHAR *)Tcl_DStringValue(&ds));
        Tcl_DStringFree(&ds);
    }
#else
    Tcl_DString ds;
    Tcl_UtfToExternalDString(NULL, Tcl_GetString(textobj), -1, &ds);
    result = PrintDoc(document, output, rect, wrap, (TCHAR *)Tcl_DStringValue(&ds));
    Tcl_DStringFree(&ds);
#endif
    return result;
}

BOOL TclPrintCmd::Opened() {
    return printerdc != NULL;
}

BOOL TclPrintCmd::Started() {
    return docinfo.cbSize > 0;
}

BOOL TclPrintCmd::PageStarted() {
    return pageactive;
}

// ?-font {name ?height? ?width? ?weight? ?italic? ?underline? ?strikeout? ?charset? ?quality? ?pitch? ?family?}?

int TclPrintCmd::ParseFontArg(struct Tcl_Obj *obj, LOGFONT *lf) {
    static char *weightsMap[] = {
        "thin",
        "extralight",
        "ultralight",
        "light",
        "normal",
        "regular",
        "medium",
        "semibold",
        "demibold",
        "bold",
        "extrabold",
        "ultrabold",
        "heavy",
        "black",
        0L
    };

    static LONG weights[] = {
        FW_THIN,
        FW_EXTRALIGHT,
        FW_ULTRALIGHT,
        FW_LIGHT,
        FW_NORMAL,
        FW_REGULAR,
        FW_MEDIUM,
        FW_SEMIBOLD,
        FW_DEMIBOLD,
        FW_BOLD,
        FW_EXTRABOLD,
        FW_ULTRABOLD,
        FW_HEAVY,
        FW_BLACK
    };

    static char *stylesMap[] = {
        "italic",
        "underline",
        "strikeout",
        0L
    };

    enum styles {
        mapItalic,
        mapUnderline,
        mapStrikeOut
    };

    static char *charsetsMap[] = {
        "ansi",
        "baltic",
        "chinesebig5",
        "default",
        "easteurope",
        "gb2312",
        "greek",
        "hangul",
        "mac",
        "oem",
        "russian",
        "shiftjis",
        "symbol",
        "turkish",
        "vietnamese",
        "johab",
        "arabic",
        "hebrew",
        "thai",
        "oem",
        "default",
        0L
    };

    static BYTE charsets[] = {
        ANSI_CHARSET,
        BALTIC_CHARSET,
        CHINESEBIG5_CHARSET,
        DEFAULT_CHARSET,
        EASTEUROPE_CHARSET,
        GB2312_CHARSET,
        GREEK_CHARSET,
        HANGUL_CHARSET,
        MAC_CHARSET,
        OEM_CHARSET,
        RUSSIAN_CHARSET,
        SHIFTJIS_CHARSET,
        SYMBOL_CHARSET,
        TURKISH_CHARSET,
        VIETNAMESE_CHARSET,
        JOHAB_CHARSET,
        ARABIC_CHARSET,
        HEBREW_CHARSET,
        THAI_CHARSET,
        OEM_CHARSET,
        DEFAULT_CHARSET
    };

    static char *qualitiesMap[] = {
        "antialiased",
//      "cleartype",
        "default",
        "draft",
        "nonantialiased",
        "proof", 
        0L
    };

    static BYTE qualities[] = {
        ANTIALIASED_QUALITY,
//      CLEARTYPE_QUALITY,
        DEFAULT_QUALITY,
        DRAFT_QUALITY,
        NONANTIALIASED_QUALITY,
        PROOF_QUALITY
    };

    static char *pitchesMap[] = {
        "default",
        "fixed",
        "variable",
        0L
    };

    static BYTE pitches[] = {
        DEFAULT_PITCH,
        FIXED_PITCH,
        VARIABLE_PITCH
    };

    static char *familiesMap[] = {
        "decorative",
        "dontcare",
        "modern",
        "roman",
        "script",
        "swiss",
        0L
    };

    static BYTE families[] = {
        FF_DECORATIVE,
        FF_DONTCARE,
        FF_MODERN,
        FF_ROMAN,
        FF_SCRIPT,
        FF_SWISS
    };

//efine FF_MASK (7<<4)
#define FF_MASK (FF_DONTCARE|FF_ROMAN|FF_SWISS|FF_MODERN|FF_SCRIPT|FF_DECORATIVE)
//efine PITCH_MASK 3
#define PITCH_MASK (DEFAULT_PITCH | FIXED_PITCH | VARIABLE_PITCH)
#define SIZEOFARRAY(a) (sizeof(a)/sizeof(a[0]))

    int objc;
    Tcl_Obj **objv;

    if ((Tcl_ListObjGetElements(tclInterp, obj, &objc, &objv) == TCL_OK) || (objc > 0)) {
        int obji = 0;
        // font face: always at 1st position
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                TCHAR *newface = NewExternalFromObj(objv[obji]);
                _tcsncpy(lf->lfFaceName, newface, SIZEOFARRAY(lf->lfFaceName));
                lf->lfFaceName[SIZEOFARRAY(lf->lfFaceName)-1] = 0;
                ckfree((char *)newface);
            } 
            obji++;
        }
        // font height: integer can be on 2nd position, skip if empty
        if (obji < objc) {
            LONG newheight = 0;
            if (Tcl_GetCharLength(objv[obji]) == 0 || Tcl_GetLongFromObj(NULL, objv[obji], &newheight) == TCL_OK) {
                if (Tcl_GetCharLength(objv[obji]) > 0)
                    lf->lfHeight = newheight;
                obji++;
                // font width: integer can be after height
                if (obji < objc) {
                    LONG newwidth = 0;
                    if (Tcl_GetCharLength(objv[obji]) == 0 || Tcl_GetLongFromObj(NULL, objv[obji], &newwidth) == TCL_OK) {
                        if (Tcl_GetCharLength(objv[obji]) > 0)
                            lf->lfWidth = newwidth;
                        obji++;
                    }
                }
            }
        }
        // font weight: integer or dictionary word
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                int index = -1; 
                LONG newweight = 0;
                if (Tcl_GetLongFromObj(NULL, objv[obji], &newweight) == TCL_OK ||
                        Tcl_GetIndexFromObj(NULL, objv[obji], (CONST char **)weightsMap, NULL, 0, &index) == TCL_OK) {
                    if (index >= 0)
                        lf->lfWeight = weights[index];
                    else 
                        lf->lfWeight = newweight;
                    obji++;
                }
            } else
                obji++;
        }
        // style 
        if (obji < objc)
            if (strcmp(Tcl_GetString(objv[obji]), "italic") == 0) {
                lf->lfItalic = TRUE;
                obji++;
            } else if (Tcl_GetCharLength(objv[obji]) == 0)
                obji++;
        if (obji < objc) {
            if (strcmp(Tcl_GetString(objv[obji]), "underline") == 0) {
                lf->lfUnderline = TRUE;
                obji++;
            } else if (Tcl_GetCharLength(objv[obji]) == 0)
                obji++;
        }
        if (obji < objc) {
            if (strcmp(Tcl_GetString(objv[obji]), "strikeout") == 0) {
                lf->lfStrikeOut = TRUE;
                obji++;
            } else if (Tcl_GetCharLength(objv[obji]) == 0)
                obji++;
        }
        // charset
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                int index;
                if (Tcl_GetIndexFromObj(NULL, objv[obji], (CONST char **)charsetsMap, NULL, 0, &index) == TCL_OK) {
                    lf->lfCharSet = charsets[index];
                    obji++;
                }
            } else
                obji++;
        }
        // quality
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                int index;
                if (Tcl_GetIndexFromObj(NULL, objv[obji], (CONST char **)qualitiesMap, NULL, 0, &index) == TCL_OK) {
                    lf->lfQuality = qualities[index];
                    obji++;
                }
            } else
                obji++;
        }
        // pitch
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                int index;
                if (Tcl_GetIndexFromObj(NULL, objv[obji], (CONST char **)pitchesMap, NULL, 0, &index) == TCL_OK) {
                    lf->lfPitchAndFamily = (lf->lfPitchAndFamily & ~PITCH_MASK) | pitches[index];
                    obji++;
                }
            } else
                obji++;
        }
        // family
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                int index;
                if (Tcl_GetIndexFromObj(NULL, objv[obji], (CONST char **)familiesMap, NULL, 0, &index) == TCL_OK) {
                    if ((lf->lfPitchAndFamily & FF_MASK) != families[index])
                        lf->lfPitchAndFamily = (lf->lfPitchAndFamily & ~FF_MASK) | families[index];
                } else {
                    Tcl_AppendResult(tclInterp, "unknown property \"", Tcl_GetStringFromObj(objv[obji], NULL), "\"", NULL);
                    return TCL_ERROR;
                }
            } else
                lf->lfPitchAndFamily = (lf->lfPitchAndFamily & ~FF_MASK);
            obji++;
        }
        if (obji < objc) {
            Tcl_AppendResult(tclInterp, "extra property \"", Tcl_GetStringFromObj(objv[obji], NULL), "\"", NULL);
            return TCL_ERROR;
        }
        return TCL_OK;
    } else {
        return TCL_ERROR;
    }
}

int TclPrintCmd::ParseAlignArg(struct Tcl_Obj *obj, UINT *format) {
    static char *flagsMap[] = {
        "top",
        "left",
        "center",
        "right",
        "vcenter",
        "bottom",
        "wordbreak",
        "singleline",
        "expandtabs",
        "noclip",
        "externalleading",
        "pathellipsis",
        "wordellipsis",
        "endellipsis",
//      "rtlreading",
        0L
    };
    static UINT flags[] = {
        DT_TOP,
        DT_LEFT,
        DT_CENTER,
        DT_RIGHT,
        DT_VCENTER,
        DT_BOTTOM,
        DT_WORDBREAK,
        DT_SINGLELINE,
        DT_EXPANDTABS,
        DT_NOCLIP,
        DT_EXTERNALLEADING,
        DT_PATH_ELLIPSIS,
        DT_WORD_ELLIPSIS,
        DT_END_ELLIPSIS,
//      DT_RTLREADING
    };

    int objc;
    Tcl_Obj **objv;

    if ((Tcl_ListObjGetElements(tclInterp, obj, &objc, &objv) == TCL_OK)) {
        int index;
        for (int i = 0; i < objc; i++) {
            if (Tcl_GetIndexFromObj(tclInterp, objv[i], (CONST char **)flagsMap, "flag", 0, &index) == TCL_OK)
                *format |= flags[index];
            else
                return TCL_ERROR;
        }
    } else
        return TCL_ERROR;

    return TCL_OK;
}

// -rect {?x0? ?y0? ?x1? ?y1?} 

int TclPrintCmd::ParseRectArg(struct Tcl_Obj *obj, RECT *rect) {
    int objc;
    Tcl_Obj **objv;
    if ((Tcl_ListObjGetElements(tclInterp, obj, &objc, &objv) == TCL_OK))
        if (objc > 4)
            Tcl_AppendResult(tclInterp, "too many elements", NULL);
        else if ((objc <= 0 || Tcl_GetLongFromObj(tclInterp, objv[0], &rect->left) == TCL_OK) &&
                 (objc <= 1 || Tcl_GetLongFromObj(tclInterp, objv[1], &rect->top) == TCL_OK) &&
                 (objc <= 2 || Tcl_GetLongFromObj(tclInterp, objv[2], &rect->right) == TCL_OK) &&
                 (objc <= 3 || Tcl_GetLongFromObj(tclInterp, objv[3], &rect->bottom) == TCL_OK))
            return TCL_OK;
    return TCL_ERROR;
}
