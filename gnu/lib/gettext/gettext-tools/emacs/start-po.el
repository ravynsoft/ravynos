;;; start-po.el --- autoload definitions for viewing and editing PO files

;; Copyright (C) 1995-2004, 2016, 2019 Free Software Foundation, Inc.
;;
;; This file is part of GNU gettext.
;;
;; GNU gettext is free software; you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 2, or (at your option)
;; any later version.
;;
;; GNU gettext is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.
;;
;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs; see the file COPYING.  If not, see
;; <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This file provides a minimal amount of definitions that will autoload
;; the complete support for viewing and editing PO files when necessary.
;; It is meant to be installed in such a way that it will be part of the
;; dumped Emacs image, or loaded automatically when Emacs is started,
;; regardless how the user's .emacs looks like.

;;; Code:

;; For editing PO files.

(autoload 'po-mode "po-mode"
  "Major mode for translators when they edit PO files.

Special commands:
\\{po-mode-map}
Turning on PO mode calls the value of the variable 'po-mode-hook',
if that value is non-nil.  Behaviour may be adjusted through some variables,
all reachable through 'M-x customize', in group 'Emacs.Editing.I18n.Po'."
  t)
(setq auto-mode-alist
      (cons '("\\.po\\'\\|\\.po\\." . po-mode) auto-mode-alist))

;; For viewing PO and POT files.

;; To use the right coding system automatically.
(unless (fboundp 'po-find-file-coding-system)
  (autoload 'po-find-file-coding-system "po-compat"
    "\
Return a Mule (DECODING . ENCODING) pair, according to PO file charset.
Called through file-coding-system-alist, before the file is visited for real."))
(modify-coding-system-alist 'file "\\.po[t]?\\'\\|\\.po\\."
                            'po-find-file-coding-system)
