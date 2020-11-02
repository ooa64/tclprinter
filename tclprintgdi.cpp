// $Id$

#include "tclprinter.hpp"
#include "tclprintgdi.hpp"

#ifdef _DEBUG
#include <assert.h>
#endif

int TclPrintGdiCmd::Command (int objc, struct Tcl_Obj *CONST objv[])
{
    static CONST char *commands[] = {
        "info",
        "start",
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
        "abort",
        0L
    };
    enum commands {
        cmInfo,
        cmStart,
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
        cmClose,
        cmAbort,
    };
    int index;

    if (objc < 2) {
        Tcl_WrongNumArgs(tclInterp, 1, objv, "command");
        return TCL_ERROR;
    }

    if (Tcl_GetIndexFromObj(tclInterp, objv[1], commands, "command", 0, &index) != TCL_OK) {
        return TCL_ERROR;
    }

    switch ((enum commands)(index)) {

    case cmInfo:
        if (objc == 3) {
            if (strcmp(Tcl_GetString(objv[2]), "protocol") == 0)
                Tcl_SetResult(tclInterp, "gdi", NULL);
            else if (strcmp(Tcl_GetString(objv[2]), "name") == 0)
                Tcl_SetResult(tclInterp, Tcl_DStringValue(&printername), NULL);
            else if (strcmp(Tcl_GetString(objv[2]), "page") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(pageno));
            else if (strcmp(Tcl_GetString(objv[2]), "height") == 0) 
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, VERTRES))));
            else if (strcmp(Tcl_GetString(objv[2]), "width") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, HORZRES))));
            else if (strcmp(Tcl_GetString(objv[2]), "pheight") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, VERTSIZE))));
            else if (strcmp(Tcl_GetString(objv[2]), "pwidth") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, HORZSIZE))));
            else if (strcmp(Tcl_GetString(objv[2]), "iheight") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, LOGPIXELSY))));
            else if (strcmp(Tcl_GetString(objv[2]), "iwidth") == 0)
                Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(_(::GetDeviceCaps(printerdc, LOGPIXELSX))));
            else if (strcmp(Tcl_GetString(objv[2]), "cheight") == 0) {
                SIZE s;
                if (_(::GetTextExtentPoint32(printerdc, _T("x"), 1, &s)))
                    Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(s.cy));
//              TEXTMETRIC tm;
//              if _(::GetTextMetrics(printerdc, &tm))
//                  Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(tm.tmHeight));
                else
                   Tcl_SetResult(tclInterp, "", NULL);
            } else if (strcmp(Tcl_GetString(objv[2]), "cwidth") == 0) {
                SIZE s;
                if (_(::GetTextExtentPoint32(printerdc, _T("x"), 1, &s)))
                    Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(s.cx));
