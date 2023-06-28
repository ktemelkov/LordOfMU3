
  function lib_onClickExit()
  {
    window.external.ExitApplication();
  }

  function lib_onClickRun()
  {
    window.external.RunGame();
  }
  
  function lib_onClickHelp()
  {
    window.external.ShowHelp();
  }
   
  function lib_onAppLoad()
  {
    var fileName = window.external.GetMuPath();
    var obj = document.getElementById("muPath");
    
    if (obj)
      obj.value = fileName;
  }
  
  function lib_onBtnMouseOvr(btn)
  {
    btn.style.backgroundColor = "#BB0000";
  }

  function lib_onBtnMouseOut(btn)
  {
    btn.style.backgroundColor = "#770000";
  }
 
  function lib_onClickBrowse()
  {
    var fileName = window.external.BrowseForFile();
    
    if (fileName != "")
    {
      var obj = document.getElementById("muPath");
      
      if (obj)
        obj.value = fileName;
    }
  }
  
  function lib_extGetMuPath()
  {
      var obj = document.getElementById("muPath");
      return obj.value;
  }
  