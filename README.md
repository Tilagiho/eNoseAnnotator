# eNoseAnnotator

eNoseAnnotator is a program used to view, annotate and store measurements of the eNose sensor.

## How to install
Go to the release section of this repository and open the assets of the latest release.

### On Windows:
Download "eNoseAnnotator-Windows.zip" from the assets of the latest release.
Unpack the zip file where you want to use the program. Double-click "eNoseAnnotator.exe" in order to run the program.

### On Linux:
Download "eNoseAnnotator-\*.AppImage" from the assets of the latest release. "\*" stands for the latest version.

Make the AppImage executable: 
- In Nautilus (default ubuntu file browser): Right-click on the "eNoseAnnotator-\*.AppImage" in your file-browser and select "Properties". Under "Permissions" check "Allow executing file as program".
- In terminal: Open terminal in the directory of the .AppImage. Execute `chmod +x eNoseAnnotator-*.AppImage`.
You can now run eNoseAnnotator by double-clicking the .AppImage in the file-browser or executing `./eNoseAnnotator-*.AppImage` in the terminal.

## How to use

### Connecting a sensor

Plug in a eNose sensor. In the menu bar open "Connection->Set USB connection..." and select the usb-port of the sensor. 

To start the measurement press "Measurement->Start" in the menu bar. After calculating the base vector of the sensor the measurement will be started. 

Select "Measurement->Stop" in order to stop receiving measurements and "Measurement->reset base level" to calculate a new base vector. 
