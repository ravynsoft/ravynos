/* GNU gettext for Java
 * Copyright (C) 2001, 2007 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

package gnu.gettext;

import java.lang.reflect.*;
import java.util.*;

/**
 * This class implements the main GNU libintl functions in Java.
 * <P>
 * Using the GNU gettext approach, compiled message catalogs are normal
 * Java ResourceBundle classes and are thus interoperable with standard
 * ResourceBundle based code.
 * <P>
 * The main differences between the Sun ResourceBundle approach and the
 * GNU gettext approach are:
 * <UL>
 *   <LI>In the Sun approach, the keys are abstract textual shortcuts.
 *       In the GNU gettext approach, the keys are the English/ASCII version
 *       of the messages.
 *   <LI>In the Sun approach, the translation files are called
 *       "<VAR>Resource</VAR>_<VAR>locale</VAR>.properties" and have non-ASCII
 *       characters encoded in the Java
 *       <CODE>\</CODE><CODE>u<VAR>nnnn</VAR></CODE> syntax. Very few editors
 *       can natively display international characters in this format. In the
 *       GNU gettext approach, the translation files are called
 *       "<VAR>Resource</VAR>.<VAR>locale</VAR>.po"
 *       and are in the encoding the translator has chosen. Many editors
 *       can be used. There are at least three GUI translating tools
 *       (Emacs PO mode, KDE KBabel, GNOME gtranslator).
 *   <LI>In the Sun approach, the function
 *       <CODE>ResourceBundle.getString</CODE> throws a
 *       <CODE>MissingResourceException</CODE> when no translation is found.
 *       In the GNU gettext approach, the <CODE>gettext</CODE> function
 *       returns the (English) message key in that case.
 *   <LI>In the Sun approach, there is no support for plural handling.
 *       Even the most elaborate MessageFormat strings cannot provide decent
 *       plural handling. In the GNU gettext approach, we have the
 *       <CODE>ngettext</CODE> function.
 * </UL>
 * <P>
 * To compile GNU gettext message catalogs into Java ResourceBundle classes,
 * the <CODE>msgfmt</CODE> program can be used.
 *
 * @author Bruno Haible
 */
public abstract class GettextResource extends ResourceBundle {

  public static boolean verbose = false;

  /**
   * Like gettext(catalog,msgid), except that it returns <CODE>null</CODE>
   * when no translation was found.
   */
  private static String gettextnull (ResourceBundle catalog, String msgid) {
    try {
      return (String)catalog.getObject(msgid);
    } catch (MissingResourceException e) {
      return null;
    }
  }

  /**
   * Returns the translation of <VAR>msgid</VAR>.
   * @param catalog a ResourceBundle
   * @param msgid the key string to be translated, an ASCII string
   * @return the translation of <VAR>msgid</VAR>, or <VAR>msgid</VAR> if
   *         none is found
   */
  public static String gettext (ResourceBundle catalog, String msgid) {
    String result = gettextnull(catalog,msgid);
    if (result != null)
      return result;
    return msgid;
  }

