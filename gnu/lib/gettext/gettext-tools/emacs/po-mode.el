;;; po-mode.el --- major mode for GNU gettext PO files

;; Copyright (C) 1995-2002, 2005-2008, 2010, 2013-2017, 2019-2020, 2023 Free Software Foundation, Inc.

;; Authors: François Pinard <pinard@iro.umontreal.ca>
;;          Greg McGary <gkm@magilla.cichlid.com>
;; Keywords: i18n gettext
;; Created: 1995

;; This file is part of GNU gettext.

;; This program is free software: you can redistribute it and/or modify
;; it under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3 of the License, or
;; (at your option) any later version.

;; This program is distributed in the hope that it will be useful,
;; but WITHOUT ANY WARRANTY; without even the implied warranty of
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;; GNU General Public License for more details.

;; You should have received a copy of the GNU General Public License
;; along with this program.  If not, see <https://www.gnu.org/licenses/>.

;;; Commentary:

;; This package provides the tools meant to help editing PO files,
;; as documented in the GNU gettext user's manual.  See this manual
;; for user documentation, which is not repeated here.

;; To install, merely put this file somewhere GNU Emacs will find it,
;; then add the following lines to your .emacs file:
;;
;;   (autoload 'po-mode "po-mode"
;;             "Major mode for translators to edit PO files" t)
;;   (setq auto-mode-alist (cons '("\\.po\\'\\|\\.po\\." . po-mode)
;;                               auto-mode-alist))
;;
;; To use the right coding system automatically under Emacs 20 or newer,
;; also add:
;;
;;   (autoload 'po-find-file-coding-system "po-compat")
;;   (modify-coding-system-alist 'file "\\.po\\'\\|\\.po\\."
;;                               'po-find-file-coding-system)
;;
;; You may also adjust some variables, below, by defining them in your
;; '.emacs' file, either directly or through command 'M-x customize'.

;; TODO:
;; Plural form editing:
;;  - When in edit mode, currently it highlights (in green) the msgid;
;;    it should also highlight the msgid_plural string, I would say, since
;;    the translator has to look at both.
;;  - After the translator finished the translation of msgstr[0], it would
;;    be nice if the cursor would automatically move to the beginning of the
;;    msgstr[1] line, so that the translator just needs to press RET to edit
;;    that.
;;  - If msgstr[1] is empty but msgstr[0] is not, it would be ergonomic if the
;;    contents of msgstr[0] would be copied. (Not sure if this should happen
;;    at the end of the editing msgstr[0] or at the beginning of the editing
;;    of msgstr[1].) Reason: These two strings are usually very similar.

;;; Code:

(defconst po-mode-version-string "2.28" "\
Version number of this version of po-mode.el.")

;;; Emacs portability matters - part I.
;;; Here is the minimum for customization to work.  See part II.

;; Experiment with Emacs LISP message internationalisation.
(eval-and-compile
  (or (fboundp 'set-translation-domain)
      (defsubst set-translation-domain (string) nil))
  (or (fboundp 'translate-string)
      (defsubst translate-string (string) string)))
(defsubst _ (string) (translate-string string))
(defsubst N_ (string) string)

;; Handle missing 'customs' package.
(eval-and-compile
  (condition-case ()
      (require 'custom)
    (error nil))
  (if (and (featurep 'custom) (fboundp 'custom-declare-variable))
      nil
    (defmacro defgroup (&rest args)
      nil)
    (defmacro defcustom (var value doc &rest args)
      `(defvar ,var ,value ,doc))))

;;; Customisation.

(defgroup po nil
  "Major mode for editing PO files"
  :group 'i18n)

(defcustom po-auto-edit-with-msgid nil
  "*Automatically use msgid when editing untranslated entries."
  :type 'boolean
  :group 'po)

(defcustom po-auto-fuzzy-on-edit nil
  "*Automatically mark entries fuzzy when being edited."
  :type 'boolean
  :group 'po)

(defcustom po-auto-delete-previous-msgid t
  "*Automatically delete previous msgid (marked #|) when editing entry.
Value is nil, t, or ask."
  :type '(choice (const nil)
                 (const t)
                 (const ask))
  :group 'po)

(defcustom po-auto-select-on-unfuzzy nil
  "*Automatically select some new entry while making an entry not fuzzy."
  :type 'boolean
  :group 'po)

(defcustom po-keep-mo-file nil
  "*Set whether MO file should be kept or discarded after validation."
  :type 'boolean
  :group 'po)

(defcustom po-auto-update-file-header t
  "*Automatically revise headers.  Value is nil, t, or ask."
  :type '(choice (const nil)
                 (const t)
                 (const ask))
  :group 'po)

(defcustom po-auto-replace-revision-date t
  "*Automatically revise date in headers.  Value is nil, t, or ask."
  :type '(choice (const nil)
                 (const t)
                 (const ask))
  :group 'po)

(defcustom po-default-file-header "\
# SOME DESCRIPTIVE TITLE.
# Copyright (C) YEAR Free Software Foundation, Inc.
# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.
#
#, fuzzy
msgid \"\"
msgstr \"\"
\"Project-Id-Version: PACKAGE VERSION\\n\"
\"PO-Revision-Date: YEAR-MO-DA HO:MI +ZONE\\n\"
\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"
\"Language-Team: LANGUAGE <LL@li.org>\\n\"
\"MIME-Version: 1.0\\n\"
\"Content-Type: text/plain; charset=CHARSET\\n\"
\"Content-Transfer-Encoding: 8bit\\n\"
"
  "*Default PO file header."
  :type 'string
  :group 'po)

(defcustom po-translation-project-address
  "robot@translationproject.org"
  "*Electronic mail address of the Translation Project.
Typing \\[po-send-mail] (normally bound to `M') the user will send the PO file
to this email address."
  :type 'string
  :group 'po)

(defcustom po-translation-project-mail-label "TP-Robot"
  "*Subject label when sending the PO file to `po-translation-project-address'."
  :type 'string
  :group 'po)

(defcustom po-highlighting t
  "*Highlight text whenever appropriate, when non-nil.
However, on older Emacses, a yet unexplained highlighting bug causes files
to get mangled."
  :type 'boolean
  :group 'po)

(defcustom po-highlight-face 'highlight
  "*The face used for PO mode highlighting.  For Emacses with overlays.
Possible values are 'highlight', 'modeline', 'secondary-selection',
'region', and 'underline'.
This variable can be set by the user to whatever face they desire.
It's most convenient if the cursor color and highlight color are
slightly different."
  :type 'face
  :group 'po)

(defcustom po-team-name-to-code
  ;; All possible languages, a complete ISO 639 list, the inverse of
  ;; gettext-tools/src/lang-table.c, and a little more.
  '(("LANGUAGE" . "LL")
    ("(Afan) Oromo" . "om")
    ("Abkhazian" . "ab")
    ("Achinese" . "ace")
    ("Afar" . "aa")
    ("Afrikaans" . "af")
    ("Akan" . "ak")
    ("Albanian" . "sq")
    ("Amharic" . "am")
    ("Arabic" . "ar")
    ("Aragonese" . "an")
    ("Argentinian" . "es_AR")
    ("Armenian" . "hy")
    ("Assamese" . "as")
    ("Austrian" . "de_AT")
    ("Avaric" . "av")
    ("Avestan" . "ae")
    ("Awadhi" . "awa")
    ("Aymara" . "ay")
    ("Azerbaijani" . "az")
    ("Balinese" . "ban")
    ("Baluchi" . "bal")
    ("Bambara" . "bm")
    ("Bashkir" . "ba")
    ("Basque" . "eu")
    ("Beja" . "bej")
    ("Belarusian" . "be")
    ("Bemba" . "bem")
    ("Bengali" . "bn")
    ("Bhojpuri" . "bho")
    ("Bihari" . "bh")
    ("Bikol" . "bik")
    ("Bini" . "bin")
    ("Bislama" . "bi")
    ("Bosnian" . "bs")
    ("Brazilian Portuguese" . "pt_BR")
    ("Breton" . "br")
    ("Buginese" . "bug")
    ("Bulgarian" . "bg")
    ("Burmese" . "my")
    ("Catalan" . "ca")
    ("Cebuano" . "ceb")
    ("Central Khmer" . "km")
    ("Chamorro" . "ch")
    ("Chechen" . "ce")
    ("Chinese" . "zh")
    ("Chinese (Hong Kong)" . "zh_HK")
    ("Chinese (simplified)" . "zh_CN")
    ("Chinese (traditional)" . "zh_TW")
    ("Church Slavic" . "cu")
    ("Chuvash" . "cv")
    ("Cornish" . "kw")
    ("Corsican" . "co")
    ("Cree" . "cr")
    ("Croatian" . "hr")
    ("Czech" . "cs")
    ("Danish" . "da")
    ("Dinka" . "din")
    ("Divehi" . "dv")
    ("Dogri" . "doi")
    ("Dutch" . "nl")
    ("Dzongkha" . "dz")
    ("English" . "en")
    ("English (British)" . "en_GB")
    ("Esperanto" . "eo")
    ("Estonian" . "et")
    ("Ewe" . "ee")
    ("Faroese" . "fo")
    ("Fijian" . "fj")
    ("Filipino" . "fil")
    ("Finnish" . "fi")
    ("Fon" . "fon")
    ("French" . "fr")
    ("Frisian" . "fy")
    ("Fulah" . "ff")
    ("Galician" . "gl")
    ("Ganda" . "lg")
    ("Georgian" . "ka")
    ("German" . "de")
    ("Gondi" . "gon")
    ("Greek" . "el")
    ("Guarani" . "gn")
    ("Gujarati" . "gu")
    ("Haitian" . "ht")
    ("Hausa" . "ha")
    ("Hebrew" . "he")
    ("Herero" . "hz")
    ("Hiligaynon" . "hil")
    ("Hindi" . "hi")
    ("Hiri Motu" . "ho")
    ("Hmong" . "hmn")
    ("Hungarian" . "hu")
    ("Hyam" . "jab")
    ("Icelandic" . "is")
    ("Ido" . "io")
    ("Igbo" . "ig")
    ("Iloko" . "ilo")
    ("Indonesian" . "id")
    ("Interlingua" . "ia")
    ("Interlingue" . "ie")
    ("Inuktitut" . "iu")
    ("Inupiak" . "ik")
    ("Irish" . "ga")
    ("Italian" . "it")
    ("Japanese" . "ja")
    ("Javanese" . "jv")
    ("Jju" . "kaj")
    ("Kabardian" . "kbd")
    ("Kabyle" . "kab")
    ("Kagoma" . "kdm")
    ("Kalaallisut" . "kl")
    ("Kamba" . "kam")
    ("Kannada" . "kn")
    ("Kanuri" . "kr")
    ("Kashmiri" . "ks")
    ("Kashubian" . "csb")
    ("Kazakh" . "kk")
    ("Khmer" . "km") ; old name
    ("Kikuyu" . "ki")
    ("Kimbundu" . "kmb")
    ("Kinyarwanda" . "rw")
    ("Kirghiz" . "ky")
    ("Kirundi" . "rn")
    ("Komi" . "kv")
    ("Kongo" . "kg")
    ("Konkani" . "kok")
    ("Korean" . "ko")
    ("Kuanyama" . "kj")
    ("Kurdish" . "ku")
    ("Kurukh" . "kru")
    ("Laotian" . "lo")
    ("Latin" . "la")
    ("Latvian" . "lv")
    ("Letzeburgesch" . "lb")
    ("Limburgish" . "li")
    ("Lingala" . "ln")
    ("Lithuanian" . "lt")
    ("Low Saxon" . "nds")
    ("Luba-Katanga" . "lu")
    ("Luba-Lulua" . "lua")
    ("Luo" . "luo")
    ("Macedonian" . "mk")
    ("Madurese" . "mad")
    ("Magahi" . "mag")
    ("Maithili" . "mai")
    ("Makasar" . "mak")
    ("Malagasy" . "mg")
    ("Malay" . "ms")
    ("Malayalam" . "ml")
    ("Maltese" . "mt")
    ("Mandingo" . "man")
    ("Manipuri" . "mni")
    ("Manx" . "gv")
    ("Maori" . "mi")
    ("Marathi" . "mr")
    ("Marshall" . "mh")
    ("Marshallese" . "mh")
    ("Marwari" . "mwr")
    ("Mayan" . "myn")
    ("Mende" . "men")
    ("Minangkabau" . "min")
    ("Moldavian" . "mo")
    ("Mongolian" . "mn")
    ("Mossi" . "mos")
    ("Nahuatl" . "nah")
    ("Nauru" . "na")
    ("Navajo" . "nv")
    ("Ndonga" . "ng")
    ("Neapolitan" . "nap")
    ("Nepali" . "ne")
    ("North Ndebele" . "nd")
    ("Northern Sami" . "se")
    ("Northern Sotho" . "nso")
    ("Norwegian Bokmal" . "nb")
    ("Norwegian Nynorsk" . "nn")
    ("Norwegian" . "no")
    ("Nyamwezi" . "nym")
    ("Nyanja" . "ny")
    ("Nyankole" . "nyn")
    ("Occitan" . "oc")
    ("Ojibwa" . "oj")
    ("Old English" . "ang")
    ("Oriya" . "or")
    ("Ossetian" . "os")
    ("Páez" . "pbb")
    ("Pali" . "pi")
    ("Pampanga" . "pam")
    ("Pangasinan" . "pag")
    ("Pashto" . "ps")
    ("Persian" . "fa")
    ("Polish" . "pl")
    ("Portuguese" . "pt")
    ("Punjabi" . "pa")
    ("Quechua" . "qu")
    ("Rajasthani" . "raj")
    ("Rhaeto-Roman" . "rm") ; old name
    ("Romanian" . "ro")
    ("Romansh" . "rm")
    ("Russian" . "ru")
    ("Samoan" . "sm")
    ("Sango" . "sg")
    ("Sanskrit" . "sa")
    ("Santali" . "sat")
    ("Sardinian" . "sc")
    ("Sasak" . "sas")
    ("Scots" . "gd") ; old name
    ("Scottish Gaelic" . "gd")
    ("Serbian" . "sr")
    ("Serer" . "srr")
    ("Sesotho" . "st")
    ("Setswana" . "tn")
    ("Shan" . "shn")
    ("Shona" . "sn")
    ("Sichuan Yi" . "ii")
    ("Sicilian" . "scn")
    ("Sidamo" . "sid")
    ("Sindhi" . "sd")
    ("Sinhala" . "si")
    ("Sinhalese" . "si")
    ("Siswati" . "ss")
    ("Slovak" . "sk")
    ("Slovenian" . "sl")
    ("Somali" . "so")
    ("Sorbian" . "wen")
    ("South Ndebele" . "nr")
    ("Spanish" . "es")
    ("Spanish (Canary Islands)" . "es_IC")
    ("Sukuma" . "suk")
    ("Sundanese" . "su")
    ("Susu" . "sus")
    ("Swahili" . "sw")
    ("Swedish" . "sv")
    ("Swiss German" . "gsw")
    ("Tagalog" . "tl")
    ("Tahitian" . "ty")
    ("Tajik" . "tg")
    ("Tamil" . "ta")
    ("Tatar" . "tt")
    ("Telugu" . "te")
    ("Tetum" . "tet")
    ("Thai" . "th")
    ("Tibetan" . "bo")
    ("Tigrinya" . "ti")
    ("Timne" . "tem")
    ("Tiv" . "tiv")
    ("Tonga" . "to")
    ("Tsonga" . "ts")
    ("Tumbuka" . "tum")
    ("Turkish" . "tr")
    ("Turkmen" . "tk")
    ("Twi" . "tw")
    ("Tyap" . "kcg")
    ("Uighur" . "ug")
    ("Ukrainian" . "uk")
    ("Umbundu" . "umb")
    ("Urdu" . "ur")
    ("Uzbek" . "uz")
    ("Venda" . "ve")
    ("Vietnamese" . "vi")
    ("Volapuk" . "vo")
    ("Walloon" . "wa")
    ("Walamo" . "wal")
    ("Waray" . "war")
    ("Welsh" . "cy")
    ("Western Frisian" . "fy")
    ("Wolof" . "wo")
    ("Xhosa" . "xh")
    ("Yao" . "yao")
    ("Yiddish" . "yi")
    ("Yoruba" . "yo")
    ("Zapotec" . "zap")
    ("Zhuang" . "za")
    ("Zulu" . "zu")
    )
  "*Association list giving team codes from team names.
This is used for generating a submission file name for the 'M' command.
If a string instead of an alist, it is a team code to use unconditionnally."
  :type 'sexp
  :group 'po)

(defcustom po-gzip-uuencode-command "gzip -9 | uuencode -m"
  "*The filter to use for preparing a mail invoice of the PO file.
Normally \"gzip -9 | uuencode -m\", remove the -9 for lesser compression,
or remove the -m if you are not using the GNU version of 'uuencode'."
  :type 'string
  :group 'po)

(defvar po-subedit-mode-syntax-table
  (copy-syntax-table text-mode-syntax-table)
  "Syntax table used while in PO mode.")

;;; Emacs portability matters - part II.

;;; Many portability matters are addressed in this page.  The few remaining
;;; cases, elsewhere, all involve  'eval-and-compile', 'boundp' or 'fboundp'.

;; Protect string comparisons from text properties if possible.
(eval-and-compile
  (fset 'po-buffer-substring
        (symbol-function (if (fboundp 'buffer-substring-no-properties)
                             'buffer-substring-no-properties
                           'buffer-substring)))

  (if (fboundp 'match-string-no-properties)
      (fset 'po-match-string (symbol-function 'match-string-no-properties))
    (defun po-match-string (number)
      "Return string of text matched by last search."
      (po-buffer-substring (match-beginning number) (match-end number)))))

;; Handle missing 'with-temp-buffer' function.
(eval-and-compile
  (if (fboundp 'with-temp-buffer)
      (fset 'po-with-temp-buffer (symbol-function 'with-temp-buffer))

    (defmacro po-with-temp-buffer (&rest forms)
      "Create a temporary buffer, and evaluate FORMS there like 'progn'."
      (let ((curr-buffer (make-symbol "curr-buffer"))
            (temp-buffer (make-symbol "temp-buffer")))
        `(let ((,curr-buffer (current-buffer))
               (,temp-buffer (get-buffer-create
                              (generate-new-buffer-name " *po-temp*"))))
           (unwind-protect
               (progn
                 (set-buffer ,temp-buffer)
                 ,@forms)
             (set-buffer ,curr-buffer)
             (and (buffer-name ,temp-buffer)
                  (kill-buffer ,temp-buffer))))))))

;; Handle missing 'kill-new' function.
(eval-and-compile
  (if (fboundp 'kill-new)
      (fset 'po-kill-new (symbol-function 'kill-new))

    (defun po-kill-new (string)
      "Push STRING onto the kill ring, for Emacs 18 where kill-new is missing."
      (po-with-temp-buffer
        (insert string)
        (kill-region (point-min) (point-max))))))

;; Handle missing 'read-event' function.
(eval-and-compile
  (fset 'po-read-event
        (cond ((fboundp 'read-event)
               ;; GNU Emacs.
               'read-event)
              (t
               ;; Older Emacses.
               'read-char))))

;; Handle missing 'force-mode-line-update' function.
(eval-and-compile
  (if (fboundp 'force-mode-line-update)
      (fset 'po-force-mode-line-update
            (symbol-function 'force-mode-line-update))

    (defun po-force-mode-line-update ()
      "Force the mode-line of the current buffer to be redisplayed."
      (set-buffer-modified-p (buffer-modified-p)))))

;; Handle portable highlighting.  Code has been adapted (OK... stolen! :-)
;; from 'ispell.el'.

(defun po-create-overlay ()
  "Create and return a deleted overlay structure.
The variable 'po-highlight-face' selects the face to use for highlighting."
  (let ((overlay (make-overlay (point) (point))))
    (overlay-put overlay 'face po-highlight-face)
    ;; The fun thing is that a deleted overlay retains its face, and is
    ;; movable.
    (delete-overlay overlay)
    overlay))

(defun po-highlight (overlay start end &optional buffer)
  "Use OVERLAY to highlight the string from START to END.
If limits are not relative to the current buffer, use optional BUFFER."
  (move-overlay overlay start end (or buffer (current-buffer))))

(defun po-dehighlight (overlay)
  "Display normally the last string which OVERLAY highlighted.
The current buffer should be in PO mode, when this function is called."
  (delete-overlay overlay))

;;; Buffer local variables.

;; The following block of declarations has the main purpose of avoiding
;; byte compiler warnings.  It also introduces some documentation for
;; each of these variables, all meant to be local to PO mode buffers.

;; Flag telling that MODE-LINE-STRING should be displayed.  See 'Window'
;; page below.  Exceptionally, this variable is local to *all* buffers.
(defvar po-mode-flag)

;; PO buffers are kept read-only to prevent random modifications.  READ-ONLY
;; holds the value of the read-only flag before PO mode was entered.
(defvar po-read-only)

;; The current entry extends from START-OF-ENTRY to END-OF-ENTRY, it
;; includes preceding whitespace and excludes following whitespace.  The
;; start of keyword lines are START-OF-MSGID and START-OF-MSGSTR.
;; ENTRY-TYPE classifies the entry.
(defvar po-start-of-entry)
(defvar po-start-of-msgctxt) ; = po-start-of-msgid if there is no msgctxt
(defvar po-start-of-msgid)
(defvar po-start-of-msgid_plural) ; = nil if there is no msgid_plural
(defvar po-start-of-msgstr-block)
(defvar po-start-of-msgstr-form)
(defvar po-end-of-msgstr-form)
(defvar po-end-of-entry)
(defvar po-entry-type)

;; A few counters are usefully shown in the Emacs mode line.
(defvar po-translated-counter)
(defvar po-fuzzy-counter)
(defvar po-untranslated-counter)
(defvar po-obsolete-counter)
(defvar po-mode-line-string)

;; PO mode keeps track of fields being edited, for one given field should
;; have one editing buffer at most, and for exiting a PO buffer properly
;; should offer to close all pending edits.  Variable EDITED-FIELDS holds an
;; an list of "slots" of the form: (ENTRY-MARKER EDIT-BUFFER OVERLAY-INFO).
;; To allow simultaneous edition of the comment and the msgstr of an entry,
;; ENTRY-MARKER points to the msgid line if a comment is being edited, or to
;; the msgstr line if the msgstr is being edited.  EDIT-BUFFER is the
;; temporary Emacs buffer used to edit the string.  OVERLAY-INFO, when not
;; nil, holds an overlay (or if overlays are not supported, a cons of two
;; markers) for this msgid string which became highlighted for the edit.
(defvar po-edited-fields)

;; We maintain a set of movable pointers for returning to entries.
(defvar po-marker-stack)

;; SEARCH path contains a list of directories where files may be found,
;; in a format suitable for read completion.  Each directory includes
;; its trailing slash.  PO mode starts with "./" and "../".
(defvar po-search-path)

;; The following variables are meaningful only when REFERENCE-CHECK
;; is identical to START-OF-ENTRY, else they should be recomputed.
;; REFERENCE-ALIST contains all known references for the current
;; entry, each list element is (PROMPT FILE LINE), where PROMPT may
;; be used for completing read, FILE is a string and LINE is a number.
;; REFERENCE-CURSOR is a cycling cursor into REFERENCE-ALIST.
(defvar po-reference-alist)
(defvar po-reference-cursor)
(defvar po-reference-check)

;; The following variables are for marking translatable strings in program
;; sources.  KEYWORDS is the list of keywords for marking translatable
;; strings, kept in a format suitable for reading with completion.
;; STRING-CONTENTS holds the value of the most recent string found in sources,
;; and when it is not nil, then STRING-BUFFER, STRING-START and STRING-END
;; describe where it is.  MARKING-OVERLAY, if not 'nil', holds the overlay
;; which highlight the last found string; for older Emacses, it holds the cons
;; of two markers around the highlighted region.
(defvar po-keywords)
(defvar po-string-contents)
(defvar po-string-buffer)
(defvar po-string-start)
(defvar po-string-end)
(defvar po-marking-overlay)

;;; PO mode variables and constants (usually not to customize).

;; The textdomain should really be "gettext", only trying it for now.
;; All this requires more thinking, we cannot just do this like that.
(set-translation-domain "po-mode")

(defun po-mode-version ()
  "Show Emacs PO mode version."
  (interactive)
  (message (_"Emacs PO mode, version %s") po-mode-version-string))

(defconst po-help-display-string
  (_"\
PO Mode Summary           Next Previous            Miscellaneous
*: Later, /: Docum        n    p    Any type       .     Redisplay
                          t    T    Translated     /v    Version info
Moving around             f    F    Fuzzy          ?, h  This help
<    First if any         o    O    Obsolete       =     Current index
>    Last if any          u    U    Untranslated   0     Other window
/SPC Auto select                                   V     Validate
                        Msgstr Comments            M     Mail officially
Modifying entries         RET  #    Call editor    _     Undo
TAB   Remove fuzzy mark   k    K    Kill to        E     Edit out full
DEL   Fuzzy or fade out   w    W    Copy to        Q     Forceful quit
LFD   Init with msgid     y    Y    Yank from      q     Confirm and quit

gettext Keyword Marking                            Position Stack
,    Find next string     Compendiums              m  Mark and push current
M-,  Mark translatable    *c    To compendium      r  Pop and return
M-.  Change mark, mark    *M-C  Select, save       x  Exchange current/top

Program Sources           Auxiliary Files          Lexicography
s    Cycle reference      a    Cycle file          *l    Lookup translation
M-s  Select reference     C-c C-a  Select file     *M-l  Add/edit translation
S    Consider path        A    Consider PO file    *L    Consider lexicon
M-S  Ignore path          M-A  Ignore PO file      *M-L  Ignore lexicon
")
  "Help page for PO mode.")

(defconst po-mode-menu-layout
  `("PO"
    ("Moving around"
     ["Auto select" po-auto-select-entry
      :help "Jump to next interesting entry"]
     "---"
     ;; Forward
     ["Any next" po-next-entry
      :help "Jump to next entry"]
     ["Next translated" po-next-translated-entry
      :help "Jump to next translated entry"]
     ["Next fuzzy" po-next-fuzzy-entry
      :help "Jump to next fuzzy entry"]
     ["Next obsolete" po-next-obsolete-entry
      :help "Jump to next obsolete entry"]
     ["Next untranslated" po-next-untranslated-entry
      :help "Jump to next untranslated entry"]
     ["Last file entry" po-last-entry
      :help "Jump to last entry"]
     "---"
     ;; Backward
     ["Any previous" po-previous-entry
      :help "Jump to previous entry"]
     ["Previous translated" po-previous-translated-entry
      :help "Jump to previous translated entry"]
     ["Previous fuzzy" po-previous-fuzzy-entry
      :help "Jump to previous fuzzy entry"]
     ["Previous obsolete" po-previous-obsolete-entry
      :help "Jump to previous obsolete entry"]
     ["Previous untranslated" po-previous-untranslated-entry
      :help "Jump to previous untranslated entry"]
     ["First file entry" po-first-entry
      :help "Jump to first entry"]
     "---"
     ;; "Position stack"
     ["Mark and push current" po-push-location
      :help "Remember current location"]
     ["Pop and return" po-pop-location
      :help "Jump to last remembered location and forget about it"]
     ["Exchange current/top" po-exchange-location
      :help "Jump to last remembered location and remember current location"]
     "---"
     ["Redisplay" po-current-entry
      :help "Make current entry properly visible"]
     ["Current index" po-statistics
      :help "Statistical info on current translation file"])
    ("Modifying entries"
     ["Undo" po-undo
      :help "Revoke last changed entry"]
     "---"
     ;; "Msgstr"
     ["Edit msgstr" po-edit-msgstr
      :help "Edit current translation"]
     ["Ediff and merge msgstr" po-edit-msgstr-and-ediff
      :help "Call `ediff' on current translation for merging"]
     ["Cut msgstr" po-kill-msgstr
      :help "Cut (kill) current translation"]
     ["Copy msgstr" po-kill-ring-save-msgstr
      :help "Copy current translation"]
     ["Paste msgstr" po-yank-msgstr
      :help "Paste (yank) text most recently cut/copied translation"]
     "---"
     ;; "Comments"
     ["Edit comment" po-edit-comment
      :help "Edit current comment"]
     ["Ediff and merge comment" po-edit-comment-and-ediff
      :help "Call `ediff' on current comment for merging"]
     ["Cut comment" po-kill-comment
      :help "Cut (kill) current comment"]
     ["Copy comment" po-kill-ring-save-comment
      :help "Copy current translation"]
     ["Paste comment" po-yank-comment
      :help "Paste (yank) text most recently cut/copied"]
     "---"
     ["Remove fuzzy mark" po-unfuzzy
      :help "Remove \"#, fuzzy\""]
     ["Fuzzy or fade out" po-fade-out-entry
      :help "Set current entry fuzzy, or if already fuzzy delete it"]
     ["Init with msgid" po-msgid-to-msgstr
      :help "Initialize or replace current translation with the original message"])
    ("Other files"
     ["Other window" po-other-window
      :help "Select other window; if necessay split current frame"]
     "---"
     ;; "Program sources"
     ["Cycle reference in source file" po-cycle-source-reference t]
     ["Select reference" po-select-source-reference t]
     ["Consider path" po-consider-source-path t]
     ["Ignore path" po-ignore-source-path t]
     ;; "---"
     ;; ;; "Compendiums"
     ;; ["To add entry to compendium" po-save-entry nil]
     ;; ["Select from compendium, save" po-select-and-save-entry nil]
     "---"
     ;; "Auxiliary files"
     ["Cycle through auxilicary file" po-cycle-auxiliary t]
     ["Select auxilicary file" po-select-auxiliary t]
     ["Consider as auxilicary file" po-consider-as-auxiliary t]
     ["Ignore as auxilicary file" po-ignore-as-auxiliary t]
     ;; "---"
     ;; ;; "Lexicography"
     ;; ["Lookup translation" po-lookup-lexicons nil]
     ;; ["Add/edit translation" po-edit-lexicon-entry nil]
     ;; ["Consider lexicon" po-consider-lexicon-file nil]
     ;; ["Ignore lexicon" po-ignore-lexicon-file nil])
     "---"
     "Source marking"
     ["Find first string" (po-tags-search '(nil)) t]
     ["Prefer keyword" (po-select-mark-and-mark '(nil)) t]
     ["Find next string" po-tags-search t]
     ["Mark preferred" po-mark-translatable t]
     ["Mark with keyword" po-select-mark-and-mark t])
     "---"
     ["Version info" po-mode-version
      :help "Display version number of PO mode"]
     ["Help page" po-help
      :help "Show the PO mode help screen"]
     ["Validate" po-validate
      :help "Check validity of current translation file using `msgfmt'"]
     ["Mail officially" po-send-mail
      :help "Send current translation file to the Translation Robot by mail"]
     ["Edit out full" po-edit-out-full
      :help "Leave PO mode to edit translation file using fundamental mode"]
     "---"
     ["Forceful quit" po-quit
      :help "Close (kill) current translation file without saving"]
     ["Soft quit" po-confirm-and-quit
      :help "Save current translation file, than close (kill) it"]))


(defconst po-subedit-mode-menu-layout
  `("PO-Edit"
    ["Ediff and merge translation variants" po-subedit-ediff
      :help "Call `ediff' for merging variants"]
    ["Cycle through auxiliary files" po-subedit-cycle-auxiliary t]
    "---"
    ["Abort edit" po-subedit-abort
     :help "Don't change the translation"]
    ["Exit edit" po-subedit-exit
     :help "Use this text as the translation and close current edit buffer"]))

(defconst po-subedit-message
  (_"Type 'C-c C-c' once done, or 'C-c C-k' to abort edit")
  "Message to post in the minibuffer when an edit buffer is displayed.")

(defvar po-auxiliary-list nil
  "List of auxiliary PO files, in completing read format.")

(defvar po-auxiliary-cursor nil
  "Cursor into the 'po-auxiliary-list'.")

(defvar po-compose-mail-function
  (let ((functions '(compose-mail-other-window
                     message-mail-other-window
                     compose-mail
                     message-mail))
        result)
    (while (and (not result) functions)
      (if (fboundp (car functions))
          (setq result (car functions))
        (setq functions (cdr functions))))
    (cond (result)
          ((fboundp 'mail-other-window)
           (function (lambda (to subject)
                       (mail-other-window nil to subject))))
          ((fboundp 'mail)
           (function (lambda (to subject)
                       (mail nil to subject))))
          (t (function (lambda (to subject)
                         (error (_"I do not know how to mail to '%s'") to))))))
  "Function to start composing an electronic message.")

(defvar po-any-previous-msgctxt-regexp
  "^#\\(~\\)?|[ \t]*msgctxt.*\n\\(#\\(~\\)?|[ \t]*\".*\n\\)*"
  "Regexp matching a whole #| msgctxt field, whether obsolete or not.")

(defvar po-any-previous-msgid-regexp
  "^#\\(~\\)?|[ \t]*msgid.*\n\\(#\\(~\\)?|[ \t]*\".*\n\\)*"
  "Regexp matching a whole #| msgid field, whether obsolete or not.")

(defvar po-any-previous-msgid_plural-regexp
  "^#\\(~\\)?|[ \t]*msgid_plural.*\n\\(#\\(~\\)?|[ \t]*\".*\n\\)*"
  "Regexp matching a whole #| msgid_plural field, whether obsolete or not.")

(defvar po-any-msgctxt-msgid-regexp
  "^\\(#~[ \t]*\\)?msg\\(ctxt\\|id\\).*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*"
  "Regexp matching a whole msgctxt or msgid field, whether obsolete or not.")

(defvar po-any-msgid-regexp
  "^\\(#~[ \t]*\\)?msgid.*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*"
  "Regexp matching a whole msgid field, whether obsolete or not.")

(defvar po-any-msgid_plural-regexp
  "^\\(#~[ \t]*\\)?msgid_plural.*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*"
  "Regexp matching a whole msgid_plural field, whether obsolete or not.")

(defvar po-any-msgstr-block-regexp
  "^\\(#~[ \t]*\\)?msgstr\\([ \t]\\|\\[0\\]\\).*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*\\(\\(#~[ \t]*\\)?msgstr\\[[0-9]\\].*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*\\)*"
  "Regexp matching a whole msgstr or msgstr[] field, whether obsolete or not.")

(defvar po-any-msgstr-form-regexp
  ;; "^\\(#~[ \t]*\\)?msgstr.*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*"
  "^\\(#~[ \t]*\\)?msgstr\\(\\[[0-9]\\]\\)?.*\n\\(\\(#~[ \t]*\\)?\".*\n\\)*"
  "Regexp matching just one msgstr or msgstr[] field, whether obsolete or not.")

(defvar po-msgstr-idx-keyword-regexp
  "^\\(#~[ \t]*\\)?msgstr\\[[0-9]\\]"
  "Regexp matching an indexed msgstr keyword, whether obsolete or not.")

(defvar po-msgfmt-program "msgfmt"
  "Path to msgfmt program from GNU gettext package.")

;; Font lock based highlighting code.
(defconst po-font-lock-keywords
  '(
    ("^# .*\\|^#[:,]?" . font-lock-comment-face)
    ("^#:\\(.*\\)" 1 font-lock-constant-face)
    ("^#,\\(.*\\)" 1 font-lock-function-name-face)
    ("^\\(\\(msg\\(ctxt\\|id\\(_plural\\)?\\|str\\(\\[[0-9]\\]\\)?\\)\\) \\)?\"\\|\"$"
     . font-lock-keyword-face)
    ("\\\\.\\|%[*$-.0-9hjltuzL]*[a-zA-Z]" . font-lock-variable-name-face)
    )
  "Additional expressions to highlight in PO mode.")

;; Old activator for 'font lock'.  Is it still useful?  I don't think so.
;;(if (boundp 'font-lock-keywords)
;;    (put 'po-mode 'font-lock-keywords 'po-font-lock-keywords))

;; 'hilit19' based highlighting code has been disabled, as most probably
;; nobody really needs it (it also generates ugly byte-compiler warnings).
;;
;;(if (fboundp 'hilit-set-mode-patterns)
;;    (hilit-set-mode-patterns 'po-mode
;;                             '(("^# .*\\|^#$" nil comment)
;;                               ("^#[.,:].*" nil include)
;;                               ("^\\(msgid\\|msgstr\\) *\"" nil keyword)
;;                               ("^\"\\|\"$" nil keyword))))

;;; Mode activation.

;; Emacs 21.2 comes with po-find-file-coding-system. We give preference
;; to the version shipped with Emacs.
(if (not (fboundp 'po-find-file-coding-system))
  (require 'po-compat))

(defvar po-mode-abbrev-table nil
  "Abbrev table used while in PO mode.")
(define-abbrev-table 'po-mode-abbrev-table ())

(defvar po-mode-map
  ;; Use (make-keymap) because (make-sparse-keymap) does not work on Demacs.
  (let ((po-mode-map (make-keymap)))
    (suppress-keymap po-mode-map)
    (define-key po-mode-map "\C-i" 'po-unfuzzy)
    (define-key po-mode-map "\C-j" 'po-msgid-to-msgstr)
    (define-key po-mode-map "\C-m" 'po-edit-msgstr)
    (define-key po-mode-map " " 'po-auto-select-entry)
    (define-key po-mode-map "?" 'po-help)
    (define-key po-mode-map "#" 'po-edit-comment)
    (define-key po-mode-map "," 'po-tags-search)
    (define-key po-mode-map "." 'po-current-entry)
    (define-key po-mode-map "<" 'po-first-entry)
    (define-key po-mode-map "=" 'po-statistics)
    (define-key po-mode-map ">" 'po-last-entry)
    (define-key po-mode-map "a" 'po-cycle-auxiliary)
;;;;  (define-key po-mode-map "c" 'po-save-entry)
    (define-key po-mode-map "f" 'po-next-fuzzy-entry)
    (define-key po-mode-map "h" 'po-help)
    (define-key po-mode-map "k" 'po-kill-msgstr)
;;;;  (define-key po-mode-map "l" 'po-lookup-lexicons)
    (define-key po-mode-map "m" 'po-push-location)
    (define-key po-mode-map "n" 'po-next-entry)
    (define-key po-mode-map "o" 'po-next-obsolete-entry)
    (define-key po-mode-map "p" 'po-previous-entry)
    (define-key po-mode-map "q" 'po-confirm-and-quit)
    (define-key po-mode-map "r" 'po-pop-location)
    (define-key po-mode-map "s" 'po-cycle-source-reference)
    (define-key po-mode-map "t" 'po-next-translated-entry)
    (define-key po-mode-map "u" 'po-next-untranslated-entry)
    (define-key po-mode-map "v" 'po-mode-version)
    (define-key po-mode-map "w" 'po-kill-ring-save-msgstr)
    (define-key po-mode-map "x" 'po-exchange-location)
    (define-key po-mode-map "y" 'po-yank-msgstr)
    (define-key po-mode-map "A" 'po-consider-as-auxiliary)
    (define-key po-mode-map "E" 'po-edit-out-full)
    (define-key po-mode-map "F" 'po-previous-fuzzy-entry)
    (define-key po-mode-map "K" 'po-kill-comment)
;;;;  (define-key po-mode-map "L" 'po-consider-lexicon-file)
    (define-key po-mode-map "M" 'po-send-mail)
    (define-key po-mode-map "O" 'po-previous-obsolete-entry)
    (define-key po-mode-map "T" 'po-previous-translated-entry)
    (define-key po-mode-map "U" 'po-previous-untranslated-entry)
    (define-key po-mode-map "Q" 'po-quit)
    (define-key po-mode-map "S" 'po-consider-source-path)
    (define-key po-mode-map "V" 'po-validate)
    (define-key po-mode-map "W" 'po-kill-ring-save-comment)
    (define-key po-mode-map "Y" 'po-yank-comment)
    (define-key po-mode-map "_" 'po-undo)
    (define-key po-mode-map "\C-_" 'po-undo)
    (define-key po-mode-map "\C-xu" 'po-undo)
    (define-key po-mode-map "0" 'po-other-window)
    (define-key po-mode-map "\177" 'po-fade-out-entry)
    (define-key po-mode-map "\C-c\C-a" 'po-select-auxiliary)
    (define-key po-mode-map "\C-c\C-e" 'po-edit-msgstr-and-ediff)
    (define-key po-mode-map [?\C-c?\C-#] 'po-edit-comment-and-ediff)
    (define-key po-mode-map "\C-c\C-C" 'po-edit-comment-and-ediff)
    (define-key po-mode-map "\M-," 'po-mark-translatable)
    (define-key po-mode-map "\M-." 'po-select-mark-and-mark)
;;;;  (define-key po-mode-map "\M-c" 'po-select-and-save-entry)
;;;;  (define-key po-mode-map "\M-l" 'po-edit-lexicon-entry)
    (define-key po-mode-map "\M-s" 'po-select-source-reference)
    (define-key po-mode-map "\M-A" 'po-ignore-as-auxiliary)
;;;;  (define-key po-mode-map "\M-L" 'po-ignore-lexicon-file)
    (define-key po-mode-map "\M-S" 'po-ignore-source-path)
    po-mode-map)
  "Keymap for PO mode.")

;;;###autoload
(defun po-mode ()
  "Major mode for translators when they edit PO files.

Special commands:
\\{po-mode-map}
Turning on PO mode calls the value of the variable 'po-mode-hook',
if that value is non-nil.  Behaviour may be adjusted through some variables,
all reachable through 'M-x customize', in group 'Emacs.Editing.I18n.Po'."
  (interactive)
  (kill-all-local-variables)
  (setq major-mode 'po-mode
        mode-name "PO")
  (use-local-map po-mode-map)
  (if (fboundp 'easy-menu-define)
      (easy-menu-define po-mode-menu po-mode-map "" po-mode-menu-layout))
  (set (make-local-variable 'font-lock-defaults) '(po-font-lock-keywords t))

  (set (make-local-variable 'po-read-only) buffer-read-only)
  (setq buffer-read-only t)

  (make-local-variable 'po-start-of-entry)
  (make-local-variable 'po-start-of-msgctxt)
  (make-local-variable 'po-start-of-msgid)
  (make-local-variable 'po-start-of-msgid_plural)
  (make-local-variable 'po-start-of-msgstr-block)
  (make-local-variable 'po-end-of-entry)
  (make-local-variable 'po-entry-type)

  (make-local-variable 'po-translated-counter)
  (make-local-variable 'po-fuzzy-counter)
  (make-local-variable 'po-untranslated-counter)
  (make-local-variable 'po-obsolete-counter)
  (make-local-variable 'po-mode-line-string)

  (setq po-mode-flag t)

  (po-check-file-header)
  (po-compute-counters nil)

  (set (make-local-variable 'po-edited-fields) nil)
  (set (make-local-variable 'po-marker-stack) nil)
  (set (make-local-variable 'po-search-path) '(("./") ("../")))

  (set (make-local-variable 'po-reference-alist) nil)
  (set (make-local-variable 'po-reference-cursor) nil)
  (set (make-local-variable 'po-reference-check) 0)

  (set (make-local-variable 'po-keywords)
       '(("gettext") ("gettext_noop") ("_") ("N_")))
  (set (make-local-variable 'po-string-contents) nil)
  (set (make-local-variable 'po-string-buffer) nil)
  (set (make-local-variable 'po-string-start) nil)
  (set (make-local-variable 'po-string-end) nil)
  (set (make-local-variable 'po-marking-overlay) (po-create-overlay))

  (add-hook 'write-contents-hooks 'po-replace-revision-date)

  (run-hooks 'po-mode-hook)
  (message (_"You may type 'h' or '?' for a short PO mode reminder.")))

(defvar po-subedit-mode-map
  ;; Use (make-keymap) because (make-sparse-keymap) does not work on Demacs.
  (let ((po-subedit-mode-map (make-keymap)))
    (define-key po-subedit-mode-map "\C-c\C-a" 'po-subedit-cycle-auxiliary)
    (define-key po-subedit-mode-map "\C-c\C-c" 'po-subedit-exit)
    (define-key po-subedit-mode-map "\C-c\C-e" 'po-subedit-ediff)
    (define-key po-subedit-mode-map "\C-c\C-k" 'po-subedit-abort)
    po-subedit-mode-map)
  "Keymap while editing a PO mode entry (or the full PO file).")

;;; Window management.

(make-variable-buffer-local 'po-mode-flag)

(defvar po-mode-line-entry '(po-mode-flag ("  " po-mode-line-string))
  "Mode line format entry displaying MODE-LINE-STRING.")

;; Insert MODE-LINE-ENTRY in mode line, but on first load only.
(or (member po-mode-line-entry mode-line-format)
    ;; mode-line-format usually contains global-mode-string, but some
    ;; people customize this variable. As a last resort, append at the end.
    (let ((prev-entry (or (member 'global-mode-string mode-line-format)
                          (member "   " mode-line-format)
                          (last mode-line-format))))
      (setcdr prev-entry (cons po-mode-line-entry (cdr prev-entry)))))

(defun po-update-mode-line-string ()
  "Compute a new statistics string to display in mode line."
  (setq po-mode-line-string
        (concat (format "%dt" po-translated-counter)
                (if (> po-fuzzy-counter 0)
                    (format "+%df" po-fuzzy-counter))
                (if (> po-untranslated-counter 0)
                    (format "+%du" po-untranslated-counter))
                (if (> po-obsolete-counter 0)
                    (format "+%do" po-obsolete-counter))))
  (po-force-mode-line-update))

(defun po-type-counter ()
  "Return the symbol name of the counter appropriate for the current entry."
  (cond ((eq po-entry-type 'obsolete) 'po-obsolete-counter)
        ((eq po-entry-type 'fuzzy) 'po-fuzzy-counter)
        ((eq po-entry-type 'translated) 'po-translated-counter)
        ((eq po-entry-type 'untranslated) 'po-untranslated-counter)
        (t (error (_"Unknown entry type")))))

(defun po-decrease-type-counter ()
  "Decrease the counter corresponding to the nature of the current entry."
  (let ((counter (po-type-counter)))
    (set counter (1- (eval counter)))))

(defun po-increase-type-counter ()
  "Increase the counter corresponding to the nature of the current entry.
Then, update the mode line counters."
  (let ((counter (po-type-counter)))
    (set counter (1+ (eval counter))))
  (po-update-mode-line-string))

;; Avoid byte compiler warnings.
(defvar po-fuzzy-regexp)
(defvar po-untranslated-regexp)

(defun po-compute-counters (flag)
  "Prepare counters for mode line display.  If FLAG, also echo entry position."
  (and flag (po-find-span-of-entry))
  (setq po-translated-counter 0
        po-fuzzy-counter 0
        po-untranslated-counter 0
        po-obsolete-counter 0)
  (let ((position 0) (total 0) current here)
    ;; FIXME 'here' looks obsolete / 2001-08-23 03:54:26 CEST -ke-
    (save-excursion
      (po-find-span-of-entry)
      (setq current po-start-of-msgstr-block)
      (goto-char (point-min))
      ;; While counting, skip the header entry, for consistency with msgfmt.
      (po-find-span-of-entry)
      (if (string-equal (po-get-msgid) "")
          (goto-char po-end-of-entry))
      (if (re-search-forward "^msgid" (point-max) t)
          (progn
            ;; Start counting
            (while (re-search-forward po-any-msgstr-block-regexp nil t)
              (and (= (% total 20) 0)
                   (if flag
                       (message (_"Position %d/%d") position total)
                     (message (_"Position %d") total)))
              (setq here (point))
              (goto-char (match-beginning 0))
              (setq total (1+ total))
              (and flag (eq (point) current) (setq position total))
              (cond ((eq (following-char) ?#)
                     (setq po-obsolete-counter (1+ po-obsolete-counter)))
                    ((looking-at po-untranslated-regexp)
                     (setq po-untranslated-counter (1+ po-untranslated-counter)))
                    (t (setq po-translated-counter (1+ po-translated-counter))))
              (goto-char here))

            ;; Make another pass just for the fuzzy entries, kind of kludgey.
            ;; FIXME: Counts will be wrong if untranslated entries are fuzzy, yet
            ;; this should not normally happen.
            (goto-char (point-min))
            (while (re-search-forward po-fuzzy-regexp nil t)
              (setq po-fuzzy-counter (1+ po-fuzzy-counter)))
            (setq po-translated-counter (- po-translated-counter po-fuzzy-counter)))
        '()))

    ;; Push the results out.
    (if flag
        (message (_"\
Position %d/%d; %d translated, %d fuzzy, %d untranslated, %d obsolete")
                 position total po-translated-counter po-fuzzy-counter
                 po-untranslated-counter po-obsolete-counter)
      (message "")))
  (po-update-mode-line-string))

(defun po-redisplay ()
  "Redisplay the current entry."
  ;; FIXME: Should try to fit the whole entry on the window.  If this is not
  ;; possible, should try to fit the comment and the msgid.  Otherwise,
  ;; should try to fit the msgid.  Else, the first line of the msgid should
  ;; be at the top of the window.
  (goto-char po-start-of-msgid))

(defun po-other-window ()
  "Get the cursor into another window, out of PO mode."
  (interactive)
  (if (one-window-p t)
      (progn
        (split-window)
        (switch-to-buffer (other-buffer)))
    (other-window 1)))

;;; Processing the PO file header entry.

(defun po-check-file-header ()
  "Create a missing PO mode file header, or replace an oldish one.
Can be customized with the `po-auto-update-file-header' variable."
  (if (or (eq po-auto-update-file-header t)
          (and (eq po-auto-update-file-header 'ask)
               (y-or-n-p (_"May I update the PO Header Entry? "))))
      (save-excursion
        (save-restriction
          (widen) ; in case of a narrowed view to the buffer
          (let ((buffer-read-only po-read-only)
                insert-flag end-of-header)
            (goto-char (point-min))
            (if (re-search-forward po-any-msgstr-block-regexp nil t)
                (progn
                  ;; There is at least one entry.
                  (goto-char (match-beginning 0))
                  (forward-line -1)
                  (setq end-of-header (match-end 0))
                  (if (looking-at "msgid \"\"\n")
                      ;; There is indeed a PO file header.
                      (if (re-search-forward "\n\"PO-Revision-Date: "
                                             end-of-header t)
                          nil
                        ;; This is an oldish header.  Replace it all.
                        (goto-char end-of-header)
                        (while (> (point) (point-min))
                          (forward-line -1)
                          (insert "#~ ")
                          (beginning-of-line))
                        (beginning-of-line)
                        (setq insert-flag t))
                    ;; The first entry is not a PO file header, insert one.
                    (setq insert-flag t)))
              ;; Not a single entry found.
              (setq insert-flag t))
            (goto-char (point-min))
            (if insert-flag
                (progn
                  (insert po-default-file-header)
                  (if (not (eobp))
                      (insert "\n")))))))
    (message (_"PO Header Entry was not updated..."))))

(defun po-replace-revision-date ()
  "Replace the revision date by current time in the PO file header."
  (if (fboundp 'format-time-string)
      (if (or (eq po-auto-replace-revision-date t)
              (and (eq po-auto-replace-revision-date 'ask)
                   (y-or-n-p (_"May I set PO-Revision-Date? "))))
          (save-excursion
            (goto-char (point-min))
            (if (re-search-forward "^\"PO-Revision-Date:.*" nil t)
                (let* ((buffer-read-only po-read-only)
                       (time (current-time))
                       (seconds (or (car (current-time-zone time)) 0))
                       (minutes (/ (abs seconds) 60))
                       (zone (format "%c%02d%02d"
                                     (if (< seconds 0) ?- ?+)
                                     (/ minutes 60)
                                     (% minutes 60))))
                  (replace-match
                       (concat "\"PO-Revision-Date: "
                               (format-time-string "%Y-%m-%d %H:%M" time)
                               zone "\\n\"")
                       t t))))
        (message ""))
    (message (_"PO-Revision-Date should be adjusted...")))
  ;; Return nil to indicate that the buffer has not yet been saved.
  nil)

;;; Handling span of entry, entry type and entry attributes.

(defun po-find-span-of-entry ()
  "Find the extent of the PO file entry where the cursor is.
Set variables po-start-of-entry, po-start-of-msgctxt, po-start-of-msgid,
po-start-of-msgid_plural, po-start-of-msgstr-block, po-end-of-entry, and
po-entry-type to meaningful values. po-entry-type may be set to: obsolete,
fuzzy, untranslated, or translated."
  (let ((here (point)))
    (if (re-search-backward po-any-msgstr-block-regexp nil t)
        (progn
          ;; After a backward match, (match-end 0) will not extend
          ;; beyond point, in case point was *inside* the regexp.  We
          ;; need a dependable (match-end 0), so we redo the match in
          ;; the forward direction.
          (re-search-forward po-any-msgstr-block-regexp)
          (if (<= (match-end 0) here)
              (progn
                ;; We most probably found the msgstr of the previous
                ;; entry.  The current entry then starts just after
                ;; its end, save this information just in case.
                (setq po-start-of-entry (match-end 0))
                ;; However, it is also possible that we are located in
                ;; the crumb after the last entry in the file.  If
                ;; yes, we know the middle and end of last PO entry.
                (setq po-start-of-msgstr-block (match-beginning 0)
                      po-end-of-entry (match-end 0))
                (if (re-search-forward po-any-msgstr-block-regexp nil t)
                    (progn
                      ;; We definitely were not in the crumb.
                      (setq po-start-of-msgstr-block (match-beginning 0)
                            po-end-of-entry (match-end 0)))
                  ;; We were in the crumb.  The start of the last PO
                  ;; file entry is the end of the previous msgstr if
                  ;; any, or else, the beginning of the file.
                  (goto-char po-start-of-msgstr-block)
                  (setq po-start-of-entry
                        (if (re-search-backward po-any-msgstr-block-regexp nil t)
                            (match-end 0)
                          (point-min)))))
            ;; The cursor was inside msgstr of the current entry.
            (setq po-start-of-msgstr-block (match-beginning 0)
                  po-end-of-entry (match-end 0))
            ;; The start of this entry is the end of the previous
            ;; msgstr if any, or else, the beginning of the file.
            (goto-char po-start-of-msgstr-block)
            (setq po-start-of-entry
                  (if (re-search-backward po-any-msgstr-block-regexp nil t)
                      (match-end 0)
                    (point-min)))))
      ;; The cursor was before msgstr in the first entry in the file.
      (setq po-start-of-entry (point-min))
      (goto-char po-start-of-entry)
      ;; There is at least the PO file header, so this should match.
      (re-search-forward po-any-msgstr-block-regexp)
      (setq po-start-of-msgstr-block (match-beginning 0)
            po-end-of-entry (match-end 0)))
    ;; Find start of msgid.
    (goto-char po-start-of-entry)
    (re-search-forward po-any-msgctxt-msgid-regexp)
    (setq po-start-of-msgctxt (match-beginning 0))
    (goto-char po-start-of-entry)
    (re-search-forward po-any-msgid-regexp)
    (setq po-start-of-msgid (match-beginning 0))
    (save-excursion
      (goto-char po-start-of-msgid)
      (setq po-start-of-msgid_plural
            (if (re-search-forward po-any-msgid_plural-regexp
                                   po-start-of-msgstr-block t)
                (match-beginning 0)
              nil)))
    (save-excursion
      (when (>= here po-start-of-msgstr-block)
        ;; point was somewhere inside of msgstr*
        (goto-char here)
        (end-of-line)
        (re-search-backward "^\\(#~[ \t]*\\)?msgstr"))
      ;; Detect the boundaries of the msgstr we are interested in.
      (re-search-forward po-any-msgstr-form-regexp)
      (setq po-start-of-msgstr-form (match-beginning 0)
            po-end-of-msgstr-form (match-end 0)))
    ;; Classify the entry.
    (setq po-entry-type
          (if (eq (following-char) ?#)
              'obsolete
            (goto-char po-start-of-entry)
            (if (re-search-forward po-fuzzy-regexp po-start-of-msgctxt t)
                'fuzzy
              (goto-char po-start-of-msgstr-block)
              (if (looking-at po-untranslated-regexp)
                  'untranslated
                'translated))))
    ;; Put the cursor back where it was.
    (goto-char here)))

(defun po-add-attribute (name)
  "Add attribute NAME to the current entry, unless it is already there."
  (save-excursion
    (let ((buffer-read-only po-read-only))
      (goto-char po-start-of-entry)
      (if (re-search-forward "\n#, .*" po-start-of-msgctxt t)
          (save-restriction
            (narrow-to-region (match-beginning 0) (match-end 0))
            (goto-char (point-min))
            (if (re-search-forward (concat "\\b" name "\\b") nil t)
                nil
              (goto-char (point-max))
              (insert ", " name)))
        (skip-chars-forward "\n")
        (while (eq (following-char) ?#)
          (forward-line 1))
        (insert "#, " name "\n")))))

(defun po-delete-attribute (name)
  "Delete attribute NAME from the current entry, if any."
  (save-excursion
    (let ((buffer-read-only po-read-only))
      (goto-char po-start-of-entry)
      (if (re-search-forward "\n#, .*" po-start-of-msgctxt t)
          (save-restriction
            (narrow-to-region (match-beginning 0) (match-end 0))
            (goto-char (point-min))
            (if (re-search-forward
                 (concat "\\(\n#, " name "$\\|, " name "$\\| " name ",\\)")
                 nil t)
                (replace-match "" t t)))))))

;;; Entry positionning.

(defun po-say-location-depth ()
  "Tell how many entries in the entry location stack."
  (let ((depth (length po-marker-stack)))
    (cond ((= depth 0) (message (_"Empty location stack")))
          ((= depth 1) (message (_"One entry in location stack")))
          (t (message (_"%d entries in location stack") depth)))))

(defun po-push-location ()
  "Stack the location of the current entry, for later return."
  (interactive)
  (po-find-span-of-entry)
  (save-excursion
    (goto-char po-start-of-msgid)
    (setq po-marker-stack (cons (point-marker) po-marker-stack)))
  (po-say-location-depth))

(defun po-pop-location ()
  "Unstack a saved location, and return to the corresponding entry."
  (interactive)
  (if po-marker-stack
      (progn
        (goto-char (car po-marker-stack))
        (setq po-marker-stack (cdr po-marker-stack))
        (po-current-entry)
        (po-say-location-depth))
    (error (_"The entry location stack is empty"))))

(defun po-exchange-location ()
  "Exchange the location of the current entry with the top of stack."
  (interactive)
  (if po-marker-stack
      (progn
        (po-find-span-of-entry)
        (goto-char po-start-of-msgid)
        (let ((location (point-marker)))
          (goto-char (car po-marker-stack))
          (setq po-marker-stack (cons location (cdr po-marker-stack))))
        (po-current-entry)
        (po-say-location-depth))
    (error (_"The entry location stack is empty"))))

(defun po-current-entry ()
  "Display the current entry."
  (interactive)
  (po-find-span-of-entry)
  (po-redisplay))

(defun po-first-entry-with-regexp (regexp)
  "Display the first entry in the file which msgstr matches REGEXP."
  (let ((here (point)))
    (goto-char (point-min))
    (if (re-search-forward regexp nil t)
        (progn
          (goto-char (match-beginning 0))
          (po-current-entry))
      (goto-char here)
      (error (_"There is no such entry")))))

(defun po-last-entry-with-regexp (regexp)
  "Display the last entry in the file which msgstr matches REGEXP."
  (let ((here (point)))
    (goto-char (point-max))
    (if (re-search-backward regexp nil t)
        (po-current-entry)
      (goto-char here)
      (error (_"There is no such entry")))))

(defun po-next-entry-with-regexp (regexp wrap)
  "Display the entry following the current entry which msgstr matches REGEXP.
If WRAP is not nil, the search may wrap around the buffer."
  (po-find-span-of-entry)
  (let ((here (point)))
    (goto-char po-end-of-entry)
    (if (re-search-forward regexp nil t)
        (progn
          (goto-char (match-beginning 0))
          (po-current-entry))
      (if (and wrap
               (progn
                 (goto-char (point-min))
                 (re-search-forward regexp po-start-of-entry t)))
          (progn
            (goto-char (match-beginning 0))
            (po-current-entry)
            (message (_"Wrapping around the buffer")))
        (goto-char here)
        (error (_"There is no such entry"))))))

(defun po-previous-entry-with-regexp (regexp wrap)
  "Redisplay the entry preceding the current entry which msgstr matches REGEXP.
If WRAP is not nil, the search may wrap around the buffer."
  (po-find-span-of-entry)
  (let ((here (point)))
    (goto-char po-start-of-entry)
    (if (re-search-backward regexp nil t)
        (po-current-entry)
      (if (and wrap
               (progn
                 (goto-char (point-max))
                 (re-search-backward regexp po-end-of-entry t)))
          (progn
            (po-current-entry)
            (message (_"Wrapping around the buffer")))
        (goto-char here)
        (error (_"There is no such entry"))))))

;; Any entries.

(defun po-first-entry ()
  "Display the first entry."
  (interactive)
  (po-first-entry-with-regexp po-any-msgstr-block-regexp))

(defun po-last-entry ()
  "Display the last entry."
  (interactive)
  (po-last-entry-with-regexp po-any-msgstr-block-regexp))

(defun po-next-entry ()
  "Display the entry following the current entry."
  (interactive)
  (po-next-entry-with-regexp po-any-msgstr-block-regexp nil))

(defun po-previous-entry ()
  "Display the entry preceding the current entry."
  (interactive)
  (po-previous-entry-with-regexp po-any-msgstr-block-regexp nil))

;; Untranslated entries.

(defvar po-after-entry-regexp
  "\\(\\'\\|\\(#[ \t]*\\)?$\\)"
  "Regexp which should be true after a full msgstr string matched.")

(defvar po-untranslated-regexp
  (concat "^msgstr\\(\\[[0-9]\\]\\)?[ \t]*\"\"\n" po-after-entry-regexp)
  "Regexp matching a whole msgstr field, but only if active and empty.")

(defun po-next-untranslated-entry ()
  "Find the next untranslated entry, wrapping around if necessary."
  (interactive)
  (po-next-entry-with-regexp po-untranslated-regexp t))

(defun po-previous-untranslated-entry ()
  "Find the previous untranslated entry, wrapping around if necessary."
  (interactive)
  (po-previous-entry-with-regexp po-untranslated-regexp t))

(defun po-msgid-to-msgstr ()
  "Use another window to edit msgstr reinitialized with msgid."
  (interactive)
  (po-find-span-of-entry)
  (if (or (eq po-entry-type 'untranslated)
          (eq po-entry-type 'obsolete)
          (prog1 (y-or-n-p (_"Really lose previous translation? "))
                 (message "")))
      ;; In an entry with plural forms, use the msgid_plural string,
      ;; as it is more general than the msgid string.
      (if (po-set-msgstr-form (or (po-get-msgid_plural) (po-get-msgid)))
          (po-maybe-delete-previous-untranslated))))

;; Obsolete entries.

(defvar po-obsolete-msgstr-regexp
  "^#~[ \t]*msgstr.*\n\\(#~[ \t]*\".*\n\\)*"
  "Regexp matching a whole msgstr field of an obsolete entry.")

(defun po-next-obsolete-entry ()
  "Find the next obsolete entry, wrapping around if necessary."
  (interactive)
  (po-next-entry-with-regexp po-obsolete-msgstr-regexp t))

(defun po-previous-obsolete-entry ()
  "Find the previous obsolete entry, wrapping around if necessary."
  (interactive)
  (po-previous-entry-with-regexp po-obsolete-msgstr-regexp t))

;; Fuzzy entries.

(defvar po-fuzzy-regexp "^#, .*fuzzy"
  "Regexp matching the string inserted by msgmerge for translations
which does not match exactly.")

(defun po-next-fuzzy-entry ()
  "Find the next fuzzy entry, wrapping around if necessary."
  (interactive)
  (po-next-entry-with-regexp po-fuzzy-regexp t))

(defun po-previous-fuzzy-entry ()
  "Find the next fuzzy entry, wrapping around if necessary."
  (interactive)
  (po-previous-entry-with-regexp po-fuzzy-regexp t))

(defun po-unfuzzy ()
  "Remove the fuzzy attribute for the current entry."
  (interactive)
  (po-find-span-of-entry)
  (cond ((eq po-entry-type 'fuzzy)
         (po-decrease-type-counter)
         (po-delete-attribute "fuzzy")
         (po-maybe-delete-previous-untranslated)
         (po-current-entry)
         (po-increase-type-counter)))
  (if po-auto-select-on-unfuzzy
      (po-auto-select-entry))
  (po-update-mode-line-string))

;; Translated entries.

(defun po-next-translated-entry ()
  "Find the next translated entry, wrapping around if necessary."
  (interactive)
  (if (= po-translated-counter 0)
      (error (_"There is no such entry"))
    (po-next-entry-with-regexp po-any-msgstr-block-regexp t)
    (po-find-span-of-entry)
    (while (not (eq po-entry-type 'translated))
      (po-next-entry-with-regexp po-any-msgstr-block-regexp t)
      (po-find-span-of-entry))))

(defun po-previous-translated-entry ()
  "Find the previous translated entry, wrapping around if necessary."
  (interactive)
  (if (= po-translated-counter 0)
      (error (_"There is no such entry"))
    (po-previous-entry-with-regexp po-any-msgstr-block-regexp t)
    (po-find-span-of-entry)
    (while (not (eq po-entry-type 'translated))
      (po-previous-entry-with-regexp po-any-msgstr-block-regexp t)
      (po-find-span-of-entry))))

;; Auto-selection feature.

(defun po-auto-select-entry ()
  "Select the next entry having the same type as the current one.
If none, wrap from the beginning of the buffer with another type,
going from untranslated to fuzzy, and from fuzzy to obsolete.
Plain translated entries are always disregarded unless there are
no entries of the other types."
  (interactive)
  (po-find-span-of-entry)
  (goto-char po-end-of-entry)
  (if (and (= po-untranslated-counter 0)
           (= po-fuzzy-counter 0)
           (= po-obsolete-counter 0))
      ;; All entries are plain translated.  Next entry will do, or
      ;; wrap around if there is none.
      (if (re-search-forward po-any-msgstr-block-regexp nil t)
          (goto-char (match-beginning 0))
        (goto-char (point-min)))
    ;; If over a translated entry, look for an untranslated one first.
    ;; Else, look for an entry of the same type first.
    (let ((goal (if (eq po-entry-type 'translated)
                    'untranslated
                  po-entry-type)))
      (while goal
        ;; Find an untranslated entry, or wrap up for a fuzzy entry.
        (if (eq goal 'untranslated)
            (if (and (> po-untranslated-counter 0)
                     (re-search-forward po-untranslated-regexp nil t))
                (progn
                  (goto-char (match-beginning 0))
                  (setq goal nil))
              (goto-char (point-min))
              (setq goal 'fuzzy)))
        ;; Find a fuzzy entry, or wrap up for an obsolete entry.
        (if (eq goal 'fuzzy)
            (if (and (> po-fuzzy-counter 0)
                     (re-search-forward po-fuzzy-regexp nil t))
                (progn
                  (goto-char (match-beginning 0))
                  (setq goal nil))
              (goto-char (point-min))
              (setq goal 'obsolete)))
        ;; Find an obsolete entry, or wrap up for an untranslated entry.
        (if (eq goal 'obsolete)
            (if (and (> po-obsolete-counter 0)
                     (re-search-forward po-obsolete-msgstr-regexp nil t))
                (progn
                  (goto-char (match-beginning 0))
                  (setq goal nil))
              (goto-char (point-min))
              (setq goal 'untranslated))))))
  ;; Display this entry nicely.
  (po-current-entry))

;;; Killing and yanking fields.

(defun po-extract-unquoted (buffer start end)
  "Extract and return the unquoted string in BUFFER going from START to END.
Crumb preceding or following the quoted string is ignored."
  (save-excursion
    (goto-char start)
    (search-forward "\"")
    (setq start (point))
    (goto-char end)
    (search-backward "\"")
    (setq end (point)))
  (po-extract-part-unquoted buffer start end))

(defun po-extract-part-unquoted (buffer start end)
  "Extract and return the unquoted string in BUFFER going from START to END.
Surrounding quotes are already excluded by the position of START and END."
  (po-with-temp-buffer
   (insert-buffer-substring buffer start end)
   ;; Glue concatenated strings.
   (goto-char (point-min))
   (while (re-search-forward "\"[ \t]*\\\\?\n\\(#~\\)?[ \t]*\"" nil t)
     (replace-match "" t t))
   ;; Remove escaped newlines.
   (goto-char (point-min))
   (while (re-search-forward "\\\\[ \t]*\n" nil t)
     (replace-match "" t t))
   ;; Unquote individual characters.
   (goto-char (point-min))
   (while (re-search-forward "\\\\[\"abfnt\\0-7]" nil t)
     (cond ((eq (preceding-char) ?\") (replace-match "\"" t t))
           ((eq (preceding-char) ?a) (replace-match "\a" t t))
           ((eq (preceding-char) ?b) (replace-match "\b" t t))
           ((eq (preceding-char) ?f) (replace-match "\f" t t))
           ((eq (preceding-char) ?n) (replace-match "\n" t t))
           ((eq (preceding-char) ?t) (replace-match "\t" t t))
           ((eq (preceding-char) ?\\) (replace-match "\\" t t))
           (t (let ((value (- (preceding-char) ?0)))
                (replace-match "" t t)
                (while (looking-at "[0-7]")
                  (setq value (+ (* 8 value) (- (following-char) ?0)))
                  (replace-match "" t t))
                (insert value)))))
   (buffer-string)))

(defun po-eval-requoted (form prefix obsolete)
  "Eval FORM, which inserts a string, and return the string fully requoted.
If PREFIX, precede the result with its contents.  If OBSOLETE, comment all
generated lines in the returned string.  Evaluating FORM should insert the
wanted string in the buffer which is current at the time of evaluation.
If FORM is itself a string, then this string is used for insertion."
  (po-with-temp-buffer
    (if (stringp form)
        (insert form)
      (push-mark)
      (eval form))
    (goto-char (point-min))
    (let ((multi-line (re-search-forward "[^\n]\n+[^\n]" nil t)))
      (goto-char (point-min))
      (while (re-search-forward "[\"\a\b\f\n\r\t\\]" nil t)
        (cond ((eq (preceding-char) ?\") (replace-match "\\\"" t t))
              ((eq (preceding-char) ?\a) (replace-match "\\a" t t))
              ((eq (preceding-char) ?\b) (replace-match "\\b" t t))
              ((eq (preceding-char) ?\f) (replace-match "\\f" t t))
              ((eq (preceding-char) ?\n)
               (replace-match (if (or (not multi-line) (eobp))
                                  "\\n"
                                "\\n\"\n\"")
                              t t))
              ((eq (preceding-char) ?\r) (replace-match "\\r" t t))
              ((eq (preceding-char) ?\t) (replace-match "\\t" t t))
              ((eq (preceding-char) ?\\) (replace-match "\\\\" t t))))
      (goto-char (point-min))
      (if prefix (insert prefix " "))
      (insert (if multi-line "\"\"\n\"" "\""))
      (goto-char (point-max))
      (insert "\"")
      (if prefix (insert "\n"))
      (if obsolete
          (progn
            (goto-char (point-min))
            (while (not (eobp))
              (or (eq (following-char) ?\n) (insert "#~ "))
              (search-forward "\n"))))
      (buffer-string))))

(defun po-get-msgid ()
  "Extract and return the unquoted msgid string."
  (let ((string (po-extract-unquoted (current-buffer)
                                     po-start-of-msgid
                                     (or po-start-of-msgid_plural
                                         po-start-of-msgstr-block))))
    string))

(defun po-get-msgid_plural ()
  "Extract and return the unquoted msgid_plural string.
Return nil if it is not present."
  (if po-start-of-msgid_plural
      (let ((string (po-extract-unquoted (current-buffer)
                                         po-start-of-msgid_plural
                                         po-start-of-msgstr-block)))
        string)
    nil))

(defun po-get-msgstr-flavor ()
  "Helper function to detect msgstr and msgstr[] variants.
Returns one of \"msgstr\" or \"msgstr[i]\" for some i."
  (save-excursion
    (goto-char po-start-of-msgstr-form)
    (re-search-forward "^\\(#~[ \t]*\\)?\\(msgstr\\(\\[[0-9]\\]\\)?\\)")
    (match-string 2)))

(defun po-get-msgstr-form ()
  "Extract and return the unquoted msgstr string."
  (let ((string (po-extract-unquoted (current-buffer)
                                     po-start-of-msgstr-form
                                     po-end-of-msgstr-form)))
    string))

(defun po-set-msgid (form)
  "Replace the current msgid, using FORM to get a string.
Evaluating FORM should insert the wanted string in the current buffer.  If
FORM is itself a string, then this string is used for insertion.  The string
is properly requoted before the replacement occurs.

Returns 'nil' if the buffer has not been modified, for if the new msgid
described by FORM is merely identical to the msgid already in place."
  (let ((string (po-eval-requoted form "msgid" (eq po-entry-type 'obsolete))))
    (save-excursion
      (goto-char po-start-of-entry)
      (re-search-forward po-any-msgid-regexp po-start-of-msgstr-block)
      (and (not (string-equal (po-match-string 0) string))
           (let ((buffer-read-only po-read-only))
             (replace-match string t t)
             (goto-char po-start-of-msgid)
             (po-find-span-of-entry)
             t)))))

(defun po-set-msgstr-form (form)
  "Replace the current msgstr or msgstr[], using FORM to get a string.
Evaluating FORM should insert the wanted string in the current buffer.  If
FORM is itself a string, then this string is used for insertion.  The string
is properly requoted before the replacement occurs.

Returns 'nil' if the buffer has not been modified, for if the new msgstr
described by FORM is merely identical to the msgstr already in place."
  (let ((string (po-eval-requoted form
                                  (po-get-msgstr-flavor)
                                  (eq po-entry-type 'obsolete))))
    (save-excursion
      (goto-char po-start-of-msgstr-form)
      (re-search-forward po-any-msgstr-form-regexp po-end-of-msgstr-form)
      (and (not (string-equal (po-match-string 0) string))
           (let ((buffer-read-only po-read-only))
             (po-decrease-type-counter)
             (replace-match string t t)
             (goto-char po-start-of-msgid)
             (po-find-span-of-entry)
             (po-increase-type-counter)
             t)))))

(defun po-kill-ring-save-msgstr ()
  "Push the msgstr string from current entry on the kill ring."
  (interactive)
  (po-find-span-of-entry)
  (let ((string (po-get-msgstr-form)))
    (po-kill-new string)
    string))

(defun po-kill-msgstr ()
  "Empty the msgstr string from current entry, pushing it on the kill ring."
  (interactive)
  (po-kill-ring-save-msgstr)
  (if (po-set-msgstr-form "")
      (po-maybe-delete-previous-untranslated)))

(defun po-yank-msgstr ()
  "Replace the current msgstr string by the top of the kill ring."
  (interactive)
  (po-find-span-of-entry)
  (if (po-set-msgstr-form (if (eq last-command 'yank) '(yank-pop 1) '(yank)))
      (po-maybe-delete-previous-untranslated))
  (setq this-command 'yank))

(defun po-fade-out-entry ()
  "Mark an active entry as fuzzy; obsolete a fuzzy or untranslated entry;
or completely delete an obsolete entry, saving its msgstr on the kill ring."
  (interactive)
  (po-find-span-of-entry)

  (cond ((eq po-entry-type 'translated)
         (po-decrease-type-counter)
         (po-add-attribute "fuzzy")
         (po-current-entry)
         (po-increase-type-counter))

        ((or (eq po-entry-type 'fuzzy)
             (eq po-entry-type 'untranslated))
         (if (y-or-n-p (_"Should I really obsolete this entry? "))
             (progn
               (po-decrease-type-counter)
               (save-excursion
                 (save-restriction
                   (narrow-to-region po-start-of-entry po-end-of-entry)
                   (let ((buffer-read-only po-read-only))
                     (goto-char (point-min))
                     (skip-chars-forward "\n")
                     (while (not (eobp))
                       (insert "#~ ")
                       (search-forward "\n")))))
               (po-current-entry)
               (po-increase-type-counter)))
         (message ""))

        ((and (eq po-entry-type 'obsolete)
              (po-check-for-pending-edit po-start-of-msgid)
              (po-check-for-pending-edit po-start-of-msgstr-block))
         (po-decrease-type-counter)
         (po-update-mode-line-string)
         ;; TODO: Should save all msgstr forms here, not just one.
         (po-kill-new (po-get-msgstr-form))
         (let ((buffer-read-only po-read-only))
           (delete-region po-start-of-entry po-end-of-entry))
         (goto-char po-start-of-entry)
         (if (re-search-forward po-any-msgstr-block-regexp nil t)
             (goto-char (match-beginning 0))
           (re-search-backward po-any-msgstr-block-regexp nil t))
         (po-current-entry)
         (message ""))))

;;; Killing and yanking comments.

(defvar po-comment-regexp
  "^\\(#\n\\|# .*\n\\)+"
  "Regexp matching the whole editable comment part of an entry.")

(defun po-get-comment (kill-flag)
  "Extract and return the editable comment string, uncommented.
If KILL-FLAG, then add the unquoted comment to the kill ring."
  (let ((buffer (current-buffer))
        (obsolete (eq po-entry-type 'obsolete)))
    (save-excursion
      (goto-char po-start-of-entry)
      (if (re-search-forward po-comment-regexp po-end-of-entry t)
          (po-with-temp-buffer
            (insert-buffer-substring buffer (match-beginning 0) (match-end 0))
            (goto-char (point-min))
            (while (not (eobp))
              (if (looking-at (if obsolete "#\\(\n\\| \\)" "# ?"))
                  (replace-match "" t t))
              (forward-line 1))
            (and kill-flag (copy-region-as-kill (point-min) (point-max)))
            (buffer-string))
        ""))))

(defun po-set-comment (form)
  "Using FORM to get a string, replace the current editable comment.
Evaluating FORM should insert the wanted string in the current buffer.
If FORM is itself a string, then this string is used for insertion.
The string is properly recommented before the replacement occurs."
  (let ((obsolete (eq po-entry-type 'obsolete))
        string)
    (po-with-temp-buffer
      (if (stringp form)
          (insert form)
        (push-mark)
        (eval form))
      (if (not (or (bobp) (= (preceding-char) ?\n)))
          (insert "\n"))
      (goto-char (point-min))
      (while (not (eobp))
        (insert (if (= (following-char) ?\n) "#" "# "))
        (search-forward "\n"))
      (setq string (buffer-string)))
    (goto-char po-start-of-entry)
    (if (re-search-forward po-comment-regexp po-end-of-entry t)
        (if (not (string-equal (po-match-string 0) string))
            (let ((buffer-read-only po-read-only))
              (replace-match string t t)))
      (skip-chars-forward " \t\n")
      (let ((buffer-read-only po-read-only))
        (insert string))))
  (po-current-entry))

(defun po-kill-ring-save-comment ()
  "Push the msgstr string from current entry on the kill ring."
  (interactive)
  (po-find-span-of-entry)
  (po-get-comment t))

(defun po-kill-comment ()
  "Empty the msgstr string from current entry, pushing it on the kill ring."
  (interactive)
  (po-kill-ring-save-comment)
  (po-set-comment "")
  (po-redisplay))

(defun po-yank-comment ()
  "Replace the current comment string by the top of the kill ring."
  (interactive)
  (po-find-span-of-entry)
  (po-set-comment (if (eq last-command 'yank) '(yank-pop 1) '(yank)))
  (setq this-command 'yank)
  (po-redisplay))

;;; Deleting the "previous untranslated" comment.

(defun po-previous-untranslated-region-for (rx)
  "Return the list of previous untranslated regions (at most one) for the
given regular expression RX."
  (save-excursion
    (goto-char po-start-of-entry)
    (if (re-search-forward rx po-start-of-msgctxt t)
        (list (cons (copy-marker (match-beginning 0))
                    (copy-marker (match-end 0))))
      nil)))

(defun po-previous-untranslated-regions ()
  "Return the list of previous untranslated regions in the current entry."
  (append (po-previous-untranslated-region-for po-any-previous-msgctxt-regexp)
          (po-previous-untranslated-region-for po-any-previous-msgid-regexp)
          (po-previous-untranslated-region-for po-any-previous-msgid_plural-regexp)))

(defun po-delete-previous-untranslated ()
  "Delete the previous msgctxt, msgid, msgid_plural fields (marked as #|
comments) from the current entry."
  (interactive)
  (po-find-span-of-entry)
  (let ((buffer-read-only po-read-only))
    (dolist (region (po-previous-untranslated-regions))
      (delete-region (car region) (cdr region))))
  (po-redisplay))

(defun po-maybe-delete-previous-untranslated ()
  "Delete the previous msgctxt, msgid, msgid_plural fields (marked as #|
comments) from the current entry, if the user gives the permission."
  (po-find-span-of-entry)
  (let ((previous-regions (po-previous-untranslated-regions)))
    (if previous-regions
        (if (or (eq po-auto-delete-previous-msgid t)
                (and (eq po-auto-delete-previous-msgid 'ask)
                     (let ((overlays nil))
                       (unwind-protect
                           (progn
                             (setq overlays
                                   (mapcar (function
                                             (lambda (region)
                                               (let ((overlay (po-create-overlay)))
                                                 (po-highlight overlay (car region) (cdr region))
                                                 overlay)))
                                           previous-regions))
                             ;; Scroll, to show the previous-regions.
                             (goto-char (car (car previous-regions)))
                             (prog1 (y-or-n-p (_"Delete previous msgid comments? "))
                                    (message "")))
                         (mapc 'po-dehighlight overlays)))))
            (let ((buffer-read-only po-read-only))
              (dolist (region previous-regions)
                (delete-region (car region) (cdr region))))))))

;;; Editing management and submode.

;; In a string edit buffer, BACK-POINTER points to one of the slots of the
;; list EDITED-FIELDS kept in the PO buffer.  See its description elsewhere.
;; Reminder: slots have the form (ENTRY-MARKER EDIT-BUFFER OVERLAY-INFO).

(defvar po-subedit-back-pointer)

(defun po-clean-out-killed-edits ()
  "From EDITED-FIELDS, clean out any edit having a killed edit buffer."
  (let ((cursor po-edited-fields))
    (while cursor
      (let ((slot (car cursor)))
        (setq cursor (cdr cursor))
        (if (buffer-name (nth 1 slot))
            nil
          (let ((overlay (nth 2 slot)))
            (and overlay (po-dehighlight overlay)))
          (setq po-edited-fields (delete slot po-edited-fields)))))))

(defun po-check-all-pending-edits ()
  "Resume any pending edit.  Return nil if some remains."
  (po-clean-out-killed-edits)
  (or (null po-edited-fields)
      (let ((slot (car po-edited-fields)))
        (goto-char (nth 0 slot))
        (pop-to-buffer (nth 1 slot))
        (message po-subedit-message)
        nil)))

(defun po-check-for-pending-edit (position)
  "Resume any pending edit at POSITION.  Return nil if such edit exists."
  (po-clean-out-killed-edits)
  (let ((marker (make-marker)))
    (set-marker marker position)
    (let ((slot (assoc marker po-edited-fields)))
      (if slot
          (progn
            (goto-char marker)
            (pop-to-buffer (nth 1 slot))
            (message po-subedit-message)))
      (not slot))))

(defun po-edit-out-full ()
  "Get out of PO mode, leaving PO file buffer in fundamental mode."
  (interactive)
  (if (po-check-all-pending-edits)
      ;; Don't ask the user for confirmation, since he has explicitly asked
      ;; for it.
      (progn
        (setq buffer-read-only po-read-only)
        (fundamental-mode)
        (message (_"Type 'M-x po-mode RET' once done")))))

(defun po-ediff-quit ()
  "Quit ediff and exit `recursive-edit'."
  (interactive)
  (ediff-quit t)
  (exit-recursive-edit))

(add-hook 'ediff-keymap-setup-hook
          '(lambda ()
             (define-key ediff-mode-map "Q" 'po-ediff-quit)))

;; Avoid byte compiler warnings.
(defvar entry-buffer)

(defun po-ediff-buffers-exit-recursive (b1 b2 oldbuf end)
  "Ediff buffer B1 and B2, pop back to OLDBUF and replace the old variants.
This function will delete the first two variants in OLDBUF, call
`ediff-buffers' to compare both strings and replace the two variants in
OLDBUF with the contents of B2.
Once done kill B1 and B2.

For more info cf. `po-subedit-ediff'."
  (ediff-buffers b1 b2)
  (recursive-edit)
  (pop-to-buffer oldbuf)
  (delete-region (point-min) end)
  (insert-buffer-substring b2)
  (mapc 'kill-buffer `(,b1 ,b2))
  (display-buffer entry-buffer t))

(defun po-subedit-ediff ()
  "Edit the subedit buffer using `ediff'.
`po-subedit-ediff' calls `po-ediff-buffers-exit-recursive' to edit translation
variants side by side if they are actually different; if variants are equal just
delete the first one.

`msgcat' is able to produce those variants; every variant is marked with:

#-#-#-#-#  file name reference  #-#-#-#-#

Put changes in second buffer.

When done with the `ediff' session press \\[exit-recursive-edit] exit to
`recursive-edit', or call \\[po-ediff-quit] (`Q') in the ediff control panel."
  (interactive)
  (let* ((marker-regex "^#-#-#-#-#  \\(.*\\)  #-#-#-#-#\n")
         (buf1 " *po-msgstr-1") ; default if first marker is missing
         buf2 start-1 end-1 start-2 end-2
         (back-pointer po-subedit-back-pointer)
         (entry-marker (nth 0 back-pointer))
         (entry-buffer (marker-buffer entry-marker)))
    (goto-char (point-min))
    (if (looking-at marker-regex)
        (and (setq buf1 (match-string-no-properties 1))
             (forward-line 1)))
    (setq start-1 (point))
    (if (not (re-search-forward marker-regex (point-max) t))
        (error "Only 1 msgstr found")
      (setq buf2 (match-string-no-properties 1)
            end-1 (match-beginning 0))
      (let ((oldbuf (current-buffer)))
        (save-current-buffer
          (set-buffer (get-buffer-create
                       (generate-new-buffer-name buf1)))
          (setq buffer-read-only nil)
          (erase-buffer)
          (insert-buffer-substring oldbuf start-1 end-1)
          (setq buffer-read-only t))

        (setq start-2 (point))
        (save-excursion
          ;; check for a third variant; if found ignore it
          (if (re-search-forward marker-regex (point-max) t)
              (setq end-2 (match-beginning 0))
            (setq end-2 (goto-char (1- (point-max))))))
        (save-current-buffer
          (set-buffer (get-buffer-create
                       (generate-new-buffer-name buf2)))
          (erase-buffer)
          (insert-buffer-substring oldbuf start-2 end-2))

        (if (not (string-equal (buffer-substring-no-properties start-1 end-1)
                               (buffer-substring-no-properties start-2 end-2)))
            (po-ediff-buffers-exit-recursive buf1 buf2 oldbuf end-2)
          (message "Variants are equal; delete %s" buf1)
          (forward-line -1)
          (delete-region (point-min) (point)))))))

(defun po-subedit-abort ()
  "Exit the subedit buffer, merely discarding its contents."
  (interactive)
  (let* ((edit-buffer (current-buffer))
         (back-pointer po-subedit-back-pointer)
         (entry-marker (nth 0 back-pointer))
         (overlay-info (nth 2 back-pointer))
         (entry-buffer (marker-buffer entry-marker)))
    (if (null entry-buffer)
        (error (_"Corresponding PO buffer does not exist anymore"))
      (or (one-window-p) (delete-window))
      (switch-to-buffer entry-buffer)
      (goto-char entry-marker)
      (and overlay-info (po-dehighlight overlay-info))
      (kill-buffer edit-buffer)
      (setq po-edited-fields (delete back-pointer po-edited-fields)))))

(defun po-subedit-exit ()
  "Exit the subedit buffer, replacing the string in the PO buffer."
  (interactive)
  (goto-char (point-max))
  (skip-chars-backward " \t\n")
  (if (eq (preceding-char) ?<)
      (delete-region (1- (point)) (point-max)))
  (run-hooks 'po-subedit-exit-hook)
  (let ((string (buffer-string)))
    (po-subedit-abort)
    (po-find-span-of-entry)
    (cond ((= (point) po-start-of-msgid)
           (po-set-comment string)
           (po-redisplay))
          ((= (point) po-start-of-msgstr-form)
           (if (po-set-msgstr-form string)
               (progn
                 (po-maybe-delete-previous-untranslated)
                 (if (and po-auto-fuzzy-on-edit
                          (eq po-entry-type 'translated))
                     (progn
                       (po-decrease-type-counter)
                       (po-add-attribute "fuzzy")
                       (po-current-entry)
                       (po-increase-type-counter))))))
          (t (debug)))))

(defun po-edit-string (string type expand-tabs)
  "Prepare a pop up buffer for editing STRING, which is of a given TYPE.
TYPE may be 'comment or 'msgstr.  If EXPAND-TABS, expand tabs to spaces.
Run functions on po-subedit-mode-hook."
  (let ((marker (make-marker)))
    (set-marker marker (cond ((eq type 'comment) po-start-of-msgid)
                             ((eq type 'msgstr) po-start-of-msgstr-form)))
    (if (po-check-for-pending-edit marker)
        (let ((edit-buffer (generate-new-buffer
                            (concat "*" (buffer-name) "*")))
              (edit-coding buffer-file-coding-system)
              (buffer (current-buffer))
              overlay slot)
          (if (and (eq type 'msgstr) po-highlighting)
              ;; ;; Try showing all of msgid in the upper window while editing.
              ;; (goto-char (1- po-start-of-msgstr-block))
              ;; (recenter -1)
              (save-excursion
                (goto-char po-start-of-entry)
                (re-search-forward po-any-msgid-regexp nil t)
                (let ((end (1- (match-end 0))))
                  (goto-char (match-beginning 0))
                  (re-search-forward "msgid +" nil t)
                  (setq overlay (po-create-overlay))
                  (po-highlight overlay (point) end buffer))))
          (setq slot (list marker edit-buffer overlay)
                po-edited-fields (cons slot po-edited-fields))
          (pop-to-buffer edit-buffer)
          (text-mode)
          (set (make-local-variable 'po-subedit-back-pointer) slot)
          (set (make-local-variable 'indent-line-function)
               'indent-relative)
          (setq buffer-file-coding-system edit-coding)
          (setq local-abbrev-table po-mode-abbrev-table)
          (erase-buffer)
          (insert string "<")
          (goto-char (point-min))
          (and expand-tabs (setq indent-tabs-mode nil))
          (use-local-map po-subedit-mode-map)
          (if (fboundp 'easy-menu-define)
              (easy-menu-define po-subedit-mode-menu po-subedit-mode-map ""
                po-subedit-mode-menu-layout))
          (set-syntax-table po-subedit-mode-syntax-table)
          (run-hooks 'po-subedit-mode-hook)
          (message po-subedit-message)))))

(defun po-edit-comment ()
  "Use another window to edit the current translator comment."
  (interactive)
  (po-find-span-of-entry)
  (po-edit-string (po-get-comment nil) 'comment nil))

(defun po-edit-comment-and-ediff ()
  "Use `ediff' to edit the current translator comment.
This function calls `po-edit-msgstr' and `po-subedit-ediff'; for more info
read `po-subedit-ediff' documentation."
  (interactive)
  (po-edit-comment)
  (po-subedit-ediff))

(defun po-edit-msgstr ()
  "Use another window to edit the current msgstr."
  (interactive)
  (po-find-span-of-entry)
  (po-edit-string (if (and po-auto-edit-with-msgid
                           (eq po-entry-type 'untranslated))
                      (po-get-msgid)
                    (po-get-msgstr-form))
                  'msgstr
                  t))

(defun po-edit-msgstr-and-ediff ()
  "Use `ediff' to edit the current msgstr.
This function calls `po-edit-msgstr' and `po-subedit-ediff'; for more info
read `po-subedit-ediff' documentation."
  (interactive)
  (po-edit-msgstr)
  (po-subedit-ediff))

;;; String normalization and searching.

(defun po-normalize-old-style (explain)
  "Normalize old gettext style fields using K&R C multiline string syntax.
To minibuffer messages sent while normalizing, add the EXPLAIN string."
  (let ((here (point-marker))
        (counter 0)
        (buffer-read-only po-read-only))
    (goto-char (point-min))
    (message (_"Normalizing %d, %s") counter explain)
    (while (re-search-forward
            "\\(^#?[ \t]*msg\\(id\\|str\\)[ \t]*\"\\|[^\" \t][ \t]*\\)\\\\\n"
            nil t)
      (if (= (% counter 10) 0)
          (message (_"Normalizing %d, %s") counter explain))
      (replace-match "\\1\"\n\"" t nil)
      (setq counter (1+ counter)))
    (goto-char here)
    (message (_"Normalizing %d...done") counter)))

(defun po-normalize-field (field explain)
  "Normalize FIELD of all entries.  FIELD is either the symbol msgid or msgstr.
To minibuffer messages sent while normalizing, add the EXPLAIN string."
  (let ((here (point-marker))
        (counter 0))
    (goto-char (point-min))
    (while (re-search-forward po-any-msgstr-block-regexp nil t)
      (if (= (% counter 10) 0)
          (message (_"Normalizing %d, %s") counter explain))
      (goto-char (match-beginning 0))
      (po-find-span-of-entry)
      (cond ((eq field 'msgid) (po-set-msgid (po-get-msgid)))
            ((eq field 'msgstr) (po-set-msgstr-form (po-get-msgstr-form))))
      (goto-char po-end-of-entry)
      (setq counter (1+ counter)))
    (goto-char here)
    (message (_"Normalizing %d...done") counter)))

;; Normalize, but the British way! :-)
(defsubst po-normalise () (po-normalize))

(defun po-normalize ()
  "Normalize all entries in the PO file."
  (interactive)
  (po-normalize-old-style (_"pass 1/3"))
  ;; FIXME: This cannot work: t and nil are not msgid and msgstr.
  (po-normalize-field t (_"pass 2/3"))
  (po-normalize-field nil (_"pass 3/3"))
  ;; The last PO file entry has just been processed.
  (if (not (= po-end-of-entry (point-max)))
      (let ((buffer-read-only po-read-only))
        (kill-region po-end-of-entry (point-max))))
  ;; A bizarre format might have fooled the counters, so recompute
  ;; them to make sure their value is dependable.
  (po-compute-counters nil))

;;; Multiple PO files.

(defun po-show-auxiliary-list ()
  "Echo the current auxiliary list in the message area."
  (if po-auxiliary-list
      (let ((cursor po-auxiliary-cursor)
            string)
        (while cursor
          (setq string (concat string (if string " ") (car (car cursor)))
                cursor (cdr cursor)))
        (setq cursor po-auxiliary-list)
        (while (not (eq cursor po-auxiliary-cursor))
          (setq string (concat string (if string " ") (car (car cursor)))
                cursor (cdr cursor)))
        (message string))
    (message (_"No auxiliary files."))))

(defun po-consider-as-auxiliary ()
  "Add the current PO file to the list of auxiliary files."
  (interactive)
  (if (member (list buffer-file-name) po-auxiliary-list)
      nil
    (setq po-auxiliary-list
          (nconc po-auxiliary-list (list (list buffer-file-name))))
    (or po-auxiliary-cursor
        (setq po-auxiliary-cursor po-auxiliary-list)))
  (po-show-auxiliary-list))

(defun po-ignore-as-auxiliary ()
  "Delete the current PO file from the list of auxiliary files."
  (interactive)
  (setq po-auxiliary-list (delete (list buffer-file-name) po-auxiliary-list)
        po-auxiliary-cursor po-auxiliary-list)
  (po-show-auxiliary-list))

(defun po-seek-equivalent-translation (name string)
  "Search a PO file NAME for a 'msgid' STRING having a non-empty 'msgstr'.
STRING is the full quoted msgid field, including the 'msgid' keyword.  When
found, display the file over the current window, with the 'msgstr' field
possibly highlighted, the cursor at start of msgid, then return 't'.
Otherwise, move nothing, and just return 'nil'."
  (let ((current (current-buffer))
        (buffer (find-file-noselect name)))
    (set-buffer buffer)
    (let ((start (point))
          found)
      (goto-char (point-min))
      (while (and (not found) (search-forward string nil t))
        ;; Screen out longer 'msgid's.
        (if (looking-at "^msgstr ")
            (progn
              (po-find-span-of-entry)
              ;; Ignore an untranslated entry.
              (or (string-equal
                   (buffer-substring po-start-of-msgstr-block po-end-of-entry)
                   "msgstr \"\"\n")
                  (setq found t)))))
      (if found
          (progn
            (switch-to-buffer buffer)
            (po-find-span-of-entry)
            (if po-highlighting
                (progn
                  (goto-char po-start-of-entry)
                  (re-search-forward po-any-msgstr-block-regexp nil t)
                  (let ((end (1- (match-end 0))))
                    (goto-char (match-beginning 0))
                    (re-search-forward "msgstr +" nil t)
                    ;; Just "borrow" the marking overlay.
                    (po-highlight po-marking-overlay (point) end))))
            (goto-char po-start-of-msgid))
        (goto-char start)
        (po-find-span-of-entry)
        (set-buffer current))
      found)))

(defun po-cycle-auxiliary ()
  "Select the next auxiliary file having an entry with same 'msgid'."
  (interactive)
  (po-find-span-of-entry)
  (if po-auxiliary-list
      (let ((string (buffer-substring po-start-of-msgid
                                      po-start-of-msgstr-block))
            (cursor po-auxiliary-cursor)
            found name)
        (while (and (not found) cursor)
          (setq name (car (car cursor)))
          (if (and (not (string-equal buffer-file-name name))
                   (po-seek-equivalent-translation name string))
              (setq found t
                    po-auxiliary-cursor cursor))
          (setq cursor (cdr cursor)))
        (setq cursor po-auxiliary-list)
        (while (and (not found) cursor)
          (setq name (car (car cursor)))
          (if (and (not (string-equal buffer-file-name name))
                   (po-seek-equivalent-translation name string))
              (setq found t
                    po-auxiliary-cursor cursor))
          (setq cursor (cdr cursor)))
        (or found (message (_"No other translation found")))
        found)))

(defun po-subedit-cycle-auxiliary ()
  "Cycle auxiliary file, but from the translation edit buffer."
  (interactive)
  (let* ((entry-marker (nth 0 po-subedit-back-pointer))
         (entry-buffer (marker-buffer entry-marker))
         (buffer (current-buffer)))
    (pop-to-buffer entry-buffer)
    (po-cycle-auxiliary)
    (pop-to-buffer buffer)))

(defun po-select-auxiliary ()
  "Select one of the available auxiliary files and locate an equivalent entry.
If an entry having the same 'msgid' cannot be found, merely select the file
without moving its cursor."
  (interactive)
  (po-find-span-of-entry)
  (if po-auxiliary-list
      (let ((string
              (buffer-substring po-start-of-msgid po-start-of-msgstr-block))
            (name (car (assoc (completing-read (_"Which auxiliary file? ")
                                               po-auxiliary-list nil t)
                              po-auxiliary-list))))
        (po-consider-as-auxiliary)
        (or (po-seek-equivalent-translation name string)
            (find-file name)))))

;;; Original program sources as context.

(defun po-show-source-path ()
  "Echo the current source search path in the message area."
  (if po-search-path
      (let ((cursor po-search-path)
            string)
        (while cursor
          (setq string (concat string (if string " ") (car (car cursor)))
                cursor (cdr cursor)))
        (message string))
    (message (_"Empty source path."))))

(defun po-consider-source-path (directory)
  "Add a given DIRECTORY, requested interactively, to the source search path."
  (interactive "DDirectory for search path: ")
  (setq po-search-path (cons (list (if (string-match "/$" directory)
                                         directory
                                       (concat directory "/")))
                             po-search-path))
  (setq po-reference-check 0)
  (po-show-source-path))

(defun po-ignore-source-path ()
  "Delete a directory, selected with completion, from the source search path."
  (interactive)
  (setq po-search-path
        (delete (list (completing-read (_"Directory to remove? ")
                                       po-search-path nil t))
                po-search-path))
  (setq po-reference-check 0)
  (po-show-source-path))

(defun po-ensure-source-references ()
  "Extract all references into a list, with paths resolved, if necessary."
  (po-find-span-of-entry)
  (if (= po-start-of-entry po-reference-check)
      nil
    (setq po-reference-alist nil)
    (save-excursion
      (goto-char po-start-of-entry)
      (if (re-search-forward "^#:" po-start-of-msgid t)
          (let (current name line path file)
            (while (looking-at "\\(\n#:\\)? *\\([^: ]*\\):\\([0-9]+\\)")
              (goto-char (match-end 0))
              (setq name (po-match-string 2)
                    line (po-match-string 3)
                    path po-search-path)
              (if (string-equal name "")
                  nil
                (while (and (not (file-exists-p
                                  (setq file (concat (car (car path)) name))))
                            path)
                  (setq path (cdr path)))
                (setq current (and path file)))
              (if current
                  (setq po-reference-alist
                        (cons (list (concat current ":" line)
                                    current
                                    (string-to-number line))
                              po-reference-alist)))))))
    (setq po-reference-alist (nreverse po-reference-alist)
          po-reference-cursor po-reference-alist
          po-reference-check po-start-of-entry)))

(defun po-show-source-context (triplet)
  "Show the source context given a TRIPLET which is (PROMPT FILE LINE)."
  (find-file-other-window (car (cdr triplet)))
  (goto-line (car (cdr (cdr triplet))))
  (other-window 1)
  (let ((maximum 0)
        position
        (cursor po-reference-alist))
    (while (not (eq triplet (car cursor)))
      (setq maximum (1+ maximum)
            cursor (cdr cursor)))
    (setq position (1+ maximum)
          po-reference-cursor cursor)
    (while cursor
      (setq maximum (1+ maximum)
            cursor (cdr cursor)))
    (message (_"Displaying %d/%d: \"%s\"") position maximum (car triplet))))

(defun po-cycle-source-reference ()
  "Display some source context for the current entry.
If the command is repeated many times in a row, cycle through contexts."
  (interactive)
  (po-ensure-source-references)
  (if po-reference-cursor
      (po-show-source-context
       (car (if (eq last-command 'po-cycle-source-reference)
                (or (cdr po-reference-cursor) po-reference-alist)
              po-reference-cursor)))
    (error (_"No resolved source references"))))

(defun po-select-source-reference ()
  "Select one of the available source contexts for the current entry."
  (interactive)
  (po-ensure-source-references)
  (if po-reference-alist
      (po-show-source-context
       (assoc
        (completing-read (_"Which source context? ") po-reference-alist nil t)
        po-reference-alist))
    (error (_"No resolved source references"))))

;;; String marking in program sources, through TAGS table.

;; Globally defined within tags.el.
(defvar tags-loop-operate)
(defvar tags-loop-scan)

;; Locally set in each program source buffer.
(defvar po-find-string-function)
(defvar po-mark-string-function)

;; Dynamically set within po-tags-search for po-tags-loop-operate.
(defvar po-current-po-buffer)
(defvar po-current-po-keywords)

(defun po-tags-search (restart)
  "Find an unmarked translatable string through all files in tags table.
Disregard some simple strings which are most probably non-translatable.
With prefix argument, restart search at first file."
  (interactive "P")
  (require 'etags)
  ;; Ensure there is no highlighting, in case the search fails.
  (if po-highlighting
      (po-dehighlight po-marking-overlay))
  (setq po-string-contents nil)
  ;; Search for a string which might later be marked for translation.
  (let ((po-current-po-buffer (current-buffer))
        (po-current-po-keywords po-keywords))
    (pop-to-buffer po-string-buffer)
    (if (and (not restart)
             (eq (car tags-loop-operate) 'po-tags-loop-operate))
        ;; Continue last po-tags-search.
        (tags-loop-continue nil)
      ;; Start or restart po-tags-search all over.
      (setq tags-loop-scan '(po-tags-loop-scan)
            tags-loop-operate '(po-tags-loop-operate))
      (tags-loop-continue t))
    (select-window (get-buffer-window po-current-po-buffer)))
  (if po-string-contents
      (let ((window (selected-window))
            (buffer po-string-buffer)
            (start po-string-start)
            (end po-string-end))
        ;; Try to fit the string in the displayed part of its window.
        (select-window (get-buffer-window buffer))
        (goto-char start)
        (or (pos-visible-in-window-p start)
            (recenter '(nil)))
        (if (pos-visible-in-window-p end)
            (goto-char end)
          (goto-char end)
          (recenter -1))
        (select-window window)
        ;; Highlight the string as found.
        (and po-highlighting
             (po-highlight po-marking-overlay start end buffer)))))

(defun po-tags-loop-scan ()
  "Decide if the current buffer is still interesting for PO mode strings."
  ;; We have little choice, here.  The major mode is needed to dispatch to the
  ;; proper scanner, so we declare all files as interesting, to force Emacs
  ;; tags module to revisit files fully.  po-tags-loop-operate sets point at
  ;; end of buffer when it is done with a file.
  (not (eobp)))

(defun po-tags-loop-operate ()
  "Find an acceptable tag in the current buffer, according to mode.
Disregard some simple strings which are most probably non-translatable."
  (po-preset-string-functions)
  (let ((continue t)
        data)
    (while continue
      (setq data (apply po-find-string-function po-current-po-keywords nil))
      (if data
          ;; Push the string just found into a work buffer for study.
          (po-with-temp-buffer
           (insert (nth 0 data))
           (goto-char (point-min))
           ;; Accept if at least three letters in a row.
           (if (re-search-forward "[A-Za-z][A-Za-z][A-Za-z]" nil t)
               (setq continue nil)
             ;; Disregard if single letters or no letters at all.
             (if (re-search-forward "[A-Za-z][A-Za-z]" nil t)
                 ;; Here, we have two letters in a row, but never more.
                 ;; Accept only if more letters than punctuations.
                 (let ((total (buffer-size)))
                   (goto-char (point-min))
                   (while (re-search-forward "[A-Za-z]+" nil t)
                     (replace-match "" t t))
                   (if (< (* 2 (buffer-size)) total)
                       (setq continue nil))))))
        ;; No string left in this buffer.
        (setq continue nil)))
    (if data
        ;; Save information for marking functions.
        (let ((buffer (current-buffer)))
          (save-excursion
            (set-buffer po-current-po-buffer)
            (setq po-string-contents (nth 0 data)
                  po-string-buffer buffer
                  po-string-start (nth 1 data)
                  po-string-end (nth 2 data))))
      (goto-char (point-max)))
    ;; If nothing was found, trigger scanning of next file.
    (not data)))

(defun po-mark-found-string (keyword)
  "Mark last found string in program sources as translatable, using KEYWORD."
  (if (not po-string-contents)
    (error (_"No such string")))
  (and po-highlighting (po-dehighlight po-marking-overlay))
  (let ((contents po-string-contents)
        (buffer po-string-buffer)
        (start po-string-start)
        (end po-string-end)
        line string)
    ;; Mark string in program sources.
    (save-excursion
      (set-buffer buffer)
      (setq line (count-lines (point-min) start))
      (apply po-mark-string-function start end keyword nil))
    ;; Add PO file entry.
    (let ((buffer-read-only po-read-only))
      (goto-char (point-max))
      (insert "\n" (format "#: %s:%d\n"
                           (buffer-file-name po-string-buffer)
                           line))
      (save-excursion
        (insert (po-eval-requoted contents "msgid" nil) "msgstr \"\"\n"))
      (setq po-untranslated-counter (1+ po-untranslated-counter))
      (po-update-mode-line-string))
    (setq po-string-contents nil)))

(defun po-mark-translatable ()
  "Mark last found string in program sources as translatable, using '_'."
  (interactive)
  (po-mark-found-string "_"))

(defun po-select-mark-and-mark (arg)
  "Mark last found string in program sources as translatable, ask for keyword,
using completion.  With prefix argument, just ask the name of a preferred
keyword for subsequent commands, also added to possible completions."
  (interactive "P")
  (if arg
      (let ((keyword (list (read-from-minibuffer (_"Keyword: ")))))
        (setq po-keywords (cons keyword (delete keyword po-keywords))))
    (or po-string-contents (error (_"No such string")))
    (let* ((default (car (car po-keywords)))
           (keyword (completing-read (format (_"Mark with keyword? [%s] ")
                                             default)
                                     po-keywords nil t )))
      (if (string-equal keyword "") (setq keyword default))
      (po-mark-found-string keyword))))

;;; Unknown mode specifics.

(defun po-preset-string-functions ()
  "Preset FIND-STRING-FUNCTION and MARK-STRING-FUNCTION according to mode.
These variables are locally set in source buffer only when not already bound."
  (let ((pair (cond ((equal major-mode 'awk-mode)
                     '(po-find-awk-string . po-mark-awk-string))
                    ((member major-mode '(c-mode c++-mode))
                     '(po-find-c-string . po-mark-c-string))
                    ((equal major-mode 'emacs-lisp-mode)
                     '(po-find-emacs-lisp-string . po-mark-emacs-lisp-string))
                    ((equal major-mode 'python-mode)
                     '(po-find-python-string . po-mark-python-string))
                    ((and (equal major-mode 'sh-mode)
                          (string-equal mode-line-process "[bash]"))
                     '(po-find-bash-string . po-mark-bash-string))
                    (t '(po-find-unknown-string . po-mark-unknown-string)))))
    (or (boundp 'po-find-string-function)
        (set (make-local-variable 'po-find-string-function) (car pair)))
    (or (boundp 'po-mark-string-function)
        (set (make-local-variable 'po-mark-string-function) (cdr pair)))))

(defun po-find-unknown-string (keywords)
  "Dummy function to skip over a file, finding no string in it."
  nil)

(defun po-mark-unknown-string (start end keyword)
  "Dummy function to mark a given string.  May not be called."
  (error (_"Dummy function called")))

;;; Awk mode specifics.

(defun po-find-awk-string (keywords)
  "Find the next Awk string, excluding those marked by any of KEYWORDS.
Return (CONTENTS START END) for the found string, or nil if none found."
  (let (start end)
    (while (and (not start)
                (re-search-forward "[#/\"]" nil t))
      (cond ((= (preceding-char) ?#)
             ;; Disregard comments.
             (or (search-forward "\n" nil t)
                 (goto-char (point-max))))
            ((= (preceding-char) ?/)
             ;; Skip regular expressions.
             (while (not (= (following-char) ?/))
               (skip-chars-forward "^/\\\\")
               (if (= (following-char) ?\\) (forward-char 2)))
             (forward-char 1))
            ;; Else find the end of the string.
            (t (setq start (1- (point)))
               (while (not (= (following-char) ?\"))
                 (skip-chars-forward "^\"\\\\")
                 (if (= (following-char) ?\\) (forward-char 2)))
               (forward-char 1)
               (setq end (point))
               ;; Check before string either for underline, or for keyword
               ;; and opening parenthesis.
               (save-excursion
                 (goto-char start)
                 (cond ((= (preceding-char) ?_)
                        ;; Disregard already marked strings.
                        (setq start nil
                              end nil))
                       ((= (preceding-char) ?\()
                        (backward-char 1)
                        (let ((end-keyword (point)))
                          (skip-chars-backward "_A-Za-z0-9")
                          (if (member (list (po-buffer-substring
                                             (point) end-keyword))
                                      keywords)
                              ;; Disregard already marked strings.
                              (setq start nil
                                    end nil)))))))))
    (and start end
         (list (po-extract-unquoted (current-buffer) start end) start end))))

(defun po-mark-awk-string (start end keyword)
  "Mark the Awk string, from START to END, with KEYWORD.
Leave point after marked string."
  (if (string-equal keyword "_")
      (progn
        (goto-char start)
        (insert "_")
        (goto-char (1+ end)))
    (goto-char end)
    (insert ")")
    (save-excursion
      (goto-char start)
      (insert keyword "("))))

;;; Bash mode specifics.

(defun po-find-bash-string (keywords)
  "Find the next unmarked Bash string.  KEYWORDS are merely ignored.
Return (CONTENTS START END) for the found string, or nil if none found."
  (let (start end)
    (while (and (not start)
                (re-search-forward "[#'\"]" nil t))
      (cond ((= (preceding-char) ?#)
             ;; Disregard comments.
             (or (search-forward "\n" nil t)
                 (goto-char (point-max))))
            ((= (preceding-char) ?')
             ;; Skip single quoted strings.
             (while (not (= (following-char) ?'))
               (skip-chars-forward "^'\\\\")
               (if (= (following-char) ?\\) (forward-char 2)))
             (forward-char 1))
            ;; Else find the end of the double quoted string.
            (t (setq start (1- (point)))
               (while (not (= (following-char) ?\"))
                 (skip-chars-forward "^\"\\\\")
                 (if (= (following-char) ?\\) (forward-char 2)))
               (forward-char 1)
               (setq end (point))
               ;; Check before string for dollar sign.
               (save-excursion
                 (goto-char start)
                 (if (= (preceding-char) ?$)
                     ;; Disregard already marked strings.
                     (setq start nil
                           end nil))))))
    (and start end
         (list (po-extract-unquoted (current-buffer) start end) start end))))

(defun po-mark-bash-string (start end keyword)
  "Mark the Bash string, from START to END, with '$'.  KEYWORD is ignored.
Leave point after marked string."
  (goto-char start)
  (insert "$")
  (goto-char (1+ end)))

;;; C or C++ mode specifics.

;;; A few long string cases (submitted by Ben Pfaff).

;; #define string "This is a long string " \
;; "that is continued across several lines " \
;; "in a macro in order to test \\ quoting\\" \
;; "\\ with goofy strings.\\"

;; char *x = "This is just an ordinary string "
;; "continued across several lines without needing "
;; "to use \\ characters at end-of-line.";

;; char *y = "Here is a string continued across \
;; several lines in the manner that was sanctioned \
;; in K&R C compilers and still works today, \
;; even though the method used above is more esthetic.";

;;; End of long string cases.

(defun po-find-c-string (keywords)
  "Find the next C string, excluding those marked by any of KEYWORDS.
Returns (CONTENTS START END) for the found string, or nil if none found."
  (let (start end)
    (while (and (not start)
                (re-search-forward "\\([\"']\\|/\\*\\|//\\)" nil t))
      (cond ((= (preceding-char) ?*)
             ;; Disregard comments.
             (search-forward "*/"))
            ((= (preceding-char) ?/)
             ;; Disregard C++ comments.
             (end-of-line)
             (forward-char 1))
            ((= (preceding-char) ?\')
             ;; Disregard character constants.
             (forward-char (if (= (following-char) ?\\) 3 2)))
            ((save-excursion
               (beginning-of-line)
               (looking-at "^# *\\(include\\|line\\)"))
             ;; Disregard lines being #include or #line directives.
             (end-of-line))
            ;; Else, find the end of the (possibly concatenated) string.
            (t (setq start (1- (point))
                     end nil)
               (while (not end)
                 (cond ((= (following-char) ?\")
                        (if (looking-at "\"[ \t\n\\\\]*\"")
                            (goto-char (match-end 0))
                          (forward-char 1)
                          (setq end (point))))
                       ((= (following-char) ?\\) (forward-char 2))
                       (t (skip-chars-forward "^\"\\\\"))))
               ;; Check before string for keyword and opening parenthesis.
               (goto-char start)
               (skip-chars-backward " \n\t")
               (if (= (preceding-char) ?\()
                   (progn
                     (backward-char 1)
                     (skip-chars-backward " \n\t")
                     (let ((end-keyword (point)))
                       (skip-chars-backward "_A-Za-z0-9")
                       (if (member (list (po-buffer-substring (point)
                                                              end-keyword))
                                   keywords)
                           ;; Disregard already marked strings.
                           (progn
                             (goto-char end)
                             (setq start nil
                                   end nil))
                         ;; String found.  Prepare to resume search.
                         (goto-char end))))
                 ;; String found.  Prepare to resume search.
                 (goto-char end)))))
    ;; Return the found string, if any.
    (and start end
         (list (po-extract-unquoted (current-buffer) start end) start end))))

(defun po-mark-c-string (start end keyword)
  "Mark the C string, from START to END, with KEYWORD.
Leave point after marked string."
  (goto-char end)
  (insert ")")
  (save-excursion
    (goto-char start)
    (insert keyword)
    (or (string-equal keyword "_") (insert " "))
    (insert "(")))

;;; Emacs LISP mode specifics.

(defun po-find-emacs-lisp-string (keywords)
  "Find the next Emacs LISP string, excluding those marked by any of KEYWORDS.
Returns (CONTENTS START END) for the found string, or nil if none found."
  (let (start end)
    (while (and (not start)
                (re-search-forward "[;\"?]" nil t))
      (cond ((= (preceding-char) ?\;)
             ;; Disregard comments.
             (search-forward "\n"))
            ((= (preceding-char) ?\?)
             ;; Disregard character constants.
             (forward-char (if (= (following-char) ?\\) 2 1)))
            ;; Else, find the end of the string.
            (t (setq start (1- (point)))
               (while (not (= (following-char) ?\"))
                 (skip-chars-forward "^\"\\\\")
                 (if (= (following-char) ?\\) (forward-char 2)))
               (forward-char 1)
               (setq end (point))
               ;; Check before string for keyword and opening parenthesis.
               (goto-char start)
               (skip-chars-backward " \n\t")
               (let ((end-keyword (point)))
                 (skip-chars-backward "-_A-Za-z0-9")
                 (if (and (= (preceding-char) ?\()
                          (member (list (po-buffer-substring (point)
                                                             end-keyword))
                                  keywords))
                     ;; Disregard already marked strings.
                     (progn
                       (goto-char end)
                       (setq start nil
                             end nil)))))))
    ;; Return the found string, if any.
    (and start end
         (list (po-extract-unquoted (current-buffer) start end) start end))))

(defun po-mark-emacs-lisp-string (start end keyword)
  "Mark the Emacs LISP string, from START to END, with KEYWORD.
Leave point after marked string."
  (goto-char end)
  (insert ")")
  (save-excursion
    (goto-char start)
    (insert "(" keyword)
    (or (string-equal keyword "_") (insert " "))))

;;; Python mode specifics.

(defun po-find-python-string (keywords)
  "Find the next Python string, excluding those marked by any of KEYWORDS.
Also disregard strings when preceded by an empty string of the other type.
Returns (CONTENTS START END) for the found string, or nil if none found."
  (let (contents start end)
    (while (and (not contents)
                (re-search-forward "[#\"']" nil t))
      (forward-char -1)
      (cond ((= (following-char) ?\#)
             ;; Disregard comments.
             (search-forward "\n"))
            ((looking-at "\"\"'")
             ;; Quintuple-quoted string
             (po-skip-over-python-string))
            ((looking-at "''\"")
             ;; Quadruple-quoted string
             (po-skip-over-python-string))
            (t
             ;; Simple-, double-, triple- or sextuple-quoted string.
             (if (memq (preceding-char) '(?r ?R))
                 (forward-char -1))
             (setq start (point)
                   contents (po-skip-over-python-string)
                   end (point))
             (goto-char start)
             (skip-chars-backward " \n\t")
             (cond ((= (preceding-char) ?\[)
                    ;; Disregard a string used as a dictionary index.
                    (setq contents nil))
                   ((= (preceding-char) ?\()
                    ;; Isolate the keyword which precedes string.
                    (backward-char 1)
                    (skip-chars-backward " \n\t")
                    (let ((end-keyword (point)))
                      (skip-chars-backward "_A-Za-z0-9")
                      (if (member (list (po-buffer-substring (point)
                                                             end-keyword))
                                  keywords)
                          ;; Disregard already marked strings.
                          (setq contents nil)))))
             (goto-char end))))
    ;; Return the found string, if any.
    (and contents (list contents start end))))

(defun po-skip-over-python-string ()
  "Skip over a Python string, possibly made up of many concatenated parts.
Leave point after string.  Return unquoted overall string contents."
  (let ((continue t)
        (contents "")
        raw start end resume)
    (while continue
      (skip-chars-forward " \t\n")      ; whitespace
      (cond ((= (following-char) ?#)    ; comment
             (setq start nil)
             (search-forward "\n"))
            ((looking-at "\\\n")        ; escaped newline
             (setq start nil)
             (forward-char 2))
            ((looking-at "[rR]?\"\"\"") ; sextuple-quoted string
             (setq raw (memq (following-char) '(?r ?R))
                   start (match-end 0))
             (goto-char start)
             (search-forward "\"\"\"")
             (setq resume (point)
                   end (- resume 3)))
            ((looking-at "[rr]?'''")    ; triple-quoted string
             (setq raw (memq (following-char) '(?r ?R))
                   start (match-end 0))
             (goto-char start)
             (search-forward "'''")
             (setq resume (point)
                   end (- resume 3)))
            ((looking-at "[rR]?\"")     ; double-quoted string
             (setq raw (memq (following-char) '(?r ?R))
                   start (match-end 0))
             (goto-char start)
             (while (not (memq (following-char) '(0 ?\")))
               (skip-chars-forward "^\"\\\\")
               (if (= (following-char) ?\\) (forward-char 2)))
             (if (eobp)
                 (setq contents nil
                       start nil)
               (setq end (point))
               (forward-char 1))
             (setq resume (point)))
            ((looking-at "[rR]?'")      ; single-quoted string
             (setq raw (memq (following-char) '(?r ?R))
                   start (match-end 0))
             (goto-char start)
             (while (not (memq (following-char) '(0 ?\')))
               (skip-chars-forward "^'\\\\")
               (if (= (following-char) ?\\) (forward-char 2)))
             (if (eobp)
                 (setq contents nil
                       start nil)
               (setq end (point))
               (forward-char 1))
             (setq resume (point)))
            (t                          ; no string anymore
             (setq start nil
                   continue nil)))
      (if start
          (setq contents (concat contents
                                 (if raw
                                     (buffer-substring start end)
                                   (po-extract-part-unquoted (current-buffer)
                                                             start end))))))
    (goto-char resume)
    contents))

(defun po-mark-python-string (start end keyword)
  "Mark the Python string, from START to END, with KEYWORD.
If KEYWORD is '.', prefix the string with an empty string of the other type.
Leave point after marked string."
  (cond ((string-equal keyword ".")
         (goto-char end)
         (save-excursion
           (goto-char start)
           (insert (cond ((= (following-char) ?\') "\"\"")
                         ((= (following-char) ?\") "''")
                         (t "??")))))
        (t (goto-char end)
           (insert ")")
           (save-excursion
             (goto-char start)
             (insert keyword "(")))))

;;; Miscellaneous features.

(defun po-help ()
  "Provide an help window for PO mode."
  (interactive)
  (po-with-temp-buffer
   (insert po-help-display-string)
   (goto-char (point-min))
   (save-window-excursion
     (switch-to-buffer (current-buffer))
     (delete-other-windows)
     (message (_"Type any character to continue"))
     (po-read-event))))

(defun po-undo ()
  "Undo the last change to the PO file."
  (interactive)
  (let ((buffer-read-only po-read-only))
    (undo))
  (po-compute-counters nil))

(defun po-statistics ()
  "Say how many entries in each category, and the current position."
  (interactive)
  (po-compute-counters t))

(defun po-validate ()
  "Use 'msgfmt' for validating the current PO file contents."
  (interactive)
  ;; The 'compile' subsystem is autoloaded through a call to (compile ...).
  ;; We need to initialize it outside of any binding. Without this statement,
  ;; all defcustoms and defvars of compile.el would be undone when the let*
  ;; terminates.
  (require 'compile)
  (let* ((dev-null
          (cond ((boundp 'null-device) null-device) ; since Emacs 20.3
                ((memq system-type '(windows-nt windows-95)) "NUL")
                (t "/dev/null")))
         (output
          (if po-keep-mo-file
              (concat (file-name-sans-extension buffer-file-name) ".mo")
            dev-null))
         (compilation-buffer-name-function
          (function (lambda (mode-name)
                      (concat "*" mode-name " validation*"))))
         (compile-command (concat po-msgfmt-program
                                  " --statistics -c -v -o "
                                  (shell-quote-argument output) " "
                                  (shell-quote-argument buffer-file-name))))
    (po-msgfmt-version-check)
    (compile compile-command)))

(defvar po-msgfmt-version-checked nil)
(defun po-msgfmt-version-check ()
  "'msgfmt' from GNU gettext 0.10.36 or greater is required."
  (po-with-temp-buffer
    (or
     ;; Don't bother checking again.
     po-msgfmt-version-checked

     (and
      ;; Make sure 'msgfmt' is available.
      (condition-case nil
          (call-process po-msgfmt-program
                        nil t nil "--verbose" "--version")
        (file-error nil))

      ;; Make sure there's a version number in the output:
      ;; 0.11 or 0.10.36 or 0.19.5.1 or 0.11-pre1 or 0.16.2-pre1
      (progn (goto-char (point-min))
             (or (looking-at ".* \\([0-9]+\\)\\.\\([0-9]+\\)$")
                 (looking-at ".* \\([0-9]+\\)\\.\\([0-9]+\\)\\.\\([0-9]+\\)$")
                 (looking-at ".* \\([0-9]+\\)\\.\\([0-9]+\\)\\.\\([0-9]+\\)\\.\\([0-9]+\\)$")
                 (looking-at ".* \\([0-9]+\\)\\.\\([0-9]+\\)[-_A-Za-z0-9]+$")
                 (looking-at ".* \\([0-9]+\\)\\.\\([0-9]+\\)\\.\\([0-9]+\\)[-_A-Za-z0-9]+$")))

      ;; Make sure the version is recent enough.
      (>= (string-to-number
           (format "%d%03d%03d"
                   (string-to-number (match-string 1))
                   (string-to-number (match-string 2))
                   (string-to-number (or (match-string 3) "0"))))
          010036)

      ;; Remember the outcome.
      (setq po-msgfmt-version-checked t))

     (error (_"'msgfmt' from GNU gettext 0.10.36 or greater is required")))))

(defun po-guess-archive-name ()
  "Return the ideal file name for this PO file in the central archives."
  (let ((filename (file-name-nondirectory buffer-file-name))
        start-of-header end-of-header package version team)
    (save-excursion
      ;; Find the PO file header entry.
      (goto-char (point-min))
      (re-search-forward po-any-msgstr-block-regexp)
      (setq start-of-header (match-beginning 0)
            end-of-header (match-end 0))
      ;; Get the package and version.
      (goto-char start-of-header)
      (if (re-search-forward "\n\
\"Project-Id-Version: \\(GNU \\|Free \\)?\\([^\n ]+\\) \\([^\n ]+\\)\\\\n\"$"
           end-of-header t)
          (setq package (po-match-string 2)
                version (po-match-string 3)))
      (if (or (not package) (string-equal package "PACKAGE")
              (not version) (string-equal version "VERSION"))
          (error (_"Project-Id-Version field does not have a proper value")))
      ;; File name version and Project-Id-Version must match
      (cond (;; A `filename' w/o package and version info at all
             (string-match "^[^\\.]*\\.po\\'" filename))
            (;; TP Robot compatible `filename': PACKAGE-VERSION.LL.po
             (string-match (concat (regexp-quote package)
                                   "-\\(.*\\)\\.[^\\.]*\\.po\\'") filename)
             (if (not (equal version (po-match-string 1 filename)))
                 (error (_"\
Version mismatch: file name: %s; header: %s.\n\
Adjust Project-Id-Version field to match file name and try again")
                        (po-match-string 1 filename) version))))
      ;; Get the team.
      (if (stringp po-team-name-to-code)
          (setq team po-team-name-to-code)
        (goto-char start-of-header)
        (if (re-search-forward "\n\
\"Language-Team: \\([^ ].*[^ ]\\) <.+@.+>\\\\n\"$"
                               end-of-header t)
            (let ((name (po-match-string 1)))
              (if name
                  (let ((pair (assoc name po-team-name-to-code)))
                    (if pair
                        (setq team (cdr pair))
                      (setq team (read-string (format "\
Team name '%s' unknown.  What is the team code? "
                                                      name)))))))))
      (if (or (not team) (string-equal team "LL"))
          (error (_"Language-Team field does not have a proper value")))
      ;; Compose the name.
      (concat package "-" version "." team ".po"))))

(defun po-guess-team-address ()
  "Return the team address related to this PO file."
  (let (team)
    (save-excursion
      (goto-char (point-min))
      (re-search-forward po-any-msgstr-block-regexp)
      (goto-char (match-beginning 0))
      (if (re-search-forward
           "\n\"Language-Team: +\\(.*<\\(.*\\)@.*>\\)\\\\n\"$"
           (match-end 0) t)
          (setq team (po-match-string 2)))
      (if (or (not team) (string-equal team "LL"))
          (error (_"Language-Team field does not have a proper value")))
      (po-match-string 1))))

(defun po-send-mail ()
  "Start composing a letter, possibly including the current PO file."
  (interactive)
  (let* ((team-flag (y-or-n-p
                     (_"\
Write to your team?  ('n' if writing to the Translation Project robot) ")))
         (address (if team-flag
                      (po-guess-team-address)
                    po-translation-project-address)))
    (if (not (y-or-n-p (_"Include current PO file in mail? ")))
        (apply po-compose-mail-function address
               (read-string (_"Subject? ")) nil)
      (if (buffer-modified-p)
          (error (_"The file is not even saved, you did not validate it.")))
      (if (and (y-or-n-p (_"You validated ('V') this file, didn't you? "))
               (or (zerop po-untranslated-counter)
                   (y-or-n-p
                    (format (_"%d entries are untranslated, include anyway? ")
                            po-untranslated-counter)))
               (or (zerop po-fuzzy-counter)
                   (y-or-n-p
                    (format (_"%d entries are still fuzzy, include anyway? ")
                            po-fuzzy-counter)))
               (or (zerop po-obsolete-counter)
                   (y-or-n-p
                    (format (_"%d entries are obsolete, include anyway? ")
                            po-obsolete-counter))))
          (let ((buffer (current-buffer))
                (name (po-guess-archive-name))
                (transient-mark-mode nil)
                (coding-system-for-read buffer-file-coding-system)
                (coding-system-for-write buffer-file-coding-system))
            (apply po-compose-mail-function address
                   (if team-flag
                       (read-string (_"Subject? "))
                     (format "%s %s" po-translation-project-mail-label name))
                   nil)
            (goto-char (point-min))
            (re-search-forward
             (concat "^" (regexp-quote mail-header-separator) "\n"))
            (save-excursion
              (save-restriction
                (narrow-to-region (point) (point))
                (insert-buffer-substring buffer)
                (shell-command-on-region
                 (point-min) (point-max)
                 (concat po-gzip-uuencode-command " " name ".gz") t t)))))))
  (message ""))

(defun po-confirm-and-quit ()
  "Confirm if quit should be attempted and then, do it.
This is a failsafe.  Confirmation is asked if only the real quit would not."
  (interactive)
  (if (po-check-all-pending-edits)
      (progn
        (if (or (buffer-modified-p)
                (> po-untranslated-counter 0)
                (> po-fuzzy-counter 0)
                (> po-obsolete-counter 0)
                (y-or-n-p (_"Really quit editing this PO file? ")))
            (po-quit))
        (message ""))))

(defun po-quit ()
  "Save the PO file and kill buffer.
However, offer validation if appropriate and ask confirmation if untranslated
strings remain."
  (interactive)
  (if (po-check-all-pending-edits)
      (let ((quit t))
        ;; Offer validation of newly modified entries.
        (if (and (buffer-modified-p)
                 (not (y-or-n-p
                       (_"File was modified; skip validation step? "))))
            (progn
              (message "")
              (po-validate)
              ;; If we knew that the validation was all successful, we should
              ;; just quit.  But since we do not know yet, as the validation
              ;; might be asynchronous with PO mode commands, the safest is to
              ;; stay within PO mode, even if this implies that another
              ;; 'po-quit' command will be later required to exit for true.
              (setq quit nil)))
        ;; Offer to work on untranslated entries.
        (if (and quit
                 (or (> po-untranslated-counter 0)
                     (> po-fuzzy-counter 0)
                     (> po-obsolete-counter 0))
                 (not (y-or-n-p
                       (_"Unprocessed entries remain; quit anyway? "))))
            (progn
              (setq quit nil)
              (po-auto-select-entry)))
        ;; Clear message area.
        (message "")
        ;; Or else, kill buffers and quit for true.
        (if quit
            (progn
              (save-buffer)
              (kill-buffer (current-buffer)))))))

;;;###autoload (add-to-list 'auto-mode-alist '("\\.po[tx]?\\'\\|\\.po\\." . po-mode))
;;;###autoload (modify-coding-system-alist 'file "\\.po[tx]?\\'\\|\\.po\\." 'po-find-file-coding-system)

(provide 'po-mode)

;; Hey Emacs!
;; Local Variables:
;; indent-tabs-mode: nil
;; coding: utf-8
;; End:

;;; po-mode.el ends here
