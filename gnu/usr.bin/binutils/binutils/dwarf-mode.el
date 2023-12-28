;;; dwarf-mode.el --- Browser for DWARF information. -*-lexical-binding:t-*-

;; Version: 1.8

;; Copyright (C) 2012-2023 Free Software Foundation, Inc.

;; This file is not part of GNU Emacs, but is distributed under the
;; same terms:

;; GNU Emacs is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation, either version 3 of the License, or
;; (at your option) any later version.

;; GNU Emacs is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with GNU Emacs.  If not, see <http://www.gnu.org/licenses/>.

;;; Code:

(defvar dwarf-objdump-program "objdump")

(defconst dwarf-font-lock-keywords
  '(
    ;; Name and linkage name.
    ("DW_AT_[a-zA-Z_]*name\\s *:\\(?:\\s *(.*):\\)?\\s *\\(.*\\)\\s *$"
     (1 font-lock-function-name-face))

    ("Compilation Unit @ offset 0x[0-9a-f]+"
     (0 font-lock-string-face))
    ))

(defvar dwarf-file nil
  "Buffer-local variable holding the file name passed to objdump.")

(defvar dwarf--process nil
  "Running objdump process, or nil.")

(defvar dwarf--deletion-region nil
  "Region to delete before inserting text in `dwarf--filter'.")

(defun dwarf--check-running ()
  "Throw an exception if an objdump process is already running."
  (when dwarf--process
    (error "An objdump process is still running in this buffer")))

