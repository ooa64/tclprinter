#ifndef TCLPRINTGDI_H
#define TCLPRINTGDI_H

#include <windows.h>
#include "tclprint.hpp"

class TclPrintGdiCmd : public TclPrintCmd {
    friend class TclPrinterCmd;

public:

    TclPrintGdiCmd(Tcl_Interp *interp, char *name) : TclPrintCmd(interp, name) {
        printerdc = NULL;
        pageposy = 0;
        Tcl_DStringInit(&printername);
        ZeroMemory(&logfont, sizeof(logfont));
    };

    virtual ~TclPrintGdiCmd() {
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
    int AbortDoc() override;
    int EndDoc() override;
    int StartPage() override;
    int EndPage() override;

    int Configure(LOGFONT *fc);
    int Place(RECT *rect, BOOL calc, UINT align, Tcl_Obj *text);
    int Print(RECT *rect, BOOL wrap, Tcl_Obj *text);
    int PrintDoc(Tcl_Obj *document, Tcl_Obj *output, RECT *rect, BOOL wrap, Tcl_Obj *text);

    HDC GetDC() {
        return printerdc;
    };

protected:

    int pageposy;
    HDC printerdc;
    LOGFONT logfont;

    int PrintText(RECT *rect, BOOL wrap, TCHAR *text);

    int ParsePlaceParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, UINT *align, LOGFONT *font, BOOL *calc);
    int ParsePrintParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, LOGFONT *font, BOOL *wrap);
    int ParseDrawParams(int objc, Tcl_Obj *CONST objv[], int *obji, RECT *rect, int *brush);
    int ParseFontArg(Tcl_Obj *obj, LOGFONT *lf);
    int ParseAlignArg(Tcl_Obj *obj, UINT *format);
    int ParseBrushArg(Tcl_Obj *obj, int *brush);
    int ParseRectArg(Tcl_Obj *obj, RECT *rect);

private:

    int Command(int objc, struct Tcl_Obj *CONST objv[]) override;
};

#endif
