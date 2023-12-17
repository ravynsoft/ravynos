#!/usr/local/bin/clisp -C

;;; Creation of gnulib's uninames.h from the UnicodeData.txt table.

;;; Copyright (C) 2000-2023 Free Software Foundation, Inc.
;;; Written by Bruno Haible <bruno@clisp.org>, 2000-12-28.
;;;
;;; This program is free software.
;;; It is dual-licensed under "the GNU LGPLv3+ or the GNU GPLv2+".
;;; You can redistribute it and/or modify it under either
;;;   - the terms of the GNU Lesser General Public License as published
;;;     by the Free Software Foundation, either version 3, or (at your
;;;     option) any later version, or
;;;   - the terms of the GNU General Public License as published by the
;;;     Free Software Foundation; either version 2, or (at your option)
;;;     any later version, or
;;;   - the same dual license "the GNU LGPLv3+ or the GNU GPLv2+".
;;;
;;; This program is distributed in the hope that it will be useful,
;;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;;; Lesser General Public License and the GNU General Public License
;;; for more details.
;;;
;;; You should have received a copy of the GNU Lesser General Public
;;; License and of the GNU General Public License along with this
;;; program.  If not, see <https://www.gnu.org/licenses/>.

(defparameter add-comments nil)

(defstruct unicode-char
  (index nil :type integer)
  (name nil :type string)
  word-indices
  word-indices-index
)

(defstruct range
  (index nil :type integer)
  (start-code nil :type integer)
  (end-code nil :type integer)
)

(defstruct word-list
  (hashed nil :type hash-table)
  (sorted nil :type list)
  size                          ; number of characters total
  length                        ; number of words
)

