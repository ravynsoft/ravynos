<?xml version="1.0" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="UTF-8" indent="yes" />

<xsl:template match="/">
  <!-- insert docbook's DOCTYPE blurb -->
    <xsl:text disable-output-escaping = "yes"><![CDATA[
<!DOCTYPE appendix PUBLIC "-//OASIS//DTD DocBook XML V4.5//EN" "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd" [
  <!ENTITY % BOOK_ENTITIES SYSTEM "Wayland.ent">
%BOOK_ENTITIES;
]>
]]></xsl:text>

  <section id="sect-Protocol-Interfaces">
    <title>Interfaces</title>
    <para>
      The protocol includes several interfaces which are used for
      interacting with the server.  Each interface provides requests,
      events, and errors (which are really just special events) as described
      above.  Specific compositor implementations may have their own
      interfaces provided as extensions, but there are several which are
      always expected to be present.
    </para>

    <para>
      Core interfaces:
      <variablelist>
        <xsl:apply-templates select="protocol/interface" />
      </variablelist>
    </para>
  </section>
</xsl:template>

<!-- Interfaces summary -->
<xsl:template match="interface" >
<varlistentry>
  <term>
    <link linkend="protocol-spec-{@name}">
      <xsl:value-of select="@name" />
    </link>
  </term>
  <listitem>
    <simpara>
      <xsl:value-of select="description/@summary" />
    </simpara>
  </listitem>
</varlistentry>
</xsl:template>

</xsl:stylesheet>
<!-- vim: set expandtab shiftwidth=2: -->
