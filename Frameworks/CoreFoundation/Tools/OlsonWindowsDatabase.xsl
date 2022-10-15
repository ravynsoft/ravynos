<?xml version="1.0" encoding="utf-8"?>
<!--
  This XSL transforms the Windows Timezone list from the Unicode CLDR to a plist
  which is consumable by CoreFoundation.  You need to fetch the latest mapping
  from the Unicode consortium from:
    https://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml
  The XSL can be applied to this data via the `xsltproc` command.
  Running
    `xsltproc &dash;&dash;novalid OlsonWindowsDatabase.xsl windowsZones.xml`
  will generate the mapping plist.
-->
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:ext="http://exslt.org/common"
                exclude-result-prefixes="ext">
  <xsl:output encoding="utf-8" indent="yes" />

  <xsl:template name="tokenize">
    <xsl:param name="string" />
    <xsl:param name="delimiter" />

    <xsl:choose>
      <xsl:when test="not($string)"><items><item/></items></xsl:when>
      <xsl:when test="not(contains($string, $delimiter))" ><items><item><xsl:value-of select="$string" /></item></items></xsl:when>
      <xsl:otherwise><items><item><xsl:value-of select="normalize-space(substring-before($string, $delimiter))" /></item><xsl:call-template name="tokenize">
  <xsl:with-param name="string" select="substring-after($string, $delimiter)" />
  <xsl:with-param name="delimiter" select="$delimiter" />
</xsl:call-template></items>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="/">
    <xsl:text disable-output-escaping="yes">&lt;!DOCTYPE plist SYSTEM "file:///localhost/System/Library/DTDs/PropertyList.dtd"&gt;
</xsl:text>
<plist version="1.0">
  <dict>
    <xsl:for-each select="supplementalData/windowsZones/mapTimezones/mapZone[not(@territory='001')]">
<xsl:variable name="names">
  <xsl:call-template name="tokenize">
    <xsl:with-param name="string" select="@type" />
    <xsl:with-param name="delimiter" select="' '" />
  </xsl:call-template>
</xsl:variable>
    <key><xsl:value-of select="ext:node-set($names)/items/*" /></key>
    <string><xsl:value-of select="@other" /></string>
    </xsl:for-each>
  </dict>
</plist>
  </xsl:template>
</xsl:stylesheet>
