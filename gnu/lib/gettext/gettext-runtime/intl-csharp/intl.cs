/* GNU gettext for C#
 * Copyright (C) 2003-2005, 2007 Free Software Foundation, Inc.
 * Written by Bruno Haible <bruno@clisp.org>, 2003.
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

/*
 * Using the GNU gettext approach, compiled message catalogs are assemblies
 * containing just one class, a subclass of GettextResourceSet. They are thus
 * interoperable with standard ResourceManager based code.
 *
 * The main differences between the common .NET resources approach and the
 * GNU gettext approach are:
 * - In the .NET resource approach, the keys are abstract textual shortcuts.
 *   In the GNU gettext approach, the keys are the English/ASCII version
 *   of the messages.
 * - In the .NET resource approach, the translation files are called
 *   "Resource.locale.resx" and are UTF-8 encoded XML files. In the GNU gettext
 *   approach, the translation files are called "Resource.locale.po" and are
 *   in the encoding the translator has chosen. There are at least three GUI
 *   translating tools (Emacs PO mode, KDE KBabel, GNOME gtranslator).
 * - In the .NET resource approach, the function ResourceManager.GetString
 *   returns an empty string or throws an InvalidOperationException when no
 *   translation is found. In the GNU gettext approach, the GetString function
 *   returns the (English) message key in that case.
 * - In the .NET resource approach, there is no support for plural handling.
 *   In the GNU gettext approach, we have the GetPluralString function.
 * - In the .NET resource approach, there is no support for context specific
 *   translations.
 *   In the GNU gettext approach, we have the GetParticularString function.
 *
 * To compile GNU gettext message catalogs into C# assemblies, the msgfmt
 * program can be used.
 */

using System; /* String, InvalidOperationException, Console */
using System.Globalization; /* CultureInfo */
using System.Resources; /* ResourceManager, ResourceSet, IResourceReader */
using System.Reflection; /* Assembly, ConstructorInfo */
using System.Collections; /* Hashtable, ICollection, IEnumerator, IDictionaryEnumerator */
using System.IO; /* Path, FileNotFoundException, Stream */
using System.Text; /* StringBuilder */

namespace GNU.Gettext {

  /// <summary>
  /// Each instance of this class can be used to lookup translations for a
  /// given resource name. For each <c>CultureInfo</c>, it performs the lookup
  /// in several assemblies, from most specific over territory-neutral to
  /// language-neutral.
  /// </summary>
  public class GettextResourceManager : ResourceManager {

    // ======================== Public Constructors ========================

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="baseName">the resource name, also the assembly base
    ///                        name</param>
    public GettextResourceManager (String baseName)
      : base (baseName, Assembly.GetCallingAssembly(), typeof (GettextResourceSet)) {
    }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="baseName">the resource name, also the assembly base
    ///                        name</param>
    public GettextResourceManager (String baseName, Assembly assembly)
      : base (baseName, assembly, typeof (GettextResourceSet)) {
    }

    // ======================== Implementation ========================

    /// <summary>
    /// Loads and returns a satellite assembly.
    /// </summary>
    // This is like Assembly.GetSatelliteAssembly, but uses resourceName
    // instead of assembly.GetName().Name, and works around a bug in
    // mono-0.28.
    private static Assembly GetSatelliteAssembly (Assembly assembly, String resourceName, CultureInfo culture) {
      String satelliteExpectedLocation =
        Path.GetDirectoryName(assembly.Location)
        + Path.DirectorySeparatorChar + culture.Name
        + Path.DirectorySeparatorChar + resourceName + ".resources.dll";
      return Assembly.LoadFrom(satelliteExpectedLocation);
    }

    /// <summary>
    /// Loads and returns the satellite assembly for a given culture.
    /// </summary>
    private Assembly MySatelliteAssembly (CultureInfo culture) {
      return GetSatelliteAssembly(MainAssembly, BaseName, culture);
    }

