/* GNU gettext for Java
 * Copyright (C) 2001-2003, 2007 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

package gnu.gettext;

import java.lang.reflect.*;
import java.util.*;
import java.io.*;

/**
 * This programs dumps a resource as a PO file. The resource must be
 * accessible through the CLASSPATH.
 *
 * @author Bruno Haible
 */
public class DumpResource {
  private Writer out;
  private void dumpString (String str) throws IOException {
    int n = str.length();
    out.write('"');
    for (int i = 0; i < n; i++) {
      char c = str.charAt(i);
      if (c == 0x0008) {
        out.write('\\'); out.write('b');
      } else if (c == 0x000c) {
        out.write('\\'); out.write('f');
      } else if (c == 0x000a) {
        out.write('\\'); out.write('n');
      } else if (c == 0x000d) {
        out.write('\\'); out.write('r');
      } else if (c == 0x0009) {
        out.write('\\'); out.write('t');
      } else if (c == '\\' || c == '"') {
        out.write('\\'); out.write(c);
      } else
        out.write(c);
    }
    out.write('"');
  }
  private void dumpMessage (String msgid, String msgid_plural, Object msgstr) throws IOException {
    int separatorPos = msgid.indexOf('\u0004');
    if (separatorPos >= 0) {
      String msgctxt = msgid.substring(0,separatorPos);
      msgid = msgid.substring(separatorPos+1);
      out.write("msgctxt "); dumpString(msgctxt);
    }
    out.write("msgid "); dumpString(msgid); out.write('\n');
    if (msgid_plural != null) {
      out.write("msgid_plural "); dumpString(msgid_plural); out.write('\n');
      for (int i = 0; i < ((String[])msgstr).length; i++) {
        out.write("msgstr[" + i + "] ");
        dumpString(((String[])msgstr)[i]);
        out.write('\n');
      }
    } else {
      out.write("msgstr "); dumpString((String)msgstr); out.write('\n');
    }
    out.write('\n');
  }
  private ResourceBundle catalog;
  private Method lookupMethod;
  // Lookup the value corresponding to a key found in catalog.getKeys().
  // Here we assume that the catalog returns a non-inherited value for
  // these keys. FIXME: Not true. Better see whether handleGetObject is
  // public - it is in ListResourceBundle and PropertyResourceBundle.
  private Object lookup (String key) {
    Object value = null;
    if (lookupMethod != null) {
      try {
        value = lookupMethod.invoke(catalog, new Object[] { key });
      } catch (IllegalAccessException e) {
        e.printStackTrace();
      } catch (InvocationTargetException e) {
        e.getTargetException().printStackTrace();
      }
    } else {
      try {
        value = catalog.getObject(key);
      } catch (MissingResourceException e) {
      }
    }
    return value;
  }
  private void dump () throws IOException {
    lookupMethod = null;
    try {
      lookupMethod = catalog.getClass().getMethod("lookup", new Class[] { java.lang.String.class });
    } catch (NoSuchMethodException e) {
    } catch (SecurityException e) {
    }
    Method pluralMethod = null;
    try {
      pluralMethod = catalog.getClass().getMethod("get_msgid_plural_table", new Class[0]);
    } catch (NoSuchMethodException e) {
    } catch (SecurityException e) {
    }
    Field pluralField = null;
    try {
      pluralField = catalog.getClass().getField("plural");
    } catch (NoSuchFieldException e) {
    } catch (SecurityException e) {
    }
    // Search for the header entry.
    {
      Object header_entry = null;
      Enumeration keys = catalog.getKeys();
      while (keys.hasMoreElements())
        if ("".equals(keys.nextElement())) {
          header_entry = lookup("");
          break;
        }
      // If there is no header entry, fake one.
      // FIXME: This is not needed; right after po_lex_charset_init set
      // the PO charset to UTF-8.
      if (header_entry == null)
        header_entry = "Content-Type: text/plain; charset=UTF-8\n";
      dumpMessage("",null,header_entry);
    }
    // Now the other messages.
    {
      Enumeration keys = catalog.getKeys();
      Object plural = null;
      if (pluralMethod != null) {
        // msgfmt versions > 0.13.1 create a static get_msgid_plural_table()
        // method.
        try {
          plural = pluralMethod.invoke(catalog, new Object[0]);
        } catch (IllegalAccessException e) {
          e.printStackTrace();
        } catch (InvocationTargetException e) {
          e.getTargetException().printStackTrace();
        }
      } else if (pluralField != null) {
        // msgfmt versions <= 0.13.1 create a static plural field.
        try {
          plural = pluralField.get(catalog);
        } catch (IllegalAccessException e) {
          e.printStackTrace();
        }
      }
      if (plural instanceof String[]) {
        // A GNU gettext created class with plural handling, Java2 format.
        int i = 0;
        while (keys.hasMoreElements()) {
          String key = (String)keys.nextElement();
          Object value = lookup(key);
          String key_plural = (value instanceof String[] ? ((String[])plural)[i++] : null);
          if (!"".equals(key))
            dumpMessage(key,key_plural,value);
        }
        if (i != ((String[])plural).length)
          throw new RuntimeException("wrong plural field length");
      } else if (plural instanceof Hashtable) {
        // A GNU gettext created class with plural handling, Java format.
        while (keys.hasMoreElements()) {
          String key = (String)keys.nextElement();
          if (!"".equals(key)) {
            Object value = lookup(key);
            String key_plural = (value instanceof String[] ? (String)((Hashtable)plural).get(key) : null);
            dumpMessage(key,key_plural,value);
          }
        }
      } else if (plural == null) {
        // No plural handling.
        while (keys.hasMoreElements()) {
          String key = (String)keys.nextElement();
          if (!"".equals(key))
            dumpMessage(key,null,lookup(key));
        }
      } else
        throw new RuntimeException("wrong plural field value");
    }
  }

  public DumpResource (String resource_name, String locale_name) {
    // Split locale_name into language_country_variant.
    String language;
    String country;
    String variant;
    language = locale_name;
    {
      int i = language.indexOf('_');
      if (i >= 0) {
        country = language.substring(i+1);
        language = language.substring(0,i);
      } else
        country = "";
    }
    {
      int j = country.indexOf('_');
      if (j >= 0) {
        variant = country.substring(j+1);
        country = country.substring(0,j);
      } else
        variant = "";
    }
    Locale locale = new Locale(language,country,variant);
    // Get the resource.
    ResourceBundle catalog = ResourceBundle.getBundle(resource_name,locale);
    // We are only interested in the messsages belonging to the locale
    // itself, not in the inherited messages. But catalog.getLocale() exists
    // only in Java2 and sometimes differs from the given locale.
    try {
      Writer w1 = new OutputStreamWriter(System.out,"UTF8");
      Writer w2 = new BufferedWriter(w1);
      this.out = w2;
      this.catalog = catalog;
      dump();
      w2.close();
      w1.close();
      System.out.flush();
    } catch (IOException e) {
      e.printStackTrace();
      System.exit(1);
    }
  }

  public static void main (String[] args) {
    new DumpResource(args[0], args.length > 1 ? args[1] : "");
    System.exit(0);
  }
}
