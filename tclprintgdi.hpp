#ifndef TCLPRINTGDI_H
#define TCLPRINTGDI_H

#include <windows.h>
#include "tclprint.hpp"

class TclPrintGdiCmd : public TclPrintCmd {
    friend class TclPrinterCmd;

public:

    TclPrintGdiCmd(Tcl_Interp *interp, char *name):
          TclPrintCmd(interp, name),
          printerdc(NULL),
          pageposy(0) {
        Tcl_DStringInit(&printername);
        ZeroMemory(&logfont, sizeof(logfont));
    };

    virtual ~TclPrintGdiCmd() {
        Close();
    };

    virtual BOOL Opened();

    virtual void Close();

    virtual int Select(BOOL usedefault);
    virtual int Open(Tcl_Obj *printer);
    virtual int Configure(LOGFONT *fc);
    virtual int StartDoc(Tcl_Obj *document, Tcl_Obj *output);
    virtual int AbortDoc();
    virtual int EndDoc();
    virtual int StartPage();
    virtual int EndPage();

    int Place(RECT *rect, BOOL calc, UINT align, Tcl_Obj *text);
    int Print(RECT *rect, BOOL wrap, Tcl_Obj *text);
    int PrintDoc(Tcl_Obj *documentobj, Tcl_Obj *outputobj, RECT *rect, BOOL wrap, Tcl_Obj *text);

    HDC GetDC() {
        return printerdc;
    };

protected:

    int pageposy;
    HDC printerdc;
    HFONT printerfont;
    LOGFONT logfont;

    int PrintText(RECT *rect, BOOL wrap, TCHAR *text);

    int ParsePlaceParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, UINT *align, LOGFONT *font, BOOL *calc);
    int ParsePrintParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, LOGFONT *font, BOOL *wrap);
    int ParseFontArg(Tcl_Obj *obj, LOGFONT *lf);
    int ParseAlignArg(Tcl_Obj *obj, UINT *format);
    int ParseRectArg(Tcl_Obj *obj, RECT *rect);

private:

    virtual int Command(int objc, struct Tcl_Obj *CONST objv[]);
};

#endif
