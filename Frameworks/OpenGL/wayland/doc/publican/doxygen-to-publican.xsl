<?xml version="1.0" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" encoding="UTF-8" indent="yes" />
<xsl:param name="which" />

<xsl:template match="/">
  <xsl:apply-templates select="/doxygen/compounddef[@kind!='file' and @kind!='dir']" />

  <section id="{$which}-Functions">
    <title>Functions</title>
    <para />
    <variablelist>
      <xsl:apply-templates select="/doxygen/compounddef[@kind='file']/sectiondef/memberdef" />
    </variablelist>
  </section>

</xsl:template>

<xsl:template match="parameteritem">
    <varlistentry>
        <term>
          <xsl:apply-templates select="parameternamelist/parametername"/>
        </term>
      <listitem>
        <simpara><xsl:apply-templates select="parameterdescription"/></simpara>
      </listitem>
    </varlistentry>
</xsl:template>

<xsl:template match="parameterlist">
  <xsl:if test="parameteritem">
      <variablelist>
        <xsl:apply-templates select="parameteritem" />
      </variablelist>
  </xsl:if>
</xsl:template>

<xsl:template match="ref">
  <link linkend="{$which}-{@refid}"><xsl:value-of select="." /></link>
</xsl:template>

<xsl:template match="simplesect[@kind='return']">
  <variablelist>
    <varlistentry>
      <term>Returns:</term>
      <listitem>
        <simpara><xsl:apply-templates /></simpara>
      </listitem>
    </varlistentry>
  </variablelist>
</xsl:template>

<xsl:template match="simplesect[@kind='see']">
  See also: <xsl:apply-templates />
</xsl:template>

<xsl:template match="simplesect[@kind='since']">
  Since: <xsl:apply-templates />
</xsl:template>

<xsl:template match="simplesect[@kind='note']">
  <emphasis>Note: <xsl:apply-templates /></emphasis>
</xsl:template>

<xsl:template match="sp">
  <xsl:text> </xsl:text>
</xsl:template>

<xsl:template match="programlisting">
  <programlisting><xsl:apply-templates /></programlisting>
</xsl:template>

<xsl:template match="itemizedlist">
  <itemizedlist><xsl:apply-templates select="listitem" /></itemizedlist>
</xsl:template>

<xsl:template match="listitem">
  <listitem><simpara><xsl:apply-templates /></simpara></listitem>
</xsl:template>

<!-- stops cross-references in the section titles -->
<xsl:template match="briefdescription">
  <xsl:value-of select="." />
</xsl:template>

<!-- this opens a para for each detaileddescription/para. I could not find a
     way to extract the right text for the description from the
     source otherwise. Downside: we can't use para for return value, "see
     also", etc.  because they're already inside a para. So they're lists.

     It also means we don't control the order of when something is added to
     the output, it matches the input file
     -->
<xsl:template match="detaileddescription/para">
  <para><xsl:apply-templates /></para>
</xsl:template>

<xsl:template match="detaileddescription">
  <xsl:apply-templates select="para" />
</xsl:template>

<!-- methods -->
<xsl:template match="memberdef" >
  <xsl:if test="@kind = 'function' and @static = 'no' and @prot = 'public' or
                @kind !='function' and normalize-space(briefdescription) != ''">
    <varlistentry id="{$which}-{@id}">
        <term>
          <xsl:value-of select="name"/>
          <xsl:if test="normalize-space(briefdescription) != ''">
            - <xsl:apply-templates select="briefdescription" />
          </xsl:if>
        </term>
        <listitem>
          <synopsis>
            <xsl:apply-templates select="definition"/><xsl:apply-templates select="argsstring"/>
          </synopsis>
          <xsl:apply-templates select="detaileddescription" />
        </listitem>
    </varlistentry>
  </xsl:if>
</xsl:template>

<!-- classes -->
<xsl:template match="compounddef" >
    <section id="{$which}-{@id}">
        <title>
            <xsl:value-of select="compoundname" />
            <xsl:if test="normalize-space(briefdescription) != ''">
                - <xsl:apply-templates select="briefdescription" />
            </xsl:if>
        </title>
        <xsl:choose>
          <xsl:when test="normalize-space(detaileddescription) != ''">
            <xsl:apply-templates select="detaileddescription" />
          </xsl:when>
          <xsl:otherwise>
            <para />
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="sectiondef/memberdef[@kind='function' and @static='no']">
          <variablelist>
            <xsl:apply-templates select="sectiondef/memberdef" />
          </variablelist>
        </xsl:if>
    </section>
</xsl:template>
</xsl:stylesheet>
