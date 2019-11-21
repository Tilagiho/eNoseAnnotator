# eNoseAnnotator

eNoseAnnotator is a program used to view, annotate and store measurements of the eNose sensor. Currently Windows and Linux systems are supported.

## How to install
Go to the release section of this repository and open the assets of the latest release.

### On Windows:
Download "eNoseAnnotator-Windows.zip" from the assets of the latest release.
Unpack the zip file where you want to use the program. Go into the the directory created and double-click "eNoseAnnotator.exe" in order to run the program.

### On Linux:
Download "eNoseAnnotator-\*.AppImage" from the assets of the latest release. "\*" is replaced by the latest version. Move the.AppImage to the location from where you want to run the program.

Make the .AppImage executable: 
- In Nautilus (default ubuntu file browser): Right-click on the "eNoseAnnotator-\*.AppImage" in your file-browser and select "Properties". Under "Permissions" check "Allow executing file as program".
- In terminal: Open terminal in the directory of the .AppImage. Execute `chmod +x eNoseAnnotator-*.AppImage`.

You can now run eNoseAnnotator by double-clicking the .AppImage in the file-browser or executing `./eNoseAnnotator-*.AppImage` in the terminal.

## How to use

### Connecting a sensor

Plug in a eNose sensor. In the menu bar open "Connection->Set USB connection..." and select the usb-port of the sensor. 

To start the measurement press "Measurement->Start" in the menu bar. After calculating the base vector of the sensor the measurement will be started. The base vector is the average vector of the first few measurements.

Select "Measurement->Stop" in order to stop receiving measurements and "Measurement->reset base level" to calculate a new base vector. After stopping you can start a new measurement by pressing "Measurement->Start" again.

### Annotating data

The graph in the bottom left shows the the relative deviation of the measurements to the base vector. You can changing the range of the x-axis of the graph by dragging & dropping the graph left and right. It is also possible to zoom in and out using the mouse wheel. 

You can select data by pressing "Ctrl" and drawing a selection rectangle in the graph. After selection data the graph on the bottom right will show the average vector of the selection. 

In order to select a class for the selected data press "Annotation->Set class of selection...". The class of a selection always consists of a class name and its abreviation. The abreviation is used to label the data in the measurement graph and should therefore be as short as possible. 

### Loading/ Saving data

You can load and save data using "Measurement->Load..."/ "Measurement->Save" in the menu. You can save the whole measurement data as well as the current selection. 

The data is stored in the format of a .csv file.