(defun dwarf--filter (proc string)
  "Filter function for objdump processes."
  (when (buffer-live-p (process-buffer proc))
    (with-current-buffer (process-buffer proc)
      (save-excursion
	(let ((inhibit-read-only t))
	  (when dwarf--deletion-region
	    (apply #'delete-region dwarf--deletion-region)
	    (setq dwarf--deletion-region nil))
          (goto-char (process-mark proc))
          (insert string)
          (set-marker (process-mark proc) (point))
	  (set-buffer-modified-p nil))))))

(defun dwarf--sentinel (proc _status)
  (when (buffer-live-p (process-buffer proc))
    (with-current-buffer (process-buffer proc)
      (setq mode-line-process nil)
      (setq dwarf--process nil))))

(defun dwarf--invoke (start end &rest command)
  "Invoke a command and arrange to insert output into the current buffer."
  (setq mode-line-process "[Running]")
  (setq dwarf--deletion-region (list start end))
  (setq dwarf--process (make-process :name "objdump"
				     :buffer (current-buffer)
				     :command command
				     :connection-type 'pipe
				     :noquery t
				     :filter #'dwarf--filter
				     :sentinel #'dwarf--sentinel))
  (set-marker (process-mark dwarf--process) (point)))

;; Expand a "..." to show all the child DIES.  NEW-DEPTH controls how
;; deep to display the new dies; `nil' means display all of them.
(defun dwarf-do-insert-substructure (new-depth die)
  (dwarf--check-running)
  (let ((inhibit-read-only t))
    (beginning-of-line)
    (apply #'dwarf--invoke
	   (point) (save-excursion
		     (end-of-line)
		     (forward-char)
		     (point))
	   dwarf-objdump-program "-Wi" (concat "--dwarf-start=0x" die)
	   (expand-file-name dwarf-file)
	   (if new-depth (list (concat "--dwarf-depth="
				       (int-to-string new-depth)))))
    (set-buffer-modified-p nil)))

(defun dwarf-insert-substructure-button (die)
  (beginning-of-line)
  (unless (looking-at "^ <\\([0-9]+\\)>")
    (error "Unrecognized line."))
  (let ((new-depth (1+ (string-to-number (match-string 1)))))
    (dwarf-do-insert-substructure new-depth die)))

(defun dwarf-insert-substructure (arg)
  "Expand a `...' to show children of the current DIE.
By default, expands just one level of children.
A prefix argument means expand all children."
  (interactive "P")
  (beginning-of-line)
  (unless (looking-at "^ <\\([0-9]+\\)><\\([0-9a-f]+\\)>: \\.\\.\\.")
    (error "Unrecognized line."))
  (let ((die (match-string 2)))
    (if arg
	(dwarf-do-insert-substructure nil die)
      (dwarf-insert-substructure-button die))))

;; Called when a button is pressed.
;; Either follows a DIE reference, or expands a "...".
(defun dwarf-die-button-action (button)
  (let* ((die (button-get button 'die))
	 ;; Note that the first number can only be decimal.  It is
	 ;; included in this search because otherwise following a ref
	 ;; might lead to a zero-length boolean attribute in the
	 ;; previous DIE.
	 (die-rx (concat "^\\s *<[0-9]+><" die ">:"))
	 (old (point))
	 (is-ref (button-get button 'die-ref)))
    (if is-ref
	(progn
	  (goto-char (point-min))
	  (if (re-search-forward die-rx nil 'move)
	      (push-mark old)
	    (goto-char old)
	    (error "Could not find DIE <0x%s>" die)))
      (dwarf-insert-substructure-button die))))

;; Button definition.
(define-button-type 'dwarf-die-button
  'follow-link t
  'action #'dwarf-die-button-action)

;; Helper regexp to match a DIE reference.
(defconst dwarf-die-reference "\\(<0x\\([0-9a-f]+\\)>\\)")

;; Helper regexp to match a `...' indicating that there are hidden
;; children.
(defconst dwarf-die-more "^ <[0-9]+><\\([0-9a-z]+\\)>: \\([.][.][.]\\)")

;; jit-lock callback function to fontify a region.  This applies the
;; buttons, since AFAICT there is no good way to apply buttons via
;; font-lock.
(defun dwarf-fontify-region (start end)
  (save-excursion
    (let ((beg-line (progn (goto-char start) (line-beginning-position)))
	  (end-line (progn (goto-char end) (line-end-position))))
      (goto-char beg-line)
      (while (re-search-forward dwarf-die-reference end-line 'move)
	(let ((b-start (match-beginning 1))
	      (b-end (match-end 1))
	      (hex (match-string-no-properties 2)))
	  (make-text-button b-start b-end :type 'dwarf-die-button
			    'die hex 'die-ref t)))
      ;; This is a bogus approach.  Why can't we make buttons from the
      ;; font-lock defaults?
      (goto-char beg-line)
      (while (re-search-forward dwarf-die-more end-line 'move)
	(let ((hex (match-string-no-properties 1))
	      (b-start (match-beginning 2))
	      (b-end (match-end 2)))
	  (make-text-button b-start b-end :type 'dwarf-die-button
			    'die hex 'die-ref nil))))))

;; Run objdump and insert the contents into the buffer.  The arguments
;; are the way they are because this is also called as a
;; revert-buffer-function.
(defun dwarf-do-refresh (&rest _ignore)
  (dwarf--check-running)
  (let ((inhibit-read-only t))
    (dwarf--invoke (point-min) (point-max)
		   dwarf-objdump-program "-Wi" "--dwarf-depth=1"
		   (expand-file-name dwarf-file))
    (set-buffer-modified-p nil)))

(defun dwarf-refresh-all ()
  "Refresh the current buffer without eliding substructure.
Note that this can result in very voluminous output."
  (interactive)
  (dwarf--check-running)
  (let ((inhibit-read-only t))
    (dwarf--invoke (point-min) (point-max)
		   dwarf-objdump-program "-Wi"
		   (expand-file-name dwarf-file))
    (set-buffer-modified-p nil)))

(defvar dwarf-mode-syntax-table
  (let ((table (make-syntax-table)))
    ;; This at least makes it so mark-sexp on some hex digits inside
    ;; <...> does not also copy the ">".
    (modify-syntax-entry ?< "(>" table)
    (modify-syntax-entry ?> ")<" table)
    table)
  "Syntax table for dwarf-mode buffers.")

(defvar dwarf-mode-map
  (let ((map (make-sparse-keymap)))
    (set-keymap-parent map special-mode-map)
    (define-key map [(control ?m)] #'dwarf-insert-substructure)
    (define-key map "A" #'dwarf-refresh-all)
    map)
  "Keymap for dwarf-mode buffers.")

(define-derived-mode dwarf-mode special-mode "DWARF"
  "Major mode for browsing DWARF output.

\\{dwarf-mode-map}"

  (set (make-local-variable 'font-lock-defaults) '(dwarf-font-lock-keywords))
  (set (make-local-variable 'revert-buffer-function) #'dwarf-do-refresh)
  (jit-lock-register #'dwarf-fontify-region))

;;;###autoload
(defun dwarf-browse (file)
  "Invoke `objdump' and put output into a `dwarf-mode' buffer.
This is the main interface to `dwarf-mode'."
  (interactive "fFile name: ")
  (let* ((base-name (file-name-nondirectory file))
	 (buffer (generate-new-buffer (concat "*DWARF for " base-name "*"))))
    (pop-to-buffer buffer)
    (dwarf-mode)
    (setq default-directory (file-name-directory file))
    (set (make-local-variable 'dwarf-file) file)
    (set (make-local-variable 'dwarf--process) nil)
    (set (make-local-variable 'dwarf--deletion-region) nil)
    (dwarf-do-refresh)))

(provide 'dwarf-mode)

;;; dwarf-mode.el ends here
