((nil . ((show-trailing-whitespace . t)))
 (prog-mode
  (indent-tabs-mode . nil)
  (tab-width . 8)
  (c-basic-offset . 3)
  (c-file-style . "stroustrup")
  (fill-column . 78)
  (eval . (progn
	    (c-set-offset 'case-label '0)
	    (c-set-offset 'innamespace '0)
	    (c-set-offset 'inline-open '0)))
  (whitespace-style face indentation)
  (whitespace-line-column . 79)
  (eval ignore-errors
        (require 'whitespace)
        (whitespace-mode 1)))
 (makefile-mode (indent-tabs-mode . t))
 )
