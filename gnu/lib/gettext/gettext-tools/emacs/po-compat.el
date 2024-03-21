;;; po-compat.el --- basic support of PO translation files -*- coding: latin-1; -*-

;; Copyright (C) 1995-2002, 2010, 2016, 2019 Free Software Foundation, Inc.

;; Authors: François Pinard <pinard@iro.umontreal.ca>,
;;          Greg McGary <gkm@magilla.cichlid.com>,
;;          Bruno Haible <bruno@clisp.org>.
;; Keywords: i18n, files

;; This file is part of GNU gettext.

;; GNU gettext is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.

;; GNU gettext is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, see
;; <https://www.gnu.org/licenses/>.

;;; Commentary:

;; Emacs 21.2 and newer already contain this file, under the name po.el,
;; and without portability hassles.

;; This package makes sure visiting PO files decodes them correctly,
;; according to the Charset= header in the PO file.  For more support
;; for editing PO files, see po-mode.el.

;;; Code:

;;; Emacs portability matters.

(defconst po-content-type-charset-alist
  '(; Note: Emacs 21 doesn't support all encodings, thus the missing entries.
    ("ASCII" . undecided)
    ("ANSI_X3.4-1968" . undecided)
    ("US-ASCII" . undecided)
    ("ISO-8859-1" . iso-8859-1)
    ("ISO_8859-1" . iso-8859-1)
    ("ISO-8859-2" . iso-8859-2)
    ("ISO_8859-2" . iso-8859-2)
    ("ISO-8859-3" . iso-8859-3)
    ("ISO_8859-3" . iso-8859-3)
    ("ISO-8859-4" . iso-8859-4)
    ("ISO_8859-4" . iso-8859-4)
    ("ISO-8859-5" . iso-8859-5)
    ("ISO_8859-5" . iso-8859-5)
    ;("ISO-8859-6" . ??)
    ;("ISO_8859-6" . ??)
    ("ISO-8859-7" . iso-8859-7)
    ("ISO_8859-7" . iso-8859-7)
    ("ISO-8859-8" . iso-8859-8)
    ("ISO_8859-8" . iso-8859-8)
    ("ISO-8859-9" . iso-8859-9)
    ("ISO_8859-9" . iso-8859-9)
    ;("ISO-8859-13" . ??)
    ;("ISO_8859-13" . ??)
    ;("ISO-8859-14" . ??)
    ;("ISO_8859-14" . ??)
    ("ISO-8859-15" . iso-8859-15)
    ("ISO_8859-15" . iso-8859-15)
    ("KOI8-R" . koi8-r)
    ;("KOI8-U" . ??)
    ;("KOI8-T" . ??)
    ("CP437" . cp437)
    ("CP775" . cp775)
    ("CP850" . cp850)
    ("CP852" . cp852)
    ("CP855" . cp855)
    ;("CP856" . ??)
    ("CP857" . cp857)
    ("CP861" . cp861)
    ("CP862" . cp862)
    ("CP864" . cp864)
    ("CP865" . cp865)
    ("CP866" . cp866)
    ("CP869" . cp869)
    ;("CP874" . ??)
    ;("CP922" . ??)
    ;("CP932" . ??)
    ;("CP943" . ??)
    ;("CP949" . ??)
    ;("CP950" . ??)
    ;("CP1046" . ??)
    ;("CP1124" . ??)
    ;("CP1129" . ??)
    ("CP1250" . cp1250)
    ("CP1251" . cp1251)
    ("CP1252" . iso-8859-1) ; approximation
    ("CP1253" . cp1253)
    ("CP1254" . iso-8859-9) ; approximation
    ("CP1255" . iso-8859-8) ; approximation
    ;("CP1256" . ??)
    ("CP1257" . cp1257)
    ("GB2312" . cn-gb-2312)  ; also named 'gb2312' and 'euc-cn'
    ("EUC-JP" . euc-jp)
    ("EUC-KR" . euc-kr)
    ;("EUC-TW" . ??)
    ("BIG5" . big5)
    ;("BIG5-HKSCS" . ??)
    ;("GBK" . ??)
    ;("GB18030" . ??)
    ("SHIFT_JIS" . shift_jis)
    ;("JOHAB" . ??)
    ("TIS-620" . tis-620)
    ("VISCII" . viscii)
    ;("GEORGIAN-PS" . ??)
    ("UTF-8" . utf-8)
    )
  "How to convert a GNU libc/libiconv canonical charset name as seen in