    /// <summary>
    /// Converts a resource name to a class name.
    /// </summary>
    /// <returns>a nonempty string consisting of alphanumerics and underscores
    ///          and starting with a letter or underscore</returns>
    private static String ConstructClassName (String resourceName) {
      // We could just return an arbitrary fixed class name, like "Messages",
      // assuming that every assembly will only ever contain one
      // GettextResourceSet subclass, but this assumption would break the day
      // we want to support multi-domain PO files in the same format...
      bool valid = (resourceName.Length > 0);
      for (int i = 0; valid && i < resourceName.Length; i++) {
        char c = resourceName[i];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_')
              || (i > 0 && c >= '0' && c <= '9')))
          valid = false;
      }
      if (valid)
        return resourceName;
      else {
        // Use hexadecimal escapes, using the underscore as escape character.
        String hexdigit = "0123456789abcdef";
        StringBuilder b = new StringBuilder();
        b.Append("__UESCAPED__");
        for (int i = 0; i < resourceName.Length; i++) {
          char c = resourceName[i];
          if (c >= 0xd800 && c < 0xdc00
              && i+1 < resourceName.Length
              && resourceName[i+1] >= 0xdc00 && resourceName[i+1] < 0xe000) {
            // Combine two UTF-16 words to a character.
            char c2 = resourceName[i+1];
            int uc = 0x10000 + ((c - 0xd800) << 10) + (c2 - 0xdc00);
            b.Append('_');
            b.Append('U');
            b.Append(hexdigit[(uc >> 28) & 0x0f]);
            b.Append(hexdigit[(uc >> 24) & 0x0f]);
            b.Append(hexdigit[(uc >> 20) & 0x0f]);
            b.Append(hexdigit[(uc >> 16) & 0x0f]);
            b.Append(hexdigit[(uc >> 12) & 0x0f]);
            b.Append(hexdigit[(uc >> 8) & 0x0f]);
            b.Append(hexdigit[(uc >> 4) & 0x0f]);
            b.Append(hexdigit[uc & 0x0f]);
            i++;
          } else if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
                       || (c >= '0' && c <= '9'))) {
            int uc = c;
            b.Append('_');
            b.Append('u');
            b.Append(hexdigit[(uc >> 12) & 0x0f]);
            b.Append(hexdigit[(uc >> 8) & 0x0f]);
            b.Append(hexdigit[(uc >> 4) & 0x0f]);
            b.Append(hexdigit[uc & 0x0f]);
          } else
            b.Append(c);
        }
        return b.ToString();
      }
    }

    /// <summary>
    /// Instantiates a resource set for a given culture.
    /// </summary>
    /// <exception cref="ArgumentException">
    ///   The expected type name is not valid.
    /// </exception>
    /// <exception cref="ReflectionTypeLoadException">
    ///   satelliteAssembly does not contain the expected type.
    /// </exception>
    /// <exception cref="NullReferenceException">
    ///   The type has no no-arguments constructor.
    /// </exception>
    private static GettextResourceSet InstantiateResourceSet (Assembly satelliteAssembly, String resourceName, CultureInfo culture) {
      // We expect a class with a culture dependent class name.
      Type clazz = satelliteAssembly.GetType(ConstructClassName(resourceName)+"_"+culture.Name.Replace('-','_'));
      // We expect it has a no-argument constructor, and invoke it.
      ConstructorInfo constructor = clazz.GetConstructor(Type.EmptyTypes);
      return (GettextResourceSet) constructor.Invoke(null);
    }

    private static GettextResourceSet[] EmptyResourceSetArray = new GettextResourceSet[0];

    // Cache for already loaded GettextResourceSet cascades.
    private Hashtable /* CultureInfo -> GettextResourceSet[] */ Loaded = new Hashtable();

    /// <summary>
    /// Returns the array of <c>GettextResourceSet</c>s for a given culture,
    /// loading them if necessary, and maintaining the cache.
    /// </summary>
    private GettextResourceSet[] GetResourceSetsFor (CultureInfo culture) {
      //Console.WriteLine(">> GetResourceSetsFor "+culture);
      // Look up in the cache.
      GettextResourceSet[] result = (GettextResourceSet[]) Loaded[culture];
      if (result == null) {
        lock(this) {
          // Look up again - maybe another thread has filled in the entry
          // while we slept waiting for the lock.
          result = (GettextResourceSet[]) Loaded[culture];
          if (result == null) {
            // Determine the GettextResourceSets for the given culture.
            if (culture.Parent == null || culture.Equals(CultureInfo.InvariantCulture))
              // Invariant culture.
              result = EmptyResourceSetArray;
            else {
              // Use a satellite assembly as primary GettextResourceSet, and
              // the result for the parent culture as fallback.
              GettextResourceSet[] parentResult = GetResourceSetsFor(culture.Parent);
              Assembly satelliteAssembly;
              try {
                satelliteAssembly = MySatelliteAssembly(culture);
              } catch (FileNotFoundException e) {
                satelliteAssembly = null;
              }
              if (satelliteAssembly != null) {
                GettextResourceSet satelliteResourceSet;
                try {
                  satelliteResourceSet = InstantiateResourceSet(satelliteAssembly, BaseName, culture);
                } catch (Exception e) {
                  Console.Error.WriteLine(e);
                  Console.Error.WriteLine(e.StackTrace);
                  satelliteResourceSet = null;
                }
                if (satelliteResourceSet != null) {
                  result = new GettextResourceSet[1+parentResult.Length];
                  result[0] = satelliteResourceSet;
                  Array.Copy(parentResult, 0, result, 1, parentResult.Length);
                } else
                  result = parentResult;
              } else
                result = parentResult;
            }
            // Put the result into the cache.
            Loaded.Add(culture, result);
          }
        }
      }
      //Console.WriteLine("<< GetResourceSetsFor "+culture);
      return result;
    }

    /*
    /// <summary>
    /// Releases all loaded <c>GettextResourceSet</c>s and their assemblies.
    /// </summary>
    // TODO: No way to release an Assembly?
    public override void ReleaseAllResources () {
      ...
    }
    */

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> in a given culture.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or
    ///          <paramref name="msgid"/> if none is found</returns>
    public override String GetString (String msgid, CultureInfo culture) {
      foreach (GettextResourceSet rs in GetResourceSetsFor(culture)) {
        String translation = rs.GetString(msgid);
        if (translation != null)
          return translation;
      }
      // Fallback.
      return msgid;
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> and
    /// <paramref name="msgidPlural"/> in a given culture, choosing the right
    /// plural form depending on the number <paramref name="n"/>.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <param name="msgidPlural">the English plural of <paramref name="msgid"/>,
    ///                           an ASCII string</param>
    /// <param name="n">the number, should be &gt;= 0</param>
    /// <returns>the translation, or <paramref name="msgid"/> or
    ///          <paramref name="msgidPlural"/> if none is found</returns>
    public virtual String GetPluralString (String msgid, String msgidPlural, long n, CultureInfo culture) {
      foreach (GettextResourceSet rs in GetResourceSetsFor(culture)) {
        String translation = rs.GetPluralString(msgid, msgidPlural, n);
        if (translation != null)
          return translation;
      }
      // Fallback: Germanic plural form.
      return (n == 1 ? msgid : msgidPlural);
    }

    // ======================== Public Methods ========================

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> in the context
    /// of <paramref name="msgctxt"/> a given culture.
    /// </summary>
    /// <param name="msgctxt">the context for the key string, an ASCII
    ///                       string</param>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or
    ///          <paramref name="msgid"/> if none is found</returns>
    public String GetParticularString (String msgctxt, String msgid, CultureInfo culture) {
      String combined = msgctxt + "\u0004" + msgid;
      foreach (GettextResourceSet rs in GetResourceSetsFor(culture)) {
        String translation = rs.GetString(combined);
        if (translation != null)
          return translation;
      }
      // Fallback.
      return msgid;
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> and
    /// <paramref name="msgidPlural"/> in the context of
    /// <paramref name="msgctxt"/> in a given culture, choosing the right
    /// plural form depending on the number <paramref name="n"/>.
    /// </summary>
    /// <param name="msgctxt">the context for the key string, an ASCII
    ///                       string</param>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <param name="msgidPlural">the English plural of <paramref name="msgid"/>,
    ///                           an ASCII string</param>
    /// <param name="n">the number, should be &gt;= 0</param>
    /// <returns>the translation, or <paramref name="msgid"/> or
    ///          <paramref name="msgidPlural"/> if none is found</returns>
    public virtual String GetParticularPluralString (String msgctxt, String msgid, String msgidPlural, long n, CultureInfo culture) {
      String combined = msgctxt + "\u0004" + msgid;
      foreach (GettextResourceSet rs in GetResourceSetsFor(culture)) {
        String translation = rs.GetPluralString(combined, msgidPlural, n);
        if (translation != null)
          return translation;
      }
      // Fallback: Germanic plural form.
      return (n == 1 ? msgid : msgidPlural);
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> in the current
    /// culture.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or
    ///          <paramref name="msgid"/> if none is found</returns>
    public override String GetString (String msgid) {
      return GetString(msgid, CultureInfo.CurrentUICulture);
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> and
    /// <paramref name="msgidPlural"/> in the current culture, choosing the
    /// right plural form depending on the number <paramref name="n"/>.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <param name="msgidPlural">the English plural of <paramref name="msgid"/>,
    ///                           an ASCII string</param>
    /// <param name="n">the number, should be &gt;= 0</param>
    /// <returns>the translation, or <paramref name="msgid"/> or
    ///          <paramref name="msgidPlural"/> if none is found</returns>
    public virtual String GetPluralString (String msgid, String msgidPlural, long n) {
      return GetPluralString(msgid, msgidPlural, n, CultureInfo.CurrentUICulture);
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> in the context
    /// of <paramref name="msgctxt"/> in the current culture.
    /// </summary>
    /// <param name="msgctxt">the context for the key string, an ASCII
    ///                       string</param>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or
    ///          <paramref name="msgid"/> if none is found</returns>
    public String GetParticularString (String msgctxt, String msgid) {
      return GetParticularString(msgctxt, msgid, CultureInfo.CurrentUICulture);
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> and
    /// <paramref name="msgidPlural"/> in the context of
    /// <paramref name="msgctxt"/> in the current culture, choosing the
    /// right plural form depending on the number <paramref name="n"/>.
    /// </summary>
    /// <param name="msgctxt">the context for the key string, an ASCII
    ///                       string</param>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <param name="msgidPlural">the English plural of <paramref name="msgid"/>,
    ///                           an ASCII string</param>
    /// <param name="n">the number, should be &gt;= 0</param>
    /// <returns>the translation, or <paramref name="msgid"/> or
    ///          <paramref name="msgidPlural"/> if none is found</returns>
    public virtual String GetParticularPluralString (String msgctxt, String msgid, String msgidPlural, long n) {
      return GetParticularPluralString(msgctxt, msgid, msgidPlural, n, CultureInfo.CurrentUICulture);
    }

  }

  /// <summary>
  /// <para>
  /// Each instance of this class encapsulates a single PO file.
  /// </para>
  /// <para>
  /// This API of this class is not meant to be used directly; use
  /// <c>GettextResourceManager</c> instead.
  /// </para>
  /// </summary>
  // We need this subclass of ResourceSet, because the plural formula must come
  // from the same ResourceSet as the object containing the plural forms.
  public class GettextResourceSet : ResourceSet {

    /// <summary>
    /// Creates a new message catalog. When using this constructor, you
    /// must override the <c>ReadResources</c> method, in order to initialize
    /// the <c>Table</c> property. The message catalog will support plural
    /// forms only if the <c>ReadResources</c> method installs values of type
    /// <c>String[]</c> and if the <c>PluralEval</c> method is overridden.
    /// </summary>
    protected GettextResourceSet ()
      : base (DummyResourceReader) {
    }

    /// <summary>
    /// Creates a new message catalog, by reading the string/value pairs from
    /// the given <paramref name="reader"/>. The message catalog will support
    /// plural forms only if the reader can produce values of type
    /// <c>String[]</c> and if the <c>PluralEval</c> method is overridden.
    /// </summary>
    public GettextResourceSet (IResourceReader reader)
      : base (reader) {
    }

    /// <summary>
    /// Creates a new message catalog, by reading the string/value pairs from
    /// the given <paramref name="stream"/>, which should have the format of
    /// a <c>.resources</c> file. The message catalog will not support plural
    /// forms.
    /// </summary>
    public GettextResourceSet (Stream stream)
      : base (stream) {
    }

    /// <summary>
    /// Creates a new message catalog, by reading the string/value pairs from
    /// the file with the given <paramref name="fileName"/>. The file should
    /// be in the format of a <c>.resources</c> file. The message catalog will
    /// not support plural forms.
    /// </summary>
    public GettextResourceSet (String fileName)
      : base (fileName) {
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/>.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or <c>null</c> if
    ///          none is found</returns>
    // The default implementation essentially does (String)Table[msgid].
    // Here we also catch the plural form case.
    public override String GetString (String msgid) {
      Object value = GetObject(msgid);
      if (value == null || value is String)
        return (String)value;
      else if (value is String[])
        // A plural form, but no number is given.
        // Like the C implementation, return the first plural form.
        return ((String[]) value)[0];
      else
        throw new InvalidOperationException("resource for \""+msgid+"\" in "+GetType().FullName+" is not a string");
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/>, with possibly
    /// case-insensitive lookup.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <returns>the translation of <paramref name="msgid"/>, or <c>null</c> if
    ///          none is found</returns>
    // The default implementation essentially does (String)Table[msgid].
    // Here we also catch the plural form case.
    public override String GetString (String msgid, bool ignoreCase) {
      Object value = GetObject(msgid, ignoreCase);
      if (value == null || value is String)
        return (String)value;
      else if (value is String[])
        // A plural form, but no number is given.
        // Like the C implementation, return the first plural form.
        return ((String[]) value)[0];
      else
        throw new InvalidOperationException("resource for \""+msgid+"\" in "+GetType().FullName+" is not a string");
    }

    /// <summary>
    /// Returns the translation of <paramref name="msgid"/> and
    /// <paramref name="msgidPlural"/>, choosing the right plural form
    /// depending on the number <paramref name="n"/>.
    /// </summary>
    /// <param name="msgid">the key string to be translated, an ASCII
    ///                     string</param>
    /// <param name="msgidPlural">the English plural of <paramref name="msgid"/>,
    ///                           an ASCII string</param>
    /// <param name="n">the number, should be &gt;= 0</param>
    /// <returns>the translation, or <c>null</c> if none is found</returns>
    public virtual String GetPluralString (String msgid, String msgidPlural, long n) {
      Object value = GetObject(msgid);
      if (value == null || value is String)
        return (String)value;
      else if (value is String[]) {
        String[] choices = (String[]) value;
        long index = PluralEval(n);
        return choices[index >= 0 && index < choices.Length ? index : 0];
      } else
        throw new InvalidOperationException("resource for \""+msgid+"\" in "+GetType().FullName+" is not a string");
    }

    /// <summary>
    /// Returns the index of the plural form to be chosen for a given number.
    /// The default implementation is the Germanic plural formula:
    /// zero for <paramref name="n"/> == 1, one for <paramref name="n"/> != 1.
    /// </summary>
    protected virtual long PluralEval (long n) {
      return (n == 1 ? 0 : 1);
    }

    /// <summary>
    /// Returns the keys of this resource set, i.e. the strings for which
    /// <c>GetObject()</c> can return a non-null value.
    /// </summary>
    public virtual ICollection Keys {
      get {
        return Table.Keys;
      }
    }

    /// <summary>
    /// A trivial instance of <c>IResourceReader</c> that does nothing.
    /// </summary>
    // Needed by the no-arguments constructor.
    private static IResourceReader DummyResourceReader = new DummyIResourceReader();

  }

  /// <summary>
  /// A trivial <c>IResourceReader</c> implementation.
  /// </summary>
  class DummyIResourceReader : IResourceReader {

    // Implementation of IDisposable.
    void System.IDisposable.Dispose () {
    }

    // Implementation of IEnumerable.
    IEnumerator System.Collections.IEnumerable.GetEnumerator () {
      return null;
    }

    // Implementation of IResourceReader.
    void System.Resources.IResourceReader.Close () {
    }
    IDictionaryEnumerator System.Resources.IResourceReader.GetEnumerator () {
      return null;
    }

  }

}