(defun main (inputfile outputfile aliasfile)
  (declare (type string inputfile outputfile aliasfile))
  #+UNICODE (setq *default-file-encoding* charset:utf-8)
  (let ((all-chars '())
        (all-chars-hashed (make-hash-table :test #'equal))
        (all-aliases '())
        all-chars-and-aliases
        (all-ranges '())
        (name-index 0)
        range)
    ;; Read all characters and names from the input file.
    (with-open-file (istream inputfile :direction :input)
      (loop
        (let ((line (read-line istream nil nil)))
          (unless line (return))
          (let* ((i1 (position #\; line))
                 (i2 (position #\; line :start (1+ i1)))
                 (code-string (subseq line 0 i1))
                 (code (parse-integer code-string :radix 16))
                 (name-string (subseq line (1+ i1) i2)))
            ; Ignore characters whose name starts with "<".
            (unless (eql (char name-string 0) #\<)
              ; Also ignore Hangul syllables; they are treated specially.
              (unless (<= #xAC00 code #xD7A3)
                ; Also ignore CJK compatibility ideographs; they are treated
                ; specially as well.
                (unless (or (<= #xF900 code #xFA2D) (<= #xFA30 code #xFA6A)
                            (<= #xFA70 code #xFAD9) (<= #x2F800 code #x2FA1D))
                  ;; Also ignore variationselectors; they are treated
                  ;; specially as well.
                  (unless (or (<= #xFE00 code #xFE0F) (<= #xE0100 code #xE01EF))
                    (push (make-unicode-char :index name-index
                                             :name name-string)
                          all-chars
                    )
                    (setf (gethash code all-chars-hashed) (car all-chars))
                    ;; Update the contiguous range, or start a new range.
                    (if (and range (= (1+ (range-end-code range)) code))
                      (setf (range-end-code range) code)
                      (progn
                        (when range
                          (push range all-ranges))
                        (setq range (make-range :index name-index
                                                :start-code code
                                                :end-code code))))
                    (incf name-index)
            ) ) ) )
    ) ) ) )
    (setq all-chars (nreverse all-chars))
    (if range
      (push range all-ranges))
    (setq all-ranges (nreverse all-ranges))
    (when aliasfile
      ;; Read all characters and names from the alias file.
      (with-open-file (istream aliasfile :direction :input)
        (loop
          (let ((line (read-line istream nil nil)))
            (unless line (return))
            (unless (or (equal line "") (equal (subseq line 0 1) "#"))
              (let* ((i1 (position #\; line))
                     (i2 (position #\; line :start (1+ i1)))
                     (code-string (subseq line 0 i1))
                     (code (parse-integer code-string :radix 16))
                     (name-string (subseq line (1+ i1) i2))
                     (uc (gethash code all-chars-hashed)))
                (when uc
                  (push (make-unicode-char :index (unicode-char-index uc)
                                           :name name-string)
                        all-aliases
    ) ) ) ) ) ) ) )
    (setq all-aliases (nreverse all-aliases)
          all-chars-and-aliases (append all-chars all-aliases)
    )
    ;; Split into words.
    (let ((words-by-length (make-array 0 :adjustable t)))
      (dolist (name (list* "HANGUL SYLLABLE" "CJK COMPATIBILITY" "VARIATION"
                           (mapcar #'unicode-char-name all-chars-and-aliases)))
        (let ((i1 0))
          (loop
            (when (>= i1 (length name)) (return))
            (let ((i2 (or (position #\Space name :start i1) (length name))))
              (let* ((word (subseq name i1 i2))
                     (len (length word)))
                (when (>= len (length words-by-length))
                  (adjust-array words-by-length (1+ len))
                )
                (unless (aref words-by-length len)
                  (setf (aref words-by-length len)
                        (make-word-list
                          :hashed (make-hash-table :test #'equal)
                          :sorted '()
                ) )     )
                (let ((word-list (aref words-by-length len)))
                  (unless (gethash word (word-list-hashed word-list))
                    (setf (gethash word (word-list-hashed word-list)) t)
                    (push word (word-list-sorted word-list))
                ) )
              )
              (setq i1 (1+ i2))
      ) ) ) )
      ;; Sort the word lists.
      (dotimes (len (length words-by-length))
        (unless (aref words-by-length len)
          (setf (aref words-by-length len)
                (make-word-list
                  :hashed (make-hash-table :test #'equal)
                  :sorted '()
        ) )     )
        (let ((word-list (aref words-by-length len)))
          (setf (word-list-sorted word-list)
                (sort (word-list-sorted word-list) #'string<)
          )
          (setf (word-list-size word-list)
                (reduce #'+ (mapcar #'length (word-list-sorted word-list)))
          )
          (setf (word-list-length word-list)
                (length (word-list-sorted word-list))
      ) ) )
      ;; Output the tables.
      (with-open-file (ostream outputfile :direction :output
                       #+UNICODE :external-format #+UNICODE charset:ascii)
        (format ostream "/* DO NOT EDIT! GENERATED AUTOMATICALLY! */~%")
        (format ostream "/*~%")
        (format ostream " * ~A~%" (file-namestring outputfile))
        (format ostream " *~%")
        (format ostream " * Unicode character name table.~%")
        (format ostream " * Generated automatically by the gen-uninames utility.~%")
        (format ostream " */~%")
        (format ostream "/* Copyright (C) 2000-2022 Free Software Foundation, Inc.~%")
        (format ostream "~%")
        (format ostream "   This file is free software.~%")
        (format ostream "   It is dual-licensed under \"the GNU LGPLv3+ or the GNU GPLv2+\".~%")
        (format ostream "   You can redistribute it and/or modify it under either~%")
        (format ostream "     - the terms of the GNU Lesser General Public License as published~%")
        (format ostream "       by the Free Software Foundation, either version 3, or (at your~%")
        (format ostream "       option) any later version, or~%")
        (format ostream "     - the terms of the GNU General Public License as published by the~%")
        (format ostream "       Free Software Foundation; either version 2, or (at your option)~%")
        (format ostream "       any later version, or~%")
        (format ostream "     - the same dual license \"the GNU LGPLv3+ or the GNU GPLv2+\".~%")
        (format ostream "~%")
        (format ostream "   This file is distributed in the hope that it will be useful,~%")
        (format ostream "   but WITHOUT ANY WARRANTY; without even the implied warranty of~%")
        (format ostream "   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU~%")
        (format ostream "   Lesser General Public License and the GNU General Public License~%")
        (format ostream "   for more details.~%")
        (format ostream "~%")
        (format ostream "   You should have received a copy of the GNU Lesser General Public~%")
        (format ostream "   License and of the GNU General Public License along with this~%")
        (format ostream "   program.  If not, see <https://www.gnu.org/licenses/>.  */~%")
        (format ostream "~%")
        (format ostream "static const char unicode_name_words[~D] = {~%"
                        (let ((sum 0))
                          (dotimes (len (length words-by-length))
                            (let ((word-list (aref words-by-length len)))
                              (incf sum (word-list-size word-list))
                          ) )
                          sum
        )               )
        (dotimes (len (length words-by-length))
          (let ((word-list (aref words-by-length len)))
            (dolist (word (word-list-sorted word-list))
              (format ostream " ~{ '~C',~}~%" (coerce word 'list))
        ) ) )
        (format ostream "};~%")
        (format ostream "#define UNICODE_CHARNAME_NUM_WORDS ~D~%"
                        (let ((sum 0))
                          (dotimes (len (length words-by-length))
                            (let ((word-list (aref words-by-length len)))
                              (incf sum (word-list-length word-list))
                          ) )
                          sum
        )               )
        #| ; Redundant data
        (format ostream "static const uint16_t unicode_name_word_offsets[~D] = {~%"
                        (let ((sum 0))
                          (dotimes (len (length words-by-length))
                            (let ((word-list (aref words-by-length len)))
                              (incf sum (word-list-length word-list))
                          ) )
                          sum
        )               )
        (dotimes (len (length words-by-length))
          (let ((word-list (aref words-by-length len)))
            (when (word-list-sorted word-list)
              (format ostream " ")
              (do ((l (word-list-sorted word-list) (cdr l))
                   (offset 0 (+ offset (length (car l)))))
                  ((endp l))
                (format ostream "~<~% ~0,79:; ~D,~>" offset)
              )
              (format ostream "~%")
        ) ) )
        (format ostream "};~%")
        |#
        (format ostream "static const struct { uint32_t extra_offset; uint16_t ind_offset; } unicode_name_by_length[~D] = {~%"
                        (1+ (length words-by-length))
        )
        (let ((extra-offset 0)
              (ind-offset 0))
          (dotimes (len (length words-by-length))
            (let ((word-list (aref words-by-length len)))
              (format ostream "  { ~D, ~D },~%" extra-offset ind-offset)
              (incf extra-offset (word-list-size word-list))
              (incf ind-offset (word-list-length word-list))
          ) )
          (format ostream "  { ~D, ~D }~%" extra-offset ind-offset)
        )
        (format ostream "};~%")
        (let ((ind-offset 0))
          (dotimes (len (length words-by-length))
            (let ((word-list (aref words-by-length len)))
              (dolist (word (word-list-sorted word-list))
                (setf (gethash word (word-list-hashed word-list)) ind-offset)
                (incf ind-offset)
        ) ) ) )
        (dolist (word '("HANGUL" "SYLLABLE" "CJK" "COMPATIBILITY" "VARIATION"))
          (format ostream "#define UNICODE_CHARNAME_WORD_~A ~D~%" word
                          (gethash word (word-list-hashed (aref words-by-length (length word))))
        ) )
        ;; Compute the word-indices for every unicode-char.
        (dolist (uc all-chars-and-aliases)
          (let ((name (unicode-char-name uc))
                (indices '()))
            (let ((i1 0))
              (loop
                (when (>= i1 (length name)) (return))
                (let ((i2 (or (position #\Space name :start i1) (length name))))
                  (let* ((word (subseq name i1 i2))
                         (len (length word)))
                    (push (gethash word (word-list-hashed (aref words-by-length len)))
                          indices
                    )
                  )
                  (setq i1 (1+ i2))
            ) ) )
            (setf (unicode-char-word-indices uc)
                  (coerce (nreverse indices) 'vector)
            )
        ) )
        ;; Sort the list of unicode-chars by word-indices.
        (setq all-chars-and-aliases
              (sort all-chars-and-aliases
                    (lambda (vec1 vec2)
                      (let ((len1 (length vec1))
                            (len2 (length vec2)))
                        (do ((i 0 (1+ i)))
                            (nil)
                          (if (< i len2)
                            (if (< i len1)
                              (cond ((< (aref vec1 i) (aref vec2 i)) (return t))
                                    ((> (aref vec1 i) (aref vec2 i)) (return nil))
                              )
                              (return t)
                            )
                            (return nil)
                    ) ) ) )
                    :key #'unicode-char-word-indices
        )     )
        ;; Output the word-indices.
        (format ostream "static const uint16_t unicode_names[~D] = {~%"
                        (reduce #'+ (mapcar (lambda (uc) (length (unicode-char-word-indices uc))) all-chars-and-aliases))
        )
        (let ((i 0))
          (dolist (uc all-chars-and-aliases)
            (format ostream " ~{ ~D,~}"
                            (maplist (lambda (r) (+ (* 2 (car r)) (if (cdr r) 1 0)))
                                     (coerce (unicode-char-word-indices uc) 'list)
                            )
            )
            (when add-comments
              (format ostream "~40T/* ~A */" (unicode-char-name uc))
            )
            (format ostream "~%")
            (setf (unicode-char-word-indices-index uc) i)
            (incf i (length (unicode-char-word-indices uc)))
        ) )
        (format ostream "};~%")
        (format ostream "static const struct { uint16_t index; uint32_t name:24; } ATTRIBUTE_PACKED unicode_name_to_index[~D] = {~%"
                        (length all-chars-and-aliases)
        )
        (dolist (uc all-chars-and-aliases)
          (format ostream "  { 0x~4,'0X, ~D },"
                          (unicode-char-index uc)
                          (unicode-char-word-indices-index uc)
          )
          (when add-comments
            (format ostream "~21T/* ~A */" (unicode-char-name uc))
          )
          (format ostream "~%")
        )
        (format ostream "};~%")
        (format ostream "static const struct { uint16_t index; uint32_t name:24; } ATTRIBUTE_PACKED unicode_index_to_name[~D] = {~%"
                        (length all-chars)
        )
        (dolist (uc (sort (copy-list all-chars) #'< :key #'unicode-char-index))
          (format ostream "  { 0x~4,'0X, ~D },"
                          (unicode-char-index uc)
                          (unicode-char-word-indices-index uc)
          )
          (when add-comments
            (format ostream "~21T/* ~A */" (unicode-char-name uc))
          )
          (format ostream "~%")
        )
        (format ostream "};~%")
        (format ostream "#define UNICODE_CHARNAME_MAX_LENGTH ~D~%"
                        (reduce #'max (mapcar (lambda (uc) (length (unicode-char-name uc))) all-chars-and-aliases))
        )
        (format ostream "#define UNICODE_CHARNAME_MAX_WORDS ~D~%"
                        (reduce #'max (mapcar (lambda (uc) (length (unicode-char-word-indices uc))) all-chars-and-aliases))
        )
        (format ostream "static const struct { uint16_t index; uint32_t gap; uint16_t length; } unicode_ranges[~D] = {~%"
                        (length all-ranges))
        (dolist (range all-ranges)
          (format ostream "  { ~D, ~D, ~D },~%"
                  (range-index range)
                  (- (range-start-code range) (range-index range))
                  (1+ (- (range-end-code range) (range-start-code range))))
        )
        (format ostream "};~%")
      )
) ) )

(main (first *args*) (second *args*) (third *args*))
