<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  xmlns:tcxml_ns="http://www.tcxml.org/Schemas/TCXMLSchema" version="1.0">
    <xsl:output method="xml" indent="yes"/>
    <xsl:template match="@* | node()">
        <xsl:copy>
            <xsl:apply-templates select="@* | node()"/>
        </xsl:copy>
    </xsl:template>

    <xsl:template match="tcxml_ns:ImanFile">
        <xsl:variable name="nxl_file_name">
            <xsl:value-of select="@original_file_name" />
        </xsl:variable>

        <xsl:copy>
            <xsl:apply-templates select="@*"/>
            <xsl:attribute name="original_file_name">
                <xsl:variable name="endWithNXL">
                    <xsl:value-of select="contains($nxl_file_name, '.nxl') and substring-after($nxl_file_name,'.nxl') = ''" />
                </xsl:variable>
                <xsl:choose>
                    <xsl:when test="$endWithNXL = 'true'">
                        <xsl:value-of select="substring-before($nxl_file_name, '.nxl')" />
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$nxl_file_name" />
                    </xsl:otherwise>
                </xsl:choose>
            </xsl:attribute>
            <xsl:apply-templates select="tcxml_ns:GSIdentity"/>
        </xsl:copy>
    </xsl:template>
</xsl:stylesheet>