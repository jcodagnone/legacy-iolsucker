

!define PACKAGE "iolsucker"
!define VERSION "2.0.24"
!define COMPANY "Embryos Software"

SetCompressor	bzip2
ShowInstDetails	show
ShowUninstDetails show
SetDateSave	on

Name     "${PACKAGE}${VERSION}"
OutFile "${PACKAGE}-${VERSION}.exe"
InstallDir "$PROGRAMFILES\${COMPANY}\${PACKAGE}\${VERSION}"
InstallDirRegKey HKLM "SOFTWARE\${COMPANY}\${PACKAGE}" "Install_Dir"

ComponentText "Esto instalará ${PACKAGE}-${VERSION} en su computadora. "
DirText "Elija el directorio de instalación:"
UninstallText "This will uninstall ${PACKAGE}-${VERSION}. Hit next to continue."

LicenseText "No haga como siempre! leala! "
LicenseData "gpl.txt"


Section "iolsucker / iolwizard / dlls  (required)"
  SectionIn RO
  SetOutPath $INSTDIR
  File "data\*"

  WriteRegStr HKLM "SOFTWARE\${COMPANY}\${PACKAGE}" "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PACKAGE}" "DisplayName" "${PACKAGE}-${VERSION} (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PACKAGE}" "UninstallString" $INSTDIR\uninstall.exe"
  WriteUninstaller "uninstall.exe"
SectionEnd

# Section "Codigo fuente"
#   File /r "source/${PACKAGE}-${VERSION}"
# SectionEnd

Section "Start Menu Shortcuts"
  CreateDirectory "$SMPROGRAMS\${PACKAGE}"
  CreateShortCut "$SMPROGRAMS\${PACKAGE}\uninstall.lnk" "$INSTDIR\uninstall.exe" "" "" 0
  CreateShortCut "$SMPROGRAMS\${PACKAGE}\iolwizard.lnk" "$INSTDIR\iolwizard.exe" "" "" 0
  CreateShortCut "$SMPROGRAMS\${PACKAGE}\iolsucker.lnk" "$INSTDIR\iolsucker.exe" "" "" 0
  CreateShortCut "$SMPROGRAMS\${PACKAGE}\documentacion.lnk" "$INSTDIR\iolsucker.html" "" "" 0
SectionEnd


Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PACKAGE}"
  DeleteRegKey HKLM "SOFTWARE\${COMPANY}\${PACKAGE}"
  ; remove files
  Delete "$INSTDIR\iconv.dll"
  Delete "$INSTDIR\libintl-1.dll"
  Delete "$INSTDIR\libglib-2.0-0.dll"
  Delete "$INSTDIR\libgmodule-2.0-0.dll"
  Delete "$INSTDIR\libgdk-0.dll"
  Delete "$INSTDIR\libgtk-0.dll"
  Delete "$INSTDIR\libcurl.dll"
  Delete "$INSTDIR\iolsucker.exe"
  Delete "$INSTDIR\iolunch.exe"
  Delete "$INSTDIR\iolwizard.exe"
  RMdir  /r "$INSTDIR\source"

  ; MUST REMOVE UNINSTALLER, too
  Delete $INSTDIR\uninstall.exe
  Delete "$SMPROGRAMS\${PACKAGE}\*.*"
  RMDir "$SMPROGRAMS\${PACKAGE}"
  RMDir "$INSTDIR"
SectionEnd

; eof
