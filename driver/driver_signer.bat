:: Batch file for signing the BlinkyTape driver
:: Based on the excellent instructions by David Grayson:
:: http://www.davidegrayson.com/signing/
::
:: See the included README.md for instructions

set cross_signature="GlobalSign Root CA.crt"
set timestamp_server="http://timestamp.globalsign.com/scripts/timstamp.dll"
set signing_id="Blinkinlabs, LLC"

:: Path to the Windows SDK/ Driver SDK
set toolpath="C:\Program Files\Windows Kits\10\bin\x86\"

:: Delete any cat files
del *.cat

:: Make a .cat file for the .inf file
::%toolpath%inf2cat /v /driver:%CD% /driver:%~dp0 /os:XP_X86,Vista_X86,Vista_X64,7_X86,7_X64,8_X86,8_X64,6_3_X86,6_3_X64,10_X86,10_X64
%toolpath%inf2cat /v /driver:%~dp0 /os:XP_X86,Vista_X86,Vista_X64,7_X86,7_X64,8_X86,8_X64,6_3_X86,6_3_X64,10_X86,10_X64

:: Sign the .cat file, using the GlobalSign root certificate and the private certificate
:: Note: First, add the private certificate to the Windows install!
%toolpath%signtool sign /v /ac %cross_signature% /n %signing_id% /fd sha256 /tr %timestamp_server% /td sha256 *.cat

:: Finally, verify that the file was created and signed correctly.
%toolpath%signtool verify /v /tw /pa *.cat
