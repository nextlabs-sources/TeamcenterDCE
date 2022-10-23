using System;
using System.Collections.Generic;
using System.Text;
using System.IO;

namespace NxlHelper
{
    class Program
    {
        const string ACTION_IS_PROTECTED = "-isprotected";
        const string ACTION_GET_TAGS = "-gettags";
        const string ACTION_JSON_TAGS = "-jsontags";

        static void PrintUsage()
        {
            Console.Error.WriteLine("Usage: {0} <-isprotected|-gettags|-jsontags> <fileFullPath>");
        }
        static void Main(string[] args)
        {
            if (args == null || args.Length < 2)
            {
                PrintUsage();
                return;
            }
            string action = args[0];
            string file = args[1];
            if(!File.Exists(file))
            {
                Console.Write(FileFormatType.NotExist.ToString());
                return;
            }
            try
            {
                FileFormat fileFormat = GetNxlFormat(file);
                switch (action)
                {
                    case ACTION_IS_PROTECTED:
                        Console.Write(fileFormat.FormatType.ToString().ToLower());
                        break;
                    case ACTION_GET_TAGS:
                        //first line will always be file format name
                        Console.WriteLine(fileFormat.FormatType.ToString().ToLower());
                        foreach (var tag in fileFormat.GetTags(file))
                        {
                            Console.WriteLine(tag);
                        }
                        break;
                    case ACTION_JSON_TAGS:
                        //only one line is returned
                        Console.Write(fileFormat.GetJsonTags(file));
                        break;
                    default:
                        PrintUsage();
                        break;
                }
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine("Error:{0}", ex.Message);
                Console.Write(FileFormatType.Failed.ToString());
            }
        }
        static FileFormat GetNxlFormat(string filePath)
        {
            var nxlformats = new NxlFormat[] { new NxlFormat(), new NxlFormatv2() };
            using (StreamReader rd = File.OpenText(filePath))
            {
                foreach (var fmt in nxlformats)
                {
                    if (fmt.IsNxlFormat(rd))
                    {
                        return fmt;
                    }
                }
            }
            //file is not protected
            return new FileFormat();
        }
    }
}
