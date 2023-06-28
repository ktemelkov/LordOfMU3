using System;
using System.Collections.Generic;
using System.Text;
using System.Reflection;
using System.IO;

namespace MUAutoClickerUI
{
    class ProtocolSettings
    {
        public UInt32 uiClientType = 0;
        public UInt32 uiProtocolType = 1;
		public UInt32 uiFlags = 0;
		public string strMuPath = "";

        public ProtocolSettings()
        {
            Load();
        }

        public bool Load()
        {
            try
            {
                FileStream file = File.OpenRead(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + @"\protocol.dat");
                BinaryReader reader = new BinaryReader(file);

                uiClientType = reader.ReadUInt32();
                uiProtocolType = reader.ReadUInt32();
                uiFlags = reader.ReadUInt32();
                byte[] binPath = reader.ReadBytes(261); // MAX_PATH + 1
                strMuPath = ASCIIEncoding.ASCII.GetString(binPath);

                reader.Close();
            }
            catch (System.Exception)
            {
                return false;
            }

            return true;
        }


        public bool Save()
        {
            try
            {
                FileStream file = File.OpenWrite(Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + @"\protocol.dat");
                BinaryWriter writer = new BinaryWriter(file);

                writer.Write(uiClientType);
                writer.Write(uiProtocolType);
                writer.Write(uiFlags);

                byte[] binPath = ASCIIEncoding.ASCII.GetBytes(strMuPath);
                writer.Write(binPath);
                byte b = 0x00;

                for (int rest = 261 - binPath.Length; rest > 0; rest--)
                {
                    writer.Write(b);
                }
            }
            catch (System.Exception)
            {
                return false;
            }

            return true;
        }
    }
}
