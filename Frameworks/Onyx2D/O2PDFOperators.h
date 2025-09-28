/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSObject.h>

@class O2PDFScanner, O2PDFOperatorTable;

void O2PDF_render_b(O2PDFScanner *scanner, void *info);
void O2PDF_render_B(O2PDFScanner *scanner, void *info);
void O2PDF_render_b_star(O2PDFScanner *scanner, void *info);
void O2PDF_render_B_star(O2PDFScanner *scanner, void *info);
void O2PDF_render_BDC(O2PDFScanner *scanner, void *info);
void O2PDF_render_BI(O2PDFScanner *scanner, void *info);
void O2PDF_render_BMC(O2PDFScanner *scanner, void *info);
void O2PDF_render_BT(O2PDFScanner *scanner, void *info);
void O2PDF_render_BX(O2PDFScanner *scanner, void *info);
void O2PDF_render_c(O2PDFScanner *scanner, void *info);
void O2PDF_render_cm(O2PDFScanner *scanner, void *info);
void O2PDF_render_CS(O2PDFScanner *scanner, void *info);
void O2PDF_render_cs(O2PDFScanner *scanner, void *info);
void O2PDF_render_d(O2PDFScanner *scanner, void *info);
void O2PDF_render_d0(O2PDFScanner *scanner, void *info);
void O2PDF_render_d1(O2PDFScanner *scanner, void *info);
void O2PDF_render_Do(O2PDFScanner *scanner, void *info);
void O2PDF_render_DP(O2PDFScanner *scanner, void *info);
void O2PDF_render_EI(O2PDFScanner *scanner, void *info);
void O2PDF_render_EMC(O2PDFScanner *scanner, void *info);
void O2PDF_render_ET(O2PDFScanner *scanner, void *info);
void O2PDF_render_EX(O2PDFScanner *scanner, void *info);
void O2PDF_render_f(O2PDFScanner *scanner, void *info);
void O2PDF_render_F(O2PDFScanner *scanner, void *info);
void O2PDF_render_f_star(O2PDFScanner *scanner, void *info);
void O2PDF_render_G(O2PDFScanner *scanner, void *info);
void O2PDF_render_g(O2PDFScanner *scanner, void *info);
void O2PDF_render_gs(O2PDFScanner *scanner, void *info);
void O2PDF_render_h(O2PDFScanner *scanner, void *info);
void O2PDF_render_i(O2PDFScanner *scanner, void *info);
void O2PDF_render_ID(O2PDFScanner *scanner, void *info);
void O2PDF_render_j(O2PDFScanner *scanner, void *info);
void O2PDF_render_J(O2PDFScanner *scanner, void *info);
void O2PDF_render_K(O2PDFScanner *scanner, void *info);
void O2PDF_render_k(O2PDFScanner *scanner, void *info);
void O2PDF_render_l(O2PDFScanner *scanner, void *info);
void O2PDF_render_m(O2PDFScanner *scanner, void *info);
void O2PDF_render_M(O2PDFScanner *scanner, void *info);
void O2PDF_render_MP(O2PDFScanner *scanner, void *info);
void O2PDF_render_n(O2PDFScanner *scanner, void *info);
void O2PDF_render_q(O2PDFScanner *scanner, void *info);
void O2PDF_render_Q(O2PDFScanner *scanner, void *info);
void O2PDF_render_re(O2PDFScanner *scanner, void *info);
void O2PDF_render_RG(O2PDFScanner *scanner, void *info);
void O2PDF_render_rg(O2PDFScanner *scanner, void *info);
void O2PDF_render_ri(O2PDFScanner *scanner, void *info);
void O2PDF_render_s(O2PDFScanner *scanner, void *info);
void O2PDF_render_S(O2PDFScanner *scanner, void *info);
void O2PDF_render_SC(O2PDFScanner *scanner, void *info);
void O2PDF_render_sc(O2PDFScanner *scanner, void *info);
void O2PDF_render_SCN(O2PDFScanner *scanner, void *info);
void O2PDF_render_scn(O2PDFScanner *scanner, void *info);
void O2PDF_render_sh(O2PDFScanner *scanner, void *info);
void O2PDF_render_T_star(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tc(O2PDFScanner *scanner, void *info);
void O2PDF_render_Td(O2PDFScanner *scanner, void *info);
void O2PDF_render_TD(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tf(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tj(O2PDFScanner *scanner, void *info);
void O2PDF_render_TL(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tm(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tr(O2PDFScanner *scanner, void *info);
void O2PDF_render_Ts(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tw(O2PDFScanner *scanner, void *info);
void O2PDF_render_Tz(O2PDFScanner *scanner, void *info);
void O2PDF_render_v(O2PDFScanner *scanner, void *info);
void O2PDF_render_w(O2PDFScanner *scanner, void *info);
void O2PDF_render_W(O2PDFScanner *scanner, void *info);
void O2PDF_render_W_star(O2PDFScanner *scanner, void *info);
void O2PDF_render_y(O2PDFScanner *scanner, void *info);
void O2PDF_render_quote(O2PDFScanner *scanner, void *info);
void O2PDF_render_dquote(O2PDFScanner *scanner, void *info);

void O2PDF_render_populateOperatorTable(O2PDFOperatorTable *table);
