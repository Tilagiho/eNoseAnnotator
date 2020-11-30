if (-not (Test-Path -LiteralPath C:/Qwt-6.1.5)) 
{
    Start-FileDownload 'https://sourceforge.net/projects/qwt/files/qwt/6.1.5/qwt-6.1.5.zip' -FileName qwt-6.1.5.zip
    
    <#
    In order to display error messages correctly the following suffix is added to all commands:
    2>&1 | %{ "$_" }
    #>
    7z t qwt-6.1.5.zip
    7z x qwt-6.1.5.zip -oapp/lib/
    cd "app/lib/qwt-6.1.5/"
    qmake qwt.pro
    nmake
    nmake install
    cd "../../.."
}

