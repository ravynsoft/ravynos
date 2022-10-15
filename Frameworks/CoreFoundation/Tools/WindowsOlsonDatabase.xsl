<?xml version="1.0" encoding="utf-8"?>
<!--
  This XSL transforms the Windows Timezone list from the Unicode CLDR to a plist
  which is consumable by CoreFoundation.  You need to fetch the latest mapping
  from the Unicode consortium from:
    https://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml
  The XSL can be applied to this data via the `xsltproc` command.
  Running
    `xsltproc &dash;&dash;novalid WindowsOlsonDatabase.xsl windowsZones.xml`
  will generate the mapping plist.
-->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output encoding="utf-8" indent="yes" />

  <xsl:template match="/">
    <xsl:text disable-output-escaping="yes">&lt;!DOCTYPE plist SYSTEM "file:///localhost/System/Library/DTDs/PropertyList.dtd"&gt;
</xsl:text>
<plist version="1.0">
  <dict>
    <xsl:for-each select="supplementalData/windowsZones/mapTimezones/mapZone[@territory='001']">
    <key><xsl:value-of select="@other" /></key>
    <string><xsl:value-of select="@type" /></string>
    </xsl:for-each>
  </dict>
</plist>
  </xsl:template>
</xsl:stylesheet>