  /**
   * Like ngettext(catalog,msgid,msgid_plural,n), except that it returns
   * <CODE>null</CODE> when no translation was found.
   */
  private static String ngettextnull (ResourceBundle catalog, String msgid, long n) {
    // The reason why we use so many reflective API calls instead of letting
    // the GNU gettext generated ResourceBundles implement some interface,
    // is that we want the generated ResourceBundles to be completely
    // standalone, so that migration from the Sun approach to the GNU gettext
    // approach (without use of plurals) is as straightforward as possible.
    ResourceBundle origCatalog = catalog;
    do {
      // Try catalog itself.
      if (verbose)
        System.out.println("ngettext on "+catalog);
      Method handleGetObjectMethod = null;
      Method getParentMethod = null;
      try {
        handleGetObjectMethod = catalog.getClass().getMethod("handleGetObject", new Class[] { java.lang.String.class });
        getParentMethod = catalog.getClass().getMethod("getParent", new Class[0]);
      } catch (NoSuchMethodException e) {
      } catch (SecurityException e) {
      }
      if (verbose)
        System.out.println("handleGetObject = "+(handleGetObjectMethod!=null)+", getParent = "+(getParentMethod!=null));
      if (handleGetObjectMethod != null
          && Modifier.isPublic(handleGetObjectMethod.getModifiers())
          && getParentMethod != null) {
        // A GNU gettext created class.
        Method lookupMethod = null;
        Method pluralEvalMethod = null;
        try {
          lookupMethod = catalog.getClass().getMethod("lookup", new Class[] { java.lang.String.class });
          pluralEvalMethod = catalog.getClass().getMethod("pluralEval", new Class[] { Long.TYPE });
        } catch (NoSuchMethodException e) {
        } catch (SecurityException e) {
        }
        if (verbose)
          System.out.println("lookup = "+(lookupMethod!=null)+", pluralEval = "+(pluralEvalMethod!=null));
        if (lookupMethod != null && pluralEvalMethod != null) {
          // A GNU gettext created class with plural handling.
          Object localValue = null;
          try {
            localValue = lookupMethod.invoke(catalog, new Object[] { msgid });
          } catch (IllegalAccessException e) {
            e.printStackTrace();
          } catch (InvocationTargetException e) {
            e.getTargetException().printStackTrace();
          }
          if (localValue != null) {
            if (verbose)
              System.out.println("localValue = "+localValue);
            if (localValue instanceof String)
              // Found the value. It doesn't depend on n in this case.
              return (String)localValue;
            else {
              String[] pluralforms = (String[])localValue;
              long i = 0;
              try {
                i = ((Long) pluralEvalMethod.invoke(catalog, new Object[] { new Long(n) })).longValue();
                if (!(i >= 0 && i < pluralforms.length))
                  i = 0;
              } catch (IllegalAccessException e) {
                e.printStackTrace();
              } catch (InvocationTargetException e) {
                e.getTargetException().printStackTrace();
              }
              return pluralforms[(int)i];
            }
          }
        } else {
          // A GNU gettext created class without plural handling.
          Object localValue = null;
          try {
            localValue = handleGetObjectMethod.invoke(catalog, new Object[] { msgid });
          } catch (IllegalAccessException e) {
            e.printStackTrace();
          } catch (InvocationTargetException e) {
            e.getTargetException().printStackTrace();
          }
          if (localValue != null) {
            // Found the value. It doesn't depend on n in this case.
            if (verbose)
              System.out.println("localValue = "+localValue);
            return (String)localValue;
          }
        }
        Object parentCatalog = catalog;
        try {
          parentCatalog = getParentMethod.invoke(catalog, new Object[0]);
        } catch (IllegalAccessException e) {
          e.printStackTrace();
        } catch (InvocationTargetException e) {
          e.getTargetException().printStackTrace();
        }
        if (parentCatalog != catalog)
          catalog = (ResourceBundle)parentCatalog;
        else
          break;
      } else
        // Not a GNU gettext created class.
        break;
    } while (catalog != null);
    // The end of chain of GNU gettext ResourceBundles is reached.
    if (catalog != null) {
      // For a non-GNU ResourceBundle we cannot access 'parent' and
      // 'handleGetObject', so make a single call to catalog and all
      // its parent catalogs at once.
      Object value;
      try {
        value = catalog.getObject(msgid);
      } catch (MissingResourceException e) {
        value = null;
      }
      if (value != null)
        // Found the value. It doesn't depend on n in this case.
        return (String)value;
    }
    // Default: null.
    return null;
  }

  /**
   * Returns the plural form for <VAR>n</VAR> of the translation of
   * <VAR>msgid</VAR>.
   * @param catalog a ResourceBundle
   * @param msgid the key string to be translated, an ASCII string
   * @param msgid_plural its English plural form
   * @return the translation of <VAR>msgid</VAR> depending on <VAR>n</VAR>,
   *         or <VAR>msgid</VAR> or <VAR>msgid_plural</VAR> if none is found
   */
  public static String ngettext (ResourceBundle catalog, String msgid, String msgid_plural, long n) {
    String result = ngettextnull(catalog,msgid,n);
    if (result != null)
      return result;
    // Default: English strings and Germanic plural rule.
    return (n != 1 ? msgid_plural : msgid);
  }

  /* The separator between msgctxt and msgid.  */
  private static final String CONTEXT_GLUE = "\u0004";

  /**
   * Returns the translation of <VAR>msgid</VAR> in the context of
   * <VAR>msgctxt</VAR>.
   * @param catalog a ResourceBundle
   * @param msgctxt the context for the key string, an ASCII string
   * @param msgid the key string to be translated, an ASCII string
   * @return the translation of <VAR>msgid</VAR>, or <VAR>msgid</VAR> if
   *         none is found
   */
  public static String pgettext (ResourceBundle catalog, String msgctxt, String msgid) {
    String result = gettextnull(catalog,msgctxt+CONTEXT_GLUE+msgid);
    if (result != null)
      return result;
    return msgid;
  }

  /**
   * Returns the plural form for <VAR>n</VAR> of the translation of
   * <VAR>msgid</VAR> in the context of <VAR>msgctxt</VAR>.
   * @param catalog a ResourceBundle
   * @param msgctxt the context for the key string, an ASCII string
   * @param msgid the key string to be translated, an ASCII string
   * @param msgid_plural its English plural form
   * @return the translation of <VAR>msgid</VAR> depending on <VAR>n</VAR>,
   *         or <VAR>msgid</VAR> or <VAR>msgid_plural</VAR> if none is found
   */
  public static String npgettext (ResourceBundle catalog, String msgctxt, String msgid, String msgid_plural, long n) {
    String result = ngettextnull(catalog,msgctxt+CONTEXT_GLUE+msgid,n);
    if (result != null)
      return result;
    // Default: English strings and Germanic plural rule.
    return (n != 1 ? msgid_plural : msgid);
  }
}