//              TEXTMETRIC tm;
//              if _(::GetTextMetrics(printerdc, &tm))
//                  Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(tm.tmAveCharWidth));
//                  Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(tm.tmAveCharWidth + tm.tmOverhang));
//                  Tcl_SetObjResult(tclInterp, Tcl_NewIntObj((tm.tmAveCharWidth + tm.tmMaxCharWidth) / 2));
//                  Tcl_SetObjResult(tclInterp, Tcl_NewIntObj(tm.tmAveCharWidth + _(::GetTextCharacterExtra(printerdc))));
                else
                   Tcl_SetResult(tclInterp, "", NULL);
            } else {
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
            if (StartDoc(document, output) != TCL_OK) {
//              StartDoc hasn't set last error 
//              return AppendSystemError(tclInterp, "error starting document: ", GetError());
                Tcl_SetResult(tclInterp, "error starting document", NULL);
                return TCL_ERROR;
            } 
            return TCL_OK;
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

    case cmPlace:
        {
//          LOGFONT font = {};
            LOGFONT font = logfont;
            RECT rect = {0, 0, _(::GetDeviceCaps(printerdc, HORZRES)), _(::GetDeviceCaps(printerdc, VERTRES))};
            UINT align = 0;
            BOOL calc = FALSE;
            int obji = 2;

            if (!PageStarted()) {
                Tcl_AppendResult(tclInterp, "page is not started", NULL);
                return TCL_ERROR;
            }
            if (ParsePlaceParams(objc, objv, &obji, &rect, &align, &font, &calc) != TCL_OK) {
                return TCL_ERROR;
            }
            if (obji == objc-1) {
                if (Configure(&font) == TCL_OK) {
                    int result = Place(&rect, calc, align, objv[obji]);
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
//          LOGFONT font = {};
            LOGFONT font = logfont;
            RECT rect = {0, 0, _(::GetDeviceCaps(printerdc, HORZRES)), _(::GetDeviceCaps(printerdc, VERTRES))};
            BOOL wrap = FALSE;
            int obji = 2;

            if (!PageStarted()) {
                Tcl_AppendResult(tclInterp, "page is not started", NULL);
                return TCL_ERROR;
            }
            if (ParsePrintParams(objc, objv, &obji, &rect, &font, &wrap) != TCL_OK) {
                return TCL_ERROR;
            }
            if (obji == objc-1) {
                if (Configure(&font) == TCL_OK) {
                    int printed = Print(&rect, wrap, objv[obji]);
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
            int brush = WHITE_BRUSH; 
            int pen = BLACK_PEN;
            int obji = 2;
            switch ((enum commands)index) {
                case cmFrame: brush = BLACK_BRUSH; break;
                case cmFill: brush = GRAY_BRUSH; break;
            }

            if (!PageStarted()) {
                Tcl_AppendResult(tclInterp, "page is not started", NULL);
                return TCL_ERROR;
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-rect") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseRectArg(objv[obji], &rect) != TCL_OK) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for ", Tcl_GetString(objv[obji-1]), NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[obji-1]), NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji < objc) {
                if (strcmp(Tcl_GetString(objv[obji]), "-brush") == 0) {
                    obji++;
                    if (obji < objc) {
                        if (ParseBrushArg(objv[obji], &brush) != TCL_OK) {
                            Tcl_AppendResult(tclInterp, ", invalid option value for ", Tcl_GetString(objv[obji-1]), NULL);
                            return TCL_ERROR;
                        }
                        obji++;  
                    } else {
                        Tcl_AppendResult(tclInterp, "missing option value for ", Tcl_GetString(objv[obji-1]), NULL);
                        return TCL_ERROR;
                    } 
                }
            }
            if (obji == objc) {
                if (SelectObject(printerdc, GetStockObject(brush)) != NULL &&
                        SelectObject(printerdc, GetStockObject(pen)) != NULL) {
                    int result = 0;
                    switch ((enum commands)(index)) {
                    case cmFrame:
                        result = FrameRect(printerdc, &rect, (HBRUSH)GetStockObject(brush));
                        break;
                    case cmFill:
                        result = FillRect(printerdc, &rect, (HBRUSH)GetStockObject(brush));
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
                Tcl_WrongNumArgs(tclInterp, 2, objv, "?-rect rectspec? ?-brush brushspec?");
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

    case cmAbort:
        if (!Opened()) {
            Tcl_AppendResult(tclInterp, "printer is not opened", NULL);
            return TCL_ERROR;
        }
        if (objc == 2) {
            AbortDoc();
            delete this;
            return TCL_OK;
        } else {
            Tcl_WrongNumArgs(tclInterp, 2, objv, NULL);
            return TCL_ERROR;
        }

    } // switch index

    return TCL_OK;
}

void TclPrintGdiCmd::Close() {
    if (PageStarted()) 
        EndPage();
    if (Started()) 
        EndDoc();
    if (Opened())
        _(::DeleteDC(printerdc));
    if (printerfont) 
        _(::DeleteObject(printerfont));
    printerdc = NULL;
    printerfont = NULL;
    ZeroMemory(&logfont, sizeof(logfont));
    TclPrintCmd::Close();
}

BOOL TclPrintGdiCmd::Opened() {
    return printerdc != NULL;
}

#ifdef DIALOGS
int TclPrintGdiCmd::Select (BOOL setupdialog) {
    PRINTDLG printdlg = {0};
    ZeroMemory(&printdlg, sizeof(printdlg));
    printdlg.lStructSize = sizeof(printdlg);
    printdlg.Flags = PD_ALLPAGES | PD_USEDEVMODECOPIES | PD_NOPAGENUMS | PD_HIDEPRINTTOFILE | PD_NOSELECTION | PD_RETURNDC;
    if (setupdialog)
        printdlg.Flags |= PD_PRINTSETUP;
    int result = TCL_ERROR;

    Close();

    if (_(::PrintDlg(&printdlg))) {
        printerdc = printdlg.hDC;
        if (printdlg.hDevMode != NULL && printdlg.hDevNames != NULL) {
            LPDEVNAMES devnames = (LPDEVNAMES)GlobalLock(printdlg.hDevNames) ;
            ExternalToUtf((TCHAR *)((LPCTSTR)devnames+devnames->wDeviceOffset), &printername);
            GlobalUnlock(printdlg.hDevNames);
        }
        return TCL_OK;
    }
    SetError(::GetLastError());
    return result;
}
#endif

int TclPrintGdiCmd::Open(Tcl_Obj *printer) {
    Tcl_DString ds;
    Tcl_DStringInit(&ds);
    UtfToExternal(Tcl_GetString(printer), &ds);

    Close();

    printerdc = _(::CreateDC(_T("WINSPOOL"), (TCHAR *)Tcl_DStringValue(&ds), NULL, NULL));

    Tcl_DStringFree(&ds);

    if (printerdc) {
        Tcl_DStringAppend(&printername, Tcl_GetString(printer), -1);
        return TCL_OK;
    } else {
        SetError(::GetLastError());
        return TCL_ERROR;
    }
}

int TclPrintGdiCmd::OpenDefault() {
    DWORD size = 0;
    TCHAR* buffer = NULL;
    int result = TCL_ERROR;

    Close();

    _(::GetDefaultPrinter(NULL, &size));
    if (size > 0) {
        buffer = (TCHAR *)ckalloc(size);
        if (_(::GetDefaultPrinter(buffer, &size))) {
            printerdc = _(::CreateDC(_T("WINSPOOL"), buffer, NULL, NULL));
            if (printerdc) {
                ExternalToUtf(buffer, &printername);
                result = TCL_OK;
            }
        }
        ckfree((char *)buffer);
    }
    if (result == TCL_ERROR) 
        SetError(::GetLastError());
    return result;
}

int TclPrintGdiCmd::Configure(LOGFONT *lf) {
    int result = TCL_OK;

    SetError(0);

    if (memcmp(&logfont, lf, sizeof(logfont)) != 0) {
        LONG userheight = lf->lfHeight;
        LONG userwidth = lf->lfWidth;
        lf->lfHeight = -MulDiv(userheight, _(::GetDeviceCaps(printerdc, LOGPIXELSY)), 72);
        lf->lfWidth = -MulDiv(userwidth, _(::GetDeviceCaps(printerdc, LOGPIXELSX)), 72);

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

int TclPrintGdiCmd::StartDoc(Tcl_Obj *document, Tcl_Obj *output) {
    int result;
    Tcl_DString documentds, outputds;
    Tcl_DStringInit(&documentds);
    Tcl_DStringInit(&outputds);

    DOCINFO docinfo;
    ZeroMemory(&docinfo, sizeof(DOCINFO));
    docinfo.cbSize = sizeof(DOCINFO);
    if (document) {
        UtfToExternal(Tcl_GetString(document), &documentds);
        docinfo.lpszDocName = (TCHAR *)Tcl_DStringValue(&documentds);
    }
    if (output) {
        UtfToExternal(Tcl_GetString(output), &outputds);
        docinfo.lpszOutput = (TCHAR *)Tcl_DStringValue(&outputds);
    }

    if (_(::StartDoc(printerdc, &docinfo) <= 0)) {
        _(::AbortDoc(printerdc));
        SetError(0); // StartDoc does not set LastError
        pageno = 0;
        pageposy = 0;
        result = TCL_ERROR;
    } else
        result = TclPrintCmd::StartDoc(document, output);

    Tcl_DStringFree(&outputds);
    Tcl_DStringFree(&documentds);
    return result;
}

int TclPrintGdiCmd::AbortDoc() {
    if (_(::AbortDoc(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageposy = 0;
        return TclPrintCmd::AbortDoc();
    }
}

int TclPrintGdiCmd::EndDoc() {
    if (pageactive) 
        EndPage(); // ignore errors
    if (_(::EndDoc(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        pageposy = 0;
        return TclPrintCmd::EndDoc();
    }
}

int TclPrintGdiCmd::StartPage() {
    if (_(::StartPage(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        _(::MoveToEx(printerdc, 0, 0, NULL));
        pageposy = 0;
        return TclPrintCmd::StartPage();
    }
}

int TclPrintGdiCmd::EndPage() {
    if (_(::EndPage(printerdc)) <= 0) {
        SetError(::GetLastError());
        return TCL_ERROR;
    } else {
        return TclPrintCmd::EndPage();
    }
}

int TclPrintGdiCmd::Place(RECT *rect, BOOL calc, UINT align, Tcl_Obj *text) {
    UINT format = align | DT_NOPREFIX;

    Tcl_DString textds;
    Tcl_DStringInit(&textds);
    UtfToExternal(Tcl_GetString(text), &textds);

    _(::SetTextAlign(printerdc, TA_TOP | TA_LEFT | TA_NOUPDATECP));

    int result = _(::DrawText(printerdc, (TCHAR *)Tcl_DStringValue(&textds), -1, rect, (calc ? (format | DT_CALCRECT) : format)));

    Tcl_DStringFree(&textds);

    SetError(result == TCL_OK ? 0 : ::GetLastError());
    return result;
}

int TclPrintGdiCmd::Print(RECT *rect, BOOL wrap, Tcl_Obj *text) {
    Tcl_DString textds;
    Tcl_DStringInit(&textds);
    UtfToExternal(Tcl_GetString(text), &textds);

    int printed = PrintText(rect, wrap, (TCHAR *)Tcl_DStringValue(&textds));

    Tcl_DStringFree(&textds);
    return printed;
}

int TclPrintGdiCmd::PrintDoc(Tcl_Obj *document, Tcl_Obj *output, RECT *rect, BOOL wrap, Tcl_Obj *text) {
    Tcl_DString textds;
    Tcl_DStringInit(&textds);
    UtfToExternal(Tcl_GetString(text), &textds);
    TCHAR *textchars = (TCHAR *)Tcl_DStringValue(&textds);
    TCHAR *start = textchars;

    int result = StartDoc(document, output);
    if (result == TCL_OK) {
        result = StartPage();
        if (result == TCL_OK) {
            int restlength = (int)_tcsclen(textchars);
            int printed;
            while (restlength > 0) {
                printed = PrintText(rect, wrap, start);
                if (printed <= 0) {
                    SetError(::GetLastError());
                    result = TCL_ERROR;
                    break;
                } else if (printed < restlength) {
                    result = EndPage();
                    if (result == TCL_OK) {
                        result = StartPage();
                    }
                    if (result != TCL_OK)
                        break;
                    restlength -= printed;
                    start += printed;
                } else {
#ifdef _DEBUG
                    assert(printed == restlength);
#endif
                    start += printed;
                    break;
                }
            }
            result = EndPage();
        }
        result = EndDoc();
    }
    Tcl_DStringFree(&textds);
    if (result == TCL_OK)
        return start - textchars;
    else 
        return -1;
}

// privates

int TclPrintGdiCmd::PrintText(RECT *rect, BOOL wrap, TCHAR *text) {
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

                _(::GetTextExtentPoint32(printerdc, start, (int)(nextstop - start), &outsize));

                if (pageposx + outsize.cx < rect->right) {
                    stop = nextstop;
                } else {
                    if (start == stop) // very long word
                        stop = nextstop;
                    break;
                }
            }
            drawlength = (int)(stop - start);
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

int TclPrintGdiCmd::ParsePlaceParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, UINT *align, LOGFONT *font, BOOL *calc) {
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-rect") == 0) {
            (*obji)++;
            if (*obji < objc) {
                if (ParseRectArg(objv[*obji], rect) != TCL_OK) {
                    Tcl_AppendResult(tclInterp, ", invalid option value for -rect", NULL);
                    return TCL_ERROR;
                }
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for -rect", NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-align") == 0) {
            (*obji)++;
            if (*obji < objc) {
                if (ParseAlignArg(objv[*obji], align) != TCL_OK) {
                    Tcl_AppendResult(tclInterp, ", invalid option value for -align", NULL);
                    return TCL_ERROR;
                }
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for -align", NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-font") == 0) {
            (*obji)++;
            if (*obji < objc) {
                if (ParseFontArg(objv[*obji], font) != TCL_OK) {
                    Tcl_AppendResult(tclInterp, ", invalid option value for -font", NULL);
                    return TCL_ERROR;
                }
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for -font", NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-calcrect") == 0) {
            *calc = TRUE;
            (*obji)++;
        }
    }
    return TCL_OK;
}

int TclPrintGdiCmd::ParsePrintParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, LOGFONT *font, BOOL *wrap) {
    RECT margins = {0, 0, 0, 0};
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-margins") == 0) {
            (*obji)++;
            if (*obji < objc) {
                RECT margins = {0, 0, 0, 0};
                if (ParseRectArg(objv[*obji], &margins) != TCL_OK) {
                    Tcl_AppendResult(tclInterp, ", invalid option value for -margins", NULL);
                    return TCL_ERROR;
                }
                rect->left += margins.left;
                rect->top += margins.top;
                rect->right -= margins.right;
                rect->bottom -= margins.bottom;
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for -margins", NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-font") == 0) {
            (*obji)++;
            if (*obji < objc) {
                if (ParseFontArg(objv[*obji], font) != TCL_OK) {
                    Tcl_AppendResult(tclInterp, ", invalid option value for -font", NULL);
                    return TCL_ERROR;
                }
                (*obji)++;  
            } else {
                Tcl_AppendResult(tclInterp, "missing option value for -font", NULL);
                return TCL_ERROR;
            } 
        }
    }
    if (*obji < objc) {
        if (strcmp(Tcl_GetString(objv[*obji]), "-wordwrap") == 0) {
            *wrap = TRUE;
            (*obji)++;
        }
    }
    return TCL_OK;
}

// ?-font {name ?height? ?width? ?weight? ?italic? ?underline? ?strikeout? ?charset? ?quality? ?pitch? ?family?}?

int TclPrintGdiCmd::ParseFontArg(struct Tcl_Obj *obj, LOGFONT *lf) {
    static CONST char *weightsMap[] = {
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

    static CONST char *stylesMap[] = {
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

    static CONST char *charsetsMap[] = {
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

    static CONST char *qualitiesMap[] = {
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

    static CONST char *pitchesMap[] = {
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

    static CONST char *familiesMap[] = {
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

    int objc;
    Tcl_Obj **objv;

    if ((Tcl_ListObjGetElements(tclInterp, obj, &objc, &objv) == TCL_OK) || (objc > 0)) {
        int obji = 0;
        // font face: always at 1st position
        if (obji < objc) {
            if (Tcl_GetCharLength(objv[obji]) > 0) {
                Tcl_DString ds;
                Tcl_DStringInit(&ds);
                UtfToExternal(Tcl_GetString(objv[obji]), &ds);
                unsigned len = min(Tcl_DStringLength(&ds), SIZEOFARRAY(lf->lfFaceName)-1);
                memset(lf->lfFaceName, 0, sizeof(lf->lfFaceName));
                memcpy(lf->lfFaceName, Tcl_DStringValue(&ds), len);
                Tcl_DStringFree(&ds);
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
                        Tcl_GetIndexFromObj(NULL, objv[obji], weightsMap, NULL, 0, &index) == TCL_OK) {
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
                if (Tcl_GetIndexFromObj(NULL, objv[obji], charsetsMap, NULL, 0, &index) == TCL_OK) {
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
                if (Tcl_GetIndexFromObj(NULL, objv[obji], qualitiesMap, NULL, 0, &index) == TCL_OK) {
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
                if (Tcl_GetIndexFromObj(NULL, objv[obji], pitchesMap, NULL, 0, &index) == TCL_OK) {
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
                if (Tcl_GetIndexFromObj(NULL, objv[obji], familiesMap, NULL, 0, &index) == TCL_OK) {
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

int TclPrintGdiCmd::ParseAlignArg(struct Tcl_Obj *obj, UINT *format) {
    static CONST char *flagsMap[] = {
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
            if (Tcl_GetIndexFromObj(tclInterp, objv[i], flagsMap, "flag", 0, &index) == TCL_OK)
                *format |= flags[index];
            else
                return TCL_ERROR;
        }
    } else
        return TCL_ERROR;

    return TCL_OK;
}

#ifndef DC_BRUSH
#define DC_BRUSH 18
#endif

int TclPrintGdiCmd::ParseBrushArg(struct Tcl_Obj *obj, int *brush) {
    static CONST char *brushesMap[] = {
        "black",
        "dkgray",
        "dc",
        "gray",
        "hollow",
        "ltgray",
        "null",
        "white",
        0L
    };
    static int brushes[] = {
        BLACK_BRUSH,
        DKGRAY_BRUSH,
        DC_BRUSH,
        GRAY_BRUSH,
        HOLLOW_BRUSH,
        LTGRAY_BRUSH,
        NULL_BRUSH,
        WHITE_BRUSH 
    };

    int index = 0;
    int result = Tcl_GetIndexFromObj(tclInterp, obj, brushesMap, "brush", 0, &index);
    if (result == TCL_OK)
        *brush = brushes[index];
    return result;
}

// -rect {?x0? ?y0? ?x1? ?y1?} 

int TclPrintGdiCmd::ParseRectArg(struct Tcl_Obj *obj, RECT *rect) {
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
