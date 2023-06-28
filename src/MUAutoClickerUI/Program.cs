using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace MUAutoClickerUI
{
    static class Program
    {
        static int MainEntry(String arg)
        {
            arg = arg.Replace("ui.php", "ui/ui.php");

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Form1 mainWnd = new Form1(arg);
            Application.Run(mainWnd);

            return mainWnd.Result;
        }
    }
}
