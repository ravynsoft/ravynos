;;; Copyright (C) 2023 Free Software Foundation, Inc.
;;; This file is part of the GNU LIBICONV Tools.
;;;
;;; This program is free software: you can redistribute it and/or modify
;;; it under the terms of the GNU General Public License as published by
;;; the Free Software Foundation; either version 3 of the License, or
;;; (at your option) any later version.
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;; GNU General Public License for more details.
;;;
;;; You should have received a copy of the GNU General Public License
;;; along with this program; if not, see <https://www.gnu.org/licenses/>.

;;; Transform UnicodeData.txt to a part of translit.def.
;;; translit.def consists of lines of the form
;;;   Unicode <tab> utf-8 replacement <tab> # comment
;;; This program produces the lines for the code points >= U+3200
;;; and reasonable approximations for the code points < U+3200.

(defun starts-with (s prefix)
  (locally (declare (compile)))
  (and (>= (length s) (length prefix))
       (equal (substring s 0 (length prefix)) prefix)))

(defun generate-part (infilename outfilename)
  (locally (declare (compile)))
  (with-open-file (in infilename)
    (with-open-file (out outfilename :direction :output
                                     :external-format CHARSET:UTF-8)
      (loop
        (let ((line (read-line in nil nil)))
          (unless line (return))
          (let* ((semicolon1 (position #\; line))
                 (semicolon2 (position #\; line :start (+ semicolon1 1)))
                 (semicolon3 (position #\; line :start (+ semicolon2 1)))
                 (semicolon4 (position #\; line :start (+ semicolon3 1)))
                 (semicolon5 (position #\; line :start (+ semicolon4 1)))
                 (semicolon6 (position #\; line :start (+ semicolon5 1)))
                 (code (let ((*read-base* 16)) (read-from-string (substring line 0 semicolon1))))
                 (name (substring line (+ semicolon1 1) semicolon2))
                 (category (substring line (+ semicolon2 1) semicolon3))
                 (combining (substring line (+ semicolon3 1) semicolon4))
                 (bidi (substring line (+ semicolon4 1) semicolon5))
                 (decomposition (substring line (+ semicolon5 1) semicolon6)))
            (declare (ignore category combining bidi))
            (when (or (starts-with decomposition "<circle> ")
                      (starts-with decomposition "<compat> ")
                      (starts-with decomposition "<font> ")
                      (starts-with decomposition "<narrow> ")
                      (starts-with decomposition "<small> ")
                      (starts-with decomposition "<square> ")
                      (starts-with decomposition "<wide> ")
                      (and (starts-with name "CJK COMPATIBILITY IDEOGRAPH-")
                           (> (length decomposition) 0))
                  )
              (let ((replacement '()))
                (let ((i (+ (or (position #\> decomposition) -2) 2)))
                  (loop
                    (let* ((space (position #\Space decomposition :start i))
                           (hexcode (substring decomposition i (or space (length decomposition)))))
                      (push
                        (int-char (let ((*read-base* 16)) (read-from-string hexcode)))
                        replacement)
                      (unless space
                        (return))
                      (setq i (+ space 1)))))
                (setq replacement (nreverse replacement))
                (when (starts-with decomposition "<circle> ")
                  (setq replacement (concatenate 'list '(#\() replacement '(#\)))))
                (setq replacement (mapcan (lambda (c)
                                            (cond ((eql c #\DIVISION_SLASH) (list #\/))
                                                  ((eql c #\SUPERSCRIPT_TWO) (list #\^ #\2))
                                                  ((eql c #\SUPERSCRIPT_THREE) (list #\^ #\3))
                                                  ((eql c #\SCRIPT_SMALL_L) (list #\l))
                                                  (t (list c))))
                                          replacement))
                (format out "~4,'0X~A~{~A~}~A# ~A~%" code #\Tab replacement #\Tab name)
            ) )
) ) ) ) ) )

; (generate-part "/home/bruno/notes/UnicodeData-15.0.0.txt" "translit-part-15.0.0.def")
