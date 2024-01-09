<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <xsl:variable name="apidoc-prefix">https://gnome.pages.gitlab.gnome.org/libxml2/devhelp/libxml2-</xsl:variable>

  <xsl:template match="include">
    <xsl:variable name="header" select="substring-before(substring-after(., '/'), '&gt;')"/>
    <xsl:variable name="doc" select="concat($apidoc-prefix, $header, 'tml')"/>
    <li><a href="{$doc}"><xsl:value-of select="."/></a></li>
  </xsl:template>

  <xsl:template match="typedef">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="header" select="concat(@file, '.h')"/>
    <xsl:variable name="doc" select="concat($apidoc-prefix, @file, '.html#', $name)"/>
    <li> line <xsl:value-of select="@line"/>: Type <a href="{$doc}"><xsl:value-of select="$name"/></a> from <xsl:value-of select="$header"/></li>
  </xsl:template>

  <xsl:template match="function">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="header" select="concat(@file, '.h')"/>
    <xsl:variable name="doc" select="concat($apidoc-prefix, @file, '.html#', $name)"/>
    <li> line <xsl:value-of select="@line"/>: Function <a href="{$doc}"><xsl:value-of select="$name"/></a> from <xsl:value-of select="$header"/></li>
  </xsl:template>

  <xsl:template match="macro">
    <xsl:variable name="name" select="@name"/>
    <xsl:variable name="header" select="concat(@file, '.h')"/>
    <xsl:variable name="doc" select="concat($apidoc-prefix, @file, '.html#', $name)"/>
    <li> line <xsl:value-of select="@line"/>: Macro <a href="{$doc}"><xsl:value-of select="$name"/></a> from <xsl:value-of select="$header"/></li>
  </xsl:template>

  <xsl:template match="example">
    <xsl:variable name="filename" select="string(@filename)"/>
    <h3><a name="{$filename}" href="{$filename}"><xsl:value-of select="$filename"/></a>: <xsl:value-of select="synopsis"/></h3>
    <p><xsl:value-of select="purpose"/></p>
    <p>Includes:</p>
    <ul>
    <xsl:for-each select="includes/include">
      <xsl:sort select="@line" data-type="number"/>
      <xsl:apply-templates select='.'/>
    </xsl:for-each>
    </ul>
    <p>Uses:</p>
    <ul>
    <xsl:for-each select="uses/*">
      <xsl:sort select="@line" data-type="number"/>
      <xsl:apply-templates select='.'/>
    </xsl:for-each>
    </ul>
    <p>Usage:</p>
    <p><xsl:value-of select="usage"/></p>
    <p>Author: <xsl:value-of select="author"/></p>
  </xsl:template>

  <xsl:template match="section">
    <li><p> <a href="#{@name}"><xsl:value-of select="@name"/></a> :</p>
    <ul>
    <xsl:for-each select="example">
      <xsl:sort select='.'/>
      <xsl:variable name="filename" select="@filename"/>
      <li> <a href="#{$filename}"><xsl:value-of select="$filename"/></a>: <xsl:value-of select="/examples/example[@filename = $filename]/synopsis"/></li>
    </xsl:for-each>
    </ul>
    </li>
  </xsl:template>

  <xsl:template match="sections">
    <p> The examples are stored per section depending on the main focus
    of the example:</p>
    <ul>
    <xsl:for-each select="section">
      <xsl:sort select='.'/>
      <xsl:apply-templates select='.'/>
    </xsl:for-each>
    </ul>
    <p> Getting the compilation options and libraries dependencies needed
to generate binaries from the examples is best done on Linux/Unix by using
the xml2-config script which should have been installed as part of <i>make
install</i> step or when installing the libxml2 development package:</p>
<pre>gcc -o example `xml2-config --cflags` example.c `xml2-config --libs`</pre>
  </xsl:template>

  <xsl:template name="sections-list">
    <xsl:for-each select="sections/section">
      <xsl:variable name="section" select="@name"/>
      <h2> <a name="{$section}"></a><xsl:value-of select="$section"/> Examples</h2>
      <xsl:apply-templates select='/examples/example[section = $section]'/>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="examples">
    <xsl:variable name="title">Libxml2 set of examples</xsl:variable>
      <xsl:document href="index.html" method="xml" indent="yes" omit-xml-declaration="yes"
          doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
          doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
      <html>
        <head>
	  <title>
	    <xsl:value-of select="$title"/>
	  </title>
        </head>
        <body>
	  <h1><xsl:value-of select="$title"/></h1>
          <xsl:apply-templates select="sections"/>
          <xsl:call-template name="sections-list"/>
        </body>
      </html>
    </xsl:document>
  </xsl:template>

</xsl:stylesheet>
