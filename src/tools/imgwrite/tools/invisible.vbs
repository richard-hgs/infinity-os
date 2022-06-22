Option Explicit

Dim objDummy, strSignature, objPrimary, objSecondary, objContainer, objWshShell, objWshShellExec, strResult
Dim arg1
Dim arg2
Dim command

arg1 = WScript.Arguments.Item(0)
If Wscript.Arguments.Count > 1 Then
    arg2 = WScript.Arguments.Item(1)
End If

' this block is executed only in the secondary script flow, after primary script runs cscript
If WScript.Arguments.Named.Exists("signature") Then
    ' retrieve signature string from argument
    strSignature = WScript.Arguments.Named("signature")
    Do
        ' loop through all explorer windows
        For Each objContainer In CreateObject("Shell.Application").Windows
            ' check if the explorer's property with signature name contains the reference to the live script
            If ChkVBScriptTypeInfo(objContainer.getProperty(strSignature)) Then
                Exit Do
            End If
        Next
        WScript.Sleep 1
    Loop
    ' create shell object within secondary script
    Set objWshShell = CreateObject("WScript.Shell")
    ' retrieve the primary script me object reference from explorer's property with signature name
    Set objPrimary = objContainer.getProperty(strSignature)
    ' quit explorer window to release memory as it's no longer needed
    objContainer.Quit
    ' assign the secondary script me object to the primary script's variable
    Set objPrimary.objSecondary = Me
    ' emtpy loop while primary script is working
    Do While ChkVBScriptTypeInfo(objPrimary)
        WScript.Sleep 1
    Loop
    ' terminate secondary
    WScript.Quit
End If

' the code below is executed first in the primary script flow 
' create signature string
strSignature = Left(CreateObject("Scriptlet.TypeLib").Guid, 38)
' create new hidden explorer window as container to transfer a reference between script processes
Set objContainer = GetObject("new:{C08AFD90-F2A1-11D1-8455-00A0C91F3880}")
' put this script's me object reference into explorer's property
objContainer.putProperty strSignature, Me
' launch new secondary process of the same script file via cscript.exe with hidden console window, providing signature string in named argument to identify host script
CreateObject("WScript.Shell").Run ("""" & Replace(LCase(WScript.FullName), "wscript", "cscript") & """ //nologo """ & WScript.ScriptFullName & """ ""/signature:" & strSignature & """"), 0
' wait until secondary script has been initialized and put his me object into this script variable
Do Until ChkVBScriptTypeInfo(objSecondary)
    WScript.Sleep 1
Loop

' here is your code starts...
' create exec object within hidden console window of secondary script, execute cmd instruction
Set objWshShellExec = objSecondary.objWshShell.Exec("%comspec% /c " & arg1)

' read cmd output
strResult = objWshShellExec.StdOut.ReadAll()
' WScript.Echo strResult

If Wscript.Arguments.Count > 1 Then
    Dim objFSO : Set objFSO = CreateObject("Scripting.FileSystemObject")
    Dim objFile : Set objFile = objFSO.CreateTextFile(arg2, True)
    objFile.Write strResult & vbCrLf
    objFile.Close
End If
' ...

' utility check if me object is live
Function ChkVBScriptTypeInfo(objSample)
    On Error Resume Next
    If TypeName(objSample) <> "VBScriptTypeInfo" Then
        ChkVBScriptTypeInfo = False
        Exit Function
    End If
    ChkVBScriptTypeInfo = True
End Function

' Option Explicit

' Const WshRunning = 0
' Const WshFinished = 1
' Const WshFailed = 2

' ' arg1 : Set arg1 = WScript.Arguments.Item(0)
' ' Set command = "cmd /c "
' ' command = command & chr(34) & arg1 & chr(34)

' Dim arg1
' Dim arg2
' Dim command

' arg1 = WScript.Arguments.Item(0)
' If Wscript.Arguments.Count > 1 Then
'     arg2 = WScript.Arguments.Item(1)
' End If
' command = "powershell -window minimized -command "
' command = command & chr(34) & arg1 & chr(34)

' ' Dim fso : Set fso = CreateObject("Scripting.FileSystemObject")
' ' Dim stdout : Set stdout = fso.GetStandardStream(1)
' ' Dim stderr : Set stderr = fso.GetStandardStream(2)
' ' stdout.WriteLine "This will go to standard output."
' ' stderr.WriteLine "This will go to error output."

' Dim command2
' command2 = "cmd /c /q " & chr(34) & "echo 123 & timeout 5" & chr(34)

' WScript.Echo command2

' Dim shell : Set shell = CreateObject("WScript.Shell")
' Dim exec : Set exec = shell.Exec(command2)

' While exec.Status = WshRunning
'     WScript.Sleep 50
' Wend

' Dim output

' If exec.Status = WshFailed Then
'     output = exec.StdErr.ReadAll
' Else
'     output = exec.StdOut.ReadAll
' End If

' ' WScript.Echo output & arg2

' If Wscript.Arguments.Count > 1 Then
'     Dim objFSO : Set objFSO = CreateObject("Scripting.FileSystemObject")
'     Dim objFile : Set objFile = objFSO.CreateTextFile(arg2, True)
'     objFile.Write output & vbCrLf
'     objFile.Close
' End If

' ' Set WshShell = WScript.CreateObject("WScript.Shell")

' ' arg1 = WScript.Arguments.Item(0)
' ' command = "cmd /c "
' ' command = command & chr(34) & arg1 & chr(34)
' ' Wscript.Echo command
' ' ' WshReturn = WshShell.Run(command, 1, true)

' ' Option Explicit

' ' Const WshRunning = 0
' ' Const WshFinished = 1
' ' Const WshFailed = 2

' ' Dim exec : Set exec = WshShell.Exec(command)

' ' While exec.Status = WshRunning
' '     WScript.Sleep 50
' ' Wend

' ' Dim output

' ' If exec.Status = WshFailed Then
' '     output = exec.StdErr.ReadAll
' ' Else
' '     output = exec.StdOut.ReadAll
' ' End If

' ' WScript.Echo output