Content-Type into a Mule coding system.")

(defun po-find-charset (filename)
  "Return PO file charset value."
  (interactive)
  (let ((charset-regexp
         "^\"Content-Type: text/plain;[ \t]*charset=\\(.*\\)\\\\n\"")
        (short-read nil))
    ;; Try the first 4096 bytes.  In case we cannot find the charset value
    ;; within the first 4096 bytes (the PO file might start with a long
    ;; comment) try the next 4096 bytes repeatedly until we'll know for sure
    ;; we've checked the empty header entry entirely.
    (while (not (or short-read (re-search-forward "^msgid" nil t)))
      (save-excursion
        (goto-char (point-max))
        (let ((pair (insert-file-contents-literally filename nil
                                                    (1- (point))
                                                    (1- (+ (point) 4096)))))
          (setq short-read (< (nth 1 pair) 4096)))))
    (cond ((re-search-forward charset-regexp nil t) (match-string 1))
          (short-read nil)
          ;; We've found the first msgid; maybe, only a part of the msgstr
          ;; value was loaded.  Load the next 1024 bytes; if charset still
          ;; isn't available, give up.
          (t (save-excursion
               (goto-char (point-max))
               (insert-file-contents-literally filename nil
                                               (1- (point))
                                               (1- (+ (point) 1024))))
             (if (re-search-forward charset-regexp nil t)
                 (match-string 1))))))

;;;###autoload (autoload 'po-find-file-coding-system "po-compat")

(defun po-find-file-coding-system-guts (operation filename)
  "\
Return a Mule (DECODING . ENCODING) pair, according to PO file charset.
Called through file-coding-system-alist, before the file is visited for real."
  (and (eq operation 'insert-file-contents)
       (file-exists-p filename)
       (po-with-temp-buffer
        (let* ((coding-system-for-read 'no-conversion)
               (charset (or (po-find-charset filename) "ascii"))
               (charset-upper (upcase charset))
               (charset-lower (downcase charset))
               (candidate
                (cdr (assoc charset-upper po-content-type-charset-alist)))
               (try-symbol (or candidate (intern-soft charset-lower)))
               (try-string
                (if try-symbol (symbol-name try-symbol) charset-lower)))
          (list (cond ((and try-symbol (coding-system-p try-symbol))
                       try-symbol)
                      ((and (not (string-lessp "23" emacs-version))
                            (string-match "\\`cp[1-9][0-9][0-9]?\\'"
                                          try-string)
                            (assoc (substring try-string 2)
                                   (cp-supported-codepages)))
                       (codepage-setup (substring try-string 2))
                       (intern try-string))
                      (t
                       'no-conversion)))))))

(defun po-find-file-coding-system (arg-list)
  "\
Return a Mule (DECODING . ENCODING) pair, according to PO file charset.
Called through file-coding-system-alist, before the file is visited for real."
  (po-find-file-coding-system-guts (car arg-list) (car (cdr arg-list))))

(provide 'po-compat)

;;; Testing this file:

;; For each pofile in {
;;   cs.po           ; gettext/po/cs.el, charset=ISO-8859-2
;;   cs-modified.po  ; gettext/po/cs.el, charset=ISO_8859-2
;;   de.po           ; gettext/po/de.el, charset=UTF-8, if $emacsimpl = emacs
;; } do
;;   Start $emacsimpl
;;   M-x load-file  po-compat.el RET
;;   C-x C-f  $pofile RET
;;   Verify charset marker in status line ('2' = ISO-8859-2, 'u' = UTF-8).

;;; po-compat.el ends here
