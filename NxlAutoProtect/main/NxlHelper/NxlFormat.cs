using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace NxlHelper
{
    enum FileFormatType { Failed, NxlV1, NxlV2, NonNxl, NotExist };
    class FileFormat
    {
        public virtual FileFormatType FormatType { get { return FileFormatType.NonNxl; } }
        public virtual IList<string> GetTags(string filePath)
        {
            return new List<string>();
        }
        public virtual string GetJsonTags(string filePath)
        {
            //not supported
            return string.Empty;
        }
    }
    class NxlFormat: FileFormat
    {
        public string Header { get; protected set; }
        public virtual Encoding TagEncoding { get; protected set; }
        public virtual int TagSectionStart { get; protected set; }
        public virtual int TagSectionEnd { get; protected set; }
        public NxlFormat()
        {
            Header = "NXLFMT!";
            //normally tag section starts from 8k
            TagSectionStart = 0x2000;
            //tag section size is 4k
            TagSectionEnd = TagSectionStart + 0x1000;
            TagEncoding = Encoding.Unicode;
        }
        public override FileFormatType FormatType { get { return FileFormatType.NxlV1; } }
        public virtual bool IsNxlFormat(StreamReader rd)
        {
            rd.BaseStream.Seek(0, SeekOrigin.Begin);
            rd.DiscardBufferedData();
            char[] header = new char[Header.Length];
            if (rd.Read(header, 0, header.Length) == header.Length)
            {
                string headerString = new string(header);
                if (string.Equals(Header, headerString, StringComparison.Ordinal))
                {
                    return true;
                }
            }
            return false;
        }
        public override IList<string> GetTags(string filePath)
        {
            var tags = new List<string>();
            using (var rd = new StreamReader(filePath, TagEncoding))
            {
                rd.BaseStream.Seek(TagSectionStart, SeekOrigin.Begin);
                if (!rd.EndOfStream)
                {
                    StringBuilder sb = new StringBuilder();
                    char[] buff = new char[1];
                    while (!rd.EndOfStream
                        && rd.Read(buff, 0, buff.Length) > 0)
                    {
                        if (buff[0] == '\0')
                        {
                            //end of a tag
                            if (sb.Length > 0)
                            {
                                tags.Add(sb.ToString());
                                sb.Length = 0;
                            }
                            else
                            {
                                //end of tag section
                                break;
                            }
                        }
                        else
                        {
                            sb.Append(buff);
                        }
                    }
                }
            }
            return tags;
        }
        public override string GetJsonTags(string filePath)
        {
            var tags = GetTags(filePath);
            StringBuilder sb = new StringBuilder();
            sb.Append("{");
            for(int i=0; i<tags.Count; i++)
            {
                if (i > 0)
                {
                    sb.Append(", ");
                }
                var kvp = tags[i].Split(new[] { '=' }, 2);
                if (kvp.Length == 2)
                {
                    sb.AppendFormat("\"{0}\":[\"{1}\"]", kvp[0], kvp[1]);
                }
            }
            sb.Append("}");
            return sb.ToString();
        }
    }
    class NxlFormatv2 : NxlFormat
    {
        public NxlFormatv2()
        {
            Header = "NXLFMT@";
            TagSectionStart = 0x3000;
            TagSectionEnd = TagSectionStart + 0x1000;
            TagEncoding = Encoding.ASCII;//Encoding.UTF8??
        }
        public override FileFormatType FormatType { get { return FileFormatType.NxlV2; } }
        private int FindNextChar(string str, int istart, char c)
        {
            bool inquote = false;
            for(int i = istart; i < str.Length; i++)
            {
                if (!inquote&&str[i] == c) return i;
                switch (str[i])
                {
                    case '\\':
                        i++;
                        break;
                    case '\"':
                        inquote = !inquote;
                        break;
                    default:
                        break;
                }
            }
            return -1;
        }
        public override IList<string> GetTags(string filePath)
        {
            var tags = new List<string>();
            var json = GetJsonTags(filePath).Trim();
            int i = FindNextChar(json, 0, '{');
            if (i<0) return tags;//invalid
            //parse json
            while (i < json.Length)
            {
                //search key value pair one by one
                int ikeystart = FindNextChar(json, i + 1, '\"');
                if (ikeystart < 0) break;//invalid
                int ikeyend = FindNextChar(json, ikeystart + 1, '\"');
                if (ikeyend < 0) break;//invalid
                string key = string.Empty;
                if (ikeyend - ikeystart >= 2)
                    key = json.Substring(ikeystart+1, ikeyend - ikeystart - 1);

                //found key, start search value array
                int iArrayStart = FindNextChar(json, ikeyend + 1, '[');
                if (iArrayStart < 0) break;//invalid
                int iArrayEnd = FindNextChar(json, iArrayStart, ']');
                if (iArrayEnd < 0) break;//invalid
                int ivalueEnd = iArrayStart;
                int ivalueStart;
                while ((ivalueStart = FindNextChar(json, ivalueEnd + 1, '\"')) < iArrayEnd)
                {
                    if (ivalueStart < 0) break;
                    ivalueEnd = FindNextChar(json, ivalueStart + 1, '\"');
                    if (ivalueEnd < 0) break;//invalid;
                    string value = string.Empty;
                    if (ivalueEnd - ivalueStart >= 2)
                        value = json.Substring(ivalueStart + 1, ivalueEnd - ivalueStart - 1);
                    //add key value
                    tags.Add(string.Format("{0}={1}", key, value));
                }

                //end of array
                i = iArrayEnd + 1;
            }
            return tags;
        }
        public override string GetJsonTags(string filePath)
        {
            StringBuilder json = new StringBuilder();
            using (var rd = new StreamReader(filePath, TagEncoding))
            {
                rd.BaseStream.Seek(TagSectionStart, SeekOrigin.Begin);
                if (!rd.EndOfStream)
                {
                    char[] buff = new char[1];
                    while (!rd.EndOfStream
                        && rd.Read(buff, 0, buff.Length) > 0)
                    {
                        if (buff[0] == '\0') break;
                        json.Append(buff);
                    }
                }
            }
            return json.ToString();
        }
    }
}
