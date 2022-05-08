reg add "HKCR\directory\background\shell\CloseScreen" /ve /d "关闭显示器"  /f
reg add "HKCR\directory\background\shell\CloseScreen\command" /ve /d "%~dp0CloseScreen.exe" /f