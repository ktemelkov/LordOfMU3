using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;
using System.Security.Permissions;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace MUAutoClickerUI
{
    [PermissionSet(SecurityAction.Demand, Name="FullTrust")]
    [System.Runtime.InteropServices.ComVisibleAttribute(true)]
    public partial class Form1 : Form
    {
        public int Result = 0;
        private ProtocolSettings mProtocol = new ProtocolSettings();
        private string mUrl = "";
        private bool mfLoaded = false;

        public Form1(String baseUrl)
        {
            InitializeComponent();
            String vers = "x.xxx";
            mUrl = baseUrl;

            Regex re = new Regex(@"arg2=([^\&]*)");
            Match match = re.Match(baseUrl);

            if (match.Success && match.Groups.Count == 2)
            {
                byte[] versBytes = Convert.FromBase64String(match.Groups[1].ToString());

                Encoding ascii = Encoding.ASCII;
                char[] asciiVers = new char[ascii.GetCharCount(versBytes, 0, versBytes.Length)];

                ascii.GetChars(versBytes, 0, versBytes.Length, asciiVers, 0);

                vers = new string(asciiVers);
            }

            this.Text = "  MU AutoClicker V" + vers;

        //    baseUrl = @"file:///C:/Users/KTemelkov/Projects/Personal/LordOfMU3/art/test/uitest.html";
            webBrowser1.ScriptErrorsSuppressed = true;
            webBrowser1.ObjectForScripting = this;
            webBrowser1.DocumentText = @"<html><body style='text-align: center; border: 1px solid #000000;'><br/><br/><br/>Loading ...</body></html>";
            webBrowser1.WebBrowserShortcutsEnabled = false;

            mfLoaded = false;
            webBrowser1.Navigated += new System.Windows.Forms.WebBrowserNavigatedEventHandler(webBrowser1_Navigated); 
        }

        public void webBrowser1_Navigated(object sender, WebBrowserNavigatedEventArgs e)
        {
            if (!mfLoaded)
            {
                mfLoaded = true;
                webBrowser1.Navigate(mUrl);
            }
        }

        public void ExitApplication()
        {
            mProtocol.strMuPath = webBrowser1.Document.InvokeScript("lib_extGetMuPath").ToString();
            mProtocol.Save();
            this.Close();
        }

        public void RunGame()
        {
            mProtocol.strMuPath = webBrowser1.Document.InvokeScript("lib_extGetMuPath").ToString();
            mProtocol.Save();

            this.Result = 1;
            this.Close();
        }

        public void OpenUrl(string url)
        {
            Process p = new Process();

            p.StartInfo.FileName = url;
            p.Start();

        }

        public void SetWindowSize(int cx, int cy)
        {
            this.Width = cx;
            this.Height = cy;
            this.CenterToScreen();
        }

        public string BrowseForFile()
        {
            FileDialog dlg = new OpenFileDialog();
            dlg.CheckFileExists = true;

            dlg.Filter = "Executable Files (*.exe;*.bat;*.cmd;.lnk)|*.exe;*.bat;*.cmd;.lnk|All Files (*.*)|*.*";
            dlg.FilterIndex = 1;

            if (dlg.ShowDialog() == DialogResult.OK)
                return dlg.FileName;

            return "";
        }

        public string GetMuPath()
        {
            return mProtocol.strMuPath;
        }

        public void ShowHelp()
        {
            Process p = new Process();

            p.StartInfo.FileName = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location) + @"\help\index.html";
            p.Start();
        }
    }
}
