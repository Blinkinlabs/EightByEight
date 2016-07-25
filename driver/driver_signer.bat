:: Batch file for signing the BlinkyTape driver
:: Based on the excellent instructions by David Grayson:
:: http://www.davidegrayson.com/signing/
::
:: To compile this, first install the Windows SDK and Driver SDK, and
:: download the GlobalSign Root Certificate from Microsoft. Then, simply
:: run this batch file.


:: Path to the Windows SDK/ Driver SDK
set inftocatpath="C:\Program Files (x86)\Windows Kits\8.0\bin\x86\"
set toolpath="C:\Program Files (x86)\Windows Kits\8.1\bin\x86\"

:: Make a .cat file for the .inf file
%inftocatpath%inf2cat /v /driver:%CD% /os:XP_X86,Vista_X86,Vista_X64,7_X86,7_X64,8_X86,8_X64

:: Sign the .cat file, using the GlobalSign root certificate and the private certificate
:: Note: First, add the private certificate to the Windows install!
%toolpath%signtool sign /v /ac "GlobalSign Root CA.crt" /n "Blinkinlabs, LLC" /tr http://tsa.starfieldtech.com *.cat

:: Finally, verify that the file was created and signed correctly.
%toolpath%signtool verify /v /pa Blinkinlabs.cat